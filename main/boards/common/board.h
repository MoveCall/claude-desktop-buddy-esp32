#ifndef BOARD_H
#define BOARD_H

#include <string>
#include <functional>

#include "led/led.h"
#include "backlight.h"

class Display;

void* create_board();

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
    virtual bool GetBatteryLevel(int& level, bool& charging, bool& discharging);
    virtual bool GetTemperature(float& esp32temp);
};

void* create_board();

#define DECLARE_BOARD(BOARD_CLASS_NAME) \
void* create_board() { \
    return new BOARD_CLASS_NAME(); \
}

#endif // BOARD_H
