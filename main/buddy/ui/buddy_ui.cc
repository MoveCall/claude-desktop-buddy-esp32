#include "buddy_ui.h"

#include <esp_log.h>
#include <esp_lvgl_port.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <font_awesome.h>
#include <cstdio>
#include <ctime>
#include <lvgl.h>

#include "../pet/buddy_pet.h"
#include "../pet/gif_character.h"
#include "../ble/nus_service.h"
#include "../core/buddy_app.h"
#include <esp_system.h>
#include <esp_mac.h>

extern const lv_font_t BUILTIN_TEXT_FONT;
extern const lv_font_t BUILTIN_ICON_FONT;

#define TAG "buddy_ui"

static uint32_t now_ms() {
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

// 240x240 round display — usable inscribed circle ~200px diameter
// Safe content zone: centered 200x200, avoid corners
#define SCREEN_W 240
#define SCREEN_H 240
#define SAFE_TOP    40
#define SAFE_BOTTOM 40

// Dark theme colors
#define COLOR_BG        lv_color_hex(0x0f0f1a)
#define COLOR_TEXT      lv_color_hex(0xe8e8e8)
#define COLOR_DIM       lv_color_hex(0x666680)
#define COLOR_GREEN     lv_color_hex(0x4ade80)
#define COLOR_RED       lv_color_hex(0xef4444)
#define COLOR_YELLOW    lv_color_hex(0xfbbf24)
#define COLOR_BLUE      lv_color_hex(0x60a5fa)
#define COLOR_PURPLE    lv_color_hex(0xa78bfa)
#define COLOR_PINK      lv_color_hex(0xf472b6)
#define COLOR_ORANGE    lv_color_hex(0xfb923c)

static lv_obj_t* s_screen = nullptr;

// Pet mode elements
static lv_obj_t* s_icon_label = nullptr;
static lv_obj_t* s_state_label = nullptr;
static lv_obj_t* s_status_label = nullptr;
static lv_obj_t* s_tool_label = nullptr;
static lv_obj_t* s_hint_label = nullptr;
static lv_obj_t* s_action_label = nullptr;
static lv_obj_t* s_dot = nullptr;
static lv_obj_t* s_session_label = nullptr;
static lv_obj_t* s_clock_label = nullptr;
static lv_obj_t* s_particle_label = nullptr;
static lv_obj_t* s_passkey_container = nullptr;
static lv_obj_t* s_passkey_label = nullptr;
static lv_obj_t* s_passkey_title = nullptr;

// Info mode elements
static lv_obj_t* s_info_panel = nullptr;
static lv_obj_t* s_info_text = nullptr;

// Display mode: 0=pet, 1+=info pages
static uint8_t s_display_mode = 0;
static const uint8_t INFO_PAGES = 5;
static uint32_t s_mode_switch_ms = 0;
static const uint32_t AUTO_RETURN_MS = 10000;  // stats, sessions, device

static PersonaState s_last_persona = PersonaState::SLEEP;
static bool s_last_has_prompt = false;
static bool s_last_connected = false;
static uint8_t s_last_total = 255;

static const char* persona_icon(PersonaState state) {
    switch (state) {
        case PersonaState::SLEEP:     return FONT_AWESOME_SLEEPY;
        case PersonaState::IDLE:      return FONT_AWESOME_NEUTRAL;
        case PersonaState::BUSY:      return FONT_AWESOME_THINKING;
        case PersonaState::ATTENTION: return FONT_AWESOME_SURPRISED;
        case PersonaState::CELEBRATE: return FONT_AWESOME_HAPPY;
        case PersonaState::DIZZY:     return FONT_AWESOME_CONFUSED;
        case PersonaState::HEART:     return FONT_AWESOME_LOVING;
        default:                      return FONT_AWESOME_NEUTRAL;
    }
}

static const char* persona_text(PersonaState state) {
    switch (state) {
        case PersonaState::SLEEP:     return "Disconnected";
        case PersonaState::IDLE:      return "Connected";
        case PersonaState::BUSY:      return "Working...";
        case PersonaState::ATTENTION: return "Approve?";
        case PersonaState::CELEBRATE: return "Done!";
        case PersonaState::DIZZY:     return "Dizzy";
        case PersonaState::HEART:     return "Approved!";
        default:                      return "---";
    }
}

static lv_color_t persona_color(PersonaState state) {
    switch (state) {
        case PersonaState::SLEEP:     return COLOR_DIM;
        case PersonaState::IDLE:      return COLOR_GREEN;
        case PersonaState::BUSY:      return COLOR_YELLOW;
        case PersonaState::ATTENTION: return COLOR_ORANGE;
        case PersonaState::CELEBRATE: return COLOR_GREEN;
        case PersonaState::DIZZY:     return COLOR_PURPLE;
        case PersonaState::HEART:     return COLOR_PINK;
        default:                      return COLOR_TEXT;
    }
}

void buddy_ui_init() {
    vTaskDelay(pdMS_TO_TICKS(500));

    if (!lvgl_port_lock(5000)) {
        ESP_LOGE(TAG, "Failed to lock LVGL");
        return;
    }

    s_screen = lv_scr_act();
    lv_obj_clean(s_screen);
    lv_obj_set_style_bg_color(s_screen, COLOR_BG, 0);
    lv_obj_set_style_bg_opa(s_screen, LV_OPA_COVER, 0);
    lv_obj_set_scrollbar_mode(s_screen, LV_SCROLLBAR_MODE_OFF);

    // --- Top area: connection dot ---
    s_dot = lv_obj_create(s_screen);
    lv_obj_remove_style_all(s_dot);
    lv_obj_set_size(s_dot, 8, 8);
    lv_obj_set_style_radius(s_dot, 4, 0);
    lv_obj_set_style_bg_color(s_dot, COLOR_DIM, 0);
    lv_obj_set_style_bg_opa(s_dot, LV_OPA_COVER, 0);
    lv_obj_align(s_dot, LV_ALIGN_TOP_MID, 0, SAFE_TOP);

    // --- Session count (below dot) ---
    s_session_label = lv_label_create(s_screen);
    lv_obj_set_style_text_font(s_session_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_session_label, COLOR_DIM, 0);
    lv_label_set_text(s_session_label, "");
    lv_obj_align(s_session_label, LV_ALIGN_TOP_MID, 0, SAFE_TOP + 14);

    // --- Center: ASCII pet ---
    s_icon_label = lv_label_create(s_screen);
    lv_obj_set_style_text_font(s_icon_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_icon_label, COLOR_TEXT, 0);
    lv_obj_set_style_text_align(s_icon_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(s_icon_label, 180);
    lv_label_set_text(s_icon_label, "  /\\_/\\\n ( - - )\n (  w  )\n (\")_(\")");
    lv_obj_align(s_icon_label, LV_ALIGN_CENTER, 0, -30);

    // --- State name ---
    s_state_label = lv_label_create(s_screen);
    lv_obj_set_width(s_state_label, 180);
    lv_obj_set_style_text_font(s_state_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(s_state_label, COLOR_DIM, 0);
    lv_obj_set_style_text_align(s_state_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(s_state_label, "Disconnected");
    lv_obj_align(s_state_label, LV_ALIGN_CENTER, 0, 5);

    // --- Status message ---
    s_status_label = lv_label_create(s_screen);
    lv_obj_set_width(s_status_label, 170);
    lv_obj_set_style_text_font(s_status_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_status_label, COLOR_DIM, 0);
    lv_obj_set_style_text_align(s_status_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(s_status_label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(s_status_label, "Waiting for Claude...");
    lv_obj_align(s_status_label, LV_ALIGN_CENTER, 0, 30);

    // --- Clock (shown when idle + time synced) ---
    s_clock_label = lv_label_create(s_screen);
    lv_obj_set_width(s_clock_label, 180);
    lv_obj_set_style_text_font(s_clock_label, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(s_clock_label, COLOR_DIM, 0);
    lv_obj_set_style_text_align(s_clock_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(s_clock_label, "");
    lv_obj_align(s_clock_label, LV_ALIGN_CENTER, 0, 50);
    lv_obj_add_flag(s_clock_label, LV_OBJ_FLAG_HIDDEN);

    // --- Particle overlay (Zzz, !, etc) ---
    s_particle_label = lv_label_create(s_screen);
    lv_obj_set_style_text_font(s_particle_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_particle_label, COLOR_DIM, 0);
    lv_label_set_text(s_particle_label, "");
    lv_obj_align(s_particle_label, LV_ALIGN_CENTER, 50, -45);

    // --- Approval mode: tool name ---
    s_tool_label = lv_label_create(s_screen);
    lv_obj_set_width(s_tool_label, 170);
    lv_obj_set_style_text_font(s_tool_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(s_tool_label, COLOR_YELLOW, 0);
    lv_obj_set_style_text_align(s_tool_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(s_tool_label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(s_tool_label, "");
    lv_obj_align(s_tool_label, LV_ALIGN_CENTER, 0, 10);
    lv_obj_add_flag(s_tool_label, LV_OBJ_FLAG_HIDDEN);

    // --- Approval mode: hint ---
    s_hint_label = lv_label_create(s_screen);
    lv_obj_set_width(s_hint_label, 160);
    lv_obj_set_style_text_font(s_hint_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_hint_label, COLOR_DIM, 0);
    lv_obj_set_style_text_align(s_hint_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(s_hint_label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(s_hint_label, "");
    lv_obj_align(s_hint_label, LV_ALIGN_CENTER, 0, 35);
    lv_obj_add_flag(s_hint_label, LV_OBJ_FLAG_HIDDEN);

    // --- Bottom: action hint ---
    s_action_label = lv_label_create(s_screen);
    lv_obj_set_width(s_action_label, 180);
    lv_obj_set_style_text_font(s_action_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_action_label, COLOR_GREEN, 0);
    lv_obj_set_style_text_align(s_action_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(s_action_label, "");
    lv_obj_align(s_action_label, LV_ALIGN_BOTTOM_MID, 0, -SAFE_BOTTOM);
    lv_obj_add_flag(s_action_label, LV_OBJ_FLAG_HIDDEN);

    // --- Info panel (hidden by default, shown on mode switch) ---
    s_info_panel = lv_obj_create(s_screen);
    lv_obj_remove_style_all(s_info_panel);
    lv_obj_set_size(s_info_panel, SCREEN_W, SCREEN_H);
    lv_obj_set_style_bg_color(s_info_panel, COLOR_BG, 0);
    lv_obj_set_style_bg_opa(s_info_panel, LV_OPA_COVER, 0);
    lv_obj_center(s_info_panel);
    lv_obj_set_scrollbar_mode(s_info_panel, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t* info_title = lv_label_create(s_info_panel);
    lv_obj_set_style_text_font(info_title, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(info_title, COLOR_DIM, 0);
    lv_label_set_text(info_title, "Click to switch pages");
    lv_obj_align(info_title, LV_ALIGN_BOTTOM_MID, 0, -SAFE_BOTTOM);

    s_info_text = lv_label_create(s_info_panel);
    lv_obj_set_width(s_info_text, 180);
    lv_obj_set_style_text_font(s_info_text, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_info_text, COLOR_TEXT, 0);
    lv_obj_set_style_text_align(s_info_text, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_long_mode(s_info_text, LV_LABEL_LONG_WRAP);
    lv_label_set_text(s_info_text, "");
    lv_obj_align(s_info_text, LV_ALIGN_CENTER, 0, 5);

    lv_obj_add_flag(s_info_panel, LV_OBJ_FLAG_HIDDEN);

    // --- Passkey overlay ---
    s_passkey_container = lv_obj_create(s_screen);
    lv_obj_remove_style_all(s_passkey_container);
    lv_obj_set_size(s_passkey_container, SCREEN_W, SCREEN_H);
    lv_obj_set_style_bg_color(s_passkey_container, COLOR_BG, 0);
    lv_obj_set_style_bg_opa(s_passkey_container, LV_OPA_COVER, 0);
    lv_obj_center(s_passkey_container);
    lv_obj_set_scrollbar_mode(s_passkey_container, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t* pk_icon = lv_label_create(s_passkey_container);
    lv_obj_set_style_text_font(pk_icon, &BUILTIN_ICON_FONT, 0);
    lv_obj_set_style_text_color(pk_icon, COLOR_BLUE, 0);
    lv_label_set_text(pk_icon, FONT_AWESOME_BLUETOOTH);
    lv_obj_align(pk_icon, LV_ALIGN_CENTER, 0, -50);

    s_passkey_title = lv_label_create(s_passkey_container);
    lv_obj_set_style_text_font(s_passkey_title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_passkey_title, COLOR_DIM, 0);
    lv_label_set_text(s_passkey_title, "Enter on desktop:");
    lv_obj_align(s_passkey_title, LV_ALIGN_CENTER, 0, -15);

    s_passkey_label = lv_label_create(s_passkey_container);
    lv_obj_set_style_text_font(s_passkey_label, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(s_passkey_label, COLOR_TEXT, 0);
    lv_obj_set_style_text_letter_space(s_passkey_label, 6, 0);
    lv_label_set_text(s_passkey_label, "000000");
    lv_obj_align(s_passkey_label, LV_ALIGN_CENTER, 0, 20);

    lv_obj_add_flag(s_passkey_container, LV_OBJ_FLAG_HIDDEN);

    lvgl_port_unlock();
    ESP_LOGI(TAG, "Buddy UI ready");
}

void buddy_ui_update(const TamaState& state, PersonaState persona, bool approval_sent) {
    if (!lvgl_port_lock(100)) return;

    // Connection dot (always visible)
    if (state.connected != s_last_connected) {
        lv_obj_set_style_bg_color(s_dot,
            state.connected ? COLOR_GREEN : COLOR_DIM, 0);
        s_last_connected = state.connected;
    }

    // Force pet mode when approval is pending
    if (state.has_prompt() && s_display_mode != 0) {
        s_display_mode = 0;
        lv_obj_add_flag(s_info_panel, LV_OBJ_FLAG_HIDDEN);
    }

    // Auto-return to pet mode after 10s
    if (s_display_mode > 0 && (now_ms() - s_mode_switch_ms > AUTO_RETURN_MS)) {
        s_display_mode = 0;
        lv_obj_add_flag(s_info_panel, LV_OBJ_FLAG_HIDDEN);
    }

    // Info pages
    if (s_display_mode >= 1) {
        char buf[300];
        auto& stats = BuddyApp::GetInstance().GetStats();

        if (s_display_mode == 1) {
            // Page 1: Pet Stats
            // Hearts for mood
            char hearts[12] = "";
            for (int i = 0; i < 5; i++) hearts[i] = i < stats.mood ? 'v' : '.';
            hearts[5] = '\0';
            // Energy bars
            char energy[12] = "";
            for (int i = 0; i < 5; i++) energy[i] = i < stats.energy ? '|' : '.';
            energy[5] = '\0';
            // Fed pips
            char fed_bar[12] = "";
            for (int i = 0; i < 10; i++) fed_bar[i] = i < stats.fed ? '#' : '.';
            fed_bar[10] = '\0';

            snprintf(buf, sizeof(buf),
                "Mood:   [%s]\n"
                "Energy: [%s]\n"
                "Fed:    [%s]\n"
                "\n"
                "Level: %d\n"
                "Approved: %d\n"
                "Denied: %d\n"
                "Tokens: %lu\n"
                "Pet: %s",
                hearts, energy, fed_bar,
                stats.level, stats.approvals, stats.denials,
                (unsigned long)stats.tokens,
                buddy_pet_get_species_name()
            );
        } else if (s_display_mode == 2) {
            // Page 2: Session details
            snprintf(buf, sizeof(buf),
                "Sessions: %d total\n"
                "  Running: %d\n"
                "  Waiting: %d\n"
                "\n"
                "Tokens today: %lu\n"
                "\n"
                "Status: %s",
                state.sessions_total,
                state.sessions_running,
                state.sessions_waiting,
                (unsigned long)state.tokens_today,
                state.msg
            );
        } else if (s_display_mode == 3) {
            // Page 3: Device info
            size_t heap = esp_get_free_heap_size();
            uint32_t uptime = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS / 1000);
            time_t now = time(nullptr);
            struct tm* tm = localtime(&now);
            char time_str[16] = "not synced";
            if (tm->tm_year >= (2025 - 1900)) {
                strftime(time_str, sizeof(time_str), "%H:%M:%S", tm);
            }

            snprintf(buf, sizeof(buf),
                "BLE: %s%s\n"
                "Time: %s\n"
                "Uptime: %lum %lus\n"
                "Free heap: %uK\n"
                "Pet: %s (#%d/%d)",
                state.connected ? "Connected" : "Disconnected",
                nus_secure() ? " (enc)" : "",
                time_str,
                (unsigned long)(uptime / 60), (unsigned long)(uptime % 60),
                (unsigned)(heap / 1024),
                buddy_pet_get_species_name(),
                buddy_pet_get_species() + 1,
                buddy_pet_species_count()
            );
        } else if (s_display_mode == 4) {
            // Page 4: Bluetooth info
            uint8_t mac[6];
            esp_read_mac(mac, ESP_MAC_BT);
            snprintf(buf, sizeof(buf),
                "Bluetooth\n\n"
                "MAC: %02X:%02X:%02X:%02X:%02X:%02X\n"
                "Name: %s-%02X%02X\n"
                "Paired: %s\n"
                "Encrypted: %s\n"
                "MTU: connected",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
                CONFIG_BUDDY_DEVICE_NAME_PREFIX, mac[4], mac[5],
                state.connected ? "Yes" : "No",
                nus_secure() ? "Yes" : "No"
            );
        } else {
            // Page 5: About
            snprintf(buf, sizeof(buf),
                "claude-desktop-buddy\n"
                "        -esp32\n\n"
                "Version: 0.2.0\n"
                "Board: %s\n\n"
                "github.com/MoveCall/\n"
                "claude-desktop-\n"
                "buddy-esp32",
                BOARD_NAME
            );
        }

        lv_label_set_text(s_info_text, buf);
        lvgl_port_unlock();
        return;
    }

    // Session count
    if (state.sessions_total != s_last_total) {
        if (state.connected && state.sessions_total > 0) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%d session%s",
                     state.sessions_total, state.sessions_total > 1 ? "s" : "");
            lv_label_set_text(s_session_label, buf);
        } else {
            lv_label_set_text(s_session_label, "");
        }
        s_last_total = state.sessions_total;
    }

    // Persona — update pet animation (ASCII or GIF)
    if (gif_character_loaded()) {
        gif_character_set_state((uint8_t)persona);
        lv_label_set_text(s_icon_label, "");
    } else {
        buddy_pet_set_state((uint8_t)persona);
        const char* frame = buddy_pet_get_frame();
        lv_label_set_text(s_icon_label, frame);
        lv_obj_set_style_text_color(s_icon_label, persona_color(persona), 0);
    }

    // Particle effects
    static uint32_t particle_tick = 0;
    particle_tick++;
    switch (persona) {
        case PersonaState::SLEEP: {
            const char* zzz[] = {"z", " Z", "  z"};
            lv_label_set_text(s_particle_label, zzz[(particle_tick / 8) % 3]);
            lv_obj_set_style_text_color(s_particle_label, COLOR_DIM, 0);
            break;
        }
        case PersonaState::ATTENTION: {
            lv_label_set_text(s_particle_label, (particle_tick / 4) & 1 ? "!" : "");
            lv_obj_set_style_text_color(s_particle_label, COLOR_ORANGE, 0);
            break;
        }
        case PersonaState::CELEBRATE: {
            const char* conf[] = {"*", ".", "*", " "};
            lv_label_set_text(s_particle_label, conf[(particle_tick / 3) % 4]);
            lv_obj_set_style_text_color(s_particle_label, COLOR_GREEN, 0);
            break;
        }
        case PersonaState::HEART: {
            lv_label_set_text(s_particle_label, (particle_tick / 6) & 1 ? "v" : "");
            lv_obj_set_style_text_color(s_particle_label, COLOR_PINK, 0);
            break;
        }
        default:
            lv_label_set_text(s_particle_label, "");
            break;
    }

    if (persona != s_last_persona) {
        lv_label_set_text(s_state_label, persona_text(persona));
        lv_obj_set_style_text_color(s_state_label, persona_color(persona), 0);
        s_last_persona = persona;
    }

    bool has_prompt = state.has_prompt();

    if (has_prompt && !approval_sent) {
        // Approval mode
        lv_obj_add_flag(s_status_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_state_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_clock_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(s_tool_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(s_hint_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(s_action_label, LV_OBJ_FLAG_HIDDEN);

        if (!s_last_has_prompt) {
            lv_label_set_text(s_tool_label, state.prompt_tool);
            lv_label_set_text(s_hint_label, state.prompt_hint);
            lv_label_set_text(s_action_label, "Click = Yes    Hold = No");
            lv_obj_set_style_text_color(s_action_label, COLOR_GREEN, 0);
        }
    } else if (has_prompt && approval_sent) {
        lv_label_set_text(s_action_label, "Sent!");
        lv_obj_set_style_text_color(s_action_label, COLOR_BLUE, 0);
    } else {
        // Normal mode
        lv_obj_remove_flag(s_status_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(s_state_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_tool_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_hint_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_action_label, LV_OBJ_FLAG_HIDDEN);

        if (state.connected) {
            lv_label_set_text(s_status_label, state.msg);
        } else {
            lv_label_set_text(s_status_label, "Waiting for Claude...");
        }

        // Show clock when connected + idle (no sessions running)
        time_t now = time(nullptr);
        struct tm* tm = localtime(&now);
        if (tm->tm_year >= (2025 - 1900) && state.connected && state.sessions_running == 0 && !state.has_prompt()) {
            char time_buf[8];
            strftime(time_buf, sizeof(time_buf), "%H:%M", tm);
            lv_label_set_text(s_clock_label, time_buf);
            lv_obj_remove_flag(s_clock_label, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(s_clock_label, LV_OBJ_FLAG_HIDDEN);
        }
    }

    s_last_has_prompt = has_prompt;
    lvgl_port_unlock();
}

void buddy_ui_show_passkey(uint32_t passkey) {
    if (!lvgl_port_lock(100)) return;
    char buf[8];
    snprintf(buf, sizeof(buf), "%06lu", (unsigned long)passkey);
    lv_label_set_text(s_passkey_label, buf);
    lv_obj_remove_flag(s_passkey_container, LV_OBJ_FLAG_HIDDEN);
    lvgl_port_unlock();
}

void buddy_ui_hide_passkey() {
    if (!lvgl_port_lock(100)) return;
    lv_obj_add_flag(s_passkey_container, LV_OBJ_FLAG_HIDDEN);
    lvgl_port_unlock();
}

void buddy_ui_next_mode() {
    if (!lvgl_port_lock(100)) return;

    s_display_mode = (s_display_mode + 1) % (1 + INFO_PAGES);
    s_mode_switch_ms = now_ms();

    if (s_display_mode == 0) {
        lv_obj_add_flag(s_info_panel, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_remove_flag(s_info_panel, LV_OBJ_FLAG_HIDDEN);
    }

    lvgl_port_unlock();
}
