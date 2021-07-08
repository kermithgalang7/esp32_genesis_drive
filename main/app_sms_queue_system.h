#ifndef __APP_SMS_QUEUE_SYSTEM_H__
#define __APP_SMS_QUEUE_SYSTEM_H__

#define KEYPAD_1        '1'
#define KEYPAD_2        '2'
#define KEYPAD_3        '3'
#define KEYPAD_4        '4'
#define KEYPAD_5        '5'
#define KEYPAD_6        '6'
#define KEYPAD_7        '7'
#define KEYPAD_8        '8'
#define KEYPAD_9        '9'
#define KEYPAD_0        '0'
#define KEYPAD_A        'A'
#define KEYPAD_B        'B'
#define KEYPAD_C        'C'
#define KEYPAD_D        'D'
#define KEYPAD_SHARP    '#'
#define KEYPAD_AST      '*'
#define KEYPAD_NONE     0

#define KEYPAD_COIN     '^'
#define KEYPAD_LID      '!'

#define MAX_KEYPAD_DEBOUNCE     1000
#define MAX_DAYS_RUNNING        10

#define LABEL_DEV_DAYS          "remaining_days"
#define LABEL_DEV_HR            "remaining_hours"
#define LABEL_DEV_MIN           "remaining_mins"
#define LABEL_CUR_COUNT         "current_count"

#define ILP_DISP_SPLASH         0
#define ILP_DISP_SECURITY       1
#define ILP_DISP_MAIN           2
#define ILP_DISP_INPUT1         3
#define ILP_DISP_INPUT2         4
#define ILP_DISP_MENU1          5
#define ILP_DISP_MENU2          6
#define ILP_DISP_CLEAR          98
#define ILP_DISP_LOCKDOWN       99

#define ILP_SMS_APP_COIN        22
#define ILP_SMS_APP_BANKLID     22        
// #define ILP_SMS_APP_BUZZER      18
#define ILP_SMS_APP_BUZZER      3                          //RX0 GPIO3
#define ILP_SMS_APP_INC_BUT     23

#define ILP_KEYA                36
#define ILP_KEYB                39
#define ILP_KEYC                34
#define ILP_KEYD                35
#define ILP_KEY1                32
#define ILP_KEY2                33
#define ILP_KEY3                25
#define ILP_KEY4                26

#define LED_SHIFTREGBIT_ENA1    9
#define LED_SHIFTREGBIT_ENA2    8
#define LED_SHIFTREGBIT_ENA3    7
#define LED_SHIFTREGBIT_SEG1    6
#define LED_SHIFTREGBIT_SEG2    5
#define LED_SHIFTREGBIT_SEG3    4
#define LED_SHIFTREGBIT_SEG4    3
#define LED_SHIFTREGBIT_SEG5    2
#define LED_SHIFTREGBIT_SEG6    1
#define LED_SHIFTREGBIT_SEG7    0

void app_sms_queue_system_clock(void* param);
void app_init_sms_queue_system(void);
void app_sms_queue_system(void* param);



#endif //__APP_SMS_QUEUE_SYSTEM_H__