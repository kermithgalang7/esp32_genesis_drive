#ifndef __OLED_DISPLAY_SSD1306_H__
#define __OLED_DISPLAY_SSD1306_H__

#include "freertos/FreeRTOS.h"

// SELECT FONTS
// #define GLCDFONT
#define FONT88BASIC

// DISPLAY CHARS
#define DISPLAY_WIDTH 127
#define DISPLAY_HEIGHT 63

/* Command Table (ssd1306 datasheet) */

// Control bytes (page 20)
#define COMMAND_MODE 0x00
#define SINGLE_COMMAND_MODE 0x80
#define DATA_MODE 0x40

// Fundamental Command (page 28)
#define DISPLAY_LINE_START 0x40
#define CONTRAST_CONTROL 0x81
#define DISABLE_ENTIRE_DISPLAY 0xA4
#define DISPLAY_ON_REG02 0xA5
#define NORMAL_DISPLAY 0xA6
#define DISPLAY_INVERSE  0xA7
#define DISPLAY_RESET 0xAE
#define DISPLAY_ON 0xAF

// Addressing Setting (page 30)
#define MEM_ADDR_MODE 0x20
#define LOWER_COL_START_ADDR 0x00
#define HIGHER_COL_START_ADDR 0x10
#define PAGE_START_ADDR 0xB0

// Scrolling
#define DEACTIVATE_SCROLL 0x2E
#define ACTIVATE_SCROLL 0x2F
#define VERTICAL_AND_RIGHT_HOR_SCROLL 0x29
#define DUMMY_BYTE 0x00
#define SIX_FRAMES_PER_SEC 0x00
#define VERTICAL_OFFSET_ONE 0x01


// Hardware Config (page 31)
#define DISPLAY_START_LINE 0x40
#define SEGMENT_REMAP 0xA0
#define MULTIPLEX_RATIO 0xA8
#define COM_OUTPUT_SCAN_DIR 0xC0
#define DISPLAY_OFFSET 0xD3
#define COM_PINS_HARDWARE_CONFIG 0xDA

// Timing & Driving Scheme (page 32)
#define DISPLAY_CLK_RATIO 0xD5
#define PRE_CHARGE_PER 0xD9
#define VCOMH_DESELECT_LEVEL 0xDB
#define NOOPERATION 0xE3

// Charge Pump (page 60)
#define CHARGE_PUMP_SET 0x8D

// Functions
esp_err_t oled_ssd1306_init();
void oled_ssd1306_test(void *ignore);
esp_err_t oled_ssd1306_clear(void *ignore);
esp_err_t oled_ssd1306_print(void *ignore);
esp_err_t oled_ssd1306_screensaver(void *ignore);

//helper functions
esp_err_t oled_display_ssd1306_init();
int oled_set_i2c_address(unsigned char addr);
int oled_write_text(char* data);

#define MAX_TEXT_BUFFER     200


#endif //__OLED_DISPLAY_SSD1306_H__