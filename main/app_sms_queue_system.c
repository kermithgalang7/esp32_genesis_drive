/* Application SMS Queue System
 *
 *  SMS queue system is a top box device that will display a counter
 *  for the customer. should they choose to reserve. they need to pay
 *  a coin so that the number is registered
 *  using SIM800L GSM Module to send SMS to registered number and will send an
 *  sms once the current number is close to their target number.   
 * 
 *  NOTE regarding time counter, unit will run starting 0:00 until it reach
 *  max time it can run. when reach, it needs servicing resetting the 
 *  time counter to 0:00
 * 
 *  GPIO23 - coin slot trigger
 *  GPIO22 - coin bank open lid trigger
 *  GPIO21 - 5V buzzer REMOVED !!!
 * 
 *  GPIO19 - shift register clock
 *  GPIO21 - shift register data
 *  GPIO22 - shift register latch
 * 
 *  GPIO5  - SIM800 reset
 *  GPIO17 - TXU2 - SIM800 TX
 *  GPIO16 - RXU2 - SIM800 RX
 * 
 *  GPIO6  - Reserved / cannot use
 *  GPIO7  - Reserved / cannot use
 *  GPIO8  - Reserved / cannot use
 *  GPIO9  - Reserved / cannot use
 *  GPIO10 - Reserved / cannot use
 *  GPIO11 - Reserved / cannot use
 * 
 *  GPIO34 - Input Only / No pulls up/down functions
 *  GPIO35 - Input Only / No pulls up/down functions
 *  GPIO36 - Input Only / No pulls up/down functions
 *  GPIO37 - Input Only / No pulls up/down functions
 *  GPIO38 - Input Only / No pulls up/down functions
 *  GPIO39 - Input Only / No pulls up/down functions
 * 
 *  GPIO2  - LCD_RS             //register select
 *  GPIO15 - LCD_E              //lcd enable
 *  GPIO27 - LCD_DB4
 *  GPIO14 - LCD_DB5
 *  GPIO12 - LCD_DB6
 *  GPIO13 - LCD_DB7
 * 
 *  GPIO36 - 4x4 ROW1
 *  GPIO39 - 4x4 ROW2
 *  GPIO34 - 4x4 ROW3
 *  GPIO35 - 4x4 ROW4
 *  GPIO32 - 4x4 COL1
 *  GPIO33 - 4x4 COL2
 *  GPIO25 - 4x4 COL3
 *  GPIO26 - 4x4 COL4
 * 
 *  GPIOXX - 7 segment
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

#include "log_wrapper.h"
#include "common.h"
#include "wifi.h"
#include "gpio_helper.h"
#include "rest_api_helper.h"
#include "non_volatile_mem.h"
#include "uart.h"
#include "dev_sim800.h"
#include "lcd_display.h"
#include "shift_register.h"

#include "app_sms_queue_system.h"

static const char* TAG = "SMSQueueApp";

int system_ready = NO;
int is_the_coin_bank_opened = NO;
int current_coin_counter = 0;

int coin_insert_cooldown = 0;
int push_increment_debounce = 0;
int keypad_debounce = 0;
int current_keyinput = KEYPAD_NONE;
int current_display = ILP_DISP_CLEAR;
int current_counter = 0;
char current_3digit_display[3] = "000";
int ok_flag = NO;
int flag_sim = NO;
int flag_7seg = 0;
char numberbufferlist[500];
char sms_now_numb[50];
char sms_now_msg[200];
int sms_retry_count = 5;

int gsm_scaler = 0;
int display_scaler = 0;

int device_running_days = 0;
int device_running_hour = 0;
int device_running_mins = 0;
int device_running_sec = 0;

//to delete
int test_counter = 1;

//protoypes
int ilp_set_display(int displayscreen);
int set_current_count(int count);

// #define TEST_STR_LABEL              "sample_test"
#define MAX_CELLNUM_LEN             200
int app_set_numberlist(int callnumber, char* celnumbers)
{
    char buff[500];
    char storedmsg[200];
    char cnumb_label[100];

    memset(cnumb_label, 0, 100);
    sprintf(cnumb_label, "callnumber%d", callnumber);

    memset(buff, 0, 500);
    memset(storedmsg, 0, 200);
    ilp_get_string(cnumb_label, storedmsg, MAX_CELLNUM_LEN);
    if(strlen(storedmsg) > 0)
    {
        sprintf(buff, "%s:%s", storedmsg, celnumbers);
    }
    else
        sprintf(buff, ":%s", celnumbers);
    ilp_set_string(cnumb_label, buff);
    return 0;
}
int app_get_numberlist(int callnumber, char* bufferlist)
{
    char buff[200];
    char cnumb_label[100];

    memset(cnumb_label, 0, 100);
    sprintf(cnumb_label, "callnumber%d", callnumber);

    memset(buff, 0, 200);
    memset(bufferlist, 0, 200);
    ilp_get_string(cnumb_label, buff, MAX_CELLNUM_LEN);
    strncpy(bufferlist, buff, 200);

    return 0;
}
int app_erase_numberlist(int callnumber)
{
    char buff[200];
    char cnumb_label[100];

    memset(cnumb_label, 0, 100);
    sprintf(cnumb_label, "callnumber%d", callnumber);

    memset(buff, 0, 200);
    ilp_set_string(cnumb_label, buff);

    return 0;
}
int app_push_sms_queue(char* newlist)
{
    if(strlen(numberbufferlist) == 0)
    {
        memset(numberbufferlist, 0, 500);
        strcpy(numberbufferlist, newlist);
    }
    else
    {
        ILP_LOGI(TAG, "Number List not Empty\n");
        return -1;
    }

    return 0;
}
int app_send_sms_now(char* smsnumber, char* smsmessage)
{
    memset(sms_now_numb, 0, 50);
    memset(sms_now_msg, 0, 200);

    strcpy(sms_now_numb, smsnumber);
    strcpy(sms_now_msg, smsmessage);
    return 0;
}

void update_current_counter(void)
{
    char buff[50];
    char newlist[500];

    // if(push_increment_debounce == 0)
    {
        current_counter++;
        set_current_count(current_counter);
        // ilp_set_display(ILP_DISP_CLEAR);
        // ilp_set_display(ILP_DISP_MAIN);
        if(current_display == ILP_DISP_MAIN)
        {
            memset(buff, 0, 50);
            sprintf(buff, "%d", current_counter);
            lcd_erase_line(3);
            lcd_write_line(3, buff);
        }
        memset(newlist, 0, 500);
        app_get_numberlist(current_counter, newlist);
        if(strlen(newlist) > 0)
        {
            ILP_LOGI(TAG, "%d NewList [%s]\n", current_counter, newlist);
            app_push_sms_queue(newlist);
            app_erase_numberlist(current_counter);
        }
        else
            ILP_LOGI(TAG, "Empty List\n");
    }
    // else
    //     ILP_LOGI(TAG, "Increment Push Button Cooldown\n");
    
}
int set_current_count(int count)
{
    ilp_set_int(LABEL_CUR_COUNT, count);

    return 0;
}
int get_current_count(void)
{
    int temp_count = 0;
    ilp_get_int(LABEL_CUR_COUNT, &temp_count);

    return temp_count;
}
int get_current_day(void)
{
    int remaining_days = 0;
    ilp_get_int(LABEL_DEV_DAYS, &remaining_days);

    return remaining_days;
}
int set_current_day(int day)
{
    ILP_LOGI(TAG, "System clock setting day %d\n", day);
    ilp_set_int(LABEL_DEV_DAYS, day);
    
    return 0;
}
int get_current_hour(void)
{
    int remaining_hours = 0;
    ilp_get_int(LABEL_DEV_HR, &remaining_hours);

    return remaining_hours;
}
int set_current_hour(int hour)
{
    ILP_LOGI(TAG, "System clock setting hour %d\n", hour);
    ilp_set_int(LABEL_DEV_HR, hour);
    
    return 0;
}
int get_current_min(void)
{
    int remaining_mins = 0;
    ilp_get_int(LABEL_DEV_MIN, &remaining_mins);

    return remaining_mins;
}
int set_current_min(int min)
{
    ILP_LOGI(TAG, "System clock setting mins %d\n", min);
    ilp_set_int(LABEL_DEV_MIN, min);
    
    return 0;
}
// system_clock increments until it reach max running time
void app_sms_queue_system_clock(void* param)
{
    ILP_LOGI(TAG, "System clock running\n");
    device_running_days = get_current_day();
    device_running_hour = get_current_hour();
    device_running_mins = get_current_min();

    if(device_running_days == 0)
        set_current_day(0);

    ILP_LOGI(TAG, "Device life %d days %d hours %d mins\n", 
        MAX_DAYS_RUNNING - device_running_days,
        24 - device_running_hour,
        60 - device_running_mins
        );
    while(1)
    {
        if(device_running_sec < 60)
            device_running_sec++;
        else
        {
            device_running_sec = 0;            
            if(device_running_mins < 60)
                device_running_mins++;
            else
            {
                device_running_mins = 0;
                if(device_running_hour < 24)
                    device_running_hour++;
                else
                {
                    device_running_hour = 0;
                    device_running_days++;
                    ILP_LOGI(TAG, "Storing DAY!\n");
                    set_current_day(device_running_days);
                }                
                ILP_LOGI(TAG, "Storing HOUR!\n");
                set_current_hour(device_running_hour);
            }
            set_current_min(device_running_mins);
        }
        if(device_running_days == MAX_DAYS_RUNNING)
        {
            ilp_set_display(ILP_DISP_LOCKDOWN);
        }
        // ILP_LOGI(TAG, "1 second count!\n");
        ilp_delay_in_millis(1000);
    }
}

//this should erase all stored data except time
void app_erase_all_data(void)
{
    ilp_erase_all_storage();
    set_current_day(device_running_days);
    set_current_hour(device_running_hour);
    set_current_min(device_running_mins);
    current_counter = 0;
}

int ilp_set_display(int displayscreen)
{
    char buff[50];

    memset(buff, 0, 50);
    if(current_display == displayscreen)
    {
        ILP_LOGI(TAG, "Screen is currently same!\n");
        return 0;
    }        

    ILP_LOGI(TAG, "Changing screen %d\n", displayscreen);
    switch(displayscreen)
    {
        case ILP_DISP_SPLASH:
            lcd_erase_line(1);
            lcd_write_line(2, "SMS Queueing System");
            lcd_erase_line(3);
            lcd_erase_line(4);
            current_display = ILP_DISP_SPLASH;
            break;
        case ILP_DISP_SECURITY:
            sprintf(buff, 
                "%2d days and %2d hrs", 
                MAX_DAYS_RUNNING - device_running_days,
                24 - device_running_hour);
            lcd_erase_line(1);
            lcd_write_line(2, "Device will run for ");
            lcd_write_line(3, buff);            
            lcd_erase_line(4);
            current_display = ILP_DISP_SECURITY;
            break;
        case ILP_DISP_MAIN:
            lcd_erase_line(1);
            lcd_erase_line(2);
            lcd_erase_line(3);
            lcd_erase_line(4);
            if(flag_sim == YES)
                sprintf(buff, "SIM OK       Coin:%02d", current_coin_counter);
            else
                sprintf(buff, "SIM None     Coin:%02d", current_coin_counter);
            lcd_write_line(1, buff);
            lcd_write_line(2, "Current Number is");
            sprintf(buff, "%d", current_counter);
            lcd_write_line(3, buff);
            lcd_write_line(4, "A: START #: CANCEL");
            current_display = ILP_DISP_MAIN;
            break;
        case ILP_DISP_INPUT1:
            lcd_erase_line(1);
            lcd_erase_line(2);
            lcd_erase_line(3);
            lcd_write_line(1, "Enter CellNumber");
            lcd_write_line(4, "*: OK    #: CANCEL");
            current_display = ILP_DISP_INPUT1;
            lcd_write_line(2, ":"); //last write for alignment of text
            break;
        case ILP_DISP_INPUT2:
            lcd_erase_line(1);
            lcd_erase_line(2);
            lcd_erase_line(3);
            lcd_write_line(1, "Force Change");
            lcd_write_line(2, "Count Number");
            lcd_write_line(4, "*: OK    #: CANCEL");
            lcd_write_line(3, ":");
            current_display = ILP_DISP_INPUT2;
            break;
        case ILP_DISP_MENU1:
            lcd_erase_line(1);
            lcd_erase_line(2);
            lcd_erase_line(3);
            lcd_write_line(1, "1: Set SMS Queue");
            lcd_write_line(2, "2: Set CurrentNum");            
            lcd_write_line(4, "         #: CANCEL");
            current_display = ILP_DISP_MENU1;
            break;
        case ILP_DISP_MENU2:
            lcd_erase_line(1);
            lcd_erase_line(2);
            lcd_erase_line(3);
            lcd_write_line(1, "1: ERASE ALL");
            lcd_write_line(2, "2: ERASE ONE");
            lcd_write_line(4, "         #: CANCEL");
            current_display = ILP_DISP_MENU2;
            break;
        case ILP_DISP_CLEAR:
            lcd_erase_line(1);
            lcd_erase_line(2);
            lcd_erase_line(3);
            lcd_erase_line(4);
            current_display = ILP_DISP_CLEAR;
            break;
        case ILP_DISP_LOCKDOWN:
            lcd_erase_line(1);
            lcd_write_line(2, "Device is");
            lcd_write_line(3, "Locked");
            lcd_erase_line(4);
            current_display = ILP_DISP_LOCKDOWN;
            break;
    }

    return 0;
}

void ilp_display_service(void)
{
    //3 digit 7 segment display
    //this version - large 7segment is disabled
    return;

    switch(flag_7seg)
    {
        case 0:
            ilp_set_bit_value(LED_SHIFTREGBIT_ENA1, 1);
            ilp_set_bit_value(LED_SHIFTREGBIT_ENA2, 0);
            ilp_set_bit_value(LED_SHIFTREGBIT_ENA3, 0);
            ilp_set_bit_value(LED_SHIFTREGBIT_SEG1, 1);
            ilp_set_bit_value(LED_SHIFTREGBIT_SEG2, 0);
            ilp_set_bit_value(LED_SHIFTREGBIT_SEG3, 0);
            ilp_set_bit_value(LED_SHIFTREGBIT_SEG4, 0);
            ilp_set_bit_value(LED_SHIFTREGBIT_SEG5, 0);
            ilp_set_bit_value(LED_SHIFTREGBIT_SEG6, 0);
            ilp_set_bit_value(LED_SHIFTREGBIT_SEG7, 0);
            break;
        case 1:
            ilp_set_bit_value(LED_SHIFTREGBIT_ENA1, 0);
            ilp_set_bit_value(LED_SHIFTREGBIT_ENA2, 1);
            ilp_set_bit_value(LED_SHIFTREGBIT_ENA3, 0);
            ilp_set_bit_value(LED_SHIFTREGBIT_SEG1, 1);
            ilp_set_bit_value(LED_SHIFTREGBIT_SEG2, 0);
            ilp_set_bit_value(LED_SHIFTREGBIT_SEG3, 0);
            ilp_set_bit_value(LED_SHIFTREGBIT_SEG4, 0);
            ilp_set_bit_value(LED_SHIFTREGBIT_SEG5, 0);
            ilp_set_bit_value(LED_SHIFTREGBIT_SEG6, 0);
            ilp_set_bit_value(LED_SHIFTREGBIT_SEG7, 0);
            break;
        case 2:
            ilp_set_bit_value(LED_SHIFTREGBIT_ENA1, 0);
            ilp_set_bit_value(LED_SHIFTREGBIT_ENA2, 0);
            ilp_set_bit_value(LED_SHIFTREGBIT_ENA3, 1);
            ilp_set_bit_value(LED_SHIFTREGBIT_SEG1, 1);
            ilp_set_bit_value(LED_SHIFTREGBIT_SEG2, 0);
            ilp_set_bit_value(LED_SHIFTREGBIT_SEG3, 0);
            ilp_set_bit_value(LED_SHIFTREGBIT_SEG4, 0);
            ilp_set_bit_value(LED_SHIFTREGBIT_SEG5, 0);
            ilp_set_bit_value(LED_SHIFTREGBIT_SEG6, 0);
            ilp_set_bit_value(LED_SHIFTREGBIT_SEG7, 0);
            break;
    }

    if(flag_7seg <= 3)
        flag_7seg++;
    else
        flag_7seg = 0;
}
void ilp_input_service(void)
{
    int i;

    if(keypad_debounce > 0)
    {
        keypad_debounce--;
        return;
    }

    if(current_keyinput != KEYPAD_NONE)
        return;

    // if(ilp_gpio_read(ILP_SMS_APP_COIN) == ILP_GPIO_LOW)
    // {
    //     ILP_LOGI(TAG, "Coin Accepted\n");
    //     current_keyinput = KEYPAD_COIN;
    // }
    // if(ilp_gpio_read(ILP_SMS_APP_BANKLID) == ILP_GPIO_LOW)
    // {
    //     ILP_LOGI(TAG, "Coin Lid is opened, reset it now\n");
    //     current_keyinput = KEYPAD_LID;
    // }

    for(i = 0; i < 4; i++)
    {
        ilp_gpio_set_high(ILP_KEY1);
        ilp_gpio_set_high(ILP_KEY2);
        ilp_gpio_set_high(ILP_KEY3);
        ilp_gpio_set_high(ILP_KEY4);
        if(i == 0) 
        {
            ilp_gpio_set_low(ILP_KEY1);

            if(ilp_gpio_read(ILP_KEYA) == ILP_GPIO_LOW)
                current_keyinput = KEYPAD_D;
            if(ilp_gpio_read(ILP_KEYB) == ILP_GPIO_LOW)
                current_keyinput = KEYPAD_SHARP;
            if(ilp_gpio_read(ILP_KEYC) == ILP_GPIO_LOW)
                current_keyinput = KEYPAD_0;
            if(ilp_gpio_read(ILP_KEYD) == ILP_GPIO_LOW)
                current_keyinput = KEYPAD_AST;
        }
        if(i == 1) 
        {
            ilp_gpio_set_low(ILP_KEY2);

            if(ilp_gpio_read(ILP_KEYA) == ILP_GPIO_LOW)
                current_keyinput = KEYPAD_C;
            if(ilp_gpio_read(ILP_KEYB) == ILP_GPIO_LOW)
                current_keyinput = KEYPAD_9;
            if(ilp_gpio_read(ILP_KEYC) == ILP_GPIO_LOW)
                current_keyinput = KEYPAD_8;
            if(ilp_gpio_read(ILP_KEYD) == ILP_GPIO_LOW)
                current_keyinput = KEYPAD_7;
        }
        if(i == 2) 
        {
            ilp_gpio_set_low(ILP_KEY3);

            if(ilp_gpio_read(ILP_KEYA) == ILP_GPIO_LOW)
                current_keyinput = KEYPAD_B;            
            if(ilp_gpio_read(ILP_KEYB) == ILP_GPIO_LOW)
                current_keyinput = KEYPAD_6;
            if(ilp_gpio_read(ILP_KEYC) == ILP_GPIO_LOW)
                current_keyinput = KEYPAD_5;
            if(ilp_gpio_read(ILP_KEYD) == ILP_GPIO_LOW)
                current_keyinput = KEYPAD_4;

        }
        if(i == 3) 
        {
            ilp_gpio_set_low(ILP_KEY4);

            if(ilp_gpio_read(ILP_KEYA) == ILP_GPIO_LOW)
                current_keyinput = KEYPAD_A;
            if(ilp_gpio_read(ILP_KEYB) == ILP_GPIO_LOW)
                current_keyinput = KEYPAD_3;
            if(ilp_gpio_read(ILP_KEYC) == ILP_GPIO_LOW)
                current_keyinput = KEYPAD_2;
            if(ilp_gpio_read(ILP_KEYD) == ILP_GPIO_LOW)
                current_keyinput = KEYPAD_1;
        }
    }

    if(current_keyinput != KEYPAD_NONE)
        keypad_debounce = 3;
}
void ilp_gsmmodule_service(void)
{
    // ILP_LOGI(TAG, "Service running\n");
#if 0
    //check if gsm is ready
    if(gsm_scaler == 100)
    {
        ILP_LOGI(TAG, "Sending AT\n");
        if(ilp_send_sim800("AT", "OK") == 0)
            ILP_LOGI(TAG, "OK received\n");       
    }
#endif
#if 0
    if(gsm_scaler == 100)
    {       
        ILP_LOGI(TAG, "Sending SMS Format\n");
        if(ilp_send_sim800("AT+CMGF=1", "OK") == 0)
        {
            ILP_LOGI(TAG, "OK received\n");
            flag_sim = YES; //if success
        }
        else
            flag_sim = NO; //if success

        ILP_LOGI(TAG, "Sending AT\n");
        if(ilp_send_sim800("AT", "OK") == 0)
            ILP_LOGI(TAG, "OK received\n");

        ilp_delay_in_millis(2000);
        ILP_LOGI(TAG, "Sending SMS\n");
        if(ilp_sim800_send_sms("09568001466", "Sample", "OK") == 0)
            ILP_LOGI(TAG, "OK SMS Done\n");

        ilp_delay_in_millis(2000);
        ILP_LOGI(TAG, "Sending SMS\n");
        if(ilp_sim800_send_sms("09152453491", "Gising Na", "OK") == 0)
            ILP_LOGI(TAG, "OK SMS Done\n");
    }
#endif

    if(gsm_scaler < 100)
        gsm_scaler++;
    else
        gsm_scaler = 0;
}
int ilp_wait_for_gsmmodule_to_ready(void)
{
    flag_sim = NO;
    ilp_sim800_reset_device();
    // ilp_init_dev_sim800();

    sms_retry_count++;

    //TODO: debug when missing SMS modem
    // return 0;

    while(flag_sim == NO)
    {
        ilp_delay_in_millis(1000 * sms_retry_count);

        // ILP_LOGI(TAG, "Sending AT\n");
        if(ilp_send_sim800("AT", "OK") == 0)
            ILP_LOGI(TAG, "AT OK received\n");

        ILP_LOGI(TAG, "Sending SMS Format\n");
        if(ilp_send_sim800("AT+CMGF=1", "OK") == 0)
        {
            ILP_LOGI(TAG, "AT+CMGF OK received\n");
            flag_sim = YES; //if success
        }
        else
            flag_sim = NO; //if success
    }

    return 0;
}
void ilp_gsmmodule_msg_handler(void* param)
{
    char delim[] = ":";
    char* listptr;
    char tempbuff[200];
    int ret_smssend = -1;

    memset(numberbufferlist, 0, 500);
    memset(sms_now_numb, 0, 50);
    memset(sms_now_msg, 0, 200);

    //need delay for the gsm to prepare call ready and sms ready
    //much better if these are handled instead of delayed
    ilp_delay_in_millis(5000);
    ILP_LOGI(TAG, "GSM msg handler running\n");
    
    while(1)
    {   
        // ILP_LOGI(TAG, "Sending AT\n");
        if(ilp_send_sim800("AT", "OK") == 0)
        {
            // ILP_LOGI(TAG, "AT OK received\n");
            flag_sim = YES; //if success
        }
        else
            flag_sim = NO; //if success

        //check for urgent sms now message
        if(strlen(sms_now_numb) > 0)
        {
            if(ilp_sim800_send_sms(sms_now_numb, sms_now_msg, "OK") == 0)
            {
                ILP_LOGI(TAG, "NOW SMS Sending Done\n");
                memset(sms_now_numb, 0, 50);
                memset(sms_now_msg, 0, 200);
                sms_retry_count = 5;
            }
            else
            {
                ilp_wait_for_gsmmodule_to_ready();
            }
        }

        //check buffer list for number
        if(strlen(numberbufferlist) > 0)
        {
            //parse number
            listptr = strtok(numberbufferlist, delim);
            while(listptr != NULL)
            {
                ILP_LOGI(TAG, "List [%s]\n", listptr);
                //send sms
                memset(tempbuff, 0, 200);
                sprintf(tempbuff, "Current number is %d", current_counter);
                ret_smssend = -1;
                while(ret_smssend == -1)
                {
                    ret_smssend = ilp_sim800_send_sms(listptr, tempbuff, "OK");
                    if(ret_smssend == -1)
                    {
                        ilp_wait_for_gsmmodule_to_ready();
                    }
                }
                sms_retry_count = 5;
                listptr = strtok(NULL, delim);
            }
            
            //erase number and save remaining list
            memset(numberbufferlist, 0, 500);
        }

        ilp_delay_in_millis(500);
    }
}

int ilp_interrupt_handler(int gpio, int level)
{
    char buff[50];

    //should increment something here
    ILP_LOGI(TAG, "Interrupt Handler\n");
    switch(gpio)
    {
        case ILP_SMS_APP_COIN:
            ILP_LOGI(TAG, "Coin Inserted!!! %d\n", coin_insert_cooldown);
            coin_insert_cooldown += 5;
            if(coin_insert_cooldown > 10)
            {
                current_coin_counter++;
                coin_insert_cooldown = 0;
                if(current_display == ILP_DISP_INPUT1)
                {

                }
                else if(current_display == ILP_DISP_MAIN)
                {
                    if(flag_sim == YES)
                        sprintf(buff, "SIM OK       Coin:%02d", current_coin_counter);
                    else
                        sprintf(buff, "SIM None     Coin:%02d", current_coin_counter);
                    lcd_write_line(1, buff);
                }
            }
            break;
        case ILP_SMS_APP_INC_BUT:
            ILP_LOGI(TAG, "Increment Count pressed\n");
            if(push_increment_debounce == 0)
                push_increment_debounce = 125;  //push button debounce
            else
                ILP_LOGI(TAG, "Cooldown\n");
            break;
    }
    return 0;
}

int ilp_consume_input(void)
{
    int temp;

    if(current_keyinput != KEYPAD_NONE)
    {
        temp = current_keyinput;
        current_keyinput = KEYPAD_NONE;
        return temp;
    }
    else
        return KEYPAD_NONE;
}

void ilp_buzzer_on(void)
{
    ilp_gpio_set_high(ILP_SMS_APP_BUZZER);
}
void ilp_buzzer_off(void)
{
    ilp_gpio_set_low(ILP_SMS_APP_BUZZER);
}

void ilp_set_3_digit_value(int value)
{
    // 3 digit value of display
}

/**************************************************************/

int app_sms_init_coin_slot(void)
{
    ILP_LOGI(TAG, "Init Coin trigger GPIO\n");

    //this needs to be revised to isr, 
    // ilp_gpio_config_input(ILP_SMS_APP_COIN);
    // ilp_gpio_config_input(ILP_SMS_APP_BANKLID);

    //for coin insert interrupt
    ilp_gpio_config_interrupt(ILP_SMS_APP_COIN);
    ilp_register_gpio_interrupt(ILP_SMS_APP_COIN);

    //for push button to increment interrupt
    ilp_gpio_config_interrupt(ILP_SMS_APP_INC_BUT);
    ilp_register_gpio_interrupt(ILP_SMS_APP_INC_BUT);

    ilp_register_isr(ilp_interrupt_handler);
    return 0;
}
int app_sms_init_led(void)
{
    // this is for led 7 segment display
    return 0;
}
int app_sms_init_lcd_display(void)
{
    // this is for the 20 x 4 LCD display
    lcd_init();

    return 0;
}
int app_sms_init_buzzer(void)
{
    ILP_LOGI(TAG, "Init Buzzer IO\n");
    ilp_gpio_config_output(ILP_SMS_APP_BUZZER);
    ilp_gpio_set_low(ILP_SMS_APP_BUZZER);
    // ilp_gpio_set_high(ILP_SMS_APP_BUZZER);
    return 0;
}
int app_sms_init_keypad(void)
{
    ILP_LOGI(TAG, "Init Keypad GPIO\n");
    ilp_gpio_config_output(ILP_KEY1);
    ilp_gpio_config_output(ILP_KEY2);
    ilp_gpio_config_output(ILP_KEY3);
    ilp_gpio_config_output(ILP_KEY4);
    ilp_gpio_config_input(ILP_KEYA);
    ilp_gpio_config_input(ILP_KEYB);
    ilp_gpio_config_input(ILP_KEYC);
    ilp_gpio_config_input(ILP_KEYD);

    return 0;
}
int app_sms_init_gsmmodule(void)
{
    // ilp_init_uart2();
    ilp_init_dev_sim800();
    ilp_wait_for_gsmmodule_to_ready();

    return 0;
}

void app_init_sms_queue_system(void)
{
    int result = 0;
    
    ilp_gpio_helper_init();     // required if gpio int is used
    result += ilp_init_shift_register();

    result += app_sms_init_coin_slot();
    result += app_sms_init_led();
    result += app_sms_init_lcd_display();
    result += app_sms_init_buzzer();
    result += app_sms_init_keypad();
    result += app_sms_init_gsmmodule();

    if(result == 0)
        system_ready = YES;
}

void app_sms_queue_system(void* param)
{
    char usercellnumber[20];
    char usernumber [20];
    int flag_cellnum = 0;
    int flag_num = 0;
    int input_ctr = 0;
    int temp_input = KEYPAD_NONE;
    int erase_all_countdown = 0;
    char temp_message[200];

    ilp_delay_in_millis(1000);
    //wait for all device to initialize
    ILP_LOGI(TAG, "SMSQueue Running...\n");
    app_init_sms_queue_system();

    //TODO: for demo only, need to delete
    // set_current_day(2);
    // set_current_hour(4);
    // set_current_count(23);

    current_counter = get_current_count();

    ilp_create_thread(&app_sms_queue_system_clock, "app_sms_clock");
    ilp_create_thread(&ilp_gsmmodule_msg_handler, "ilp_gsmmodule_msg_handler");    
    
    // app_erase_all_data();
    // memset(celllist, 0, 200);    
    // app_get_numberlist(1, celllist);
    // ILP_LOGI(TAG, "Cell List 1 [%s]\n", celllist);

    // memset(celllist, 0, 200);    
    // app_get_numberlist(2, celllist);
    // ILP_LOGI(TAG, "Cell List 2 [%s]\n", celllist);
    // app_set_numberlist(2, "09568001466");
    

    // ilp_set_display(ILP_DISP_SPLASH);
    ilp_set_display(ILP_DISP_MAIN);
    while(1)
    {
        if(system_ready == NO)
        {
            ILP_LOGI(TAG, "System not ready\n");
            ilp_delay_in_millis(1000);
            continue;
        }

        temp_input = ilp_consume_input();
        if(temp_input != KEYPAD_NONE)
            ILP_LOGI(TAG, "Keypressed %c\n", temp_input);

        if(display_scaler == 100)
        {
            if(current_display == ILP_DISP_SECURITY)
            {
                display_scaler = 0;
                ilp_set_display(ILP_DISP_MAIN);
            }
            if(current_display == ILP_DISP_SPLASH)
            {
                display_scaler = 0;
                ilp_set_display(ILP_DISP_SECURITY);
            }
        }
        if(display_scaler == 1000)
            ilp_set_display(ILP_DISP_MAIN);

        if(current_display == ILP_DISP_MAIN)
        {
            display_scaler = 0;
            if(temp_input == KEYPAD_A)
            {
                ilp_set_display(ILP_DISP_MENU1);
            }
            if(temp_input == KEYPAD_B)
            {
                ilp_set_display(ILP_DISP_MENU2);
            }
            if(temp_input == KEYPAD_SHARP)
            {
                ilp_set_display(ILP_DISP_MAIN);
            }
        }
        if(current_display == ILP_DISP_MENU1)
        {
            if(temp_input == KEYPAD_1)
            {
                temp_input = KEYPAD_NONE;
                memset(usercellnumber, 0, 20);
                memset(usernumber, 0, 20);
                flag_cellnum = 1;
                flag_num = 0;
                input_ctr = 0;
                ilp_set_display(ILP_DISP_INPUT1);
            }
            if(temp_input == KEYPAD_2)
            {
                temp_input = KEYPAD_NONE;
                memset(usernumber, 0, 20);
                input_ctr = 0;
                ilp_set_display(ILP_DISP_INPUT2);
            }
            if(temp_input == KEYPAD_B)
            {
                temp_input = KEYPAD_NONE;
                ilp_set_display(ILP_DISP_MENU2);
            }
            if(temp_input == KEYPAD_SHARP)
            {
                ilp_set_display(ILP_DISP_MAIN);
            }
        }
        if(current_display == ILP_DISP_MENU2)
        {
            if(temp_input == KEYPAD_1)
            {
                lcd_erase_line(1);
                lcd_erase_line(2);
                lcd_erase_line(3);
                lcd_write_line(1, "Erasing All Data");
                // lcd_write_line(4, "         #: CANCEL");
                lcd_write_line(2, ":");
                erase_all_countdown = 100;
            }
            if(temp_input == KEYPAD_A)
            {
                erase_all_countdown = 0;
                ilp_set_display(ILP_DISP_MENU1);
            }
            if(temp_input == KEYPAD_SHARP)
            {
                erase_all_countdown = 0;
                ilp_set_display(ILP_DISP_MAIN);
            }
        }
        if(current_display == ILP_DISP_INPUT1)
        {   
            if(temp_input == KEYPAD_AST)
            {
                if(flag_cellnum == 1)
                {
                    lcd_write_line(1, "Enter CountNumber");
                    lcd_write_line(3, ":");
                    flag_cellnum = 0;
                    flag_num = 1;
                    input_ctr = 0;
                }
                else if (flag_num == 1)
                {                    
                    if(current_coin_counter > 0)
                    {
                        //store the info then go back to main
                        ILP_LOGI(TAG, "User number %s\n", usercellnumber);
                        ILP_LOGI(TAG, "Number %d\n", atoi(usernumber));
                        flag_cellnum = 0;
                        flag_num = 0;
                        current_coin_counter--;
                        //set number to list
                        //-1 to previous usernumber
                        if((atoi(usernumber) - current_counter) > 0)
                        {
                            ILP_LOGI(TAG, "Alarming at %d [%s]\n", 
                                atoi(usernumber) - 1,
                                usercellnumber);
                                app_set_numberlist(
                                    atoi(usernumber) - 1, 
                                    usercellnumber);
                        }
                        //somewhere between current and usernumber
                        if((atoi(usernumber) - current_counter) >= 2)
                        {
                            if((atoi(usernumber) - current_counter) > 2)
                            {
                                ILP_LOGI(TAG, "Alarming at %d [%s]\n",
                                    ((atoi(usernumber)-current_counter) / 2) + current_counter,
                                    usercellnumber);
                                app_set_numberlist(
                                    ((atoi(usernumber)-current_counter) / 2) + current_counter, 
                                    "09568001466");
                            }
                            else if((atoi(usernumber) - current_counter) >= 2)
                            {
                                ILP_LOGI(TAG, "Alarming at %d [%s]\n",
                                    atoi(usernumber) - 2,
                                    usercellnumber);
                                app_set_numberlist(
                                    atoi(usernumber) - 2, 
                                    "09568001466");
                            }
                        }
                        //sms now to verify usernumber
                        memset(temp_message, 0 ,200);
                        sprintf(temp_message, 
                            "Register Complete : %s",
                             usernumber);
                        app_send_sms_now(usercellnumber, temp_message);
                        ilp_set_display(ILP_DISP_MAIN);
                    }
                    else
                    {
                        lcd_write_line(4, "   Insert Coin    ");
                        lcd_write_line(4, "*: OK    #: CANCEL");
                    }
                }
                temp_input = KEYPAD_NONE;
            }         
            else if(temp_input == KEYPAD_SHARP)
            {
                ilp_set_display(ILP_DISP_MAIN);
            }
            else
            {
                if(temp_input != KEYPAD_NONE)
                {
                    lcd_write_char(temp_input);
                    if(flag_cellnum == 1)
                        usercellnumber[input_ctr] = temp_input;
                    else if (flag_num == 1)
                        usernumber[input_ctr] = temp_input;
                    input_ctr++;
                    if(input_ctr >= 20)
                    {
                        lcd_write_line(2, "Number Too Long");
                        ilp_set_display(ILP_DISP_MAIN);
                    }
                }
            }

        }
        if(current_display == ILP_DISP_INPUT2)
        {
            if(temp_input == KEYPAD_AST)
            {
                if( (atoi(usernumber) > 0) && (atoi(usernumber) <= 999) )
                {
                    current_counter = atoi(usernumber);
                    set_current_count(current_counter);
                }
                else
                {
                    lcd_write_line(2, "Invalid Number");
                }
                ilp_set_display(ILP_DISP_MAIN);
            }
            else if(temp_input == KEYPAD_SHARP)
            {
                ilp_set_display(ILP_DISP_MAIN);
            }
            else if(temp_input != KEYPAD_NONE)
            {
                lcd_write_char(temp_input);
                usernumber[input_ctr] = temp_input;
                input_ctr++;
                if(input_ctr >= 20)
                {
                    lcd_write_line(2, "Number Too Long");
                    ilp_set_display(ILP_DISP_MAIN);
                }
            }
        }

        ilp_input_service();
        ilp_display_service();
        ilp_gsmmodule_service();
        ilp_delay_in_millis(100);

        if(push_increment_debounce == 125)
            update_current_counter();
        if(push_increment_debounce > 0)
        {
            if((push_increment_debounce % 10) >= 5)
                ilp_buzzer_on();
            else
                ilp_buzzer_off();
            push_increment_debounce--;
        }
        if(push_increment_debounce == 1)
        {
            ILP_LOGI(TAG, "Increment Push Button READY\n"); 
            ilp_buzzer_off();
        }

        if(display_scaler <= 1000)
            display_scaler++;
        else
            display_scaler = 0;

        if(coin_insert_cooldown > 0)
            coin_insert_cooldown--;
        if(coin_insert_cooldown == 1)
            ILP_LOGI(TAG, "Coin Timed Out\n");
        
        if(erase_all_countdown > 0)
        {
            erase_all_countdown--;
            if((erase_all_countdown % 10) == 0)
                lcd_write_char((erase_all_countdown / 10) + '0');
            if(erase_all_countdown == 1)
            {
                ILP_LOGI(TAG, "Erasing All Data NOW!\n");
                app_erase_all_data();
                erase_all_countdown = 0;
                ilp_set_display(ILP_DISP_MAIN);
            }
        }
    }
}
