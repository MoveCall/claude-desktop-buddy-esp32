#include "pdm_buzzer.h"
#include <esp_log.h>
#include <esp_rom_gpio.h>
#include <soc/gpio_sig_map.h>

// Signal name differs across chips
#ifndef I2SO_SD_OUT_IDX
#define I2SO_SD_OUT_IDX I2S0O_SD_OUT_IDX
#endif
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <math.h>
#include <string.h>

#define TAG "pdm_buzzer"
#define SAMPLE_RATE 24000
#define TONE_BUF_SAMPLES 480

PdmBuzzer::PdmBuzzer(gpio_num_t dout, gpio_num_t enable_pin,
                     gpio_num_t din, i2s_port_t port)
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

    i2s_pdm_tx_config_t pdm_cfg = {
        .clk_cfg = I2S_PDM_TX_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .slot_cfg = I2S_PDM_TX_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .clk = GPIO_NUM_NC,
            .dout = dout,
        },
    };

    esp_err_t ret = i2s_channel_init_pdm_tx_mode(tx_handle_, &pdm_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PDM TX init failed: %s", esp_err_to_name(ret));
        i2s_del_channel(tx_handle_);
        tx_handle_ = nullptr;
        return;
    }

    gpio_set_drive_capability(dout, GPIO_DRIVE_CAP_0);

    // Differential output: connect inverted PDM signal to negative pin
    if (din != GPIO_NUM_NC) {
        gpio_set_direction(din, GPIO_MODE_OUTPUT);
        esp_rom_gpio_connect_out_signal(din, I2SO_SD_OUT_IDX, 1, 0);
        gpio_set_drive_capability(din, GPIO_DRIVE_CAP_0);
    }

    initialized_ = true;
    ESP_LOGI(TAG, "PDM buzzer initialized on GPIO %d (neg=%d)", dout, din);
}

PdmBuzzer::~PdmBuzzer() {
    if (enable_pin_ != GPIO_NUM_NC) gpio_set_level(enable_pin_, 0);
    if (tx_handle_) {
        i2s_channel_disable(tx_handle_);
        i2s_del_channel(tx_handle_);
    }
}

void PdmBuzzer::Tone(uint16_t freq_hz, uint16_t duration_ms) {
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
