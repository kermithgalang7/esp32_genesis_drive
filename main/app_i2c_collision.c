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
#include "esp_err.h"

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

unsigned int i2c_collision_led_err_cooldown = 0;
unsigned int current_time = 0;
int i2c_collision_running = 1;
// int i2c_collision_led_status = I2C_COL_LED_STAT_UNKNOWN;
int i2c_collision_led_status = I2C_COL_LED_STAT_IDLE;
char text_buffer[MAX_OLED_SCREEN_BUFFER]; //actually 15x5=75 only not 100

//modify this esp32_device_number, we have 3 in the test setup
int esp32_device_number = 1;
int success_ctr = 0;
int fail_ctr = 0;
int is_oled1_init_success = 0;
int is_oled2_init_success = 0;

void i2c_collision_init()
{
    //enable led status
    ilp_gpio_config_output(ESP32_WROOM_BLUE_LED_GPIO);
}

int app_i2c_collision_generate_msg(char* buff)
{
    memset(buff, 0, MAX_OLED_SCREEN_BUFFER);
    sprintf(buff, 
        "ESP DEV: %d\n" 
        "success: %d\n"
        "failure: %d\n"
        "oled1 init: %d\n"
        "oled2 init: %d\n"
        "time: %d",
        esp32_device_number,
        success_ctr,
        fail_ctr,
        is_oled1_init_success,
        is_oled2_init_success,
        current_time
    );
    return 0;
}

void app_i2c_collision_fail_handler(char* nametag, esp_err_t err)
{
    ILP_LOGI(TAG, "I2C %s %s\n", nametag, esp_err_to_name(err));
    fail_ctr++;
    i2c_collision_led_status = I2C_COL_LED_STAT_I2C_ERR;
    i2c_collision_led_err_cooldown = esp_random() % MAX_COOLDOWN_MULTIPLIER;
    ilp_delay_in_millis(i2c_collision_led_err_cooldown);
    i2c_collision_led_status = I2C_COL_LED_STAT_IDLE;
}

void time_counter(void* arg)
{
    while(1)
    {
        current_time++;
        ilp_delay_in_millis(1000);
    }
}

void led_status(void* arg)
{
    while(1)
    {
        switch(i2c_collision_led_status)
        {
            case I2C_COL_LED_STAT_IDLE:
                ilp_gpio_set_low(ESP32_WROOM_BLUE_LED_GPIO);
                ilp_delay_in_millis(2000);

                ilp_gpio_set_high(ESP32_WROOM_BLUE_LED_GPIO);
                ilp_delay_in_millis(2000);
                break;
            case I2C_COL_LED_STAT_I2C_ERR:
                ilp_gpio_set_low(ESP32_WROOM_BLUE_LED_GPIO);
                ilp_delay_in_millis(150);

                ilp_gpio_set_high(ESP32_WROOM_BLUE_LED_GPIO);
                ilp_delay_in_millis(150);
                break;
            case I2C_COL_LED_STAT_ERR:
                ilp_delay_in_millis(1000);
                ilp_gpio_set_high(ESP32_WROOM_BLUE_LED_GPIO);
                ilp_delay_in_millis(100);
                ilp_gpio_set_low(ESP32_WROOM_BLUE_LED_GPIO);
                ilp_delay_in_millis(100);
                ilp_gpio_set_high(ESP32_WROOM_BLUE_LED_GPIO);
                ilp_delay_in_millis(100);
                ilp_gpio_set_low(ESP32_WROOM_BLUE_LED_GPIO);
                ilp_delay_in_millis(100);
                break;
            case I2C_COL_LED_STAT_UNKNOWN:
                ilp_delay_in_millis(1000);
                ilp_gpio_set_low(ESP32_WROOM_BLUE_LED_GPIO);
                break;
        }
    }
}

void app_i2c_collision(void* param)
{
    int ret;

    ILP_LOGI(TAG, "I2C Collision thread started\n");
    i2c_collision_init();

    //generate device clock counter in sec increment
    ilp_create_thread(&time_counter, "time_counter");
    //led status thread
    ilp_create_thread(&led_status, "led_status");
#if 0
    //need to uncomment vTaskDelete() in oled_display_ssd1306.c
    oled_display_ssd1306_init();
    ilp_create_thread(&oled_ssd1306_clear, "oled_ssd1306_clear");
    ilp_delay_in_millis(2000);
    ilp_create_thread(&oled_ssd1306_screensaver, "oled_ssd1306_screensaver");
    ilp_delay_in_millis(2000);
#endif

    while(i2c_collision_running == 1)
    {
#if 1
        oled_set_i2c_address(SSD1306_OLED_ADDR1);
        if(is_oled1_init_success == 0)
        {
            ret = oled_display_ssd1306_init();
            if(ret == ESP_OK)
            {
                ret = oled_ssd1306_clear(NULL);
                if(ret == ESP_OK)
                    is_oled1_init_success = 1;
                else
                {
                    app_i2c_collision_fail_handler("clear", ret);
                }
            }
            else
            {
                app_i2c_collision_fail_handler("init", ret);
            }
        }
        else
        {
            // oled_write_text("This is an12345\nEmpty\nMessage\n1\n2\n3\n4\n5");
            app_i2c_collision_generate_msg(text_buffer);
            ret = oled_write_text(text_buffer);
            if(ret == ESP_OK)
            {
                success_ctr++;
            }
            else
            {
                app_i2c_collision_fail_handler("write", ret);
            }
        }
        //force delay for 2nd oled
        ilp_delay_in_millis(200);
#endif
#if 0
        oled_set_i2c_address(SSD1306_OLED_ADDR2);
        if(is_oled2_init_success == 0)
        {
            ret = oled_display_ssd1306_init();
            if(ret == ESP_OK)
            {
                ret = oled_ssd1306_clear(NULL);
                if(ret == ESP_OK)
                    is_oled2_init_success = 1;
                else
                {
                    app_i2c_collision_fail_handler("clear", ret);
                }
            }
            else
            {
                app_i2c_collision_fail_handler("init", ret);
            }
        }
        else
        {
            app_i2c_collision_generate_msg(text_buffer);
            ret = oled_write_text(text_buffer);
            if(ret == ESP_OK)
            {
                success_ctr++;
            }
            else
            {
                app_i2c_collision_fail_handler("write", ret);
            }
        }        
        //force delay for watchdog to work
        ilp_delay_in_millis(200);
#endif
    }
    ILP_LOGI(TAG, "Must not reach this line!!!\n");
}