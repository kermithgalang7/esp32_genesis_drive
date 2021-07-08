#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "non_volatile_mem.h"
#include "log_wrapper.h"
#include "common.h"

// #define STORAGE_NAMESPACE           "app_sms_queue_system_nvs_label"
#define STORAGE_NAMESPACE           "app_sms_label"

static const char* TAG = "NVS API";

/* 
 *  NOTE: since we are not protected from power failures during
 *      write operation. its best to write another backup
 *      that way. we can know that the last write is corrupted
 *      and we can restart it right away. a much better recovery 
 *      would be having 3 nvs. if one is corrupted, other storage
 *      can still write the values
 * 
 */

int ilp_set_string(char* label, char* value)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if(err != ESP_OK) 
    {
        ILP_LOGE(TAG, "STR Cannot open NVS\n");
        return -1;
    }
    
    err = nvs_set_str(nvs_handle, label, value);
    if(err != ESP_OK) 
    {
        ILP_LOGE(TAG, "STR Cannot write to NVS\n");
        return -1;
    }

    err = nvs_commit(nvs_handle);
    if(err != ESP_OK) 
    {
        ILP_LOGE(TAG, "STR Cannot commit to NVS\n");
        return -1;
    }

    nvs_close(nvs_handle);
    return 0;
}

int ilp_get_string(char* label, char* buffer, int len)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;  
    size_t size = len;

    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if(err != ESP_OK) 
    {
        ILP_LOGE(TAG, "Cannot open NVS\n");
        return -1;
    }

    //to get the actual size stored
    // err = nvs_get_str(nvs_handle, label, NULL, &size);
    // ILP_LOGI(TAG, "size required %d\n", size);
    // char newbuffer[size+1];
    // err = nvs_get_str(nvs_handle, label, newbuffer, &size);
    
    err = nvs_get_str(nvs_handle, label, buffer, &size);
    switch(err)
    {
        case ESP_OK:
            // ILP_LOGI(TAG, "string value %s\n", buffer);        
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ILP_LOGE(TAG, "Cannot read label %s\n", label);
            return -1;
            break;
        case ESP_ERR_NVS_INVALID_LENGTH :
            ILP_LOGE(TAG, "Wrong len\n");
            return -2;
            break;
        default:
            ILP_LOGE(TAG, "Error\n");
            return -3;
    }

    nvs_close(nvs_handle);
    return 0;
}

int ilp_set_int(char* label, int value)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if(err != ESP_OK) 
    {
        ILP_LOGE(TAG, "Cannot open NVS\n");
        return -1;
    }
    
    err = nvs_set_i32(nvs_handle, label, value);
    if(err != ESP_OK) 
    {
        ILP_LOGE(TAG, "Cannot write to NVS\n");
        return -1;
    }

    err = nvs_commit(nvs_handle);
    if(err != ESP_OK) 
    {
        ILP_LOGE(TAG, "Cannot commit to NVS\n");
        return -1;
    }

    nvs_close(nvs_handle);
    return 0;
}

int ilp_get_int(char* label, int* buffer)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;  
    int temp;

    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if(err != ESP_OK) 
    {
        ILP_LOGE(TAG, "Cannot open NVS\n");
        return -1;
    }
    
    err = nvs_get_i32(nvs_handle, label, &temp);
    if(err != ESP_OK) 
    {
        ILP_LOGE(TAG, "Cannot read label %s\n", label);
        return -1;
    }

    // ILP_LOGI(TAG, "temp value %d\n", temp);
    *buffer = temp;

    nvs_close(nvs_handle);
    return 0;
}

int ilp_erase_all_storage(void)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if(err != ESP_OK) 
    {
        ILP_LOGE(TAG, "Cannot open NVS\n");
        return -1;
    }
    
    nvs_erase_all(nvs_handle);

    err = nvs_commit(nvs_handle);
    if(err != ESP_OK) 
    {
        ILP_LOGE(TAG, "Cannot commit to NVS\n");
        return -1;
    }

    nvs_close(nvs_handle);
    return 0;
}