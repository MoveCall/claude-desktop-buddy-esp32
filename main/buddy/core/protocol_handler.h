#pragma once

#include "tama_state.h"

void protocol_init();
void protocol_poll(TamaState* state);
void protocol_send_permission(const char* id, const char* decision);
void protocol_send_ack(const char* cmd, bool ok);
