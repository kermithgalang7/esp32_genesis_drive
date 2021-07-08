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

#include "common.h"
#include "log_wrapper.h"

static const char* TAG = "Common";

void ilp_esp32_specific_init(void)
{
    ILP_LOGI(TAG, "Initializing ESP32\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU cores, WiFi%s%s, ",
            CHIP_NAME,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    printf("silicon revision %d, ", chip_info.revision);
    printf("%dMB %s flash\n", 
      spi_flash_get_chip_size() / (1024 * 1024),
      (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

}

void ilp_delay_in_millis(int millisec)
{
    vTaskDelay(millisec / portTICK_PERIOD_MS);
}

int ilp_random(int maxvalue)
{

  return esp_random() % maxvalue;
}

int ilp_reset_device(void)
{
  ILP_LOGI(TAG, "Resetting ESP32!!!\n");

  esp_restart();
  return 0;
}

int ilp_create_thread(void* function_thread, char* thread_name)
{

    xTaskCreate(function_thread, thread_name, 8192, NULL, 5, NULL);
    return 0;
}

int ilp_delete_thread(void* function_thread)
{


    return 0;
}