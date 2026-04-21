#include "gif_character.h"

#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <esp_log.h>
#include <esp_littlefs.h>
#include <cJSON.h>
#include <lvgl.h>
#include <esp_lvgl_port.h>
#include "gif/lvgl_gif.h"

#pragma GCC diagnostic ignored "-Wformat-truncation"

#define TAG "gif_char"

static const char* STATE_NAMES[] = {
    "sleep", "idle", "busy", "attention", "celebrate", "dizzy", "heart"
};
static const uint8_t N_STATES = 7;
static const uint8_t MAX_GIFS = 32;

static bool s_loaded = false;
static bool s_fs_mounted = false;
static char s_base_path[48] = "";
static char s_gif_paths[MAX_GIFS][32];
static uint8_t s_state_start[N_STATES];
static uint8_t s_state_count[N_STATES];
static uint8_t s_gif_total = 0;
static uint8_t s_cur_state = 0xFF;

static lv_obj_t* s_gif_img = nullptr;
static LvglGif* s_gif = nullptr;
static uint8_t* s_gif_data = nullptr;
static size_t s_gif_data_size = 0;

static void ensure_fs() {
    if (s_fs_mounted) return;
    esp_vfs_littlefs_conf_t conf = {};
    conf.base_path = "/littlefs";
    conf.partition_label = "assets";
    conf.format_if_mount_failed = true;
    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret == ESP_OK || ret == ESP_ERR_INVALID_STATE) {
        s_fs_mounted = true;
    }
}

static uint8_t* read_file(const char* path, size_t* out_size) {
    FILE* f = fopen(path, "rb");
    if (!f) return nullptr;
    fseek(f, 0, SEEK_END);
    size_t sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t* buf = (uint8_t*)malloc(sz);
    if (!buf) { fclose(f); return nullptr; }
    fread(buf, 1, sz, f);
    fclose(f);
    *out_size = sz;
    return buf;
}

static void load_gif_for_state(uint8_t state) {
    if (state >= N_STATES || s_state_count[state] == 0) return;

    // Free previous
    if (s_gif) { delete s_gif; s_gif = nullptr; }
    if (s_gif_data) { free(s_gif_data); s_gif_data = nullptr; }

    uint8_t idx = s_state_start[state];
    char full[96];
    snprintf(full, sizeof(full), "%s/%s", s_base_path, s_gif_paths[idx]);

    size_t sz = 0;
    s_gif_data = read_file(full, &sz);
    if (!s_gif_data) {
        ESP_LOGE(TAG, "Failed to read: %s", full);
        return;
    }
    s_gif_data_size = sz;

    lv_img_dsc_t img_dsc = {};
    img_dsc.data = s_gif_data;
    img_dsc.data_size = sz;

    s_gif = new LvglGif(&img_dsc);
    if (s_gif->IsLoaded()) {
        s_gif->SetLoopCount(-1);
        s_gif->Start();
        ESP_LOGI(TAG, "Playing: %s (%dx%d, %u bytes)",
                 s_gif_paths[idx], s_gif->width(), s_gif->height(), (unsigned)sz);
    } else {
        ESP_LOGE(TAG, "Failed to decode: %s", full);
        delete s_gif;
        s_gif = nullptr;
    }
}

bool gif_character_init(const char* name) {
    ensure_fs();
    if (!s_fs_mounted) return false;

    // If no name, scan for first installed character
    static char scanned[24];
    if (!name) {
        DIR* d = opendir("/littlefs/characters");
        if (!d) return false;
        struct dirent* ent;
        while ((ent = readdir(d)) != nullptr) {
            if (ent->d_type == DT_DIR && ent->d_name[0] != '.') {
                strncpy(scanned, ent->d_name, sizeof(scanned) - 1);
                scanned[sizeof(scanned) - 1] = '\0';
                name = scanned;
                break;
            }
        }
        closedir(d);
        if (!name) {
            ESP_LOGI(TAG, "No characters installed");
            return false;
        }
    }

    snprintf(s_base_path, sizeof(s_base_path), "/littlefs/characters/%s", name);

    // Read manifest
    char mpath[96];
    snprintf(mpath, sizeof(mpath), "%s/manifest.json", s_base_path);
    size_t msz = 0;
    uint8_t* mdata = read_file(mpath, &msz);
    if (!mdata) {
        ESP_LOGE(TAG, "Manifest not found: %s", mpath);
        return false;
    }

    cJSON* doc = cJSON_ParseWithLength((const char*)mdata, msz);
    free(mdata);
    if (!doc) {
        ESP_LOGE(TAG, "Manifest parse failed");
        return false;
    }

    // Parse state → GIF path mappings
    cJSON* states = cJSON_GetObjectItem(doc, "states");
    s_gif_total = 0;
    for (uint8_t i = 0; i < N_STATES; i++) {
        s_state_start[i] = s_gif_total;
        s_state_count[i] = 0;
        cJSON* v = states ? cJSON_GetObjectItem(states, STATE_NAMES[i]) : nullptr;
        if (v && cJSON_IsArray(v)) {
            cJSON* e;
            cJSON_ArrayForEach(e, v) {
                if (s_gif_total >= MAX_GIFS) break;
                if (cJSON_IsString(e)) {
                    snprintf(s_gif_paths[s_gif_total], 32, "%s", e->valuestring);
                    s_gif_total++;
                    s_state_count[i]++;
                }
            }
        } else if (v && cJSON_IsString(v)) {
            if (s_gif_total < MAX_GIFS) {
                snprintf(s_gif_paths[s_gif_total], 32, "%s", v->valuestring);
                s_gif_total++;
                s_state_count[i] = 1;
            }
        }
    }

    cJSON_Delete(doc);
    s_loaded = true;
    s_cur_state = 0xFF;
    ESP_LOGI(TAG, "Loaded character '%s' (%d GIFs)", name, s_gif_total);
    return true;
}

bool gif_character_loaded() { return s_loaded; }

void gif_character_set_state(uint8_t state) {
    if (!s_loaded || state >= N_STATES || state == s_cur_state) return;
    s_cur_state = state;

    if (!lvgl_port_lock(100)) return;
    load_gif_for_state(state);

    // Update the LVGL image if we have a GIF
    if (s_gif && s_gif->IsLoaded() && s_gif_img) {
        lv_image_set_src(s_gif_img, s_gif->image_dsc());
    }
    lvgl_port_unlock();
}

void gif_character_close() {
    if (lvgl_port_lock(100)) {
        if (s_gif) { delete s_gif; s_gif = nullptr; }
        if (s_gif_data) { free(s_gif_data); s_gif_data = nullptr; }
        lvgl_port_unlock();
    }
    s_loaded = false;
    s_cur_state = 0xFF;
}
