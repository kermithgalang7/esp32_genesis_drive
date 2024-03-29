/* Genesis Drive Firmware 

    Base Firmware for driving the Genesis Drive
    Will be used for Hydrogen Converter

    GPIO for HV combustion
    GPIO for Converter Power
    PWM for Converter Power --reconsider
    GPIO for LCD Display
    Wifi --?? lack fo gpio input 
    GPIO for LED --reconsider
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
#include "uart.h"
#include "lcd_display.h"

#include  "app_genesis_drive.h"
#include "app_genesis_drive_wrapper.h"

static const char* TAG = "GenesisDrive";

int app_running = 1;

int h_gen_high_multiplier = 5;
int h_gen_low_multiplier = 5;
int ice_multiplier[MAX_ICE_COUNT] = {20, 20, 20, 20};
int ice_multiplier_index = 0;

void genesis_drive_h_genertor_thread(void* param)
{
    ILP_LOGI(TAG, "Hydrogen generator thread started\n");

    while(app_running == 1)
    {
        genesis_drive_h_gen_enable();
        // ilp_delay_in_millis(250 * h_gen_high_multiplier);
        ilp_delay_in_millis(1000 * 60);
        genesis_drive_h_gen_disable();
        // ilp_delay_in_millis(250 * h_gen_low_multiplier);
        ilp_delay_in_millis(1000 * 10);
    }
    ILP_LOGI(TAG, "Must not reach this line!!!\n");
}

void genesis_drive_ice_thread(void* param)
{
    ILP_LOGI(TAG, "Internal Combustion thread started\n");

    while(app_running == 1)
    {
        //all off
        genesis_drive_ice1_disable();
        genesis_drive_ice2_disable();
        genesis_drive_ice3_disable();
        genesis_drive_ice4_disable();

#if 0
        switch(ice_multiplier_index)
        {
            case 0:
                genesis_drive_ice1_enable();
                break;
            case 1:
                genesis_drive_ice2_enable();
                break;
            case 2:
                genesis_drive_ice3_enable();
                break;
            case 3:
                genesis_drive_ice4_enable();
                break;
        }
#endif 

        ilp_delay_in_millis(10 * ice_multiplier[ice_multiplier_index]);
        if(ice_multiplier_index < MAX_ICE_COUNT)
            ice_multiplier_index++;
        else
            ice_multiplier_index = 0;
    }
    ILP_LOGI(TAG, "Must not reach this line!!!\n");
}

void app_genesis_drive(void* param)
{
    //NOTE: most hardware are already initialized in main
    //components_init();
    genesis_drive_init_peripherals();

    ilp_create_thread(&genesis_drive_h_genertor_thread, "genesis_drive_h_genertor_thread");
    ilp_create_thread(&genesis_drive_ice_thread, "genesis_drive_ice_thread");

    lcd_erase_line(1);
    lcd_erase_line(2);
    lcd_erase_line(3);
    lcd_erase_line(4);
    lcd_write_line(1, "Genesis Drive System");

    while(app_running == 1)
    {

        ilp_delay_in_millis(1000);
    }
    ILP_LOGI(TAG, "Must not reach this line!!!\n");
}