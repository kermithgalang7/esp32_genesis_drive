/* Wrapper function

    Same purpose as the peripheral but
    renamed or application specific

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

static const char* TAG = "GenesisDriveWrapper";

void genesis_drive_init_peripherals()
{
    ILP_LOGI(TAG, "Genesis Drive Init!!!\n");

    // return -1;
}

int genesis_drive_h_gen_enable()
{
    ilp_set_bit_value(8, 0);
    ilp_set_bit_value(9, 0);
    return -1;
}
int genesis_drive_h_gen_disable()
{
    ilp_set_bit_value(8, 1);
    ilp_set_bit_value(9, 1);
    return -1;
}

int genesis_drive_ice1_enable()
{
    ilp_set_bit_value(12, 0);
    return -1;
}
int genesis_drive_ice1_disable()
{
    ilp_set_bit_value(12, 1);
    return -1;
}
int genesis_drive_ice2_enable()
{
    ilp_set_bit_value(13, 0);
    return -1;
}
int genesis_drive_ice2_disable()
{
    ilp_set_bit_value(13, 1);
    return -1;
}
int genesis_drive_ice3_enable()
{
    ilp_set_bit_value(14, 0);
    return -1;
}
int genesis_drive_ice3_disable()
{
    ilp_set_bit_value(14, 1);
    return -1;
}
int genesis_drive_ice4_enable()
{
    ilp_set_bit_value(15, 0);
    return -1;
}
int genesis_drive_ice4_disable()
{
    ilp_set_bit_value(15, 1);
    return -1;
}
