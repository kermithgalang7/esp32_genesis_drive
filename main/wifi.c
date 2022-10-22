/*  NOTE: this wifi helper needs revision
 *      much better if this can be a message type api
 *      using a new thread to initialize and passing content
 *      or command 
 *
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "common.h"
#include "wifi.h"
#include "log_wrapper.h"

#define SSID        "GlobeAtHome_27646"
#define PW          "D78789C1"

#define AP_SSID     "IOTLaunchPad"
#define AP_PW       "12345678"

static const char* TAG = "WiFi";

int is_wifi_initialized = NO;
int current_wifi_status = WIFI_EVENT_STA_DISCONNECTED;
int current_retry = 0;
/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;
/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static void wifi_event_handler(void* arg,
    esp_event_base_t event_base,
    int32_t event_id, 
    void* event_data)
{
    ip_event_got_ip_t* event;

    current_wifi_status = event_id;
    switch(event_id)
    {
        case WIFI_EVENT_STA_START:
            ILP_LOGI(TAG, "Wifi staconnected\n");
            esp_wifi_connect();
        break;
        case WIFI_EVENT_STA_DISCONNECTED:
            if(current_retry < ILP_WIFI_MAX_RETRY)
            {
                esp_wifi_connect();
                current_retry++;
                ILP_LOGI(TAG, "Wifi Connect FAILED, retrying...\n");
            }
            else
            {
                xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
                ILP_LOGI(TAG, "Wifi STA Disconnected\n");
            }
        break;
        case IP_EVENT_STA_GOT_IP:
            event = (ip_event_got_ip_t*) event_data;
            ILP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
            current_retry = 0;
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        break;
        case WIFI_EVENT_AP_STACONNECTED:
            ILP_LOGI(TAG, "Wifi AP staconnected\n");
        break;
        case WIFI_EVENT_AP_STADISCONNECTED:
            ILP_LOGI(TAG, "Wifi AP stadisconnected\n");    
        break;
        default:
            ILP_LOGI(TAG, "Wifi unknown\n");
    }
}

/*********** below are public functions *******************************/

//same as turnon wifi
int ilp_wifi_enable(void)
{
    return 0;
}
//same as turnoff wifi
int ilp_wifi_disable(void)
{
    return 0;
}

//this should be called first after enabling
void ilp_wifi_init(void)
{
    if(is_wifi_initialized == YES)
    {
        ILP_LOGI(TAG, "Wifi already Initialized\n");
        return;
    }
    ILP_LOGI(TAG, "Wifi helper started\n");
    
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap(); //why is this here???
    esp_netif_create_default_wifi_sta(); //same here    
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, 
        ESP_EVENT_ANY_ID, 
        &wifi_event_handler, 
        NULL)
    );
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, 
        IP_EVENT_STA_GOT_IP, 
        &wifi_event_handler, 
        NULL)
    );

    is_wifi_initialized = YES;
}
void ilp_wifi_deinit(void)
{
    ILP_LOGI(TAG, "Wifi Deinitializing\n");

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, 
        IP_EVENT_STA_GOT_IP, 
        &wifi_event_handler)
    );
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, 
        ESP_EVENT_ANY_ID, 
        &wifi_event_handler)
    );
    vEventGroupDelete(s_wifi_event_group);

    is_wifi_initialized = NO;
    esp_wifi_deinit();
}

int ilp_wifi_config_client(char* ssid, char* pw)
{
    wifi_config_t wifi_config;

    memset(&wifi_config, 0, sizeof(wifi_config_t));
    ILP_LOGI(TAG, "Wifi connecting to AP [%s] PW [%s]\n", ssid, pw);
    memcpy(wifi_config.sta.ssid, ssid, strlen(ssid));
    memcpy(wifi_config.sta.password, pw, strlen(pw));
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );

    ILP_LOGI(TAG, "Wifi connecting ...\n");
    return 0;
}

//TODO: need to check why I need to add bracket to ssid and password
int ilp_wifi_config_ap(char* ap_ssid, char* ap_pw)
{
    wifi_config_t wifi_config;
    
    memset(&wifi_config, 0, sizeof(wifi_config_t));
    ILP_LOGI(TAG, "Wifi configuring to AP [%s] PW [%s]\n", ap_ssid, ap_pw);

#if 1
    // wifi_config.ap
    memcpy(wifi_config.ap.ssid, ap_ssid, strlen(ap_ssid));
    wifi_config.ap.ssid_len = strlen(ap_ssid);
    memcpy(wifi_config.ap.password, ap_pw, strlen(ap_pw));
    //wifi_config.ap.max_connection = 1;
    wifi_config.ap.authmode  = WIFI_AUTH_WPA_WPA2_PSK;
#endif
    if(strlen(ap_pw) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));    

    ILP_LOGI(TAG, "Wifi configuring AP DONE!\n");
    return 0;
}

int ilp_get_wifi_status(void)
{
    return current_wifi_status;
}

int ilp_wifi_connect(void)
{
    ILP_LOGI(TAG, "Wifi starting...\n");
    ESP_ERROR_CHECK(esp_wifi_start());

    return 0;
}
int ilp_wifi_disconnect(void)
{
    ESP_ERROR_CHECK(esp_wifi_stop());

    return 0;
}

int ilp_get_ipchar(char* ipstr)
{
    tcpip_adapter_ip_info_t ipinfo;

    tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ipinfo);
    ipstr[0] = ipinfo.ip.addr & 0xFF;
    ipstr[1] = (ipinfo.ip.addr >> 8) & 0xFF;
    ipstr[2] = (ipinfo.ip.addr >> 16) & 0xFF;
    ipstr[3] = (ipinfo.ip.addr >> 24) & 0xFF;
    // sprintf(test, "ipinfo %x", ipinfo.ip.addr);
    // sprintf(test, "ipinfo %d.%d.%d.%d", 
    //     ipstr[0],
    //     ipstr[1],
    //     ipstr[2],
    //     ipstr[3]
    // );

    return 0;
}
int ilp_get_netmaskstr(char* netmaskstr);