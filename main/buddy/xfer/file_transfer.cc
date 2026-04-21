#include "file_transfer.h"
#include "../ble/nus_service.h"
#include "../pet/gif_character.h"

#include <cstring>
#include <cstdio>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <esp_log.h>
#include <esp_littlefs.h>
#include <mbedtls/base64.h>

#pragma GCC diagnostic ignored "-Wformat-truncation"

#define TAG "xfer"

static FILE* s_file = nullptr;
static uint32_t s_expected = 0;
static uint32_t s_written = 0;
static char s_char_name[24] = "";
static bool s_active = false;
static uint32_t s_total = 0;
static uint32_t s_total_written = 0;
static bool s_fs_mounted = false;

static void xfer_ack(const char* what, bool ok, uint32_t n = 0) {
    char buf[96];
    int len = snprintf(buf, sizeof(buf),
        "{\"ack\":\"%s\",\"ok\":%s,\"n\":%lu}\n",
        what, ok ? "true" : "false", (unsigned long)n);
    if (len > 0) nus_write((const uint8_t*)buf, len);
}

static void xfer_ack_error(const char* what, const char* error) {
    char buf[128];
    int len = snprintf(buf, sizeof(buf),
        "{\"ack\":\"%s\",\"ok\":false,\"n\":0,\"error\":\"%s\"}\n", what, error);
    if (len > 0) nus_write((const uint8_t*)buf, len);
}

static void ensure_fs() {
    if (s_fs_mounted) return;

    esp_vfs_littlefs_conf_t conf = {};
    conf.base_path = "/littlefs";
    conf.partition_label = "assets";
    conf.format_if_mount_failed = true;
    conf.dont_mount = false;

    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret == ESP_OK || ret == ESP_ERR_INVALID_STATE) {
        s_fs_mounted = true;
        mkdir("/littlefs/characters", 0775);
        ESP_LOGI(TAG, "LittleFS mounted");
    } else {
        ESP_LOGE(TAG, "LittleFS mount failed: %s", esp_err_to_name(ret));
    }
}

static void wipe_dir(const char* path) {
    DIR* dir = opendir(path);
    if (!dir) return;
    struct dirent* ent;
    while ((ent = readdir(dir)) != nullptr) {
        if (ent->d_type == DT_REG) {
            char full[320];
            snprintf(full, sizeof(full), "%s/%s", path, ent->d_name);
            unlink(full);
        }
    }
    closedir(dir);
}

static void wipe_all_chars() {
    DIR* root = opendir("/littlefs/characters");
    if (!root) return;
    struct dirent* ent;
    while ((ent = readdir(root)) != nullptr) {
        if (ent->d_type == DT_DIR && ent->d_name[0] != '.') {
            char sub[320];
            snprintf(sub, sizeof(sub), "/littlefs/characters/%s", ent->d_name);
            wipe_dir(sub);
            rmdir(sub);
        }
    }
    closedir(root);
}

bool xfer_command(cJSON* doc) {
    cJSON* cmd_obj = cJSON_GetObjectItem(doc, "cmd");
    if (!cmd_obj || !cJSON_IsString(cmd_obj)) return false;
    const char* cmd = cmd_obj->valuestring;

    // char_begin — start a new character transfer
    if (strcmp(cmd, "char_begin") == 0) {
        ensure_fs();
        if (!s_fs_mounted) {
            xfer_ack_error("char_begin", "filesystem not available");
            return true;
        }

        cJSON* name = cJSON_GetObjectItem(doc, "name");
        const char* n = (name && cJSON_IsString(name)) ? name->valuestring : "pet";
        cJSON* total = cJSON_GetObjectItem(doc, "total");
        s_total = total ? (uint32_t)total->valuedouble : 0;

        strncpy(s_char_name, n, sizeof(s_char_name) - 1);
        s_char_name[sizeof(s_char_name) - 1] = '\0';

        wipe_all_chars();
        char dir[64];
        snprintf(dir, sizeof(dir), "/littlefs/characters/%s", s_char_name);
        mkdir(dir, 0775);

        s_total_written = 0;
        s_active = true;
        ESP_LOGI(TAG, "char_begin: %s, total=%lu", s_char_name, (unsigned long)s_total);
        xfer_ack("char_begin", true);
        return true;
    }

    if (!s_active) {
        if (strcmp(cmd, "permission") == 0) return false;
        return false;
    }

    // file — open a new file for writing
    if (strcmp(cmd, "file") == 0) {
        cJSON* path = cJSON_GetObjectItem(doc, "path");
        cJSON* size = cJSON_GetObjectItem(doc, "size");
        s_expected = size ? (uint32_t)size->valuedouble : 0;
        s_written = 0;

        if (!path || !cJSON_IsString(path)) {
            xfer_ack("file", false);
            return true;
        }

        // Reject path traversal
        if (strstr(path->valuestring, "..") || path->valuestring[0] == '/') {
            xfer_ack("file", false);
            return true;
        }

        char full[96];
        snprintf(full, sizeof(full), "/littlefs/characters/%s/%s",
                 s_char_name, path->valuestring);

        if (s_file) { fclose(s_file); s_file = nullptr; }
        s_file = fopen(full, "wb");
        ESP_LOGI(TAG, "file: %s (%lu bytes)", full, (unsigned long)s_expected);
        xfer_ack("file", s_file != nullptr);
        return true;
    }

    // chunk — base64-encoded data chunk
    if (strcmp(cmd, "chunk") == 0) {
        cJSON* d = cJSON_GetObjectItem(doc, "d");
        if (!d || !cJSON_IsString(d) || !s_file) {
            xfer_ack("chunk", false);
            return true;
        }

        uint8_t buf[300];
        size_t out_len = 0;
        int rc = mbedtls_base64_decode(buf, sizeof(buf), &out_len,
                                       (const uint8_t*)d->valuestring,
                                       strlen(d->valuestring));
        if (rc != 0) {
            xfer_ack("chunk", false);
            return true;
        }

        fwrite(buf, 1, out_len, s_file);
        s_written += out_len;
        s_total_written += out_len;
        xfer_ack("chunk", true, s_written);
        return true;
    }

    // file_end — close current file
    if (strcmp(cmd, "file_end") == 0) {
        bool ok = s_file && (s_written == s_expected || s_expected == 0);
        if (s_file) { fclose(s_file); s_file = nullptr; }
        ESP_LOGI(TAG, "file_end: wrote %lu/%lu bytes", (unsigned long)s_written, (unsigned long)s_expected);
        xfer_ack("file_end", ok, s_written);
        return true;
    }

    // char_end — transfer complete
    if (strcmp(cmd, "char_end") == 0) {
        s_active = false;
        gif_character_close();
        bool ok = gif_character_init(s_char_name);
        ESP_LOGI(TAG, "char_end: %s, total written=%lu, init=%s",
                 s_char_name, (unsigned long)s_total_written, ok ? "ok" : "fail");
        xfer_ack("char_end", ok);
        return true;
    }

    return false;
}

bool xfer_active() { return s_active; }
uint32_t xfer_progress() { return s_total_written; }
uint32_t xfer_total() { return s_total; }
