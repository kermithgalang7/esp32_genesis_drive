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
#include "rest_api_handler.h"

#include "app_plant_grow.h"

static const char* TAG = "PlantGrowFW";

char app_plant_grow_running = 1;

void plant_grow_callback_http_get_request(char* querystr)
{
    ILP_LOGI(TAG, "Full Query %s\n", querystr);

}

void app_plant_grow_main(void* param)
{
    ilp_wifi_enable();
    ilp_wifi_init();
    ilp_wifi_config_client("GalangWiFi", "11223344");
    ilp_wifi_connect();

    init_http_server();
    // "/gettest" - slash is mandatory
    register_get_handler("/setcfg", &plant_grow_callback_http_get_request);

    //gpio out init
    ilp_gpio_config_output(ESP32_WROOM_BLUE_LED_GPIO);

    //gpio in adc init


    ILP_LOGI(TAG, "Plant Grow FW\n");

    while(app_plant_grow_running == 1)
    {

        ilp_delay_in_millis(1000);
    }
}