#include "board.h"
#include "display/lcd_display.h"
#include "button.h"
#include "config.h"
#include "i2s_buzzer.h"
#include "buddy/core/buddy_app.h"
#include "buddy/core/demo_mode.h"
#include "buddy/pet/buddy_pet.h"
#include "buddy/ui/buddy_ui.h"
#include "buddy/storage/buddy_nvs.h"

#include <esp_log.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include "esp_lcd_gc9d01n.h"
#include <driver/gpio.h>
#include <driver/spi_master.h>

#define TAG "LilygoTCircleS3Buddy"

class LilygoTCircleS3Board : public Board {
private:
    Button boot_button_;
    Display* display_;
    I2sBuzzer* buzzer_;

    void InitializeSpi() {
        spi_bus_config_t buscfg = {};
        buscfg.mosi_io_num = DISPLAY_MOSI;
        buscfg.miso_io_num = GPIO_NUM_NC;
        buscfg.sclk_io_num = DISPLAY_SCLK;
        buscfg.quadwp_io_num = GPIO_NUM_NC;
        buscfg.quadhd_io_num = GPIO_NUM_NC;
        buscfg.max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);
        ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO));
    }

    void InitializeDisplay() {
        esp_lcd_panel_io_handle_t panel_io = nullptr;
        esp_lcd_panel_handle_t panel = nullptr;

        esp_lcd_panel_io_spi_config_t io_config = {};
        io_config.cs_gpio_num = DISPLAY_CS;
        io_config.dc_gpio_num = DISPLAY_DC;
        io_config.spi_mode = 0;
        io_config.pclk_hz = 40 * 1000 * 1000;
        io_config.trans_queue_depth = 10;
        io_config.lcd_cmd_bits = 8;
        io_config.lcd_param_bits = 8;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI3_HOST, &io_config, &panel_io));

        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = DISPLAY_RST;
        panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
        panel_config.bits_per_pixel = 16;
        ESP_ERROR_CHECK(esp_lcd_new_panel_gc9d01n(panel_io, &panel_config, &panel));

        esp_lcd_panel_reset(panel);
        esp_lcd_panel_init(panel);
        esp_lcd_panel_invert_color(panel, false);
        esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY);
        esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);

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
            BuddyApp::GetInstance().NotifyActivity();
            uint8_t next = (buddy_pet_get_species() + 1) % buddy_pet_species_count();
            buddy_pet_set_species(next);
            buddy_nvs_save_species(next);
        });
        boot_button_.OnMultipleClick([]() {
            if (demo_mode_active()) demo_mode_stop();
            else demo_mode_start();
        }, 3);
    }

public:
    LilygoTCircleS3Board() : boot_button_(BOOT_BUTTON_GPIO) {
        InitializeSpi();
        InitializeDisplay();
        InitializeButtons();
        buzzer_ = new I2sBuzzer(AUDIO_SPKR_BCLK, AUDIO_SPKR_LRCLK,
                                AUDIO_SPKR_DATA, AUDIO_SPKR_ENABLE, I2S_NUM_1);
        GetBacklight()->RestoreBrightness();
    }

    virtual std::string GetBoardType() override { return "lilygo-t-circle-s3"; }
    virtual Display* GetDisplay() override { return display_; }
    virtual Buzzer* GetBuzzer() override { return buzzer_; }

    virtual Backlight* GetBacklight() override {
        static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN, DISPLAY_BACKLIGHT_OUTPUT_INVERT);
        return &backlight;
    }
};

DECLARE_BOARD(LilygoTCircleS3Board);
