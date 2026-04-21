#pragma once

#include <cstdint>

void buddy_pet_init();
void buddy_pet_set_state(uint8_t persona_state);
void buddy_pet_set_species(uint8_t idx);
uint8_t buddy_pet_get_species();
uint8_t buddy_pet_species_count();
const char* buddy_pet_get_frame();
