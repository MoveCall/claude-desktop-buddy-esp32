#include "buddy_app.h"
#include "protocol_handler.h"
#include "demo_mode.h"
#include "../ble/nus_service.h"
#include "../ui/buddy_ui.h"
#include "../ui/buddy_touch.h"
#include "../storage/buddy_nvs.h"
#include "../pet/buddy_pet.h"
#include "../pet/gif_character.h"

#include <esp_log.h>
#include <esp_mac.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "boards/common/board.h"
#include "display/display.h"
#include "led/single_led.h"

#define TAG "buddy"

static BuddyApp s_instance;

BuddyApp& BuddyApp::GetInstance() {
    return s_instance;
}

static uint32_t now_ms() {
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

void BuddyApp::Initialize() {
    ESP_LOGI(TAG, "Initializing Claude Desktop Buddy");

    auto& board = Board::GetInstance();

    // Load persistent stats
    buddy_nvs_init();
    buddy_nvs_load_stats(&stats_);
    ESP_LOGI(TAG, "Stats: approvals=%d denials=%d level=%d tokens=%lu",
             stats_.approvals, stats_.denials, stats_.level,
             (unsigned long)stats_.tokens);

    buddy_pet_init();
    uint8_t species = buddy_nvs_load_species();
    if (species < buddy_pet_species_count()) {
        buddy_pet_set_species(species);
    }

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_BT);
    char ble_name[32];
    snprintf(ble_name, sizeof(ble_name), "%s-%02X%02X",
             CONFIG_BUDDY_DEVICE_NAME_PREFIX, mac[4], mac[5]);

    nus_init(ble_name);
    protocol_init();
    state_.reset();

    buddy_ui_init();

    if (board.HasTouchScreen()) {
        buddy_touch_init(nullptr);
        ESP_LOGI(TAG, "Touch screen enabled");
    }

    // Try loading a previously installed GIF character
    if (gif_character_init()) {
        ESP_LOGI(TAG, "GIF character loaded from LittleFS");
    }

    ESP_LOGI(TAG, "Buddy initialized, advertising as '%s'", ble_name);
}

void BuddyApp::Run() {
    auto* backlight = Board::GetInstance().GetBacklight();

    uint32_t last_activity = now_ms();
    uint32_t last_passkey = 0;
    uint32_t last_energy_decay = now_ms();
    bool was_prompt = false;

    while (true) {
        // Demo mode overrides protocol, but auto-exit when BLE data arrives
        if (demo_mode_active()) {
            if (nus_available()) {
                demo_mode_stop();
                protocol_poll(&state_);
            } else {
                demo_mode_tick();
            }
        } else {
            protocol_poll(&state_);
        }
        UpdatePersona();

        // Reset approval_sent when prompt clears
        if (!state_.has_prompt() && was_prompt) {
            approval_sent_ = false;
        }
        // Track when approval prompt arrives
        if (state_.has_prompt() && !was_prompt) {
            prompt_start_ms_ = now_ms();
            auto* led = static_cast<SingleLed*>(Board::GetInstance().GetLed());
            if (led) led->StartContinuousBlink(500);
            auto* buzzer = Board::GetInstance().GetBuzzer();
            if (buzzer) buzzer->AttentionTone();
        }
        if (!state_.has_prompt() && was_prompt) {
            auto* led = static_cast<SingleLed*>(Board::GetInstance().GetLed());
            if (led) led->TurnOff();
        }
        was_prompt = state_.has_prompt();

        // Update token-based stats from heartbeat
        if (state_.connected && state_.tokens_today > 0) {
            stats_.tokens = state_.tokens_today;
            stats_.update_fed();
            uint8_t new_level = (uint8_t)(stats_.tokens / 50000);
            if (new_level > stats_.level) {
                stats_.level = new_level;
                one_shot_state_ = PersonaState::CELEBRATE;
                one_shot_until_ = now_ms() + 3000;
                auto* buzzer = Board::GetInstance().GetBuzzer();
                if (buzzer) buzzer->CelebrateMelody();
                buddy_nvs_save_stats(&stats_);
            }
        }

        // Energy decay: -1 every 2 hours
        if (now_ms() - last_energy_decay > 7200000) {
            if (stats_.energy > 0) stats_.energy--;
            last_energy_decay = now_ms();
        }

        // Handle passkey display
        uint32_t pk = nus_passkey();
        if (pk != last_passkey) {
            if (pk != 0) {
                buddy_ui_show_passkey(pk);
            } else {
                buddy_ui_hide_passkey();
            }
            last_passkey = pk;
        }

        // Update main UI
        if (pk == 0) {
            buddy_ui_update(state_, active_state_, approval_sent_);
            buddy_touch_update(state_.has_prompt(), approval_sent_);
        }

        // IMU: shake detection → dizzy one-shot
        auto* imu = Board::GetInstance().GetImu();
        if (imu) {
            float ax, ay, az;
            if (imu->GetAccelData(ax, ay, az)) {
                float mag = ax * ax + ay * ay + az * az;
                if (mag > 4.0f) {  // ~2g threshold
                    one_shot_state_ = PersonaState::DIZZY;
                    one_shot_until_ = now_ms() + 2000;
                }
                // Face-down nap detection (z < -0.7g)
                if (az < -0.7f && ax < 0.4f && ay < 0.4f) {
                    if (stats_.energy < 5) stats_.energy = 5;
                }
            }
        }

        // Screen auto-off
        if (state_.has_prompt() || state_.connected || pk != 0) {
            last_activity = now_ms();
        }
        if (backlight) {
            uint32_t idle_ms = now_ms() - last_activity;
            if (idle_ms > CONFIG_BUDDY_SCREEN_OFF_MS) {
                backlight->SetBrightness(0);
            } else {
                backlight->RestoreBrightness();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void BuddyApp::UpdatePersona() {
    base_state_ = persona_derive(state_);

    uint32_t now = now_ms();
    if (one_shot_until_ > 0 && now < one_shot_until_) {
        active_state_ = one_shot_state_;
    } else {
        one_shot_until_ = 0;
        active_state_ = base_state_;
    }
}

void BuddyApp::Approve() {
    if (!state_.has_prompt() || approval_sent_) return;
    protocol_send_permission(state_.prompt_id, "once");
    approval_sent_ = true;

    stats_.approvals++;

    // Record response time
    uint32_t response_ms = now_ms() - prompt_start_ms_;
    uint8_t response_sec = (response_ms > 255000) ? 255 : (uint8_t)(response_ms / 1000);
    stats_.record_velocity(response_sec);
    stats_.update_mood();

    buddy_nvs_save_stats(&stats_);

    auto* led = static_cast<SingleLed*>(Board::GetInstance().GetLed());
    if (led) led->BlinkOnce();

    auto* buzzer = Board::GetInstance().GetBuzzer();
    if (buzzer) buzzer->Beep();

    one_shot_state_ = PersonaState::HEART;
    one_shot_until_ = now_ms() + 2000;

    ESP_LOGI(TAG, "Approved: %s (total approvals: %d)", state_.prompt_id, stats_.approvals);
}

void BuddyApp::Deny() {
    if (!state_.has_prompt() || approval_sent_) return;
    protocol_send_permission(state_.prompt_id, "deny");
    approval_sent_ = true;

    stats_.denials++;
    stats_.update_mood();
    buddy_nvs_save_stats(&stats_);

    auto* led = static_cast<SingleLed*>(Board::GetInstance().GetLed());
    if (led) led->Blink(2, 200);

    auto* buzzer = Board::GetInstance().GetBuzzer();
    if (buzzer) buzzer->DoubleBeep();

    ESP_LOGI(TAG, "Denied: %s (total denials: %d)", state_.prompt_id, stats_.denials);
}

void BuddyApp::OnButtonClick() {
    auto* backlight = Board::GetInstance().GetBacklight();
    if (backlight) backlight->RestoreBrightness();

    if (state_.has_prompt() && !approval_sent_) {
        Approve();
    }
}

void BuddyApp::OnButtonLongPress() {
    auto* backlight = Board::GetInstance().GetBacklight();
    if (backlight) backlight->RestoreBrightness();

    if (state_.has_prompt() && !approval_sent_) {
        Deny();
    }
}
