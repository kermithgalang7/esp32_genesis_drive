#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "driver/ledc.h"

#include "log_wrapper.h"
#include "common.h"
#include "gpio_helper.h"

#include "lcd_display.h"

// #define LCD_USES_SHIFTREG           1
#define LCD_USES_GPIO               1

#ifdef LCD_USES_SHIFTREG
#include "shift_register.h"
#endif

static const char* TAG = "LCD Display";

void lcd_write_4(int data)
{
    int data4bit = 0x0F & data;

#ifdef LCD_USES_GPIO    
    if((data4bit & 0x08) > 0)
        ilp_gpio_set_high(LCD_DB7);
    else
        ilp_gpio_set_low(LCD_DB7);

    if((data4bit & 0x04) > 0)
        ilp_gpio_set_high(LCD_DB6);
    else
        ilp_gpio_set_low(LCD_DB6);

    if((data4bit & 0x02) > 0)
        ilp_gpio_set_high(LCD_DB5);
    else
        ilp_gpio_set_low(LCD_DB5);

    if((data4bit & 0x01) > 0)
        ilp_gpio_set_high(LCD_DB4);
    else
        ilp_gpio_set_low(LCD_DB4);

    ilp_gpio_set_high(LCD_E);
    ilp_delay_in_millis(10);
    ilp_gpio_set_low(LCD_E);
    ilp_delay_in_millis(10);
#endif
#ifdef LCD_USES_SHIFTREG
    if((data4bit & 0x08) > 0)
        ilp_set_bit_value(LCD_SHIFTREGBIT_DB7, 1);
    else
        ilp_set_bit_value(LCD_SHIFTREGBIT_DB7, 0);

    if((data4bit & 0x04) > 0)
        ilp_set_bit_value(LCD_SHIFTREGBIT_DB6, 1);
    else
        ilp_set_bit_value(LCD_SHIFTREGBIT_DB6, 0);

    if((data4bit & 0x02) > 0)
        ilp_set_bit_value(LCD_SHIFTREGBIT_DB5, 1);
    else
        ilp_set_bit_value(LCD_SHIFTREGBIT_DB5, 0);

    if((data4bit & 0x01) > 0)
        ilp_set_bit_value(LCD_SHIFTREGBIT_DB4, 1);
    else
        ilp_set_bit_value(LCD_SHIFTREGBIT_DB4, 0);

    ilp_set_bit_value(LCD_SHIFTREGBIT_E, 1);    
    ilp_delay_in_millis(10);
    ilp_set_bit_value(LCD_SHIFTREGBIT_E, 0);    
    ilp_delay_in_millis(10);
#endif
}

void lcd_write_instruction_4d(int cmd)
{
    int high = cmd >> 4;
    int low = cmd & 0x0F;

#ifdef LCD_USES_GPIO
    ilp_gpio_set_low(LCD_E);
    ilp_gpio_set_low(LCD_RS);
#endif
#ifdef LCD_USES_SHIFTREG
    ilp_set_bit_value(LCD_SHIFTREGBIT_E, 0);    
    ilp_set_bit_value(LCD_SHIFTREGBIT_RS, 0);   
#endif
    lcd_write_4(high);
    lcd_write_4(low);
}

void lcd_write_character_4d(char letter)
{
    int high = letter >> 4;
    int low = letter & 0x0F;

#ifdef LCD_USES_GPIO
    ilp_gpio_set_low(LCD_E);
    ilp_gpio_set_high(LCD_RS);
#endif
#ifdef LCD_USES_SHIFTREG
    ilp_set_bit_value(LCD_SHIFTREGBIT_E, 0);    
    ilp_set_bit_value(LCD_SHIFTREGBIT_RS, 1);   
#endif
    lcd_write_4(high);
    lcd_write_4(low);
}

void lcd_write_string_4d(char* msg)
{
    int i = 0;

    while(msg[i] != 0)
    {
        lcd_write_character_4d(msg[i]);
        i++;
        // ilp_delay_in_millis(1);  //working without this!!!
    }
}

void lcd_init_4d(void)
{
#ifdef LCD_USES_GPIO
    ilp_gpio_set_low(LCD_E);        //lcd enable
    ilp_gpio_set_low(LCD_RS);       //register select
#endif
#ifdef LCD_USES_SHIFTREG
    ilp_set_bit_value(LCD_SHIFTREGBIT_E, 0);    //lcd enable
    ilp_set_bit_value(LCD_SHIFTREGBIT_RS, 0);   //register select
#endif

    ilp_delay_in_millis(100);     //powerup delay ??? maybe no need

    // lcd_write_4(LCD_CMD_FRESET >> 4);
    lcd_write_4(LCD_CMD_FRESET >> 4);       //looks like >> is really needed
    ilp_delay_in_millis(10);
    lcd_write_4(LCD_CMD_FRESET >> 4);
    ilp_delay_in_millis(1);
    lcd_write_4(LCD_CMD_FRESET >> 4);        //as per datasheet !!!
    ilp_delay_in_millis(1);

    //set to 4bit mode
    lcd_write_4(LCD_CMD_FSET4BIT >> 4);
    ilp_delay_in_millis(1);
    lcd_write_instruction_4d(LCD_CMD_FSET4BIT);
    ilp_delay_in_millis(1);

    //initialization routine as per datasheet !!!
    lcd_write_instruction_4d(LCD_CMD_DISPOFF);
    ilp_delay_in_millis(1);
    lcd_write_instruction_4d(LCD_CMD_CLEAR);
    ilp_delay_in_millis(1);
    lcd_write_instruction_4d(LCD_CMD_ENTRYMODE);
    ilp_delay_in_millis(1);

    //turn on lcd
    lcd_write_instruction_4d(LCD_CMD_DISPON);
    ilp_delay_in_millis(1);
}

/********************************************************/

int lcd_init(void)
{
    // char testmsg1[100] = "Hello World";
    // char testmsg2[100] = "LCD Testing";

    ILP_LOGI(TAG, "LCD init\n");

#ifdef LCD_USES_GPIO
    ilp_gpio_config_output(LCD_DB4);
    ilp_gpio_config_output(LCD_DB5);
    ilp_gpio_config_output(LCD_DB6);
    ilp_gpio_config_output(LCD_DB7);
    ilp_gpio_config_output(LCD_E);
    ilp_gpio_config_output(LCD_RS);

    ilp_gpio_set_high(LCD_DB4);
    ilp_gpio_set_high(LCD_DB5);
    ilp_gpio_set_high(LCD_DB6);
    ilp_gpio_set_high(LCD_DB7);
    ilp_gpio_set_low(LCD_E);        //lcd enable
    ilp_gpio_set_low(LCD_RS);       //register select
#endif
#ifdef LCD_USES_SHIFTREG
    ilp_set_bit_value(LCD_SHIFTREGBIT_DB4, 1);
    ilp_set_bit_value(LCD_SHIFTREGBIT_DB5, 1);
    ilp_set_bit_value(LCD_SHIFTREGBIT_DB6, 1);
    ilp_set_bit_value(LCD_SHIFTREGBIT_DB7, 1);
    ilp_set_bit_value(LCD_SHIFTREGBIT_E, 1);
    ilp_set_bit_value(LCD_SHIFTREGBIT_RS, 1);
#endif

    lcd_init_4d();

    // lcd_write_string_4d(testmsg1);
    // lcd_write_instruction_4d(LCD_CMD_SETCUR | LCD_CMD_LINE2);
    // lcd_write_string_4d(testmsg2);

    return 0;
}

int lcd_deinit(void)
{

    return 0;
}

int lcd_erase_line(int linetoerase)
{
    char erasemsg[20] = "                    ";
    erasemsg[20] = 0;
    switch(linetoerase)
    {
        case 1:
            lcd_write_instruction_4d(LCD_CMD_SETCUR | LCD_CMD_LINE1);
        break;
        case 2:
            lcd_write_instruction_4d(LCD_CMD_SETCUR | LCD_CMD_LINE2);
        break;
        case 3:
            lcd_write_instruction_4d(LCD_CMD_SETCUR | LCD_CMD_LINE3);
        break;
        case 4:
            lcd_write_instruction_4d(LCD_CMD_SETCUR | LCD_CMD_LINE4);
        break;
    }
    lcd_write_string_4d(erasemsg);

    return 0;
}

int lcd_write_line(int linenumber, char* displaymsg)
{
    if(strlen(displaymsg) > 20)
        displaymsg[20] = 0;

    switch(linenumber)
    {
        case 1:
            lcd_write_instruction_4d(LCD_CMD_SETCUR | LCD_CMD_LINE1);
        break;
        case 2:
            lcd_write_instruction_4d(LCD_CMD_SETCUR | LCD_CMD_LINE2);
        break;
        case 3:
            lcd_write_instruction_4d(LCD_CMD_SETCUR | LCD_CMD_LINE3);
        break;
        case 4:
            lcd_write_instruction_4d(LCD_CMD_SETCUR | LCD_CMD_LINE4);
        break;
    }
    lcd_write_string_4d(displaymsg);

    return 0;
}

int lcd_write_char(char letter)
{
    lcd_write_character_4d(letter);

    return 0;    
}