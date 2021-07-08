#ifndef __UART_H__
#define __UART_H__

#include "driver/uart.h"
#include "driver/gpio.h"


#define ILP_TX0         GPIO_NUM_1
#define ILP_RX0         GPIO_NUM_3

#define ILP_TX1         GPIO_NUM_10
#define ILP_RX1         GPIO_NUM_9

#define ILP_TX2         GPIO_NUM_17
#define ILP_RX2         GPIO_NUM_16

#define ILP_DONTCHANGE  UART_PIN_NO_CHANGE

#define ILP_UART0       UART_NUM_0
#define ILP_UART1       UART_NUM_1
#define ILP_UART2       UART_NUM_2

#define ILP_UART_BUFF_SIZE      1024

// default init should be 115200 8N1
//uart0 TX-GPIO1 RX-GPIO3
int ilp_init_uart0(void);
//uart1 TX-GPIO10 RX-GPIO9
int ilp_init_uart1(void);
//uart1 TX-GPIO17 RX-GPIO16
int ilp_init_uart2(void);

int ilp_deinit_uart0(void);
int ilp_deinit_uart1(void);
int ilp_deinit_uart2(void);

/*  
 * uartnumber - UART_NUM_0, UART_NUM_1 or UART_NUM_2
 * UART_PIN_NO_CHANGE - espressif default uart pin
 * TODO - not yet working
 */
int ilp_init_uart(int uartnumber, int baudrate); 
int ilp_uart_config(
    int uartnumber,
    int partitybits,
    int stopbit,
    int flowcontrl
    );

int ilp_uart_read_data(int uartnumber, char* buffer, int size);
int ilp_uart_write_data(int uartnumber, char* data, int size);
int ilp_deinit_uart(int uartnumber);

#endif //__UART_H__