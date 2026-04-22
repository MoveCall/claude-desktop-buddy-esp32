#ifndef BOARD_H
#define BOARD_H

#include <string>
#include <functional>

#include "led/led.h"
#include "backlight.h"

class Display;

void* create_board();

// IMU interface for boards with accelerometer
class Imu {
public:
    virtual ~Imu() = default;
    virtual bool GetAccelData(float& x, float& y, float& z) = 0;
};

// Buzzer interface for boards with speaker/buzzer
class Buzzer {
public:
    virtual ~Buzzer() = default;
    virtual void Tone(uint16_t freq_hz, uint16_t duration_ms) = 0;
    virtual void Beep() { Tone(1000, 100); }
    virtual void DoubleBeep() { Tone(1000, 80); Tone(0, 80); Tone(1000, 80); }
    virtual void CelebrateMelody() {
        Tone(523, 100);  // C5
        Tone(659, 100);  // E5
        Tone(784, 100);  // G5
        Tone(1047, 200); // C6
    }
    virtual void AttentionTone() {
        Tone(600, 150);
        Tone(800, 150);
        Tone(1000, 200);
    }
};

class Board {
private:
    Board(const Board&) = delete;
    Board& operator=(const Board&) = delete;

protected:
    Board();
    std::string GenerateUuid();
    std::string uuid_;

public:
    static Board& GetInstance() {
        static Board* instance = static_cast<Board*>(create_board());
        return *instance;
    }

    virtual ~Board() = default;
    virtual std::string GetBoardType() = 0;
    virtual std::string GetUuid() { return uuid_; }
    virtual Backlight* GetBacklight() { return nullptr; }
    virtual Led* GetLed();
    virtual Display* GetDisplay();
    virtual Imu* GetImu() { return nullptr; }
    virtual Buzzer* GetBuzzer() { return nullptr; }
    virtual bool HasTouchScreen() { return false; }
    virtual bool GetBatteryLevel(int& level, bool& charging, bool& discharging);
    virtual bool GetTemperature(float& esp32temp);
};

void* create_board();

#define DECLARE_BOARD(BOARD_CLASS_NAME) \
void* create_board() { \
    return new BOARD_CLASS_NAME(); \
}

#endif // BOARD_H
