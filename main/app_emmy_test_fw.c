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

#include "app_emmy_test_fw.h"

static const char* TAG = "EmmyTestFW";

int app_emmy_running = 1;

void app_emmy_main(void* param)
{
    lcd_init();
    ilp_init_shift_register();

    lcd_erase_line(1);
    lcd_erase_line(2);
    lcd_erase_line(3);
    lcd_erase_line(4);
    lcd_write_line(1, "Emmy Test FW");

    ilp_set_bit_value(8, 1);
    ilp_set_bit_value(9, 0);
    ilp_set_bit_value(10, 1);
    ilp_set_bit_value(11, 0);
    ilp_set_bit_value(12, 1);
    ilp_set_bit_value(13, 0);
    ilp_set_bit_value(14, 1);
    ilp_set_bit_value(15, 0);

    ILP_LOGI(TAG, "Emmy Test Firmware App started\n");

    while(app_emmy_running == 1)
    {



        ilp_delay_in_millis(1000);
    }

    ILP_LOGI(TAG, "Must not reach this line!!!\n");
}