/*
 *  I2C Collision test software
 *
 *      intended to simulate multiple master and 
 *      multiple slave in one single i2c bus to 
 *      test the arbitration lost and line busy so 
 *      that we can handle multiple error without
 *      affecting the performance, 
 * 
 *      but consider that there will be delay
 *      due to recovery time each collision
 * 
 */
#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_spi_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "log_wrapper.h"
#include "common.h"

#include "wifi.h"
#include "gpio_helper.h"
#include "rest_api_helper.h"
#include "non_volatile_mem.h"

#include "i2c_helper.h"
#include "app_i2c_collision.h"
#include "oled_display_ssd1306.h"

static const char* TAG = "I2Ccollision";

int i2c_collision_running = 1;

void i2c_collision_init()
{
    //enable led status
    ilp_gpio_config_output(ESP32_WROOM_BLUE_LED_GPIO);
}

void app_i2c_collision(void* param)
{
    ILP_LOGI(TAG, "I2C Collision thread started\n");
    i2c_collision_init();

#if 0
    oled_display_ssd1306_init();
    ilp_create_thread(&oled_ssd1306_clear, "oled_ssd1306_clear");
    ilp_delay_in_millis(2000);
    ilp_create_thread(&oled_ssd1306_screensaver, "oled_ssd1306_screensaver");
    ilp_delay_in_millis(2000);
#endif
#if 1
    oled_set_i2c_address(SSD1306_OLED_ADDR1);
    oled_display_ssd1306_init();
    oled_ssd1306_clear(NULL);
    ilp_delay_in_millis(2000);
    oled_ssd1306_screensaver(NULL);
    ilp_delay_in_millis(2000);

    oled_set_i2c_address(SSD1306_OLED_ADDR2);
    oled_display_ssd1306_init();
    oled_ssd1306_clear(NULL);
    ilp_delay_in_millis(2000);
    oled_ssd1306_screensaver(NULL);
    ilp_delay_in_millis(2000);
#endif

    while(i2c_collision_running == 1)
    {
        ilp_gpio_set_low(ESP32_WROOM_BLUE_LED_GPIO);
        ilp_delay_in_millis(1000);

        ilp_gpio_set_high(ESP32_WROOM_BLUE_LED_GPIO);
        ilp_delay_in_millis(1000);
        
        //force delay for watchdog to work
        // ilp_delay_in_millis(1000);
    }
    ILP_LOGI(TAG, "Must not reach this line!!!\n");
}