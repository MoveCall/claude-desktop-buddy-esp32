# Adding a New Board

This guide explains how to add support for a new hardware board to claude-desktop-buddy-esp32.

## Prerequisites

- A board supported by [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) (or any ESP32 board with a display)
- Display driver available as an ESP-IDF component
- Working ESP-IDF build environment

## Steps

### 1. Create board directory

```
main/boards/<board-name>/
├── config.h          # Pin definitions
├── config.json       # Build metadata
├── <board-name>.cc   # Board class implementation
└── README.md         # Brief description
```

### 2. Define pins in config.h

```c
#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

#define BUILTIN_LED_GPIO        GPIO_NUM_21
#define BOOT_BUTTON_GPIO        GPIO_NUM_0

#define DISPLAY_WIDTH           240
#define DISPLAY_HEIGHT          240
#define DISPLAY_MIRROR_X        true
#define DISPLAY_MIRROR_Y        false
#define DISPLAY_SWAP_XY         false
#define DISPLAY_OFFSET_X        0
#define DISPLAY_OFFSET_Y        0

#define DISPLAY_BACKLIGHT_PIN   GPIO_NUM_16
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT false

// SPI display pins
#define DISPLAY_SPI_SCLK_PIN    GPIO_NUM_12
#define DISPLAY_SPI_MOSI_PIN    GPIO_NUM_10
#define DISPLAY_SPI_CS_PIN      GPIO_NUM_13
#define DISPLAY_SPI_DC_PIN      GPIO_NUM_14
#define DISPLAY_SPI_RESET_PIN   GPIO_NUM_11
#define DISPLAY_SPI_SCLK_HZ    (40 * 1000 * 1000)

#endif
```

### 3. Implement board class

Create `<board-name>.cc` extending `Board`:

```cpp
#include "board.h"
#include "display/lcd_display.h"
#include "button.h"
#include "config.h"
#include "led/single_led.h"
#include "buddy/core/buddy_app.h"
#include "buddy/pet/buddy_pet.h"
#include "buddy/ui/buddy_ui.h"
#include "buddy/storage/buddy_nvs.h"

// Include your display driver
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_gc9a01.h>  // or your driver
#include <driver/gpio.h>
#include <driver/spi_master.h>

class MyBoard : public Board {
private:
    Button boot_button_;
    Display* display_;

    void InitializeSpi() {
        // Initialize SPI bus
    }

    void InitializeDisplay() {
        // Initialize LCD panel and create SpiLcdDisplay
    }

    void InitializeButtons() {
        boot_button_.OnClick([]() {
            auto& app = BuddyApp::GetInstance();
            if (app.GetState().has_prompt()) {
                app.Approve();
            } else {
                buddy_ui_next_mode();
            }
        });
        boot_button_.OnLongPress([]() {
            BuddyApp::GetInstance().Deny();
        });
        boot_button_.OnDoubleClick([]() {
            uint8_t next = (buddy_pet_get_species() + 1) % buddy_pet_species_count();
            buddy_pet_set_species(next);
            buddy_nvs_save_species(next);
        });
    }

public:
    MyBoard() : boot_button_(BOOT_BUTTON_GPIO) {
        InitializeSpi();
        InitializeDisplay();
        InitializeButtons();
        GetBacklight()->RestoreBrightness();
    }

    virtual std::string GetBoardType() override { return "<board-name>"; }

    virtual Led* GetLed() override {
        static SingleLed led(BUILTIN_LED_GPIO);
        return &led;
    }

    virtual Display* GetDisplay() override { return display_; }

    virtual Backlight* GetBacklight() override {
        static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN, DISPLAY_BACKLIGHT_OUTPUT_INVERT);
        return &backlight;
    }
};

DECLARE_BOARD(MyBoard);
```

### 4. Add to Kconfig

In `main/Kconfig.projbuild`, add inside the `choice BOARD_TYPE` block:

```
config BOARD_TYPE_MY_BOARD
    bool "My Board (ESP32-S3, GC9A01 240x240)"
    depends on IDF_TARGET_ESP32S3
```

### 5. Add to CMakeLists.txt

In `main/CMakeLists.txt`, add in the board type selection:

```cmake
elseif(CONFIG_BOARD_TYPE_MY_BOARD)
    set(BOARD_TYPE "<board-name>")
    set(BUILTIN_TEXT_FONT font_puhui_basic_20_4)
    set(BUILTIN_ICON_FONT font_awesome_20_4)
```

### 6. Add display driver component (if needed)

In `main/idf_component.yml`, add the driver:

```yaml
espressif/esp_lcd_gc9a01: ==2.0.1  # or your driver
```

### 7. Create config.json

```json
{
    "target": "esp32s3",
    "builds": [
        {
            "name": "<board-name>"
        }
    ]
}
```

### 8. Build and test

```bash
idf.py set-target esp32s3
idf.py menuconfig  # Select your board
idf.py build
idf.py -p PORT flash
```

## Display Types

The project supports these display classes:

| Class | Interface | Example |
|-------|-----------|---------|
| `SpiLcdDisplay` | SPI | GC9A01, ST7789, ILI9341 |
| `OledDisplay` | I2C | SSD1306, SH1106 |

For OLED boards, the buddy UI will need layout adjustments for small screens.

## Tips

- Copy the closest existing board as a starting point
- CuiCan and Moji are good references for SPI round LCD boards
- Moji2 is a reference for QSPI displays
- If your board uses USB Serial/JTAG, add `CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y`
- Update [HARDWARE.md](../HARDWARE.md) with your board's status

## Button Mapping

### Single button (e.g. Movecall boards)

One BOOT button handles everything via click/long-press/double-click:

| Action | No approval pending | Approval pending |
|--------|-------------------|-----------------|
| Click | Switch display mode | Approve |
| Long press | — | Deny |
| Double click | Switch pet species | Switch pet species |

### Two buttons (e.g. ESP-BOX, M5Stack)

With two buttons you get a cleaner UX — no overloaded gestures:

```cpp
// Button A — primary action
button_a_.OnClick([]() {
    auto& app = BuddyApp::GetInstance();
    if (app.GetState().has_prompt()) {
        app.Approve();
    } else {
        buddy_ui_next_mode();  // cycle: pet → info → pet
    }
});

// Button B — secondary action / deny
button_b_.OnClick([]() {
    auto& app = BuddyApp::GetInstance();
    if (app.GetState().has_prompt()) {
        app.Deny();
    } else {
        uint8_t next = (buddy_pet_get_species() + 1) % buddy_pet_species_count();
        buddy_pet_set_species(next);
        buddy_nvs_save_species(next);
    }
});
```

### Three+ buttons or rotary encoder

With more inputs you can add a settings menu, brightness control, etc:

```cpp
// Button A — approve / next mode
// Button B — deny / switch species
// Button C or encoder press — open settings menu

button_c_.OnClick([]() {
    // Toggle brightness levels
    auto* bl = Board::GetInstance().GetBacklight();
    static uint8_t levels[] = {32, 64, 128, 255};
    static uint8_t idx = 2;
    idx = (idx + 1) % 4;
    bl->SetBrightness(levels[idx], true);
});
```

### Touch screen

For boards with touch displays (e.g. Waveshare AMOLED), you can implement on-screen approve/deny buttons instead of physical buttons. This requires adding LVGL touch input handling in your board class.
