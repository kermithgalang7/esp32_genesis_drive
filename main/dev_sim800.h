#ifndef __DEV_SIM800_H__
#define __DEV_SIM800_H__

#include "uart.h"

//AT COMMANDS
#define ATCMD_SMS_MSG_FORMAT                "AT+CMGF=1"
#define ATCMD_SMS_UDP_FORMAT                "AT+CMGF=0"

#define ILP_DEV_SIM800_RESET        5
#define ILP_DEV_SIM800_UART         ILP_UART2

int ilp_sim800_reset_device(void);
int ilp_sim800_reset_enable(void);
int ilp_sim800_reset_disable(void);

int ilp_init_dev_sim800(void);
int ilp_send_sim800(char* buff, char* retmsg);
int ilp_read_sim800(char* buff, int len);
//will return 0 if none, or mailboxnumber
int ilp_check_if_new_sms_received(void);
int ilp_sim800_send_sms(char* number, char* msg, char* retmsg);
int ilp_sim800_read_sms(int mailboxnumber, char* fromnumber, char* rcv_msg);
int ilp_sim800_delete_sms(int mailboxnumber);
int ilp_deinit_dev_sim800(void);


#endif //__DEV_SIM800_H__