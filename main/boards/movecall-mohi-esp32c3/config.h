#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

#define BUILTIN_LED_GPIO        GPIO_NUM_8   // WS2812 x4
#define BOOT_BUTTON_GPIO        GPIO_NUM_9

// Display - ST7789 160x80 SPI (landscape, swap_xy)
#define DISPLAY_WIDTH   160
#define DISPLAY_HEIGHT  80
#define DISPLAY_MIRROR_X false
#define DISPLAY_MIRROR_Y true
#define DISPLAY_SWAP_XY true
#define DISPLAY_OFFSET_X 0
#define DISPLAY_OFFSET_Y 24

#define DISPLAY_SPI_MOSI_PIN GPIO_NUM_4
#define DISPLAY_SPI_SCLK_PIN GPIO_NUM_5
#define DISPLAY_SPI_DC_PIN   GPIO_NUM_10
#define DISPLAY_SPI_CS_PIN   GPIO_NUM_NC
#define DISPLAY_SPI_RST_PIN  GPIO_NUM_NC

#define DISPLAY_BACKLIGHT_PIN GPIO_NUM_NC
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT false

// PDM speaker (for buzzer tone generation)
#define AUDIO_PDM_GPIO_P GPIO_NUM_6
#define AUDIO_PDM_GPIO_N GPIO_NUM_7
#define AUDIO_PA_CTL_GPIO GPIO_NUM_3

#endif // _BOARD_CONFIG_H_
