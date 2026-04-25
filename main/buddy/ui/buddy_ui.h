#pragma once

#include "buddy/core/tama_state.h"
#include "buddy/core/persona.h"

void buddy_ui_init();
void buddy_ui_update(const TamaState& state, PersonaState persona, bool approval_sent);
void buddy_ui_show_passkey(uint32_t passkey);
void buddy_ui_hide_passkey();
void buddy_ui_next_mode();
void buddy_ui_show_settings();
void buddy_ui_settings_next();
void buddy_ui_settings_select();
bool buddy_ui_in_settings();
