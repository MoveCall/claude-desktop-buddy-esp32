#pragma once

#include <cstdint>

bool gif_character_init(const char* name = nullptr);
bool gif_character_loaded();
void gif_character_set_state(uint8_t state);
void gif_character_close();
