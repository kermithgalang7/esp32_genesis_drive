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
#include "uart.h"
#include "dev_sim800.h"

static const char* TAG = "SIM800";

int ilp_sim800_reset_device(void)
{
    ILP_LOGI(TAG, "Restarting SIM800\n");
    ilp_gpio_set_low(ILP_DEV_SIM800_RESET);
    ilp_delay_in_millis(500);
    ilp_gpio_set_high(ILP_DEV_SIM800_RESET);
    ilp_delay_in_millis(1000);
    return 0;
}
int ilp_sim800_reset_enable(void)
{
    ilp_gpio_set_low(ILP_DEV_SIM800_RESET);
    return 0;
}
int ilp_sim800_reset_disable(void)
{
    ilp_gpio_set_high(ILP_DEV_SIM800_RESET);
    return 0;
}

int ilp_init_dev_sim800(void)
{
    char buff[500];

    ILP_LOGI(TAG, "Initializing SIM800\n");

    //initialize gpio for sim800 reset
    ilp_gpio_config_output(ILP_DEV_SIM800_RESET);    
    ilp_sim800_reset_device();

    //initialize uart2
    ilp_init_uart2();

    //configure gsm module to sms data, not udp
    ilp_uart_read_data(ILP_UART2, buff, 500);
    //NEED TO FIX THIS
    if(ilp_send_sim800("AT+CMGF=1", "OK") == 0)
        ILP_LOGI(TAG, "OK received\n");
    ilp_delay_in_millis(100);

    //consume all uart data
    //why 500, hmmm at commands are very small messages
    memset(buff, 0, 500);
    ilp_uart_read_data(ILP_UART2, buff, 500);
    ILP_LOGI(TAG, "init reply [%s]\n", buff);

    return 0;
}

int ilp_send_sim800(char* buff, char* retmsg)
{
    char* gsm_msg_buff;
    int len = strlen(buff) + 10;
    char retbuff[500];
    int i;

    gsm_msg_buff = malloc(len);
    if(gsm_msg_buff == NULL)
    {
        ILP_LOGE(TAG, "Cant Allocate\n");
        return -1;
    }
    memset(gsm_msg_buff, 0, len);
    sprintf(gsm_msg_buff, "%s\r", buff);
    for(i = 0; gsm_msg_buff[i] != 0; i++);
    
    ilp_uart_write_data(ILP_UART2, gsm_msg_buff, i);
    free(gsm_msg_buff);

    ilp_delay_in_millis(500);
    memset(retbuff, 0, 500);
    ilp_uart_read_data(ILP_UART2, retbuff, 500);
    // ILP_LOGI(TAG, "sim800 reply [%s]\n", retbuff);
    if(strstr(retbuff, retmsg) == NULL)        
        return -1;
    else
        return 0;
}

int ilp_read_sim800(char* buff, int len)
{
    ilp_uart_read_data(ILP_UART2, buff, len);
    return 0;
}
int ilp_check_if_new_sms_received(void)
{
    return 0;
}
int ilp_sim800_send_sms(char* number, char* msg, char* retmsg)
{
#if 0
    char* gsm_msg;
    int total_len = strlen(number) + strlen(msg) + 20; //20 buffer for at cmd
    int usable_len = 0;

    gsm_msg = malloc(total_len);
    if(gsm_msg == NULL)
    {
        ILP_LOGE(TAG, "Cant Allocate sms sent\n");
        return -1;
    }
    memset(gsm_msg, 0, total_len);
    sprintf(gsm_msg, "AT+CMGS=\"%s\"\r", number);
    //find \r
    for(usable_len = 0; gsm_msg[usable_len] != '\r'; usable_len++)
    ;
    usable_len++;
    ilp_uart_write_data(ILP_UART2, gsm_msg, usable_len);

    memset(gsm_msg, 0, total_len);
    sprintf(gsm_msg, "%s\26", msg);
    //find \26
    for(usable_len = 0; gsm_msg[usable_len] != '\26'; usable_len++)
    ;
    usable_len++;
    ilp_uart_write_data(ILP_UART2, gsm_msg, usable_len);
    free(gsm_msg);
#endif
#if 1
    char* gsm_msg_buff;
    int len = strlen(number) + strlen(msg) + 20; //20 buffer for at cmd
    char retbuff[500];
    int i;

    if(len > 500)
    {
        ILP_LOGE(TAG, "Message too long\n");
        return -1;
    }

    gsm_msg_buff = malloc(len);
    if(gsm_msg_buff == NULL)
    {
        ILP_LOGE(TAG, "Cant Allocate\n");
        return -1;
    }

    //send at command number first
    memset(gsm_msg_buff, 0, len);
    sprintf(gsm_msg_buff, "AT+CMGS=\"%s\"\r", number);
    for(i = 0; gsm_msg_buff[i] != 0; i++);
    ilp_uart_write_data(ILP_UART2, gsm_msg_buff, i);
    
    //wait for ">" response before sending entire message
    ilp_delay_in_millis(500);
    memset(retbuff, 0, 500);
    ilp_uart_read_data(ILP_UART2, retbuff, 500);
    ILP_LOGI(TAG, "sim800 reply [%s]\n", retbuff);
    if(strstr(retbuff, ">") == NULL)        
    {
        ILP_LOGI(TAG, "No response sending sms\n");
        free(gsm_msg_buff);
        return -1;
    }

    //send entire message
    memset(gsm_msg_buff, 0, len);
    sprintf(gsm_msg_buff, "%s\32", msg);
    for(i = 0; gsm_msg_buff[i] != 0; i++);
    ilp_uart_write_data(ILP_UART2, gsm_msg_buff, i);
    
    //wait for "OK" response if success or fail
    ilp_delay_in_millis(3000);
    memset(retbuff, 0, 500);
    ilp_uart_read_data(ILP_UART2, retbuff, 500);
    ILP_LOGI(TAG, "sim800 reply2 [%s]\n", retbuff);
    if(strstr(retbuff, retmsg) != NULL)
    {
        ILP_LOGI(TAG, "sending sms complete\n");
        free(gsm_msg_buff);
        return 0;        
    }
    if(strstr(retbuff, "ERROR") != NULL)
        ILP_LOGI(TAG, "SMS Returned Error\n");
    else
        ILP_LOGI(TAG, "SMS Sending Failed\n");
    free(gsm_msg_buff);
    return -1;
#endif
}
int ilp_sim800_read_sms(int mailboxnumber, char* fromnumber, char* rcv_msg)
{

    // escape 26 for ctrl+z
    return 0;
}

int ilp_deinit_dev_sim800(void)
{
    return 0;
}