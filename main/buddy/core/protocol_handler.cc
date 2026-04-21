#include "protocol_handler.h"
#include "../ble/nus_service.h"
#include "../xfer/file_transfer.h"
#include "buddy_app.h"

#include <cstring>
#include <cstdio>
#include <cJSON.h>
#include <esp_log.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/time.h>

#define TAG "proto"

static uint32_t s_last_live_ms = 0;
static char s_owner_name[32] = "Unknown";
static char s_device_name[32] = "Buddy";

static uint32_t now_ms() {
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

static void send_status_ack() {
    char buf[512];
    uint32_t uptime = now_ms() / 1000;
    size_t heap = esp_get_free_heap_size();
    auto& stats = BuddyApp::GetInstance().GetStats();

    int n = snprintf(buf, sizeof(buf),
        "{\"ack\":\"status\",\"ok\":true,\"n\":0,"
        "\"data\":{"
        "\"name\":\"%s\","
        "\"owner\":\"%s\","
        "\"sec\":%s,"
        "\"bat\":{\"pct\":100,\"mV\":4200,\"mA\":0,\"usb\":true},"
        "\"sys\":{\"up\":%lu,\"heap\":%u,\"fsFree\":0,\"fsTotal\":0},"
        "\"stats\":{\"appr\":%d,\"deny\":%d,\"vel\":0,\"nap\":%lu,\"lvl\":%d}"
        "}}\n",
        s_device_name, s_owner_name,
        nus_secure() ? "true" : "false",
        (unsigned long)uptime, (unsigned)heap,
        stats.approvals, stats.denials,
        (unsigned long)stats.nap_seconds, stats.level);
    if (n > 0 && n < (int)sizeof(buf)) {
        ESP_LOGI(TAG, "Sending status ack (%d bytes)", n);
        nus_write((const uint8_t*)buf, n);
    }
}

static void apply_json(const char* line, TamaState* out) {
    ESP_LOGI(TAG, "RX: %s", line);

    cJSON* doc = cJSON_Parse(line);
    if (!doc) {
        ESP_LOGW(TAG, "JSON parse failed");
        return;
    }

    // File transfer commands take priority
    if (xfer_command(doc)) {
        s_last_live_ms = now_ms();
        cJSON_Delete(doc);
        return;
    }

    // Time sync: {"time":[epoch, tz_offset]}
    cJSON* t = cJSON_GetObjectItem(doc, "time");
    if (t && cJSON_IsArray(t) && cJSON_GetArraySize(t) == 2) {
        time_t epoch = (time_t)cJSON_GetArrayItem(t, 0)->valuedouble;
        int tz_off = (int)cJSON_GetArrayItem(t, 1)->valuedouble;
        struct timeval tv = { .tv_sec = epoch + tz_off, .tv_usec = 0 };
        settimeofday(&tv, nullptr);
        ESP_LOGI(TAG, "Time synced");
        s_last_live_ms = now_ms();
        cJSON_Delete(doc);
        return;
    }

    // Turn events: {"evt":"turn",...}
    cJSON* evt = cJSON_GetObjectItem(doc, "evt");
    if (evt && cJSON_IsString(evt)) {
        ESP_LOGI(TAG, "Event: %s", evt->valuestring);
        s_last_live_ms = now_ms();
        cJSON_Delete(doc);
        return;
    }

    // Command dispatch: {"cmd":"..."}
    cJSON* cmd = cJSON_GetObjectItem(doc, "cmd");
    if (cmd && cJSON_IsString(cmd)) {
        const char* c = cmd->valuestring;
        ESP_LOGI(TAG, "Command: %s", c);

        if (strcmp(c, "owner") == 0) {
            cJSON* name = cJSON_GetObjectItem(doc, "name");
            if (name && cJSON_IsString(name)) {
                strncpy(s_owner_name, name->valuestring, sizeof(s_owner_name) - 1);
                s_owner_name[sizeof(s_owner_name) - 1] = '\0';
            }
            protocol_send_ack("owner", true);
        } else if (strcmp(c, "name") == 0) {
            cJSON* name = cJSON_GetObjectItem(doc, "name");
            if (name && cJSON_IsString(name)) {
                strncpy(s_device_name, name->valuestring, sizeof(s_device_name) - 1);
                s_device_name[sizeof(s_device_name) - 1] = '\0';
            }
            protocol_send_ack("name", true);
        } else if (strcmp(c, "status") == 0) {
            send_status_ack();
        } else if (strcmp(c, "unpair") == 0) {
            nus_clear_bonds();
            protocol_send_ack("unpair", true);
        } else if (strcmp(c, "species") == 0) {
            cJSON* idx_obj = cJSON_GetObjectItem(doc, "idx");
            if (idx_obj) {
                uint8_t idx = (uint8_t)idx_obj->valueint;
                extern void buddy_pet_set_species(uint8_t);
                extern uint8_t buddy_pet_species_count();
                extern void buddy_nvs_save_species(uint8_t);
                if (idx < buddy_pet_species_count()) {
                    buddy_pet_set_species(idx);
                    buddy_nvs_save_species(idx);
                }
            }
            protocol_send_ack("species", true);
        } else {
            protocol_send_ack(c, true);
        }
        s_last_live_ms = now_ms();
        cJSON_Delete(doc);
        return;
    }

    // Heartbeat fields
    cJSON* v;
    if ((v = cJSON_GetObjectItem(doc, "total")))   out->sessions_total = v->valueint;
    if ((v = cJSON_GetObjectItem(doc, "running")))  out->sessions_running = v->valueint;
    if ((v = cJSON_GetObjectItem(doc, "waiting")))  out->sessions_waiting = v->valueint;
    if ((v = cJSON_GetObjectItem(doc, "completed"))) out->recently_completed = cJSON_IsTrue(v);
    if ((v = cJSON_GetObjectItem(doc, "tokens_today"))) out->tokens_today = (uint32_t)v->valuedouble;

    if ((v = cJSON_GetObjectItem(doc, "msg")) && cJSON_IsString(v)) {
        strncpy(out->msg, v->valuestring, sizeof(out->msg) - 1);
        out->msg[sizeof(out->msg) - 1] = '\0';
    }

    cJSON* entries = cJSON_GetObjectItem(doc, "entries");
    if (entries && cJSON_IsArray(entries)) {
        uint8_t n = 0;
        cJSON* item;
        cJSON_ArrayForEach(item, entries) {
            if (n >= 8) break;
            if (cJSON_IsString(item)) {
                strncpy(out->lines[n], item->valuestring, 91);
                out->lines[n][91] = '\0';
            }
            n++;
        }
        if (n != out->n_lines) out->line_gen++;
        out->n_lines = n;
    }

    cJSON* pr = cJSON_GetObjectItem(doc, "prompt");
    if (pr && cJSON_IsObject(pr)) {
        cJSON* pid = cJSON_GetObjectItem(pr, "id");
        cJSON* pt = cJSON_GetObjectItem(pr, "tool");
        cJSON* ph = cJSON_GetObjectItem(pr, "hint");
        auto safe_copy = [](char* dst, cJSON* src, size_t sz) {
            const char* s = (src && cJSON_IsString(src)) ? src->valuestring : "";
            strncpy(dst, s, sz - 1);
            dst[sz - 1] = '\0';
        };
        safe_copy(out->prompt_id, pid, sizeof(out->prompt_id));
        safe_copy(out->prompt_tool, pt, sizeof(out->prompt_tool));
        safe_copy(out->prompt_hint, ph, sizeof(out->prompt_hint));
    } else {
        out->clear_prompt();
    }

    ESP_LOGI(TAG, "Heartbeat: total=%d running=%d waiting=%d",
             out->sessions_total, out->sessions_running, out->sessions_waiting);

    out->last_updated = now_ms();
    s_last_live_ms = now_ms();
    cJSON_Delete(doc);
}

static char s_line_buf[1024];
static uint16_t s_line_len = 0;

void protocol_init() {
    s_line_len = 0;
    s_last_live_ms = 0;
}

void protocol_poll(TamaState* state) {
    while (nus_available()) {
        int c = nus_read();
        if (c < 0) break;

        if (c == '\n' || c == '\r') {
            if (s_line_len > 0) {
                s_line_buf[s_line_len] = '\0';
                if (s_line_buf[0] == '{') {
                    apply_json(s_line_buf, state);
                }
                s_line_len = 0;
            }
        } else if (s_line_len < sizeof(s_line_buf) - 1) {
            s_line_buf[s_line_len++] = (char)c;
        }
    }

    uint32_t now = now_ms();
    state->connected = (s_last_live_ms != 0) && (now - s_last_live_ms <= 30000);
    if (!state->connected && state->sessions_total > 0) {
        state->reset();
    }
}

void protocol_send_permission(const char* id, const char* decision) {
    char buf[128];
    int n = snprintf(buf, sizeof(buf),
        "{\"cmd\":\"permission\",\"id\":\"%s\",\"decision\":\"%s\"}\n",
        id, decision);
    if (n > 0) {
        ESP_LOGI(TAG, "TX permission: %s -> %s", id, decision);
        nus_write((const uint8_t*)buf, n);
    }
}

void protocol_send_ack(const char* cmd, bool ok) {
    char buf[64];
    int n = snprintf(buf, sizeof(buf),
        "{\"ack\":\"%s\",\"ok\":%s,\"n\":0}\n",
        cmd, ok ? "true" : "false");
    if (n > 0) {
        ESP_LOGI(TAG, "TX ack: %s", buf);
        nus_write((const uint8_t*)buf, n);
    }
}
