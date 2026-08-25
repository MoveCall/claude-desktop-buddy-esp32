#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>
#include <aw88298_dac.h>

#define BUILTIN_LED_GPIO        GPIO_NUM_NC
// CoreS3 has no BOOT button - only POWER and RESET, on the side - and GPIO0
// here is AUDIO_I2S_GPIO_MCLK below. The I2S peripheral claims the pin after
// the button does, so a button on it sits on a ~4MHz clock output and can
// never report a press. Input on this board is the touch screen.
#define BOOT_BUTTON_GPIO        GPIO_NUM_NC

// I2C bus (shared by PMIC, IO expander, audio codec)
#define I2C_SDA_PIN GPIO_NUM_12
#define I2C_SCL_PIN GPIO_NUM_11

// I2S audio (AW88298 amplifier)
#define AUDIO_I2S_GPIO_MCLK  GPIO_NUM_0
#define AUDIO_I2S_GPIO_WS    GPIO_NUM_33
#define AUDIO_I2S_GPIO_BCLK  GPIO_NUM_34
#define AUDIO_I2S_GPIO_DOUT  GPIO_NUM_13
#define AW88298_I2C_ADDR     AW88298_CODEC_DEFAULT_ADDR

// Display - ILI9341 SPI
#define DISPLAY_WIDTH   320
#define DISPLAY_HEIGHT  240
#define DISPLAY_MIRROR_X false
#define DISPLAY_MIRROR_Y false
#define DISPLAY_SWAP_XY false
#define DISPLAY_OFFSET_X  0
#define DISPLAY_OFFSET_Y  0

#define DISPLAY_SPI_MOSI_PIN  GPIO_NUM_37
#define DISPLAY_SPI_SCLK_PIN  GPIO_NUM_36
#define DISPLAY_SPI_CS_PIN    GPIO_NUM_3
#define DISPLAY_SPI_DC_PIN    GPIO_NUM_35

#define DISPLAY_BACKLIGHT_PIN GPIO_NUM_NC
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT true

#endif // _BOARD_CONFIG_H_
