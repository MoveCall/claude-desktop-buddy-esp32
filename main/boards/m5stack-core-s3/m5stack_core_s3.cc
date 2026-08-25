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
#include <driver/i2s_std.h>
#include <esp_lcd_touch_ft5x06.h>
#include <esp_lvgl_port.h>
#include <esp_codec_dev.h>
#include <esp_codec_dev_defaults.h>
#include <math.h>
#include <string.h>

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
    void ResetAw88298() {
        WriteReg(0x02, 0b00000011);
        vTaskDelay(pdMS_TO_TICKS(10));
        WriteReg(0x02, 0b00000111);
        vTaskDelay(pdMS_TO_TICKS(50));
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

// AW88298 buzzer using esp_codec_dev framework
class Aw88298Buzzer : public Buzzer {
public:
    Aw88298Buzzer(i2c_master_bus_handle_t i2c_bus, gpio_num_t mclk,
                  gpio_num_t bclk, gpio_num_t ws, gpio_num_t dout) {
        // Create I2S TX channel
        i2s_chan_config_t chan_cfg = {
            .id = I2S_NUM_0,
            .role = I2S_ROLE_MASTER,
            .dma_desc_num = 4,
            .dma_frame_num = 320,
            .auto_clear_after_cb = true,
        };
        ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle_, nullptr));

        i2s_std_config_t std_cfg = {
            .clk_cfg = {
                .sample_rate_hz = 16000,
                .clk_src = I2S_CLK_SRC_DEFAULT,
                .mclk_multiple = I2S_MCLK_MULTIPLE_256,
            },
            .slot_cfg = {
                .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,
                .slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO,
                .slot_mode = I2S_SLOT_MODE_STEREO,
                .slot_mask = I2S_STD_SLOT_BOTH,
                .ws_width = I2S_DATA_BIT_WIDTH_16BIT,
                .ws_pol = false,
                .bit_shift = true,
            },
            .gpio_cfg = {
                .mclk = mclk,
                .bclk = bclk,
                .ws = ws,
                .dout = dout,
                .din = GPIO_NUM_NC,
                .invert_flags = { .mclk_inv = false, .bclk_inv = false, .ws_inv = false },
            },
        };
        ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle_, &std_cfg));

        // Init AW88298 via esp_codec_dev
        audio_codec_i2s_cfg_t i2s_cfg = {
            .port = I2S_NUM_0,
            .rx_handle = nullptr,
            .tx_handle = tx_handle_,
        };
        data_if_ = audio_codec_new_i2s_data(&i2s_cfg);

        audio_codec_i2c_cfg_t i2c_cfg = {
            .port = I2C_NUM_0,
            .addr = AW88298_I2C_ADDR,
            .bus_handle = i2c_bus,
        };
        ctrl_if_ = audio_codec_new_i2c_ctrl(&i2c_cfg);
        gpio_if_ = audio_codec_new_gpio();

        aw88298_codec_cfg_t aw_cfg = {};
        aw_cfg.ctrl_if = ctrl_if_;
        aw_cfg.gpio_if = gpio_if_;
        aw_cfg.reset_pin = GPIO_NUM_NC;
        aw_cfg.hw_gain.pa_voltage = 5.0;
        aw_cfg.hw_gain.codec_dac_voltage = 3.3;
        aw_cfg.hw_gain.pa_gain = 1;
        codec_if_ = aw88298_codec_new(&aw_cfg);

        esp_codec_dev_cfg_t dev_cfg = {
            .dev_type = ESP_CODEC_DEV_TYPE_OUT,
            .codec_if = codec_if_,
            .data_if = data_if_,
        };
        dev_ = esp_codec_dev_new(&dev_cfg);

        esp_codec_dev_sample_info_t fs = {
            .bits_per_sample = 16,
            .channel = 1,
            .channel_mask = 0,
            .sample_rate = 16000,
        };
        esp_codec_dev_open(dev_, &fs);
        esp_codec_dev_set_out_vol(dev_, 60);

        ESP_LOGI(TAG, "AW88298 buzzer initialized via esp_codec_dev");
    }

    void Tone(uint16_t freq_hz, uint16_t duration_ms) override {
        if (!dev_) return;
        if (freq_hz == 0) {
            vTaskDelay(pdMS_TO_TICKS(duration_ms));
            return;
        }

        int16_t buf[320];
        int total = (16000 * duration_ms) / 1000;
        int written = 0;
        float phase = 0.0f;
        float inc = 2.0f * 3.14159265f * freq_hz / 16000.0f;

        while (written < total) {
            int chunk = total - written;
            if (chunk > 320) chunk = 320;
            for (int i = 0; i < chunk; i++) {
                buf[i] = (int16_t)(sinf(phase) * 3000);
                phase += inc;
                if (phase >= 2.0f * 3.14159265f) phase -= 2.0f * 3.14159265f;
            }
            esp_codec_dev_write(dev_, buf, chunk * sizeof(int16_t));
            written += chunk;
        }

        memset(buf, 0, sizeof(buf));
        esp_codec_dev_write(dev_, buf, sizeof(buf));
    }

private:
    i2s_chan_handle_t tx_handle_ = nullptr;
    const audio_codec_data_if_t* data_if_ = nullptr;
    const audio_codec_ctrl_if_t* ctrl_if_ = nullptr;
    const audio_codec_if_t* codec_if_ = nullptr;
    const audio_codec_gpio_if_t* gpio_if_ = nullptr;
    esp_codec_dev_handle_t dev_ = nullptr;
};

class M5StackCoreS3Board : public Board {
private:
    i2c_master_bus_handle_t i2c_bus_;
    Pmic* pmic_;
    Aw9523* aw9523_;
    Button boot_button_;
    Display* display_;
    Aw88298Buzzer* buzzer_;

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

    void InitializeTouch() {
        esp_lcd_touch_handle_t tp;
        esp_lcd_touch_config_t tp_cfg = {
            .x_max = DISPLAY_WIDTH,
            .y_max = DISPLAY_HEIGHT,
            .rst_gpio_num = GPIO_NUM_NC,
            .int_gpio_num = GPIO_NUM_NC,
            .levels = { .reset = 0, .interrupt = 0 },
            .flags = { .swap_xy = 0, .mirror_x = 0, .mirror_y = 0 },
        };
        esp_lcd_panel_io_handle_t tp_io = nullptr;
        esp_lcd_panel_io_i2c_config_t tp_io_cfg = {
            .dev_addr = ESP_LCD_TOUCH_IO_I2C_FT5x06_ADDRESS,
            .control_phase_bytes = 1,
            .dc_bit_offset = 0,
            .lcd_cmd_bits = 8,
            .flags = { .disable_control_phase = 1 },
        };
        tp_io_cfg.scl_speed_hz = 400 * 1000;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus_, &tp_io_cfg, &tp_io));
        ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_ft5x06(tp_io, &tp_cfg, &tp));
        const lvgl_port_touch_cfg_t touch_cfg = {
            .disp = lv_display_get_default(),
            .handle = tp,
        };
        lvgl_port_add_touch(&touch_cfg);
        ESP_LOGI(TAG, "Touch panel initialized");
    }

    void InitializeButtons() {
        // BOOT_BUTTON_GPIO is GPIO_NUM_NC on this board (see config.h), so
        // Button's constructor no-ops and every binding below is inert. They
        // are kept, not deleted: they are the shared shape every other board
        // uses, and a revision that wires a real button would just work.
        ESP_LOGI(TAG, "No boot button on this board; input is the touch screen");
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
    M5StackCoreS3Board() : boot_button_(BOOT_BUTTON_GPIO) {
        InitializeI2c();
        pmic_ = new Pmic(i2c_bus_);
        aw9523_ = new Aw9523(i2c_bus_);
        aw9523_->ResetAw88298();
        InitializeSpi();
        InitializeDisplay();
        InitializeTouch();
        InitializeButtons();
        buzzer_ = new Aw88298Buzzer(i2c_bus_,
            AUDIO_I2S_GPIO_MCLK, AUDIO_I2S_GPIO_BCLK,
            AUDIO_I2S_GPIO_WS, AUDIO_I2S_GPIO_DOUT);
        GetBacklight()->RestoreBrightness();
    }

    virtual std::string GetBoardType() override { return "m5stack-core-s3"; }
    virtual Display* GetDisplay() override { return display_; }
    virtual Buzzer* GetBuzzer() override { return buzzer_; }
    virtual bool HasTouchScreen() override { return true; }
    virtual const char* GetApprovalHint() override { return "Tap to Approve / Deny"; }

    virtual Backlight* GetBacklight() override {
        static PmicBacklight backlight(pmic_);
        return &backlight;
    }
};

DECLARE_BOARD(M5StackCoreS3Board);
