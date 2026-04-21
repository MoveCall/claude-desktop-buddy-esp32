#include "buddy_app.h"
#include "protocol_handler.h"
#include "../ble/nus_service.h"
#include "../ui/buddy_ui.h"
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
    bool was_prompt = false;

    while (true) {
        protocol_poll(&state_);
        UpdatePersona();

        // Reset approval_sent when prompt clears
        if (!state_.has_prompt() && was_prompt) {
            approval_sent_ = false;
        }
        // LED blink when new approval arrives
        if (state_.has_prompt() && !was_prompt) {
            auto* led = static_cast<SingleLed*>(Board::GetInstance().GetLed());
            if (led) led->StartContinuousBlink(500);
        }
        if (!state_.has_prompt() && was_prompt) {
            auto* led = static_cast<SingleLed*>(Board::GetInstance().GetLed());
            if (led) led->TurnOff();
        }
        was_prompt = state_.has_prompt();

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
    buddy_nvs_save_stats(&stats_);

    auto* led = static_cast<SingleLed*>(Board::GetInstance().GetLed());
    if (led) led->BlinkOnce();

    one_shot_state_ = PersonaState::HEART;
    one_shot_until_ = now_ms() + 2000;

    ESP_LOGI(TAG, "Approved: %s (total approvals: %d)", state_.prompt_id, stats_.approvals);
}

void BuddyApp::Deny() {
    if (!state_.has_prompt() || approval_sent_) return;
    protocol_send_permission(state_.prompt_id, "deny");
    approval_sent_ = true;

    stats_.denials++;
    buddy_nvs_save_stats(&stats_);

    auto* led = static_cast<SingleLed*>(Board::GetInstance().GetLed());
    if (led) led->Blink(2, 200);

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
