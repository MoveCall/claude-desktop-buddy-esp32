#include "buddy_nvs.h"

#include <nvs.h>
#include <nvs_flash.h>
#include <esp_log.h>

#define TAG "buddy_nvs"
#define NVS_NAMESPACE "buddy"

static nvs_handle_t s_handle = 0;

void buddy_nvs_init() {
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &s_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
    }
}

void buddy_nvs_load_stats(BuddyStats* stats) {
    if (!s_handle) return;
    nvs_get_u32(s_handle, "nap", &stats->nap_seconds);
    nvs_get_u16(s_handle, "appr", &stats->approvals);
    nvs_get_u16(s_handle, "deny", &stats->denials);
    nvs_get_u8(s_handle, "lvl", &stats->level);
    nvs_get_u32(s_handle, "tok", &stats->tokens);
}

void buddy_nvs_save_stats(const BuddyStats* stats) {
    if (!s_handle) return;
    nvs_set_u32(s_handle, "nap", stats->nap_seconds);
    nvs_set_u16(s_handle, "appr", stats->approvals);
    nvs_set_u16(s_handle, "deny", stats->denials);
    nvs_set_u8(s_handle, "lvl", stats->level);
    nvs_set_u32(s_handle, "tok", stats->tokens);
    nvs_commit(s_handle);
}

void buddy_nvs_load_settings(BuddySettings* settings) {
    if (!s_handle) return;
    uint8_t v;
    if (nvs_get_u8(s_handle, "s_snd", &v) == ESP_OK) settings->sound = v;
    if (nvs_get_u8(s_handle, "s_bt", &v) == ESP_OK) settings->bt = v;
    if (nvs_get_u8(s_handle, "s_led", &v) == ESP_OK) settings->led = v;
    if (nvs_get_u8(s_handle, "s_hud", &v) == ESP_OK) settings->hud = v;
    nvs_get_u8(s_handle, "s_bri", &settings->brightness);
}

void buddy_nvs_save_settings(const BuddySettings* settings) {
    if (!s_handle) return;
    nvs_set_u8(s_handle, "s_snd", settings->sound);
    nvs_set_u8(s_handle, "s_bt", settings->bt);
    nvs_set_u8(s_handle, "s_led", settings->led);
    nvs_set_u8(s_handle, "s_hud", settings->hud);
    nvs_set_u8(s_handle, "s_bri", settings->brightness);
    nvs_commit(s_handle);
}

void buddy_nvs_save_species(uint8_t idx) {
    if (!s_handle) return;
    nvs_set_u8(s_handle, "species", idx);
    nvs_commit(s_handle);
}

uint8_t buddy_nvs_load_species() {
    if (!s_handle) return 0;
    uint8_t idx = 0;
    nvs_get_u8(s_handle, "species", &idx);
    return idx;
}
