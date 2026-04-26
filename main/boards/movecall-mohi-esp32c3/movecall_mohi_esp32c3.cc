#include "board.h"
#include "display/lcd_display.h"
#include "button.h"
#include "config.h"
#include "led/circular_strip.h"
#include "pdm_buzzer.h"
#include "buddy/core/buddy_app.h"
#include "buddy/core/demo_mode.h"
#include "buddy/pet/buddy_pet.h"
#include "buddy/ui/buddy_ui.h"
#include "buddy/storage/buddy_nvs.h"

#include <esp_log.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_ili9341.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>

#define TAG "MoHiBuddy"

static const ili9341_lcd_init_cmd_t st7789_init_cmds[] = {
    {0x11, NULL, 0, 120},
    {0xB1, (uint8_t[]){0x05, 0x3A, 0x3A}, 3, 0},
    {0xB2, (uint8_t[]){0x05, 0x3A, 0x3A}, 3, 0},
    {0xB3, (uint8_t[]){0x05, 0x3A, 0x3A, 0x05, 0x3A, 0x3A}, 6, 0},
    {0xB4, (uint8_t[]){0x03}, 1, 0},
    {0xC0, (uint8_t[]){0x44, 0x04, 0x04}, 3, 0},
    {0xC1, (uint8_t[]){0xC0}, 1, 0},
    {0xC2, (uint8_t[]){0x0D, 0x00}, 2, 0},
    {0xC3, (uint8_t[]){0x8D, 0x6A}, 2, 0},
    {0xC4, (uint8_t[]){0x8D, 0xEE}, 2, 0},
    {0xC5, (uint8_t[]){0x08}, 1, 0},
    {0xE0, (uint8_t[]){0x0F, 0x10, 0x03, 0x03, 0x07, 0x02, 0x00, 0x02, 0x07, 0x0C, 0x13, 0x38, 0x0A, 0x0E, 0x03, 0x10}, 16, 0},
    {0xE1, (uint8_t[]){0x10, 0x0B, 0x04, 0x04, 0x10, 0x03, 0x00, 0x03, 0x03, 0x09, 0x17, 0x33, 0x0B, 0x0C, 0x06, 0x10}, 16, 0},
    {0x35, (uint8_t[]){0x00}, 1, 0},
    {0x3A, (uint8_t[]){0x05}, 1, 0},
    {0x36, (uint8_t[]){0xC8}, 1, 0},
    {0x29, NULL, 0, 0},
    {0x2C, NULL, 0, 0},
};

class MovecallMoHiBoard : public Board {
private:
    Button boot_button_;
    Display* display_;

    void InitializeSpi() {
        spi_bus_config_t buscfg = {};
        buscfg.mosi_io_num = DISPLAY_SPI_MOSI_PIN;
        buscfg.miso_io_num = GPIO_NUM_NC;
        buscfg.sclk_io_num = DISPLAY_SPI_SCLK_PIN;
        buscfg.quadwp_io_num = GPIO_NUM_NC;
        buscfg.quadhd_io_num = GPIO_NUM_NC;
        buscfg.max_transfer_sz = DISPLAY_WIDTH * 10 * sizeof(uint16_t);
        ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    }

    void InitializeDisplay() {
        esp_lcd_panel_io_handle_t panel_io = nullptr;
        esp_lcd_panel_handle_t panel = nullptr;

        esp_lcd_panel_io_spi_config_t io_config = {};
        io_config.cs_gpio_num = DISPLAY_SPI_CS_PIN;
        io_config.dc_gpio_num = DISPLAY_SPI_DC_PIN;
        io_config.spi_mode = 0;
        io_config.pclk_hz = 40 * 1000 * 1000;
        io_config.trans_queue_depth = 10;
        io_config.lcd_cmd_bits = 8;
        io_config.lcd_param_bits = 8;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &panel_io));

        const ili9341_vendor_config_t vendor_config = {
            .init_cmds = st7789_init_cmds,
            .init_cmds_size = sizeof(st7789_init_cmds) / sizeof(st7789_init_cmds[0]),
        };
        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = DISPLAY_SPI_RST_PIN;
        panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
        panel_config.bits_per_pixel = 16;
        panel_config.vendor_config = (void*)&vendor_config;
        ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(panel_io, &panel_config, &panel));

        esp_lcd_panel_reset(panel);
        esp_lcd_panel_init(panel);
        esp_lcd_panel_invert_color(panel, true);
        esp_lcd_panel_invert_color(panel, false);
        esp_lcd_panel_set_gap(panel, 0, 24);
        esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);
        esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY);
        esp_lcd_panel_disp_on_off(panel, true);

        display_ = new SpiLcdDisplay(panel_io, panel,
            DISPLAY_WIDTH, DISPLAY_HEIGHT,
            0, 0,
            DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY);
    }

    void InitializePa() {
        if (AUDIO_PA_CTL_GPIO != GPIO_NUM_NC) {
            gpio_config_t cfg = {};
            cfg.pin_bit_mask = 1ULL << AUDIO_PA_CTL_GPIO;
            cfg.mode = GPIO_MODE_OUTPUT;
            gpio_config(&cfg);
            gpio_set_level(AUDIO_PA_CTL_GPIO, 0);
        }
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
    MovecallMoHiBoard() : boot_button_(BOOT_BUTTON_GPIO) {
        InitializePa();
        InitializeSpi();
        InitializeDisplay();
        InitializeButtons();
    }

    virtual std::string GetBoardType() override { return "movecall-mohi-esp32c3"; }

    virtual Led* GetLed() override {
        static CircularStrip strip(BUILTIN_LED_GPIO, 4);
        return &strip;
    }

    virtual Buzzer* GetBuzzer() override {
        static PdmBuzzer buzzer(AUDIO_PDM_GPIO_P, AUDIO_PA_CTL_GPIO, AUDIO_PDM_GPIO_N);
        return &buzzer;
    }

    virtual Display* GetDisplay() override { return display_; }
};

DECLARE_BOARD(MovecallMoHiBoard);
