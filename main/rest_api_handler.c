#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include <esp_http_server.h>

#include "log_wrapper.h"
#include "common.h"

const char* TAG = "HTTPD";

void (*get_req_callback)(int) = NULL;

esp_err_t basic_get_handler(httpd_req_t *req)
{
    char sample_response[] = "OK";
    int req_len = 0;
    char* buff_req = NULL;

    char* token = NULL;
    char delim[] = "=&";
    int cmd_value = -1;

    ILP_LOGI(TAG, "GET request !!!\n");

    req_len = httpd_req_get_url_query_len(req);
    req_len++; //add 1 for endline
    if(req_len > 1)
    {
        buff_req = malloc(req_len);
        httpd_req_get_url_query_str(req, buff_req, req_len);
        ILP_LOGI(TAG, "URL: [%s]\n", buff_req);

        token = strtok(buff_req, delim);
        while(token != NULL)
        {
            // ILP_LOGI(TAG, "token: [%s]\n", token);
            if(strcmp(token, "cmd") == 0)
            {
                token = strtok(NULL, delim);
                cmd_value = atoi(token);
                ILP_LOGI(TAG, "CMD: [%s] %d\n", token, cmd_value);
                
                //check first if callback is registered
                if(get_req_callback != NULL)
                {
                    get_req_callback(cmd_value);
                }

                break;
            }
            token = strtok(NULL, delim);
        }
    }

    httpd_resp_send(req, sample_response, strlen(sample_response));

    return ESP_OK;
}

httpd_uri_t uriget = {
    .uri       = "/gettest",
    .method    = HTTP_GET,
    .handler   = basic_get_handler,
    .user_ctx  = NULL
};

/*******************************/

void register_get_handler(void (*get_req_cb)(int))
{
    get_req_callback = get_req_cb;
}

int init_http_server()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_handle_t server = NULL;

    if(httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &uriget);
    }
    else
    {
        ILP_LOGI(TAG, "HTTPD Start Failed\n");
    }


    return 0;
}

