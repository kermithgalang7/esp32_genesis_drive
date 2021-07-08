/* Hello World Example

    This is no longger the Hello World example

    Some things to consider
    Wifi - for now client only, for demo purposes
    LAN - is possible
    PWM - we need at least 3
    GPIO - for led status   
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
#include "shift_register.h"
#include "lcd_display.h"

// #define APP_SMS
#define GENESIS_DRIVE

#ifdef APP_SMS
#include "app_sms_queue_system.h"
#endif
#ifdef GENESIS_DRIVE
#include "app_genesis_drive.h"
#endif

#define WIFI_AP_SSID        "IOTLaunchPad_AP"
#define WIFI_AP_PW          "12345678"

// #define WIFI_SSID           "GlobeAtHome_27646"
// #define WIFI_PW             "D78789C1"
// #define WIFI_SSID           "iPhone"
// #define WIFI_PW             "12345678"
#define WIFI_SSID           "GalangWiFi"
#define WIFI_PW             "11223344"

static const char* TAG = "MainApp";
int ret;

void app_main(void)
{
    ILP_LOGI(TAG, "IOTLaunchPad Base Firmware for ESP32\n");

    //hardware esp32 platform specific init
    //this includes clock, interrupt, rtc, etc
    ilp_esp32_specific_init();

    //esp32 peripheral specific init
    //gpio_init();
    //bluetooth_init();

#if 0       //not sure if we need wifi now!!!
    ilp_wifi_enable();
    ilp_wifi_init();
    ilp_wifi_config_client(WIFI_SSID, WIFI_PW);
    ilp_wifi_connect();
#endif

    ilp_init_shift_register();

    //TODO: 
    //init_adc(); //needs in voltage measurement
    //init_adc(); //needs in current measurement
    //init_pwm(); //needs in hydro_converter() toggle

    lcd_init();

/********************* Application *********************/
    //application thread 
    // ilp_create_thread(&app_sms_queue_system, "app_sms_queue");
    ilp_create_thread(&app_genesis_drive, "app_genesis_drive");

    ILP_LOGI(TAG, "Base Firmware standby\n");
    while(1)
    {
        //hold for 1ms
        ilp_delay_in_millis(1000);
    }

    //should not reach this line
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}