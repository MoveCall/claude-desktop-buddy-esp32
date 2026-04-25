#pragma once

#include "board.h"
#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include <driver/i2s_std.h>

class Es8311Buzzer : public Buzzer {
public:
    Es8311Buzzer(gpio_num_t sda, gpio_num_t scl, uint8_t addr,
                 gpio_num_t mclk, gpio_num_t bclk, gpio_num_t ws,
                 gpio_num_t dout, gpio_num_t pa_pin);
    Es8311Buzzer(i2c_master_bus_handle_t i2c_bus, uint8_t addr,
                 gpio_num_t mclk, gpio_num_t bclk, gpio_num_t ws,
                 gpio_num_t dout, gpio_num_t pa_pin);
    ~Es8311Buzzer();
    void Tone(uint16_t freq_hz, uint16_t duration_ms) override;

private:
    void InitI2c(gpio_num_t sda, gpio_num_t scl);
    void InitCodec(uint8_t addr, bool use_mclk);
    void InitI2s(gpio_num_t mclk, gpio_num_t bclk, gpio_num_t ws, gpio_num_t dout);
    void InitPa();
    void WriteReg(uint8_t reg, uint8_t val);
    uint8_t ReadReg(uint8_t reg);

    i2c_master_bus_handle_t i2c_bus_;
    i2c_master_dev_handle_t codec_dev_;
    i2s_chan_handle_t tx_handle_;
    gpio_num_t pa_pin_;
    bool owns_i2c_;
    bool initialized_;
};
