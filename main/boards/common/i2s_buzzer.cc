#include "i2s_buzzer.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <math.h>
#include <string.h>

#define TAG "i2s_buzzer"
#define SAMPLE_RATE 16000
#define TONE_BUF_SAMPLES 320

I2sBuzzer::I2sBuzzer(gpio_num_t bclk, gpio_num_t lrclk, gpio_num_t dout,
                     gpio_num_t enable_pin, i2s_port_t port, gpio_num_t mclk)
    : tx_handle_(nullptr), enable_pin_(enable_pin), initialized_(false) {

    if (enable_pin_ != GPIO_NUM_NC) {
        gpio_config_t io_conf = {};
        io_conf.pin_bit_mask = 1ULL << enable_pin_;
        io_conf.mode = GPIO_MODE_OUTPUT;
        gpio_config(&io_conf);
        gpio_set_level(enable_pin_, 0);
    }

    i2s_chan_config_t chan_cfg = {};
    chan_cfg.id = port;
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
            .ws = lrclk,
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

    initialized_ = true;
    ESP_LOGI(TAG, "I2S buzzer initialized on port %d", port);
}

I2sBuzzer::~I2sBuzzer() {
    if (enable_pin_ != GPIO_NUM_NC) gpio_set_level(enable_pin_, 0);
    if (tx_handle_) {
        i2s_channel_disable(tx_handle_);
        i2s_del_channel(tx_handle_);
    }
}

void I2sBuzzer::Tone(uint16_t freq_hz, uint16_t duration_ms) {
    if (!initialized_ || !tx_handle_) return;

    if (freq_hz == 0) {
        if (enable_pin_ != GPIO_NUM_NC) gpio_set_level(enable_pin_, 0);
        vTaskDelay(pdMS_TO_TICKS(duration_ms));
        return;
    }

    if (enable_pin_ != GPIO_NUM_NC) gpio_set_level(enable_pin_, 1);
    i2s_channel_enable(tx_handle_);

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

    memset(buf, 0, sizeof(buf));
    size_t dummy;
    i2s_channel_write(tx_handle_, buf, sizeof(buf), &dummy, portMAX_DELAY);

    i2s_channel_disable(tx_handle_);
    if (enable_pin_ != GPIO_NUM_NC) gpio_set_level(enable_pin_, 0);
}
