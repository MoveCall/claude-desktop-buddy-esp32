#include "board.h"
#include "display/lcd_display.h"
#include "button.h"
#include "config.h"
#include "buddy/core/buddy_app.h"
#include "buddy/core/demo_mode.h"
#include "buddy/pet/buddy_pet.h"
#include "buddy/ui/buddy_ui.h"
#include "buddy/storage/buddy_nvs.h"

#include <esp_log.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_sh8601.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <driver/i2c_master.h>
#include <esp_io_expander_tca9554.h>

#define TAG "WaveshareAmoled18Buddy"

#define LCD_OPCODE_WRITE_CMD (0x02ULL)
#define LCD_OPCODE_WRITE_COLOR (0x32ULL)

static const sh8601_lcd_init_cmd_t vendor_specific_init[] = {
    {0x11, (uint8_t[]){0x00}, 0, 120},
    {0x44, (uint8_t[]){0x01, 0xD1}, 2, 0},
    {0x35, (uint8_t[]){0x00}, 1, 0},
    {0x53, (uint8_t[]){0x20}, 1, 10},
    {0x2A, (uint8_t[]){0x00, 0x00, 0x01, 0x6F}, 4, 0},
    {0x2B, (uint8_t[]){0x00, 0x00, 0x01, 0xBF}, 4, 0},
    {0x51, (uint8_t[]){0x00}, 1, 10},
    {0x29, (uint8_t[]){0x00}, 0, 10}
};

// Simplified I2C device helper
class I2cDev {
public:
    I2cDev(i2c_master_bus_handle_t bus, uint8_t addr) {
        i2c_device_config_t cfg = {};
        cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
        cfg.device_address = addr;
        cfg.scl_speed_hz = 400000;
        ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &cfg, &handle_));
    }

    uint8_t ReadReg(uint8_t reg) {
        uint8_t val = 0;
        ESP_ERROR_CHECK(i2c_master_transmit_receive(handle_, &reg, 1, &val, 1, -1));
        return val;
    }
    void WriteReg(uint8_t reg, uint8_t val) {
        uint8_t buf[2] = {reg, val};
        ESP_ERROR_CHECK(i2c_master_transmit(handle_, buf, 2, -1));
    }

protected:
    i2c_master_dev_handle_t handle_;
};

class Pmic : public I2cDev {
public:
    Pmic(i2c_master_bus_handle_t bus) : I2cDev(bus, 0x34) {
        WriteReg(0x22, 0b110);
        WriteReg(0x27, 0x10);
        WriteReg(0x80, 0x01);
        WriteReg(0x90, 0x00);
        WriteReg(0x91, 0x00);
        WriteReg(0x82, (3300 - 1500) / 100);
        WriteReg(0x92, (3300 - 500) / 100);
        WriteReg(0x90, 0x01);
        WriteReg(0x64, 0x02);
        WriteReg(0x61, 0x02);
        WriteReg(0x62, 0x08);
        WriteReg(0x63, 0x01);
    }
};

class AmoledBacklight : public Backlight {
public:
    AmoledBacklight(esp_lcd_panel_io_handle_t panel_io) : panel_io_(panel_io) {}

protected:
    void SetBrightnessImpl(uint8_t brightness) override {
        uint8_t data[1] = {(uint8_t)((255 * brightness) / 100)};
        int lcd_cmd = 0x51;
        lcd_cmd &= 0xff;
        lcd_cmd <<= 8;
        lcd_cmd |= LCD_OPCODE_WRITE_CMD << 24;
        esp_lcd_panel_io_tx_param(panel_io_, lcd_cmd, &data, sizeof(data));
    }

private:
    esp_lcd_panel_io_handle_t panel_io_;
};

class WaveshareAmoled18Board : public Board {
private:
    i2c_master_bus_handle_t i2c_bus_;
    Pmic* pmic_;
    esp_io_expander_handle_t io_expander_;
    Button boot_button_;
    Display* display_;
    AmoledBacklight* backlight_;

    void InitializeI2c() {
        i2c_master_bus_config_t cfg = {};
        cfg.i2c_port = I2C_NUM_0;
        cfg.sda_io_num = I2C_SDA_PIN;
        cfg.scl_io_num = I2C_SCL_PIN;
        cfg.clk_source = I2C_CLK_SRC_DEFAULT;
        cfg.glitch_ignore_cnt = 7;
        cfg.flags.enable_internal_pullup = 1;
        ESP_ERROR_CHECK(i2c_new_master_bus(&cfg, &i2c_bus_));
    }

    void InitializeTca9554() {
        ESP_ERROR_CHECK(esp_io_expander_new_i2c_tca9554(i2c_bus_, IO_EXPANDER_I2C_ADDR, &io_expander_));
        ESP_ERROR_CHECK(esp_io_expander_set_dir(io_expander_,
            IO_EXPANDER_PIN_NUM_0 | IO_EXPANDER_PIN_NUM_1 | IO_EXPANDER_PIN_NUM_2, IO_EXPANDER_OUTPUT));
        ESP_ERROR_CHECK(esp_io_expander_set_level(io_expander_,
            IO_EXPANDER_PIN_NUM_0 | IO_EXPANDER_PIN_NUM_1 | IO_EXPANDER_PIN_NUM_2, 1));
        vTaskDelay(pdMS_TO_TICKS(100));
        ESP_ERROR_CHECK(esp_io_expander_set_level(io_expander_,
            IO_EXPANDER_PIN_NUM_0 | IO_EXPANDER_PIN_NUM_1 | IO_EXPANDER_PIN_NUM_2, 0));
        vTaskDelay(pdMS_TO_TICKS(300));
        ESP_ERROR_CHECK(esp_io_expander_set_level(io_expander_,
            IO_EXPANDER_PIN_NUM_0 | IO_EXPANDER_PIN_NUM_1 | IO_EXPANDER_PIN_NUM_2, 1));
    }

    void InitializeSpi() {
        spi_bus_config_t buscfg = {};
        buscfg.sclk_io_num = DISPLAY_QSPI_SCLK;
        buscfg.data0_io_num = DISPLAY_QSPI_DATA0;
        buscfg.data1_io_num = DISPLAY_QSPI_DATA1;
        buscfg.data2_io_num = DISPLAY_QSPI_DATA2;
        buscfg.data3_io_num = DISPLAY_QSPI_DATA3;
        buscfg.max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);
        buscfg.flags = SPICOMMON_BUSFLAG_QUAD;
        ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    }

    void InitializeDisplay() {
        esp_lcd_panel_io_handle_t panel_io = nullptr;
        esp_lcd_panel_handle_t panel = nullptr;

        esp_lcd_panel_io_spi_config_t io_config = SH8601_PANEL_IO_QSPI_CONFIG(
            DISPLAY_QSPI_CS, nullptr, nullptr);
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &panel_io));

        const sh8601_vendor_config_t vendor_config = {
            .init_cmds = &vendor_specific_init[0],
            .init_cmds_size = sizeof(vendor_specific_init) / sizeof(sh8601_lcd_init_cmd_t),
            .flags = {
                .use_qspi_interface = 1,
            }
        };

        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = GPIO_NUM_NC;
        panel_config.flags.reset_active_high = 1;
        panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
        panel_config.bits_per_pixel = 16;
        panel_config.vendor_config = (void*)&vendor_config;
        ESP_ERROR_CHECK(esp_lcd_new_panel_sh8601(panel_io, &panel_config, &panel));

        esp_lcd_panel_reset(panel);
        esp_lcd_panel_init(panel);
        esp_lcd_panel_invert_color(panel, false);
        esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);
        esp_lcd_panel_disp_on_off(panel, true);

        display_ = new SpiLcdDisplay(panel_io, panel,
            DISPLAY_WIDTH, DISPLAY_HEIGHT,
            DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y,
            DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY);

        backlight_ = new AmoledBacklight(panel_io);
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
        boot_button_.OnMultipleClick([]() {
            if (demo_mode_active()) demo_mode_stop();
            else demo_mode_start();
        }, 3);
    }

public:
    WaveshareAmoled18Board() : boot_button_(BOOT_BUTTON_GPIO) {
        InitializeI2c();
        InitializeTca9554();
        pmic_ = new Pmic(i2c_bus_);
        InitializeSpi();
        InitializeDisplay();
        InitializeButtons();
        GetBacklight()->RestoreBrightness();
    }

    virtual std::string GetBoardType() override { return "waveshare-esp32s3-touch-amoled-1.8"; }
    virtual Display* GetDisplay() override { return display_; }

    virtual Backlight* GetBacklight() override {
        return backlight_;
    }
};

DECLARE_BOARD(WaveshareAmoled18Board);
