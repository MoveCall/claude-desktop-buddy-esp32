#pragma once

#include "board.h"
#include <driver/gpio.h>

class PwmBuzzer : public Buzzer {
public:
    PwmBuzzer(gpio_num_t pin);
    ~PwmBuzzer();
    void Tone(uint16_t freq_hz, uint16_t duration_ms) override;

private:
    gpio_num_t pin_;
};
