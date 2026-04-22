#pragma once

#include <cstdint>

struct BuddyStats {
    uint32_t nap_seconds = 0;
    uint16_t approvals = 0;
    uint16_t denials = 0;
    uint8_t  level = 0;
    uint32_t tokens = 0;

    // Velocity ring buffer — seconds to respond per approval
    uint8_t  velocity[8] = {};
    uint8_t  vel_idx = 0;
    uint8_t  vel_count = 0;

    // Derived stats (not persisted, computed at runtime)
    uint8_t  mood = 2;      // 0-4 hearts
    uint8_t  energy = 5;    // 0-5 bars
    uint8_t  fed = 0;       // 0-9 pips (tokens % 50000 / 5000)

    void record_velocity(uint8_t seconds) {
        velocity[vel_idx] = seconds;
        vel_idx = (vel_idx + 1) % 8;
        if (vel_count < 8) vel_count++;
    }

    uint8_t median_velocity() const {
        if (vel_count == 0) return 0;
        uint8_t sorted[8];
        for (uint8_t i = 0; i < vel_count; i++) sorted[i] = velocity[i];
        for (uint8_t i = 0; i < vel_count - 1; i++)
            for (uint8_t j = i + 1; j < vel_count; j++)
                if (sorted[i] > sorted[j]) { uint8_t t = sorted[i]; sorted[i] = sorted[j]; sorted[j] = t; }
        return sorted[vel_count / 2];
    }

    void update_mood() {
        uint8_t med = median_velocity();
        if (med == 0) { mood = 2; return; }
        if (med <= 3) mood = 4;
        else if (med <= 8) mood = 3;
        else if (med <= 15) mood = 2;
        else if (med <= 30) mood = 1;
        else mood = 0;
        // Drag down if denial rate > 33%
        if (approvals + denials > 3 && denials * 2 > approvals) {
            if (mood > 0) mood--;
        }
    }

    void update_fed() {
        static const uint32_t TOKENS_PER_LEVEL = 50000;
        fed = (uint8_t)((tokens % TOKENS_PER_LEVEL) / 5000);
    }

    void update_level() {
        static const uint32_t TOKENS_PER_LEVEL = 50000;
        level = (uint8_t)(tokens / TOKENS_PER_LEVEL);
    }
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
