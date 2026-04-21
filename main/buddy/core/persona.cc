#include "persona.h"
#include "tama_state.h"

PersonaState persona_derive(const TamaState& s) {
    if (!s.connected)            return PersonaState::SLEEP;
    if (s.sessions_waiting > 0)  return PersonaState::ATTENTION;
    if (s.recently_completed)    return PersonaState::CELEBRATE;
    if (s.sessions_running >= 3) return PersonaState::BUSY;
    if (s.sessions_running > 0)  return PersonaState::BUSY;
    return PersonaState::IDLE;
}
