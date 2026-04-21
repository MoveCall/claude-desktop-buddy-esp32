#pragma once

#include <cJSON.h>
#include <cstdint>

bool xfer_command(cJSON* doc);
bool xfer_active();
uint32_t xfer_progress();
uint32_t xfer_total();
