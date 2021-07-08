/* Hello World Example

    This is no longger the Hello World example

    Some things to consider
    Wifi - for now client only, for demo purposes
    LAN - is possible
    PWM - we need at least 3
    GPIO - for led status   
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

#define APP_SMS

#ifdef APP_SMS
#include "app_sms_queue_system.h"
#endif

#define WIFI_AP_SSID        "IOTLaunchPad_AP"
#define WIFI_AP_PW          "12345678"

// #define WIFI_SSID           "GlobeAtHome_27646"
// #define WIFI_PW             "D78789C1"
// #define WIFI_SSID           "iPhone"
// #define WIFI_PW             "12345678"
#define WIFI_SSID           "GalangWiFi"
#define WIFI_PW             "11223344"

static const char* TAG = "MainApp";
int ret;

void app_main_old(void)
{
    ILP_LOGI(TAG, "IOTLaunchPad Client Firmware for ESP32\n");

    //multi-thread
    ilp_create_thread(&app_sms_queue_system, "app_sms_queue");    

    ilp_esp32_specific_init();

    ilp_wifi_init();       
#if 0 // AP example
    ilp_wifi_config_ap(WIFI_AP_SSID, WIFI_AP_PW);
    ilp_wifi_connect();
#endif
#if 0 // STA example
    ilp_wifi_config_client(WIFI_SSID, WIFI_PW);
    ilp_wifi_connect();
    //wait for connected status
    while(ilp_get_wifi_status() != IP_EVENT_STA_GOT_IP)
    {
        ILP_LOGI(TAG, "Waiting for Wifi to connect and get IP\n");
        ilp_delay_in_millis(750);
    }
#endif
#if 0   //GET POST REQUEST    
    char* post_test = "1234567890";

    while(1)
    {   
        ILP_LOGI(TAG, "Sending get request\n");        
        ilp_get_request("http://192.168.18.6:3000/test", 3000);
        ilp_post_request(
            "http://192.168.18.6:3000/test", 
            3000,
            post_test,
            strlen(post_test)
        );
        ilp_delay_in_millis(5000);        
    }
#endif
#if 0   //POST STEAM
    char post_test[510];
    memset(post_test, 0, 510);
    for(int i = 0; i < 10; i++)
    {
        memset(post_test + (i*50), 'a' + i, 50);
    }
    ILP_LOGI(TAG, "RAW DATA\n\n[%s]\n\n", post_test);

    while(1)
    {   
        ILP_LOGI(TAG, "Sending POST STREAM\n");
        ilp_multiform_post_request(
            "http://192.168.18.6:3000/test", 
            // "http://192.168.18.6/cgi-bin/python_upload.cgi", 
            // "http://httpbin.org/post", 
            3000,
            post_test,
            500,
            "stream_test.txt"
        );        
        ilp_delay_in_millis(10000);
    }
#endif
#if 0   //component include testing PENDING
    mqtt_helper_init();
#endif
#if 0   //led blink example
    int ctr = 0;
#define GPIO_OUT_TEST_1         18
#define GPIO_OUT_TEST_2         19
#define GPIO_IN_TEST_1          4
#define GPIO_IN_TEST_2          5
    ilp_gpio_config_output(GPIO_OUT_TEST_1);
    ilp_gpio_config_output(GPIO_OUT_TEST_2);
    ilp_gpio_config_input(GPIO_IN_TEST_1);
    ilp_gpio_config_input(GPIO_IN_TEST_2);
    ILP_LOGI(TAG, "Testing GPIO pulse...\n");
    while(1)
    {
        if(ctr < 5)
        {
            ilp_gpio_set_high(GPIO_OUT_TEST_1);
            ilp_gpio_set_low(GPIO_OUT_TEST_2);
        }
        else
        {
            ilp_gpio_set_low(GPIO_OUT_TEST_1);
            ilp_gpio_set_high(GPIO_OUT_TEST_2);
        }

        if(ilp_gpio_read(GPIO_IN_TEST_1) == ILP_GPIO_LOW)
            ILP_LOGI(TAG, "Test_1 is LOW\n");
        if(ilp_gpio_read(GPIO_IN_TEST_2) == ILP_GPIO_LOW)
            ILP_LOGI(TAG, "Test_2 is LOW\n");
    
        vTaskDelay(10 / portTICK_PERIOD_MS);
        if(ctr < 10)
            ctr++;
        else
            ctr = 0;        
    }
#endif
#if 0   //gpio interrupt example
#define GPIO_INT_TEST_COIN      23
    ilp_gpio_config_interrupt(GPIO_INT_TEST_COIN);
    ilp_register_gpio_interrupt(GPIO_INT_TEST_COIN);

    // ilp_register_isr();      //this should be callback when interrupt 
#endif 
#if 0   //led PWM example
#define GPIO_OUT_TEST_1         18
#define GPIO_OUT_TEST_2         19
    ilp_pwm_helper_init();
    ilp_gpio_config_pwm_channel(GPIO_OUT_TEST_1, 0);
    ilp_gpio_config_pwm_channel(GPIO_OUT_TEST_2, 8);
    ilp_gpio_pwm_set_dutycycle(0, 4000);
    ilp_gpio_pwm_set_dutycycle(8, 7000);
#endif
#if 0   //nvs storage example
#define LABEL_INT           "integer_label"
#define LABEL_STR           "string_label"
#define MAX_NVS_STR_LEN     100
    int stored_value = 0;
    char stored_data[100] = "empty";
    ILP_LOGI(TAG, "NVS Storage Testing\n");

    ret = ilp_get_int(LABEL_INT, &stored_value);
    if(ret != 0)
        ILP_LOGI(TAG, "int not stored\n");
    else
    {
        ILP_LOGI(TAG, "stored int = %d\n", stored_value);
        stored_value--;
    }
        
    ret = ilp_get_string(LABEL_STR, stored_data, MAX_NVS_STR_LEN);
    if(ret != 0)
        ILP_LOGI(TAG, "string not stored\n");
    else
        ILP_LOGI(TAG, "stored str = [%s]\n", stored_data);        

    ret = ilp_set_int(LABEL_INT, stored_value);
    sprintf(stored_data, "saved msg %d", stored_value);
    ret = ilp_set_string(LABEL_STR, stored_data);
#endif
#if 0   //uart2 test
    ilp_init_uart2();
    char uart_data[100] = "this is sample";
    char temp[100];
    ILP_LOGI(TAG, "UART2 Test routine\n");
    while(1)
    {
        ilp_uart_write_data(ILP_UART2, uart_data, strlen(uart_data));
        memset(temp, 0, 100);
        ilp_uart_read_data(ILP_UART2, temp, 100);
        ILP_LOGI(TAG, "received uart [%s]\n", temp);

        ilp_delay_in_millis(100);
    }
#endif

    ILP_LOGI(TAG, "Entering endless loop...\n");
    while(1)
    {
        //hold for 1ms
        ilp_delay_in_millis(100);
    }

    //should not reach this line
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}