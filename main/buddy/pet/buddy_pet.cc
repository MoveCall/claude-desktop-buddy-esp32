#include "buddy_pet.h"
#include "../core/persona.h"
#include "species/species_data.h"

static uint8_t s_state = 0;
static uint32_t s_tick = 0;
static uint8_t s_species = 0;

static const SpeciesData* current_species() {
    const auto* registry = buddy_species_registry();
    uint8_t count = buddy_species_registry_count();
    if (s_species >= count) s_species = 0;
    return registry[s_species];
}

void buddy_pet_init() {
    s_state = 0;
    s_tick = 0;
    s_species = 0;
}

void buddy_pet_set_species(uint8_t idx) {
    if (idx < buddy_species_registry_count()) {
        s_species = idx;
        s_tick = 0;
    }
}

uint8_t buddy_pet_get_species() {
    return s_species;
}

uint8_t buddy_pet_species_count() {
    return buddy_species_registry_count();
}

const char* buddy_pet_get_species_name() {
    return current_species()->name;
}

void buddy_pet_set_state(uint8_t persona_state) {
    if (persona_state >= kBuddyStateCount) persona_state = static_cast<uint8_t>(PersonaState::IDLE);
    if (persona_state != s_state) {
        s_state = persona_state;
        s_tick = 0;
    }
    s_tick++;
}

const char* buddy_pet_get_frame() {
    if (s_state >= kBuddyStateCount) s_state = static_cast<uint8_t>(PersonaState::IDLE);

    const StateAnim& anim = current_species()->states[s_state];
    if (!anim.frames || anim.frame_count == 0) return "";

    uint8_t beat_div = anim.beat_div == 0 ? 1 : anim.beat_div;
    uint32_t beat = s_tick / beat_div;

    uint8_t frame_idx = 0;
    if (anim.seq && anim.seq_len > 0) {
        frame_idx = anim.seq[beat % anim.seq_len];
    } else {
        frame_idx = beat % anim.frame_count;
    }

    if (frame_idx >= anim.frame_count) frame_idx = 0;
    return anim.frames[frame_idx];
}
