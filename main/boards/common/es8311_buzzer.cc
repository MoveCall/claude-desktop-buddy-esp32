#include "es8311_buzzer.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <math.h>
#include <string.h>

#define TAG "es8311_buzzer"

#define SAMPLE_RATE 16000
#define TONE_BUF_SAMPLES 320  // 20ms at 16kHz

Es8311Buzzer::Es8311Buzzer(gpio_num_t sda, gpio_num_t scl, uint8_t addr,
                           gpio_num_t mclk, gpio_num_t bclk, gpio_num_t ws,
                           gpio_num_t dout, gpio_num_t pa_pin)
    : i2c_bus_(nullptr), codec_dev_(nullptr), tx_handle_(nullptr),
      pa_pin_(pa_pin), owns_i2c_(true), initialized_(false) {
    InitI2c(sda, scl);
    InitCodec(addr, mclk != GPIO_NUM_NC);
    InitI2s(mclk, bclk, ws, dout);
    InitPa();
    initialized_ = true;
    ESP_LOGI(TAG, "ES8311 buzzer initialized");
}

Es8311Buzzer::Es8311Buzzer(i2c_master_bus_handle_t i2c_bus, uint8_t addr,
                           gpio_num_t mclk, gpio_num_t bclk, gpio_num_t ws,
                           gpio_num_t dout, gpio_num_t pa_pin)
    : i2c_bus_(i2c_bus), codec_dev_(nullptr), tx_handle_(nullptr),
      pa_pin_(pa_pin), owns_i2c_(false), initialized_(false) {
    InitCodec(addr, mclk != GPIO_NUM_NC);
    InitI2s(mclk, bclk, ws, dout);
    InitPa();
    initialized_ = true;
    ESP_LOGI(TAG, "ES8311 buzzer initialized (shared I2C)");
}

Es8311Buzzer::~Es8311Buzzer() {
    if (pa_pin_ != GPIO_NUM_NC) {
        gpio_set_level(pa_pin_, 0);
    }
    if (tx_handle_) {
        i2s_channel_disable(tx_handle_);
        i2s_del_channel(tx_handle_);
    }
}

void Es8311Buzzer::InitPa() {
    if (pa_pin_ != GPIO_NUM_NC) {
        gpio_config_t io_conf = {};
        io_conf.pin_bit_mask = 1ULL << pa_pin_;
        io_conf.mode = GPIO_MODE_OUTPUT;
        gpio_config(&io_conf);
        gpio_set_level(pa_pin_, 0);
    }
}

void Es8311Buzzer::InitI2c(gpio_num_t sda, gpio_num_t scl) {
    i2c_master_bus_config_t cfg = {};
    cfg.i2c_port = I2C_NUM_0;
    cfg.sda_io_num = sda;
    cfg.scl_io_num = scl;
    cfg.clk_source = I2C_CLK_SRC_DEFAULT;
    cfg.glitch_ignore_cnt = 7;
    cfg.flags.enable_internal_pullup = 1;
    ESP_ERROR_CHECK(i2c_new_master_bus(&cfg, &i2c_bus_));
}

void Es8311Buzzer::InitCodec(uint8_t addr, bool use_mclk) {
    i2c_device_config_t dev_cfg = {};
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address = addr;
    dev_cfg.scl_speed_hz = 400000;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_, &dev_cfg, &codec_dev_));

    // Software reset
    WriteReg(0x00, 0x1F);
    vTaskDelay(pdMS_TO_TICKS(20));
    WriteReg(0x00, 0x80);  // Power up, CSM_ON
    vTaskDelay(pdMS_TO_TICKS(20));

    if (use_mclk) {
        // MCLK mode: clock from MCLK pin (256*fs)
        WriteReg(0x01, 0x3F);
        WriteReg(0x02, 0x00);  // MCLK divider = 1
        WriteReg(0x06, 0x07);  // BCLK divider = 8
    } else {
        // No MCLK: use BCLK as clock source
        WriteReg(0x01, 0xBF);  // bit[7]=1: select BCLK as clock source
        WriteReg(0x02, 0x00);
        WriteReg(0x06, 0x01);  // BCLK divider = 1 (BCLK is the clock)
    }
    WriteReg(0x03, 0x10);  // ADC osr
    WriteReg(0x04, 0x10);  // DAC osr
    WriteReg(0x05, 0x00);
    WriteReg(0x07, 0x00);
    WriteReg(0x08, 0xFF);

    // SDP format: I2S, 16-bit
    WriteReg(0x09, 0x0C);  // SDP in: I2S, 16-bit
    WriteReg(0x0A, 0x0C);  // SDP out: I2S, 16-bit

    // System power
    WriteReg(0x0B, 0x00);  // System: power up
    WriteReg(0x0C, 0x00);  // System: normal

    // DAC power and output
    WriteReg(0x10, 0x1C);  // VMID reference, analog bias on
    WriteReg(0x11, 0x7F);  // VSEL = 3.3V
    WriteReg(0x12, 0x28);  // DAC RAMP rate
    WriteReg(0x13, 0x00);  // DAC power on, unmute
    WriteReg(0x14, 0x80);  // DAC lineout enable
    WriteReg(0x32, 0xC0);  // DAC volume ~0dB

    ESP_LOGI(TAG, "ES8311 codec configured for DAC output");
}

void Es8311Buzzer::InitI2s(gpio_num_t mclk, gpio_num_t bclk, gpio_num_t ws, gpio_num_t dout) {
    i2s_chan_config_t chan_cfg = {};
    chan_cfg.id = I2S_NUM_0;
    chan_cfg.role = I2S_ROLE_MASTER;
    chan_cfg.dma_desc_num = 4;
    chan_cfg.dma_frame_num = TONE_BUF_SAMPLES;
    chan_cfg.auto_clear_after_cb = true;
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle_, nullptr));

    i2s_std_config_t std_cfg = {
        .clk_cfg = {
            .sample_rate_hz = SAMPLE_RATE,
            .clk_src = I2S_CLK_SRC_DEFAULT,
            .mclk_multiple = I2S_MCLK_MULTIPLE_256,
        },
        .slot_cfg = {
            .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,
            .slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO,
            .slot_mode = I2S_SLOT_MODE_MONO,
            .slot_mask = I2S_STD_SLOT_LEFT,
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
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle_, &std_cfg));
    ESP_LOGI(TAG, "I2S TX channel configured");
}

void Es8311Buzzer::WriteReg(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    esp_err_t ret = i2c_master_transmit(codec_dev_, buf, 2, -1);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "I2C write 0x%02X=0x%02X failed: %s", reg, val, esp_err_to_name(ret));
    }
}

uint8_t Es8311Buzzer::ReadReg(uint8_t reg) {
    uint8_t val = 0;
    i2c_master_transmit_receive(codec_dev_, &reg, 1, &val, 1, -1);
    return val;
}

void Es8311Buzzer::Tone(uint16_t freq_hz, uint16_t duration_ms) {
    if (!initialized_ || !tx_handle_) return;

    if (freq_hz == 0) {
        if (pa_pin_ != GPIO_NUM_NC) gpio_set_level(pa_pin_, 0);
        vTaskDelay(pdMS_TO_TICKS(duration_ms));
        return;
    }

    // Enable PA and I2S
    if (pa_pin_ != GPIO_NUM_NC) gpio_set_level(pa_pin_, 1);
    i2s_channel_enable(tx_handle_);

    // Generate sine wave PCM and write via I2S
    int16_t buf[TONE_BUF_SAMPLES];
    int total_samples = (SAMPLE_RATE * duration_ms) / 1000;
    int written = 0;
    float phase = 0.0f;
    float phase_inc = 2.0f * M_PI * freq_hz / SAMPLE_RATE;

    while (written < total_samples) {
        int chunk = total_samples - written;
        if (chunk > TONE_BUF_SAMPLES) chunk = TONE_BUF_SAMPLES;

        for (int i = 0; i < chunk; i++) {
            buf[i] = (int16_t)(sinf(phase) * 3000);
            phase += phase_inc;
            if (phase >= 2.0f * M_PI) phase -= 2.0f * M_PI;
        }

        size_t bytes_written = 0;
        i2s_channel_write(tx_handle_, buf, chunk * sizeof(int16_t), &bytes_written, portMAX_DELAY);
        written += chunk;
    }

    // Silence tail to flush DMA
    memset(buf, 0, sizeof(buf));
    size_t dummy;
    i2s_channel_write(tx_handle_, buf, sizeof(buf), &dummy, portMAX_DELAY);

    i2s_channel_disable(tx_handle_);
    if (pa_pin_ != GPIO_NUM_NC) gpio_set_level(pa_pin_, 0);
}
