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
#include <esp_lcd_ili9341.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <driver/i2c_master.h>

#define TAG "M5StackCoreS3Buddy"

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
    ~I2cDev() { i2c_master_bus_rm_device(handle_); }

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
        uint8_t data = ReadReg(0x90);
        data |= 0b10110100;
        WriteReg(0x90, data);
        WriteReg(0x99, (0b11110 - 5));
        WriteReg(0x97, (0b11110 - 2));
        WriteReg(0x69, 0b00110101);
        WriteReg(0x30, 0b111111);
        WriteReg(0x90, 0xBF);
        WriteReg(0x94, 33 - 5);
        WriteReg(0x95, 33 - 5);
    }

    void SetBrightness(uint8_t brightness) {
        brightness = ((brightness + 641) >> 5);
        WriteReg(0x99, brightness);
    }
};

class Aw9523 : public I2cDev {
public:
    Aw9523(i2c_master_bus_handle_t bus) : I2cDev(bus, 0x58) {
        WriteReg(0x02, 0b00000111);
        WriteReg(0x03, 0b10001111);
        WriteReg(0x04, 0b00011000);
        WriteReg(0x05, 0b00001100);
        WriteReg(0x11, 0b00010000);
        WriteReg(0x12, 0b11111111);
        WriteReg(0x13, 0b11111111);
    }

    void ResetLcd() {
        WriteReg(0x03, 0b10000001);
        vTaskDelay(pdMS_TO_TICKS(20));
        WriteReg(0x03, 0b10000011);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
};

class PmicBacklight : public Backlight {
public:
    PmicBacklight(Pmic* pmic) : pmic_(pmic) {}
    void SetBrightnessImpl(uint8_t brightness) override {
        pmic_->SetBrightness(target_brightness_);
        brightness_ = target_brightness_;
    }
private:
    Pmic* pmic_;
};

class M5StackCoreS3Board : public Board {
private:
    i2c_master_bus_handle_t i2c_bus_;
    Pmic* pmic_;
    Aw9523* aw9523_;
    Button boot_button_;
    Display* display_;

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

    void InitializeSpi() {
        spi_bus_config_t buscfg = {};
        buscfg.mosi_io_num = DISPLAY_SPI_MOSI_PIN;
        buscfg.miso_io_num = GPIO_NUM_NC;
        buscfg.sclk_io_num = DISPLAY_SPI_SCLK_PIN;
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
        io_config.spi_mode = 2;
        io_config.pclk_hz = 40 * 1000 * 1000;
        io_config.trans_queue_depth = 10;
        io_config.lcd_cmd_bits = 8;
        io_config.lcd_param_bits = 8;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI3_HOST, &io_config, &panel_io));

        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = GPIO_NUM_NC;
        panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR;
        panel_config.bits_per_pixel = 16;
        ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(panel_io, &panel_config, &panel));

        esp_lcd_panel_reset(panel);
        aw9523_->ResetLcd();
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
    M5StackCoreS3Board() : boot_button_(BOOT_BUTTON_GPIO) {
        InitializeI2c();
        pmic_ = new Pmic(i2c_bus_);
        aw9523_ = new Aw9523(i2c_bus_);
        InitializeSpi();
        InitializeDisplay();
        InitializeButtons();
        GetBacklight()->RestoreBrightness();
    }

    virtual std::string GetBoardType() override { return "m5stack-core-s3"; }
    virtual Display* GetDisplay() override { return display_; }

    virtual Backlight* GetBacklight() override {
        static PmicBacklight backlight(pmic_);
        return &backlight;
    }
};

DECLARE_BOARD(M5StackCoreS3Board);
