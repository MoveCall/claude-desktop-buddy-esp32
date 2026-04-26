#pragma once

#include <cstdint>

static constexpr uint8_t kBuddyStateCount = 7;

struct StateAnim {
    const char* const* frames;
    uint8_t frame_count;
    const uint8_t* seq;
    uint8_t seq_len;
    uint8_t beat_div;
};

struct SpeciesData {
    const char* name;
    StateAnim states[kBuddyStateCount];
};

#define BUDDY_ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))
#define BUDDY_ANIM_SEQ(frames_, seq_, beat_) \
    { frames_, static_cast<uint8_t>(BUDDY_ARRAY_LEN(frames_)), seq_, static_cast<uint8_t>(BUDDY_ARRAY_LEN(seq_)), beat_ }

const SpeciesData* const* buddy_species_registry();
uint8_t buddy_species_registry_count();
