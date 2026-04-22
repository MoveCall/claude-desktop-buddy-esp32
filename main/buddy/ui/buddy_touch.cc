#include "buddy_touch.h"
#include "buddy_ui.h"
#include "../core/buddy_app.h"
#include "../pet/buddy_pet.h"
#include "../storage/buddy_nvs.h"

#include <esp_log.h>
#include <esp_lvgl_port.h>

#define TAG "buddy_touch"

static lv_obj_t* s_approve_btn = nullptr;
static lv_obj_t* s_deny_btn = nullptr;
static lv_obj_t* s_pet_area = nullptr;
static bool s_touch_initialized = false;

static void approve_cb(lv_event_t* e) {
    BuddyApp::GetInstance().Approve();
}

static void deny_cb(lv_event_t* e) {
    BuddyApp::GetInstance().Deny();
}

static void pet_tap_cb(lv_event_t* e) {
    // Tap pet for heart one-shot — handled in BuddyApp
    auto& app = BuddyApp::GetInstance();
    if (!app.GetState().has_prompt()) {
        // Cycle species on tap
        uint8_t next = (buddy_pet_get_species() + 1) % buddy_pet_species_count();
        buddy_pet_set_species(next);
        buddy_nvs_save_species(next);
    }
}

static void swipe_cb(lv_event_t* e) {
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
    if (dir == LV_DIR_LEFT || dir == LV_DIR_RIGHT) {
        buddy_ui_next_mode();
    }
}

void buddy_touch_init(lv_display_t* display) {
    if (s_touch_initialized) return;

    if (!lvgl_port_lock(1000)) return;

    lv_obj_t* screen = lv_scr_act();

    // Invisible pet tap area (center of screen)
    s_pet_area = lv_obj_create(screen);
    lv_obj_remove_style_all(s_pet_area);
    lv_obj_set_size(s_pet_area, 160, 120);
    lv_obj_align(s_pet_area, LV_ALIGN_CENTER, 0, -20);
    lv_obj_add_event_cb(s_pet_area, pet_tap_cb, LV_EVENT_CLICKED, nullptr);

    // Swipe gesture on screen
    lv_obj_add_event_cb(screen, swipe_cb, LV_EVENT_GESTURE, nullptr);

    // Approve button (bottom left, hidden until prompt)
    s_approve_btn = lv_btn_create(screen);
    lv_obj_set_size(s_approve_btn, 90, 36);
    lv_obj_align(s_approve_btn, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    lv_obj_set_style_bg_color(s_approve_btn, lv_color_hex(0x4ade80), 0);
    lv_obj_set_style_radius(s_approve_btn, 8, 0);
    lv_obj_t* approve_label = lv_label_create(s_approve_btn);
    lv_label_set_text(approve_label, "Approve");
    lv_obj_center(approve_label);
    lv_obj_add_event_cb(s_approve_btn, approve_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_flag(s_approve_btn, LV_OBJ_FLAG_HIDDEN);

    // Deny button (bottom right, hidden until prompt)
    s_deny_btn = lv_btn_create(screen);
    lv_obj_set_size(s_deny_btn, 90, 36);
    lv_obj_align(s_deny_btn, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_set_style_bg_color(s_deny_btn, lv_color_hex(0xef4444), 0);
    lv_obj_set_style_radius(s_deny_btn, 8, 0);
    lv_obj_t* deny_label = lv_label_create(s_deny_btn);
    lv_label_set_text(deny_label, "Deny");
    lv_obj_center(deny_label);
    lv_obj_add_event_cb(s_deny_btn, deny_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_flag(s_deny_btn, LV_OBJ_FLAG_HIDDEN);

    lvgl_port_unlock();

    s_touch_initialized = true;
    ESP_LOGI(TAG, "Touch UI initialized");
}

// Call from buddy_ui_update to show/hide touch buttons
void buddy_touch_update(bool has_prompt, bool approval_sent) {
    if (!s_touch_initialized) return;
    if (!lvgl_port_lock(50)) return;

    if (has_prompt && !approval_sent) {
        lv_obj_remove_flag(s_approve_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(s_deny_btn, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(s_approve_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_deny_btn, LV_OBJ_FLAG_HIDDEN);
    }

    lvgl_port_unlock();
}
