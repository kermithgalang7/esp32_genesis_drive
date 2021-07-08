#ifndef __LOG_WRAPPER_H__
#define __LOG_WRAPPER_H__

#include "esp_log.h"

#define ILP_LOGE(tag, ...)       ESP_LOGE(tag, __VA_ARGS__)
#define ILP_LOGW(tag, ...)       ESP_LOGW(tag, __VA_ARGS__)
#define ILP_LOGI(tag, ...)       ESP_LOGI(tag, __VA_ARGS__)
#define ILP_LOGD(tag, ...)       ESP_LOGD(tag, __VA_ARGS__)
#define ILP_LOGV(tag, ...)       ESP_LOGV(tag, __VA_ARGS__)

    /*
    ESP_LOGE - error (lowest)
    ESP_LOGW - warning
    ESP_LOGI - info
    ESP_LOGD - debug
    ESP_LOGV - verbose (highest)
    */
#endif //__LOG_WRAPPER_H__