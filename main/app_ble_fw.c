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
#include "gpio_helper.h"
#include "rest_api_handler.h"

#include "app_ble_fw.h"
#include "ble_helper.h"

static const char* TAG = "BLE_FW";

int app_running = 1;
int current_ble_fw_status = BLE_FW_STATUS_UNKNOWN;

void app_init_ble(void)
{
    int ret;

    ret = ble_init();
    if(ret)
    {
        ILP_LOGI(TAG, "BLE Failed 0x%X\n", ret);
        current_ble_fw_status = BLE_FW_STATUS_ERROR;
        return;
    }

    current_ble_fw_status = BLE_FW_STATUS_ENABLED;
    // current_ble_fw_status = BLE_FW_STATUS_IDLE;
}

void app_ble_led_status(void* param)
{
    while(1)
    {
        switch(current_ble_fw_status)
        {
            case BLE_FW_STATUS_UNKNOWN:
                ilp_gpio_set_low(WROOM_BLUE_LED_GPIO);
                ilp_delay_in_millis(500);
                ilp_gpio_set_high(WROOM_BLUE_LED_GPIO);
                ilp_delay_in_millis(100);
                break;
            case BLE_FW_STATUS_ERROR:
                ilp_gpio_set_low(WROOM_BLUE_LED_GPIO);
                ilp_delay_in_millis(100);
                ilp_gpio_set_high(WROOM_BLUE_LED_GPIO);
                ilp_delay_in_millis(100);
                break;
            case BLE_FW_STATUS_DISABLED:
                ilp_gpio_set_low(WROOM_BLUE_LED_GPIO);
                ilp_delay_in_millis(1000);
                break;
            case BLE_FW_STATUS_ENABLED:
                ilp_gpio_set_high(WROOM_BLUE_LED_GPIO);
                ilp_delay_in_millis(1000);
                break;
            case BLE_FW_STATUS_IDLE:
                ilp_gpio_set_low(WROOM_BLUE_LED_GPIO);
                ilp_delay_in_millis(500);
                ilp_gpio_set_high(WROOM_BLUE_LED_GPIO);
                ilp_delay_in_millis(2000);
                break;
            case BLE_FW_STATUS_SEARCHING:
                ilp_gpio_set_low(WROOM_BLUE_LED_GPIO);
                ilp_delay_in_millis(500);
                ilp_gpio_set_high(WROOM_BLUE_LED_GPIO);
                ilp_delay_in_millis(500);
                break;
            case BLE_FW_STATUS_CONNECTED:
                ilp_gpio_set_low(WROOM_BLUE_LED_GPIO);
                ilp_delay_in_millis(1000);
                ilp_gpio_set_high(WROOM_BLUE_LED_GPIO);
                ilp_delay_in_millis(1000);
                break;
            default:
                ilp_gpio_set_low(WROOM_BLUE_LED_GPIO);
                ilp_delay_in_millis(1000);
        }
    }
    ILP_LOGE(TAG, "MUST Not Reach This line\n");
}

void cmd_from_user_to_esp32(char* cmd1, char* param)
{
    int cmd = atoi(cmd1);

    ILP_LOGI(TAG, "CMD 0x%x\n", cmd);
    switch(cmd)
    {
        case 0:
            ilp_gpio_set_high(GPIO_LED0);
            break;
        case 1:
            ilp_gpio_set_high(GPIO_LED1);
            break;
        case 2:
            ilp_gpio_set_high(GPIO_LED2);
            break;
        case 3:
            ilp_gpio_set_low(GPIO_LED0);
            break;
        case 4:
            ilp_gpio_set_low(GPIO_LED1);
            break;
        case 5:
            ilp_gpio_set_low(GPIO_LED2);
            break;
        default:
            ilp_gpio_set_low(GPIO_LED0);
            ilp_gpio_set_low(GPIO_LED1);
            ilp_gpio_set_low(GPIO_LED2);
    }
}

int status_from_esp32_to_user(int param)
{
    int status = 0;

    switch(param)
    {
        case 0:
            status = 0xFF;
            break;
        case 1:
            status = 0xAF;
            break;
        case 2:
            status = 0xBA;
            break;
        case 3:
            status = 0xAD;
            break;
        default:
            ;
    }
    ILP_LOGI(TAG, "Status to send 0x%x\n", status);
    return status;
}

void app_ble_fw(void* param)
{
    //NOTE: most hardware are already initialized in main
    //components_init();

    ilp_gpio_config_output(WROOM_BLUE_LED_GPIO);
    ilp_create_thread(&app_ble_led_status, "app_ble_led_status");

    //for test only, please delete
    ilp_gpio_config_output(GPIO_LED0);
    ilp_gpio_config_output(GPIO_LED1);
    ilp_gpio_config_output(GPIO_LED2);

    app_init_ble();
    ILP_LOGI(TAG, "BLE Firmware running\n");

    register_get_handler("gettest", &cmd_from_user_to_esp32);

    while(app_running == 1)
    {

        ilp_delay_in_millis(1000);

        
        ilp_delay_in_millis(1000);
    }
    ILP_LOGI(TAG, "Must not reach this line!!!\n");
}