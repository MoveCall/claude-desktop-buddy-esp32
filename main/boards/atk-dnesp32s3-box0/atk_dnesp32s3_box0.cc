#include "board.h"
#include "display/lcd_display.h"
#include "button.h"
#include "config.h"
#include "es8311_buzzer.h"
#include "buddy/core/buddy_app.h"
#include "buddy/core/demo_mode.h"
#include "buddy/pet/buddy_pet.h"
#include "buddy/ui/buddy_ui.h"
#include "buddy/storage/buddy_nvs.h"

#include <esp_log.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>

#define TAG "AtkBox0Buddy"

class AtkDnesp32s3Box0Board : public Board {
private:
    Button middle_button_;
    Button right_button_;
    Button left_button_;
    Display* display_;
    Es8311Buzzer* buzzer_;

    void InitializePower() {
        gpio_config_t cfg = {};
        cfg.mode = GPIO_MODE_OUTPUT;
        cfg.pin_bit_mask = (1ULL << GPIO_NUM_2) | (1ULL << GPIO_NUM_14);
        gpio_config(&cfg);
        gpio_set_level(GPIO_NUM_2, 1);   // SYS_POW
        gpio_set_level(GPIO_NUM_14, 1);  // CODEC_PWR
    }

    void InitializeSpi() {
        spi_bus_config_t buscfg = {};
        buscfg.mosi_io_num = DISPLAY_SPI_MOSI_PIN;
        buscfg.miso_io_num = GPIO_NUM_NC;
        buscfg.sclk_io_num = DISPLAY_SPI_SCLK_PIN;
        buscfg.quadwp_io_num = GPIO_NUM_NC;
        buscfg.quadhd_io_num = GPIO_NUM_NC;
        buscfg.max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);
        ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    }

    void InitializeDisplay() {
        esp_lcd_panel_io_handle_t panel_io = nullptr;
        esp_lcd_panel_handle_t panel = nullptr;

        esp_lcd_panel_io_spi_config_t io_config = {};
        io_config.cs_gpio_num = DISPLAY_SPI_CS_PIN;
        io_config.dc_gpio_num = DISPLAY_SPI_DC_PIN;
        io_config.spi_mode = 0;
        io_config.pclk_hz = 80 * 1000 * 1000;
        io_config.trans_queue_depth = 7;
        io_config.lcd_cmd_bits = 8;
        io_config.lcd_param_bits = 8;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &panel_io));

        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = DISPLAY_SPI_RESET_PIN;
        panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
        panel_config.bits_per_pixel = 16;
        ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(panel_io, &panel_config, &panel));

        esp_lcd_panel_reset(panel);
        esp_lcd_panel_init(panel);
        esp_lcd_panel_invert_color(panel, true);
        esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY);
        esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);

        display_ = new SpiLcdDisplay(panel_io, panel,
            DISPLAY_WIDTH, DISPLAY_HEIGHT,
            DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y,
            DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY);
    }

    void InitializeButtons() {
        // Middle: navigate (click=page, long=settings)
        middle_button_.OnClick([]() {
            auto& app = BuddyApp::GetInstance();
            if (app.IsScreenOff()) { app.NotifyActivity(); return; }
            app.NotifyActivity();
            if (buddy_ui_in_settings()) {
                buddy_ui_settings_next();
            } else {
                buddy_ui_next_mode();
            }
        });
        middle_button_.OnLongPress([]() {
            auto& app = BuddyApp::GetInstance();
            app.NotifyActivity();
            buddy_ui_show_settings();
        });

        // Right: approve / switch pet / toggle setting
        right_button_.OnClick([]() {
            auto& app = BuddyApp::GetInstance();
            if (app.IsScreenOff()) { app.NotifyActivity(); return; }
            app.NotifyActivity();
            if (buddy_ui_in_settings()) {
                buddy_ui_settings_select();
            } else if (app.GetState().has_prompt()) {
                app.Approve();
            } else {
                uint8_t next = (buddy_pet_get_species() + 1) % buddy_pet_species_count();
                buddy_pet_set_species(next);
                buddy_nvs_save_species(next);
            }
        });

        // Left: deny / demo mode / exit settings
        left_button_.OnClick([]() {
            auto& app = BuddyApp::GetInstance();
            if (app.IsScreenOff()) { app.NotifyActivity(); return; }
            app.NotifyActivity();
            if (buddy_ui_in_settings()) {
                buddy_ui_show_settings();  // toggle off
            } else if (app.GetState().has_prompt()) {
                app.Deny();
            } else {
                if (demo_mode_active()) demo_mode_stop();
                else demo_mode_start();
            }
        });
    }

public:
    AtkDnesp32s3Box0Board()
        : middle_button_(GPIO_NUM_4, true),   // M button, active high
          right_button_(GPIO_NUM_0),           // R button
          left_button_(GPIO_NUM_3) {           // L button
        InitializePower();
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

    virtual std::string GetBoardType() override { return "atk-dnesp32s3-box0"; }
    virtual Display* GetDisplay() override { return display_; }
    virtual Buzzer* GetBuzzer() override { return buzzer_; }
    virtual const char* GetApprovalHint() override { return "Right = Yes    Left = No"; }

    virtual Backlight* GetBacklight() override {
        static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN, DISPLAY_BACKLIGHT_OUTPUT_INVERT);
        return &backlight;
    }
};

DECLARE_BOARD(AtkDnesp32s3Box0Board);
