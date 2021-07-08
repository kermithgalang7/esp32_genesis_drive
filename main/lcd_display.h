#ifndef __LCD_DISPLAY_H__
#define __LCD_DISPLAY_H__

#define LCD_RS              4
#define LCD_E               2
#define LCD_DB4             15
#define LCD_DB5             27
#define LCD_DB6             14
#define LCD_DB7             13

#define LCD_SHIFTREGBIT_RS  15
#define LCD_SHIFTREGBIT_E   14
#define LCD_SHIFTREGBIT_DB4 13
#define LCD_SHIFTREGBIT_DB5 12
#define LCD_SHIFTREGBIT_DB6 11
#define LCD_SHIFTREGBIT_DB7 10

#define LCD_CMD_LINE1       0x00
#define LCD_CMD_LINE2       0x40
#define LCD_CMD_LINE3       0x14
#define LCD_CMD_LINE4       0x54

#define LCD_CMD_CLEAR       0x01        //0b0000-0001
#define LCD_CMD_HOME        0x02        //0b0000-0010
#define LCD_CMD_ENTRYMODE   0x06        //0b0000-0110
#define LCD_CMD_DISPOFF     0x08        //0b0000-1000
#define LCD_CMD_DISPON      0x0C        //0b0000-1100
#define LCD_CMD_FRESET      0x30        //0b0011-0000
#define LCD_CMD_FSET4BIT    0x28        //0b0010-1000
#define LCD_CMD_SETCUR      0x80        //0b1000-0000

int lcd_init(void);
int lcd_deinit(void);
int lcd_erase_line(int linetoerase);
int lcd_write_line(int linenumber, char* displaymsg);
int lcd_write_char(char letter);

#endif //__LCD_DISPLAY_H__