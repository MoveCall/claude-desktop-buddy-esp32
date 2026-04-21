#pragma once

#include <cstdint>

struct BuddyStats {
    uint32_t nap_seconds = 0;
    uint16_t approvals = 0;
    uint16_t denials = 0;
    uint8_t  level = 0;
    uint32_t tokens = 0;
};

struct BuddySettings {
    bool sound = true;
    bool bt = true;
    bool led = true;
    bool hud = true;
    uint8_t brightness = 128;
};

void buddy_nvs_init();
void buddy_nvs_load_stats(BuddyStats* stats);
void buddy_nvs_save_stats(const BuddyStats* stats);
void buddy_nvs_load_settings(BuddySettings* settings);
void buddy_nvs_save_settings(const BuddySettings* settings);
void buddy_nvs_save_species(uint8_t idx);
uint8_t buddy_nvs_load_species();
