#pragma once

#include <cstdint>

enum class PersonaState : uint8_t {
    SLEEP = 0,
    IDLE,
    BUSY,
    ATTENTION,
    CELEBRATE,
    DIZZY,
    HEART,
};

struct TamaState;

PersonaState persona_derive(const TamaState& s);
