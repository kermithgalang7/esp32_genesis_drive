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
#include "rest_api_handler.h"

#include "app_emmy_test_fw.h"

static const char* TAG = "EmmyTestFW";

#define LCD_MAX_CHAR    20
#define VER_RELEASE     'a'
#define VER_NUMBER      '1'

int g_status = STAT_UNKNOWN;
int app_emmy_running = 1;
int relaycontrol = 0x0;
char ipaddr[4];
char lcd_line1[LCD_MAX_CHAR];
char lcd_line2[LCD_MAX_CHAR];
char lcd_line3[LCD_MAX_CHAR];
char lcd_line4[LCD_MAX_CHAR];

#define RELAY_HIGH      1
#define RELAY_LOW       0

//control below this point
#define TEST_MODE


void emmy_relay_control_service(void* param)
{
    while(1)
    {
        if((relaycontrol & (1 << 0)) != 0) ilp_set_bit_value(8, 0);
        else ilp_set_bit_value(8, 1);
        
        if((relaycontrol & (1 << 1)) != 0) ilp_set_bit_value(9, 0);
        else ilp_set_bit_value(9, 1);

        if((relaycontrol & (1 << 2)) != 0) ilp_set_bit_value(10, 0);
        else ilp_set_bit_value(10, 1);

        if((relaycontrol & (1 << 3)) != 0) ilp_set_bit_value(11, 0);
        else ilp_set_bit_value(11, 1);
        
        if((relaycontrol & (1 << 4)) != 0) ilp_set_bit_value(12, 0);
        else ilp_set_bit_value(12, 1);

        if((relaycontrol & (1 << 5)) != 0) ilp_set_bit_value(13, 0);
        else ilp_set_bit_value(13, 1);

        if((relaycontrol & (1 << 6)) != 0) ilp_set_bit_value(14, 0);
        else ilp_set_bit_value(14, 1);

        if((relaycontrol & (1 << 7)) != 0) ilp_set_bit_value(15, 0);
        else ilp_set_bit_value(15, 1);
    
        //to prevent fast switching of relay 
        ilp_delay_in_millis(750);
    }
}

void emmy_lcd_display_service(void* param)
{
    int i;
    int j;
    char fbtemp[LCD_MAX_CHAR];
    char status[15];

    lcd_erase_line(1);
    lcd_erase_line(2);
    lcd_erase_line(3);
    lcd_erase_line(4);

    while(1)
    {
        //construct UI
        sprintf(fbtemp, "Emmy Test FW %c%c", VER_RELEASE, VER_NUMBER);
        lcd_write_line(1, fbtemp);

        memset(fbtemp, ' ', LCD_MAX_CHAR);
        fbtemp[LCD_MAX_CHAR] = 0;
        sprintf(fbtemp, "IP: %3d.%3d.%3d.%3d", 
            ipaddr[0],
            ipaddr[1],
            ipaddr[2],
            ipaddr[3]
        );
        lcd_write_line(2, fbtemp);

        memset(fbtemp, 0, LCD_MAX_CHAR);
        sprintf(fbtemp, "Relay: ");
        j = strlen(fbtemp);
        for(i = 0; i < 8; i++)
        {
            if((relaycontrol & (1 << i)) != 0)
                fbtemp[j+i] = '1';
            else
                fbtemp[j+i] = '0';
            if(i == 3) 
            {
                j++;
                fbtemp[j+i] = ' ';
            }
        }
        lcd_write_line(3, fbtemp);

        memset(fbtemp, ' ', LCD_MAX_CHAR);
        fbtemp[LCD_MAX_CHAR] = 0;
        memset(status, ' ', 15);
        switch(g_status)
        {
            case STAT_UNKNOWN:
                strcpy(status, "Unknown");
            break;
            case STAT_IDLE:
                strcpy(status, "Idle");
            break;
            case STAT_WIFI_WAIT:
                strcpy(status, "Wifi Conected");
            break;
            case STAT_WIFI_DISCON:
                strcpy(status, "Wifi Discon");
            break;
            case STAT_WIFI_ERR:
                strcpy(status, "Wifi Error");
            break;
            case STAT_TEST_RUNNING:
                strcpy(status, "Test Run");
            break;
            case STAT_TEST_ERROR:
                strcpy(status, "Test Error");
            break;
            case STAT_TEST_HOLD:
                strcpy(status, "Test Stop");
            break;
            case STAT_TEST_CONF:
                strcpy(status, "Config");
            break;
            default:
                strcpy(status, "Unknown");
        }
        // sprintf(fbtemp, "Stat:%s", status);
        sprintf(fbtemp, "Stat:");
        strncpy(fbtemp + 5, status, strlen(status));
        lcd_write_line(4, fbtemp);

        //to prevent unecessary write, this will provide centralized
        //frame buffer and fixed refresh rate
        ilp_delay_in_millis(750);
    }
}

void emmy_set_status(int status)
{
    g_status = status;
}

void emmy_set_ipaddr(char ip1, char ip2, char ip3, char ip4)
{
    ipaddr[0] = ip1;
    ipaddr[1] = ip2;
    ipaddr[2] = ip3;
    ipaddr[3] = ip4;
}

void relay1_on(void){ relaycontrol |= (1 << 0); }
void relay1_off(void){ relaycontrol &= ~(1 << 0); }
void relay2_on(void){ relaycontrol |= (1 << 1); }
void relay2_off(void){ relaycontrol &= ~(1 << 1); }
void relay3_on(void){ relaycontrol |= (1 << 2); }
void relay3_off(void){ relaycontrol &= ~(1 << 2); }
void relay4_on(void){ relaycontrol |= (1 << 3); }
void relay4_off(void){ relaycontrol &= ~(1 << 3); }
void relay5_on(void){ relaycontrol |= (1 << 4); }
void relay5_off(void){ relaycontrol &= ~(1 << 4); }
void relay6_on(void){ relaycontrol |= (1 << 5); }
void relay6_off(void){ relaycontrol &= ~(1 << 5); }
void relay7_on(void){ relaycontrol |= (1 << 6); }
void relay7_off(void){ relaycontrol &= ~(1 << 6); }
void relay8_on(void){ relaycontrol |= (1 << 7); }
void relay8_off(void){ relaycontrol &= ~(1 << 7); }

void emmy_callback_http_get_request(int data)
{
    ILP_LOGI(TAG, "Callback executed with %d\n", data);
}

void app_emmy_main(void* param)
{
    char temp_ip[4];

    lcd_init();
    ilp_init_shift_register();

    ILP_LOGI(TAG, "Emmy Test Firmware App started\n");

    ilp_create_thread(&emmy_relay_control_service, 
        "emmy_relay_control_service");
    ilp_create_thread(&emmy_lcd_display_service, 
        "emmy_lcd_display_service");

    emmy_set_status(STAT_IDLE);
    //hold so that hardware starts first before application
    ilp_delay_in_millis(1000);

#if 1   //currently off due to power not stable
    ilp_wifi_enable();
    ilp_wifi_init();
    ilp_wifi_config_client("GalangWiFi", "11223344");
    ilp_wifi_connect();

    register_get_handler(&emmy_callback_http_get_request);
    init_http_server();
#endif

    while(app_emmy_running == 1)
    {
        // g_status = STAT_IDLE;
        // if(ilp_random(200) > 175)
        //     g_status = ilp_random(8) - 1;

        // relaycontrol = 0xF0;
        if(ilp_random(200) > 175)
            relaycontrol = ilp_random(255);

        // emmy_set_ipaddr(192, 168, 1, 8);
        if(ilp_get_wifi_status() == IP_EVENT_STA_GOT_IP)
        {
            ilp_get_ipchar(temp_ip);
            emmy_set_ipaddr(
                temp_ip[0],temp_ip[1],
                temp_ip[2], temp_ip[3]
            );
            emmy_set_status(STAT_WIFI_WAIT);
        }

        ilp_delay_in_millis(1000);
    }

    ILP_LOGI(TAG, "Must not reach this line!!!\n");
}