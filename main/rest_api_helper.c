#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
// #include "protocol_examples_common.h"
#include "esp_tls.h"

#include "esp_http_client.h"

#include "log_wrapper.h"
#include "common.h"
#include "rest_api_helper.h"

static const char* TAG = "RestApiHelper";
int (*ilp_response_event_handler)(int response_len, char* response) = NULL;

/*  NOTE: something to note on this example
 *  printf("%.*s", evt->data_len, (char*)evt->data);
 *      this means
 *  int precision = 8;
 *  int biggerPrecision = 16;
 *  const char *greetings = "Hello world";
 * 
 *  printf("|%.8s|\n", greetings);
 *  printf("|%.*s|\n", precision , greetings);
 *  printf("|%16s|\n", greetings);
 *  printf("|%*s|\n", biggerPrecision , greetings);
 * 
 * |Hello wo|
 * |Hello wo|
 * |     Hello world|
 * |     Hello world|
 * 
 */ 
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ILP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ILP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ILP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ILP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ILP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // Write out data
                // printf("%.*s", evt->data_len, (char*)evt->data);
            }   // see above comments!!!
            ILP_LOGI(TAG, "HTTP_EVENT_ON_DATA, [%.*s]\n", 
                evt->data_len, 
                (char*)evt->data
            );
            if(ilp_response_event_handler != NULL)
            {
                ilp_response_event_handler(evt->data_len, (char*) evt->data);
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ILP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ILP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                ILP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ILP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            break;
    }
    return ESP_OK;
}

void ilp_register_event_handler(
    int (*response_callback)(int response_len, char* response)
)
{
    ilp_response_event_handler = response_callback;
}

int ilp_get_request(char* url, int port)
{
    esp_http_client_config_t config = {        
        // .url = "http://httpbin.org/get",
        .url = url,
        .event_handler = _http_event_handler,        
        // .port = port,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    //GET
    ILP_LOGI(TAG, "GET Request to %s at port %d\n", url, port);
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ILP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ILP_LOGI(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return 0;
}
int ilp_post_request(
    char* url, 
    int port, 
    char* post_data, 
    int post_data_len
)
{
    esp_http_client_config_t config = {        
        .url = url,
        .event_handler = _http_event_handler,        
        .port = port,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, post_data, post_data_len);

    //POST
    ILP_LOGI(TAG, "POST Request to %s at port %d\n", url, port);
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return 0;
}

int ilp_multiform_post_request(
    char* url, 
    int port, 
    char* post_data, 
    int post_data_len,
    char* filename
)
{
    int response_content_len;
    int read_len;
    esp_err_t err;
    char boundary[MAX_BOUNDARY_LEN] = "----------------------------";
    char condispo[MAX_CONDISPO] = 
        "Content-Disposition: form-data; name=\"file\"; filename=\"\"";
    char contype[] = "Content-Type: text/plain";
    int sendcounter = 0;
    int totallen = 0;
    int templen;
    int toplen = 0;
    int botlen = 0;

    if(post_data_len <= 0)
    {
        ESP_LOGE(TAG, "invalid post data lenght\n");
        return -1;        
    }

    //how many times should I send 
    templen = post_data_len;
    for(sendcounter = 1 ; templen > 0; )
    {
        templen -= MAX_DATA_PER_SEND;
        if(templen < 0)
            templen = 0;
        if(templen > 0)
            sendcounter++;
    }
    ESP_LOGI(TAG, "will send [%d] times\n", sendcounter);

    //compose content disposition filename
    if(strlen(filename) == 0)
    {
        ESP_LOGE(TAG, "invalid filename\n");
        return -1;        
    }
    if((strlen(filename) + strlen(condispo)) >= MAX_CONDISPO)
    {
        ESP_LOGE(TAG, "filename too long\n");
        return -1;
    }
    sprintf(condispo, 
        "Content-Disposition: form-data; name=\"file\"; filename=\"%s\"", 
        filename);
    ESP_LOGI(TAG, "cdispo [%s]\n", condispo);

    char *buffer = malloc(MAX_SEND_BUFFER + 1);
    if(buffer == NULL)
    {
        ESP_LOGE(TAG, "stream post req cannot allocate buffer");
        return -1;
    }

    //compose boundary random numbers
    for(int i = 0; i < MAX_BOUNDARY_RAN_NUMB; i++)
    {
        boundary[28+i] = '0' + ilp_random(10);
    }
    memset(buffer, 0, MAX_SEND_BUFFER);
    sprintf(buffer, "multipart/form-data; boundary=%s", boundary);
    ESP_LOGI(TAG, "boundary [%s]\n", buffer);

    esp_http_client_config_t config = {        
        .url = url,
        .event_handler = _http_event_handler,        
        .port = port,
        .method = HTTP_METHOD_POST,
    };
    //NOTE: when you http_client_init(), dont forget to cleanup()
    esp_http_client_handle_t client = esp_http_client_init(&config);
    ESP_LOGI(TAG, "http_client_init DONE...\n");
    
    esp_http_client_set_header(client, "accept", "*/*");
    esp_http_client_set_header(client, "connection", "keep-alive");
    esp_http_client_set_header(client, "accept-encoding", "gzip, deflate, br");
    esp_http_client_set_header(client, "content-type", buffer);

    ESP_LOGI(TAG, "headers DONE...\n");

    //compute total len if 
    //boundary + condispo + contype + RAWDATA + boundary = total len
    memset(buffer, 0, MAX_SEND_BUFFER);
    sprintf(buffer, 
        "%s\r\n"
        "%s\r\n"
        "%s\r\n\r\n",
        boundary,
        condispo,
        contype
    );
    toplen = strlen(buffer); 
    botlen = strlen(boundary);
    // totallen = (toplen + 2) * sendcounter; // +4 for \r\n\r\n
    totallen = toplen;
    totallen += botlen + 2 + 4; // +2 for -- // +4 for \r\n\r\n    
    totallen += post_data_len;
    ESP_LOGI(TAG, "toplen [%d]\n", toplen);
    ESP_LOGI(TAG, "botlen [%d]\n", botlen);
    ESP_LOGI(TAG, "total len [%d]\n", totallen);

    if ((err = esp_http_client_open(client, totallen)) != ESP_OK)     
    {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        // esp_http_client_close(client);
        esp_http_client_cleanup(client);
        free(buffer);
        return 0;
    }

    //body message assembly
    for(templen = post_data_len; templen > 0;)
    {
        if((templen - MAX_DATA_PER_SEND) > 0)
        {
            memcpy(buffer + toplen, 
                post_data + post_data_len - templen, 
                MAX_DATA_PER_SEND
            );
            // sprintf(buffer + toplen + MAX_DATA_PER_SEND, "\r\n");            
            ESP_LOGI(TAG, "buffer [%s]\n", buffer);
            esp_http_client_write(
                client, 
                buffer, 
                // toplen + MAX_DATA_PER_SEND + 2
                toplen + MAX_DATA_PER_SEND
            );
            templen -= MAX_DATA_PER_SEND;
        }
        else
        {
            #if 0
            memcpy(buffer + toplen, 
                post_data + post_data_len - templen, 
                templen
            );
            sprintf(buffer + toplen + templen, "\r\n%s--\r\n", boundary);
            ESP_LOGI(TAG, "buffer [%s]\n", buffer);
            esp_http_client_write(
                client, 
                buffer, 
                toplen + templen + 2 + botlen + 2
            );
            #else
            memcpy(buffer + toplen, 
                post_data + post_data_len - templen, 
                templen
            );
            sprintf(buffer + toplen + templen, "\r\n%s--\r\n\r\n", boundary);
            ESP_LOGI(TAG, "buffer [%s]\n", buffer + toplen);
            esp_http_client_write(
                client, 
                buffer + toplen, 
                templen + 2 + botlen + 4
            );
            #endif
            templen = 0;
        }
        // ilp_delay_in_millis(200);
    }    

    memset(buffer, 0, MAX_SEND_BUFFER);
    response_content_len = esp_http_client_fetch_headers(client);
    ESP_LOGI(TAG, "response_content_len = %d", response_content_len);
    if(response_content_len > MAX_SEND_BUFFER)
        response_content_len = MAX_SEND_BUFFER;
    read_len = esp_http_client_read(client, buffer, response_content_len);
    buffer[read_len] = 0;
    ESP_LOGI(TAG, "read_len = %d", read_len);

    ESP_LOGI(TAG, "HTTP Stream reader Status = %d, content_length = %d",
        esp_http_client_get_status_code(client),
        esp_http_client_get_content_length(client));

    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    free(buffer);
    return 0;
}

