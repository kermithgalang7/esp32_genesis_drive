/*
 * UART
 *  uart helper for esp32 sdk
 *
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

#include "driver/uart.h"
#include "driver/gpio.h"

#include "log_wrapper.h"
#include "common.h"
#include "uart.h"

// default init should be 115200 8N1
//uart0 TX-GPIO1 RX-GPIO3
int ilp_init_uart0(void)
{
    return 0;
}
//uart1 TX-GPIO10 RX-GPIO9
int ilp_init_uart1(void)
{
    return 0;
}
//uart1 TX-GPIO17 RX-GPIO16
int ilp_init_uart2(void)
{
    uart_config_t uart_config = {
        .baud_rate      =   115200,
        .data_bits      =   UART_DATA_8_BITS,
        .parity         =   UART_PARITY_DISABLE,
        .stop_bits      =   UART_STOP_BITS_1,
        .flow_ctrl      =   UART_HW_FLOWCTRL_DISABLE,
        .source_clk     =   UART_SCLK_APB,
    };
    uart_driver_install(
        UART_NUM_2,                     //uart num
        ILP_UART_BUFF_SIZE * 2,         //rx buff size
        0,                              //tx buff size
        0,                              //queue size
        NULL,                           //uart_queue
        0                               //interrup alloc flags
    );
    uart_param_config(UART_NUM_2, &uart_config);
    uart_set_pin(
        UART_NUM_2,
        ILP_TX2,
        ILP_RX2,
        ILP_DONTCHANGE,                 //RTS
        ILP_DONTCHANGE                  //CTS
    );

    return 0;
}

int ilp_deinit_uart0(void)
{
    return uart_driver_delete(UART_NUM_0);
}
int ilp_deinit_uart1(void)
{
    return uart_driver_delete(UART_NUM_1);
}
int ilp_deinit_uart2(void)
{
    return uart_driver_delete(UART_NUM_2);
}

int ilp_init_uart(int uartnumber, int baudrate)
{
    return 0;
}
int ilp_uart_config(
    int uartnumber,
    int partitybits,
    int stopbit,
    int flowcontrl
    )
{
    return 0;        
}

int ilp_uart_read_data(int uartnumber, char* buffer, int size)
{
    int temp_size;

    if(size > ILP_UART_BUFF_SIZE)
        temp_size = ILP_UART_BUFF_SIZE;
    else
        temp_size = size;

    return uart_read_bytes(
        uartnumber, 
        (uint8_t*) buffer, 
        temp_size,
        20 / portTICK_RATE_MS);
}

int ilp_uart_write_data(int uartnumber, char* data, int size)
{
    return uart_write_bytes(uartnumber, data, size);
}

int ilp_deinit_uart(int uartnumber)
{
    if(uart_driver_delete(uartnumber) == 1)
        return 0;
    else
        return -1;
}