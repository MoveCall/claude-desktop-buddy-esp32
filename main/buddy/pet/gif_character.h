#pragma once

#include <cstdint>

struct GifColors {
    uint32_t bg = 0x000000;
    uint32_t text = 0xFFFFFF;
    uint32_t text_dim = 0x808080;
    uint32_t body = 0x000000;
    bool valid = false;
};

bool gif_character_init(const char* name = nullptr);
bool gif_character_loaded();
void gif_character_set_state(uint8_t state);
void gif_character_close();
const GifColors& gif_character_colors();
