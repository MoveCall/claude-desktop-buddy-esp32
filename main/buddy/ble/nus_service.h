#pragma once

#include <cstdint>
#include <cstddef>

void nus_init(const char* device_name);
bool nus_connected();
bool nus_secure();
uint32_t nus_passkey();
void nus_clear_bonds();
size_t nus_available();
int nus_read();
size_t nus_write(const uint8_t* data, size_t len);
