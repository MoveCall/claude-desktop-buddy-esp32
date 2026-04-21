#pragma once

#include <cstdint>
#include <cstring>

struct TamaState {
    uint8_t  sessions_total = 0;
    uint8_t  sessions_running = 0;
    uint8_t  sessions_waiting = 0;
    bool     recently_completed = false;
    uint32_t tokens_today = 0;
    uint32_t last_updated = 0;
    char     msg[24] = {};
    bool     connected = false;
    char     lines[8][92] = {};
    uint8_t  n_lines = 0;
    uint16_t line_gen = 0;
    char     prompt_id[40] = {};
    char     prompt_tool[20] = {};
    char     prompt_hint[44] = {};

    bool has_prompt() const { return prompt_id[0] != '\0'; }

    void clear_prompt() {
        prompt_id[0] = '\0';
        prompt_tool[0] = '\0';
        prompt_hint[0] = '\0';
    }

    void reset() {
        sessions_total = 0;
        sessions_running = 0;
        sessions_waiting = 0;
        recently_completed = false;
        last_updated = 0;
        connected = false;
        strncpy(msg, "No Claude connected", sizeof(msg) - 1);
        msg[sizeof(msg) - 1] = '\0';
        clear_prompt();
    }
};
