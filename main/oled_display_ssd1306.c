/*
 *  OLED Display SSD1306 Driver
 *      the one that can be bought in lazada or
 *      bangood. usually 2 colors
 * 
 */

#include "freertos/FreeRTOS.h"

#include <string.h>

#include "driver/gpio.h"
#include "driver/i2c.h"

#include "log_wrapper.h"
#include "common.h"

#include "i2c_helper.h"
#include "oled_display_ssd1306.h"

#include "oled_display_font_basic.h"
#include "oled_display_font_gldc.h"

static const char* TAG = "oled_ssd1306";

unsigned char ilp_oled_i2c_addr = SSD1306_OLED_ADDR2;
char is_i2c_master_initialized = 0;
char text[MAX_TEXT_BUFFER] = "Empty";

int oled_set_i2c_address(unsigned char addr)
{
    ilp_oled_i2c_addr = addr;
    return 0;
}

esp_err_t oled_display_ssd1306_init()
{    
    return oled_ssd1306_init();
}

int oled_write_text(char* data)
{
    memset(text, 0, MAX_TEXT_BUFFER);
    strncpy(text, data, MAX_TEXT_BUFFER);
    // oled_ssd1306_clear(NULL);
    // ilp_delay_in_millis(500);
    oled_ssd1306_print(NULL);

    return 0;
}

esp_err_t oled_ssd1306_init() {
    if(is_i2c_master_initialized == 0)
    {   //one time init of i2c master
        i2c_master_init();
        is_i2c_master_initialized = 1;
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    i2c_master_write_byte(cmd, (ilp_oled_i2c_addr << 1) | WRITE_BIT, ACK_CHECK_EN);

    // Initialization (page 64)
    // The next bytes are commands
    i2c_master_write_byte(cmd, COMMAND_MODE, ACK_CHECK_EN);

    // Mux Ratio
    i2c_master_write_byte(cmd, MULTIPLEX_RATIO, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0x3F, ACK_CHECK_EN);

    // Set display offset
    i2c_master_write_byte(cmd, DISPLAY_OFFSET, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);

    // Set display line start
    i2c_master_write_byte(cmd, DISPLAY_LINE_START, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);

    // Set Segment re-map
    i2c_master_write_byte(cmd, SEGMENT_REMAP, ACK_CHECK_EN);
    // or 1 will mirror the image in combination with COM_OUTPUT_SCAN_DIR_DEC
    i2c_master_write_byte(cmd, SEGMENT_REMAP | 0x1, ACK_CHECK_EN);

    // Set COM output scan dir
    // i2c_master_write_byte(cmd, COM_OUTPUT_SCAN_DIR_INC, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, COM_OUTPUT_SCAN_DIR_DEC, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);

    // Set COM pins hardware config
    i2c_master_write_byte(cmd, COM_PINS_HARDWARE_CONFIG, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0x12, ACK_CHECK_EN);

    // Set contrast Control
    i2c_master_write_byte(cmd, CONTRAST_CONTROL, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0x7F, ACK_CHECK_EN);

    // Disable entire display ON
    i2c_master_write_byte(cmd, DISABLE_ENTIRE_DISPLAY, ACK_CHECK_EN);

    // Set normal display
    i2c_master_write_byte(cmd, NORMAL_DISPLAY, ACK_CHECK_EN);
    // Set inverse display
    // i2c_master_write_byte(cmd, DISPLAY_INVERSE, ACK_CHECK_EN);

    // Set OSC frequency
    i2c_master_write_byte(cmd, DISPLAY_CLK_RATIO, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0x80, ACK_CHECK_EN);

    // Enable charge pump regulator
    i2c_master_write_byte(cmd, CHARGE_PUMP_SET, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0x14, ACK_CHECK_EN);

    // Display on
    i2c_master_write_byte(cmd, DISPLAY_ON, ACK_CHECK_EN);

    // Stop bit
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

//what does it do??? already tried but no visiual changes
void oled_ssd1306_test(void *ignore) {
    i2c_cmd_handle_t cmd;
    static uint8_t page = 0;
    static uint8_t color = 0x00;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);

	i2c_master_write_byte(cmd, (ilp_oled_i2c_addr << 1) | WRITE_BIT, ACK_CHECK_EN);

    i2c_master_write_byte(cmd, SINGLE_COMMAND_MODE, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, (PAGE_START_ADDR | (page++)), ACK_CHECK_EN);

	i2c_master_write_byte(cmd, DATA_MODE, ACK_CHECK_EN);
    for(uint8_t col = 0; col < 128; col++) {
        i2c_master_write_byte(cmd, color, ACK_CHECK_EN);
    }
    // Stop bit
	i2c_master_stop(cmd);

    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    if(page == 8) page = 0;

	// vTaskDelete(NULL);
}

esp_err_t oled_ssd1306_clear(void *ignore) {
    i2c_cmd_handle_t cmd;
    esp_err_t ret;

    for(uint8_t page = 0; page < 8; page++) {
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);

        i2c_master_write_byte(cmd, (ilp_oled_i2c_addr << 1) | WRITE_BIT, ACK_CHECK_EN);

        i2c_master_write_byte(cmd, SINGLE_COMMAND_MODE, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, (PAGE_START_ADDR | page), ACK_CHECK_EN);

        i2c_master_write_byte(cmd, DATA_MODE, ACK_CHECK_EN);
        for(uint8_t col = 0; col < 128; col++)
            i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);

            // Stop bit
        i2c_master_stop(cmd);

        ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        if(ret != ESP_OK)
            return ret;
    }
	// vTaskDelete(NULL);
    return ret;
}

esp_err_t oled_ssd1306_print(void *args) {
    // char *text = (char*)args;
    // char text[] = "! ! ! ! ! ! ";
    esp_err_t ret;
    
    uint8_t len = strlen(text);
    uint8_t page = 0, i = 0;
    uint16_t char_index = 0;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    i2c_master_write_byte(cmd, (ilp_oled_i2c_addr << 1) | WRITE_BIT, ACK_CHECK_EN);

    i2c_master_write_byte(cmd, COMMAND_MODE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, (PAGE_START_ADDR | page++), ACK_CHECK_EN);
    i2c_master_write_byte(cmd, LOWER_COL_START_ADDR, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, HIGHER_COL_START_ADDR, ACK_CHECK_EN);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    
    if(ret != ESP_OK)
        return ret;

    for(i = 0; i < len; i++) 
    {
        // Calculate the pos of the char in font array
        char_index = (text[i] <= 0) ? 0 : font_width * text[i];

        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);

        i2c_master_write_byte(cmd, (ilp_oled_i2c_addr << 1) | WRITE_BIT, ACK_CHECK_EN);

        if(text[i] == '\n') { // new page, reset cols
            i2c_master_write_byte(cmd, COMMAND_MODE, ACK_CHECK_EN);
            i2c_master_write_byte(cmd, (PAGE_START_ADDR | page++), ACK_CHECK_EN);
            i2c_master_write_byte(cmd, LOWER_COL_START_ADDR, ACK_CHECK_EN);
            i2c_master_write_byte(cmd, HIGHER_COL_START_ADDR, ACK_CHECK_EN);
        }

        i2c_master_write_byte(cmd, DATA_MODE, ACK_CHECK_EN);

        for(uint8_t col = 0; col < font_width; col++){
            i2c_master_write_byte(cmd, font[char_index + col], ACK_CHECK_EN);
        }

        // Stop bit
        i2c_master_stop(cmd);

        ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        if(ret != ESP_OK)
            return ret;
    }
	// vTaskDelete(NULL);
    return ret;
}

esp_err_t oled_ssd1306_screensaver(void *ignore) {
    uint8_t square = 0xFF;
    i2c_cmd_handle_t cmd;
    esp_err_t ret;

    // chess pattern
    for(uint8_t page = 0; page < 8; page++) {
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);

        i2c_master_write_byte(cmd, (ilp_oled_i2c_addr << 1) | WRITE_BIT, ACK_CHECK_EN);

        i2c_master_write_byte(cmd, SINGLE_COMMAND_MODE, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, (PAGE_START_ADDR | page), ACK_CHECK_EN);

        i2c_master_write_byte(cmd, DATA_MODE, ACK_CHECK_EN);
        for(uint8_t col = 0; col < 128; col++){
            square = (col % 7 == 0) ? ~square : square;
            i2c_master_write_byte(cmd, square, ACK_CHECK_EN);
        }
        // Stop bit
        i2c_master_stop(cmd);

        ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        if(ret != ESP_OK)
            return ret;
    }

    // hor and vertical scrolling using graphic acceleration commands
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ilp_oled_i2c_addr << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, COMMAND_MODE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, DEACTIVATE_SCROLL, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, VERTICAL_AND_RIGHT_HOR_SCROLL, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, DUMMY_BYTE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, PAGE_START_ADDR, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, SIX_FRAMES_PER_SEC, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, (PAGE_START_ADDR | 0x07), ACK_CHECK_EN);
    i2c_master_write_byte(cmd, VERTICAL_OFFSET_ONE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, ACTIVATE_SCROLL, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    // vTaskDelete(NULL);
    return ret;
}