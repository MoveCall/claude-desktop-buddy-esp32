#include "demo_mode.h"
#include "tama_state.h"
#include "buddy_app.h"

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <cstring>

#define TAG "demo"

static bool s_active = false;
static uint32_t s_tick = 0;
static uint32_t s_scene_start = 0;
static uint8_t s_scene = 0;

struct DemoScene {
    const char* name;
    uint8_t total;
    uint8_t running;
    uint8_t waiting;
    const char* msg;
    const char* prompt_tool;
    const char* prompt_hint;
    uint32_t duration_ms;
};

static const DemoScene SCENES[] = {
    // Disconnected / sleep
    {"sleep", 0, 0, 0, "No Claude connected", nullptr, nullptr, 5000},
    // Connected idle
    {"idle", 1, 0, 0, "ready", nullptr, nullptr, 5000},
    // Working
    {"busy", 3, 2, 0, "reading file...", nullptr, nullptr, 5000},
    // Approval pending!
    {"attention", 2, 1, 1, "approve: Bash", "Bash", "git push origin main", 6000},
    // Approved → celebrate
    {"celebrate", 2, 1, 0, "done (success), 5 turns", nullptr, nullptr, 3000},
    // Another approval
    {"attention2", 1, 0, 1, "approve: Write", "Write", "src/main.cc", 6000},
    // Denied → back to idle
    {"idle2", 1, 0, 0, "ready", nullptr, nullptr, 4000},
    // Dizzy (shake)
    {"dizzy", 1, 0, 0, "ready", nullptr, nullptr, 3000},
    // Heart
    {"heart", 2, 0, 0, "done (success), 12 turns", nullptr, nullptr, 3000},
};

static const uint8_t NUM_SCENES = sizeof(SCENES) / sizeof(SCENES[0]);

static uint32_t now_ms() {
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

void demo_mode_start() {
    s_active = true;
    s_scene = 0;
    s_tick = 0;
    s_scene_start = now_ms();
    ESP_LOGI(TAG, "Demo mode started");
}

void demo_mode_stop() {
    s_active = false;
    auto& state = BuddyApp::GetInstance().GetState();
    state.reset();
    ESP_LOGI(TAG, "Demo mode stopped");
}

bool demo_mode_active() {
    return s_active;
}

void demo_mode_tick() {
    if (!s_active) return;

    uint32_t now = now_ms();
    s_tick++;

    // Check scene timeout
    if (now - s_scene_start >= SCENES[s_scene].duration_ms) {
        s_scene = (s_scene + 1) % NUM_SCENES;
        s_scene_start = now;
        ESP_LOGI(TAG, "Scene: %s", SCENES[s_scene].name);
    }

    const DemoScene& sc = SCENES[s_scene];
    auto& state = BuddyApp::GetInstance().GetState();

    state.sessions_total = sc.total;
    state.sessions_running = sc.running;
    state.sessions_waiting = sc.waiting;
    state.connected = (sc.total > 0);
    state.recently_completed = (strcmp(sc.name, "celebrate") == 0);
    state.last_updated = now;
    state.tokens_today = 1500 + s_tick * 3;

    strncpy(state.msg, sc.msg, sizeof(state.msg) - 1);
    state.msg[sizeof(state.msg) - 1] = '\0';

    if (sc.prompt_tool) {
        snprintf(state.prompt_id, sizeof(state.prompt_id), "demo_%d", s_scene);
        strncpy(state.prompt_tool, sc.prompt_tool, sizeof(state.prompt_tool) - 1);
        state.prompt_tool[sizeof(state.prompt_tool) - 1] = '\0';
        strncpy(state.prompt_hint, sc.prompt_hint, sizeof(state.prompt_hint) - 1);
        state.prompt_hint[sizeof(state.prompt_hint) - 1] = '\0';

        // Auto-approve after 4 seconds in the scene
        if (now - s_scene_start > 4000) {
            state.clear_prompt();
        }
    } else {
        state.clear_prompt();
    }
}
