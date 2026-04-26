#pragma once

#include "board.h"
#include <driver/gpio.h>
#include <driver/i2s_pdm.h>

class PdmBuzzer : public Buzzer {
public:
    PdmBuzzer(gpio_num_t dout, gpio_num_t enable_pin,
              gpio_num_t din = GPIO_NUM_NC, i2s_port_t port = I2S_NUM_0);
    ~PdmBuzzer();
    void Tone(uint16_t freq_hz, uint16_t duration_ms) override;

private:
    i2s_chan_handle_t tx_handle_;
    gpio_num_t enable_pin_;
    bool initialized_;
};
