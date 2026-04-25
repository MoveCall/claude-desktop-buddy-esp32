#pragma once

#include "board.h"
#include <driver/gpio.h>
#include <driver/i2s_std.h>

class I2sBuzzer : public Buzzer {
public:
    I2sBuzzer(gpio_num_t bclk, gpio_num_t lrclk, gpio_num_t dout,
              gpio_num_t enable_pin, i2s_port_t port = I2S_NUM_0,
              gpio_num_t mclk = GPIO_NUM_NC);
    ~I2sBuzzer();
    void Tone(uint16_t freq_hz, uint16_t duration_ms) override;

private:
    i2s_chan_handle_t tx_handle_;
    gpio_num_t enable_pin_;
    bool initialized_;
};
