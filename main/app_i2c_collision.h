#ifndef __APP_I2C_COLLISION_H__
#define __APP_I2C_COLLISION_H__

#define I2C_COL_LED_STAT_IDLE           0
#define I2C_COL_LED_STAT_I2C_ERR        1
#define I2C_COL_LED_STAT_ERR            2
#define I2C_COL_LED_STAT_UNKNOWN        3

#define ESP32_WROOM_BLUE_LED_GPIO       2

#define MAX_OLED_SCREEN_BUFFER          100
#define MAX_COOLDOWN_MULTIPLIER         1000

void app_i2c_collision(void* param);

#endif  //__APP_I2C_COLLISION_H__