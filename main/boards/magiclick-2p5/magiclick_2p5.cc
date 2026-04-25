#include "board.h"
#include "display/lcd_display.h"
#include "button.h"
#include "config.h"
#include "led/single_led.h"
#include "es8311_buzzer.h"
#include "buddy/core/buddy_app.h"
#include "buddy/core/demo_mode.h"
#include "buddy/pet/buddy_pet.h"
#include "buddy/ui/buddy_ui.h"
#include "buddy/storage/buddy_nvs.h"

#include <esp_log.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_gc9a01.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>

#define TAG "Magiclick2p5Buddy"

static const gc9a01_lcd_init_cmd_t gc9107_init_cmds[] = {
    {0xfe, (uint8_t[]){0x00}, 0, 0},
    {0xef, (uint8_t[]){0x00}, 0, 0},
    {0xb0, (uint8_t[]){0xc0}, 1, 0},
    {0xb1, (uint8_t[]){0x80}, 1, 0},
    {0xb2, (uint8_t[]){0x27}, 1, 0},
    {0xb3, (uint8_t[]){0x13}, 1, 0},
    {0xb6, (uint8_t[]){0x19}, 1, 0},
    {0xb7, (uint8_t[]){0x05}, 1, 0},
    {0xac, (uint8_t[]){0xc8}, 1, 0},
    {0xab, (uint8_t[]){0x0f}, 1, 0},
    {0x3a, (uint8_t[]){0x05}, 1, 0},
    {0xb4, (uint8_t[]){0x04}, 1, 0},
    {0xa8, (uint8_t[]){0x08}, 1, 0},
    {0xb8, (uint8_t[]){0x08}, 1, 0},
    {0xea, (uint8_t[]){0x02}, 1, 0},
    {0xe8, (uint8_t[]){0x2A}, 1, 0},
    {0xe9, (uint8_t[]){0x47}, 1, 0},
    {0xe7, (uint8_t[]){0x5f}, 1, 0},
    {0xc6, (uint8_t[]){0x21}, 1, 0},
    {0xc7, (uint8_t[]){0x15}, 1, 0},
    {0xf0, (uint8_t[]){0x1D, 0x38, 0x09, 0x4D, 0x92, 0x2F, 0x35, 0x52, 0x1E, 0x0C, 0x04, 0x12, 0x14, 0x1f}, 14, 0},
    {0xf1, (uint8_t[]){0x16, 0x40, 0x1C, 0x54, 0xA9, 0x2D, 0x2E, 0x56, 0x10, 0x0D, 0x0C, 0x1A, 0x14, 0x1E}, 14, 0},
    {0xf4, (uint8_t[]){0x00, 0x00, 0xFF}, 3, 0},
    {0xba, (uint8_t[]){0xFF, 0xFF}, 2, 0},
};

class Magiclick2p5Board : public Board {
private:
    Button boot_button_;
    Display* display_;
    Es8311Buzzer* buzzer_;

    void InitializeLedPower() {
        gpio_reset_pin(BUILTIN_LED_POWER_PIN);
        gpio_set_direction(BUILTIN_LED_POWER_PIN, GPIO_MODE_OUTPUT);
        gpio_set_level(BUILTIN_LED_POWER_PIN, 0);
    }

    void InitializeSpi() {
        spi_bus_config_t buscfg = {};
        buscfg.mosi_io_num = DISPLAY_SPI_SDA_PIN;
        buscfg.miso_io_num = GPIO_NUM_NC;
        buscfg.sclk_io_num = DISPLAY_SPI_SCL_PIN;
        buscfg.quadwp_io_num = GPIO_NUM_NC;
        buscfg.quadhd_io_num = GPIO_NUM_NC;
        buscfg.max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);
        ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO));
    }

    void InitializeDisplay() {
        esp_lcd_panel_io_handle_t panel_io = nullptr;
        esp_lcd_panel_handle_t panel = nullptr;

        esp_lcd_panel_io_spi_config_t io_config = {};
        io_config.cs_gpio_num = DISPLAY_SPI_CS_PIN;
        io_config.dc_gpio_num = DISPLAY_SPI_DC_PIN;
        io_config.spi_mode = 0;
        io_config.pclk_hz = 20 * 1000 * 1000;
        io_config.trans_queue_depth = 10;
        io_config.lcd_cmd_bits = 8;
        io_config.lcd_param_bits = 8;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI3_HOST, &io_config, &panel_io));

        gc9a01_vendor_config_t vendor_config = {
            .init_cmds = gc9107_init_cmds,
            .init_cmds_size = sizeof(gc9107_init_cmds) / sizeof(gc9107_init_cmds[0]),
        };
        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = DISPLAY_SPI_RST_PIN;
        panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
        panel_config.bits_per_pixel = 16;
        panel_config.vendor_config = &vendor_config;
        ESP_ERROR_CHECK(esp_lcd_new_panel_gc9a01(panel_io, &panel_config, &panel));

        esp_lcd_panel_reset(panel);
        esp_lcd_panel_init(panel);
        esp_lcd_panel_invert_color(panel, false);
        esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY);
        esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);
        esp_lcd_panel_disp_on_off(panel, true);

        display_ = new SpiLcdDisplay(panel_io, panel,
            DISPLAY_WIDTH, DISPLAY_HEIGHT,
            DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y,
            DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY);
    }

    void InitializeButtons() {
        boot_button_.OnClick([]() {
            auto& app = BuddyApp::GetInstance();
            if (app.IsScreenOff()) { app.NotifyActivity(); return; }
            app.NotifyActivity();
            if (buddy_ui_in_settings()) {
                buddy_ui_settings_next();
            } else if (app.GetState().has_prompt()) {
                app.Approve();
            } else {
                buddy_ui_next_mode();
            }
        });
        boot_button_.OnLongPress([]() {
            auto& app = BuddyApp::GetInstance();
            if (app.IsScreenOff()) { app.NotifyActivity(); return; }
            app.NotifyActivity();
            if (buddy_ui_in_settings()) {
                buddy_ui_settings_select();
            } else {
                app.Deny();
            }
        });
        boot_button_.OnDoubleClick([]() {
            BuddyApp::GetInstance().NotifyActivity();
            uint8_t next = (buddy_pet_get_species() + 1) % buddy_pet_species_count();
            buddy_pet_set_species(next);
            buddy_nvs_save_species(next);
        });
        boot_button_.OnMultipleClick([]() {
            if (buddy_ui_in_settings()) return;
            if (demo_mode_active()) demo_mode_stop();
            else demo_mode_start();
        }, 3);
    }

public:
    Magiclick2p5Board() : boot_button_(BOOT_BUTTON_GPIO) {
        InitializeLedPower();
        InitializeSpi();
        InitializeDisplay();
        InitializeButtons();
        buzzer_ = new Es8311Buzzer(
            AUDIO_CODEC_I2C_SDA_PIN, AUDIO_CODEC_I2C_SCL_PIN, 0x18,
            AUDIO_I2S_GPIO_MCLK, AUDIO_I2S_GPIO_BCLK,
            AUDIO_I2S_GPIO_WS, AUDIO_I2S_GPIO_DOUT,
            AUDIO_CODEC_PA_PIN);
        GetBacklight()->RestoreBrightness();
    }

    virtual std::string GetBoardType() override { return "magiclick-2p5"; }

    virtual Led* GetLed() override {
        static SingleLed led(BUILTIN_LED_GPIO);
        return &led;
    }

    virtual Display* GetDisplay() override { return display_; }
    virtual Buzzer* GetBuzzer() override { return buzzer_; }

    virtual Backlight* GetBacklight() override {
        static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN, DISPLAY_BACKLIGHT_OUTPUT_INVERT);
        return &backlight;
    }
};

DECLARE_BOARD(Magiclick2p5Board);
