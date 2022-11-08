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

#include "rest_api_handler.h"
#include "log_wrapper.h"
#include "common.h"

const char* TAG = "HTTPD";

httpd_config_t config = HTTPD_DEFAULT_CONFIG();
httpd_handle_t server = NULL;
//for simplicity of modularization, max of 10 uri handler
httpd_uri_t uriptr[MAX_URI_HANDLER];
struct callbacklist_t callbacklist[MAX_URI_HANDLER];

char is_httpd_server_running = 0;

esp_err_t basic_get_handler(httpd_req_t *req)
{
    char sample_response[] = "OK";
    // char label[50];
    // char value[50];
    char* uri;
    int req_len = 0;
    char* buff_req = NULL;

    char* token = NULL;
    char delim[] = "=&/?";
    // int cmd_value = -1;

    void (*get_req_callback)(char*) = NULL;
    char* tempuri;

    ILP_LOGI(TAG, "GET request !!!\n");

    // ILP_LOGI(TAG, "URI: %s\n", req->uri);
    uri = malloc(strlen(req->uri) + 1);
    memset(uri, 0, strlen(req->uri) + 1);
    strcpy(uri, req->uri);
    token = strtok(uri, delim);
    if(token != NULL)
    {
        ILP_LOGI(TAG, "current uri: %s\n", token);
    }

    for(int i = 0; i < MAX_URI_HANDLER; i++)
    {
        tempuri = callbacklist[i].uri;
        if(tempuri != NULL)
        {
            // ILP_LOGI(TAG, "token %s cblist %s\n", token, tempuri);
            tempuri++;
            if(strcmp(token, tempuri) == 0)
            {
                // ILP_LOGI(TAG, "Found matched URI Callback\n");
                get_req_callback = callbacklist[i].get_req_callback;
                break;
            }
        }
    }
    free(uri);

    req_len = httpd_req_get_url_query_len(req);
    req_len++; //add 1 for endline
    if(req_len > 1)
    {
        buff_req = malloc(req_len);
        httpd_req_get_url_query_str(req, buff_req, req_len);
        // ILP_LOGI(TAG, "URL: [%s]\n", buff_req);
        if(get_req_callback != NULL)
        {
            get_req_callback(buff_req);
        }
        free(buff_req);
    }

    httpd_resp_send(req, sample_response, strlen(sample_response));

    return ESP_OK;
}

// Modularize attempt 1
// register_get_handler(char* url, (void) function pointer (char* , char*))
// when NULL, NULL was passed, this will display the whole screen or
// when specific param was passed, will call with param.
//TODO: needs to modularize this custom uri for GET request
httpd_uri_t uriget = {
    .uri       = "/gettest",
    .method    = HTTP_GET,
    .handler   = basic_get_handler,
    .user_ctx  = NULL
};

/*******************************/
int register_get_handler(char* uri, void (*get_req_cb)(char*))
{
    int i;

    if(is_httpd_server_running == 0)
    {
        ILP_LOGI(TAG, "HTTPD did not start!!!\n");
        return -1;
    }

    for(i = 0; i < MAX_URI_HANDLER; i++)
    {
        if(uriptr[i].uri == NULL)
        {
            // "/gettest" - slash is mandatory
            ILP_LOGI(TAG, "Registering URI handler %s\n", uri);
            uriptr[i].uri = uri;
            uriptr[i].method        = HTTP_GET;
            uriptr[i].handler       = basic_get_handler;
            uriptr[i].user_ctx      = NULL;
            httpd_register_uri_handler(server, &uriptr[i]);

            callbacklist[i].uri = uri;
            callbacklist[i].get_req_callback = get_req_cb;

            return 0;
        }
    }
    ILP_LOGI(TAG, "No more empty space for uri\n");
    return -1;
}

int init_http_server()
{
    int i = 0;

    //uri list cleanup
    for(i = 0; i < MAX_URI_HANDLER; i++)
    {
        uriptr[i].uri = NULL;
        callbacklist[i].uri = NULL;
    }

    //run httpd daemon
    if(httpd_start(&server, &config) == ESP_OK)
    {
        ILP_LOGI(TAG, "HTTPD Started\n");
        is_httpd_server_running = 1;
    }
    else
    {
        ILP_LOGI(TAG, "HTTPD Start Failed\n");
    }

    return 0;
}