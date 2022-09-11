/*
 *  I2C Driver built in or ESP32 WROOM
 *      this is a generic driver
 *      the pins are fixed
 * 
 */

#include "freertos/FreeRTOS.h"

#include "driver/gpio.h"
#include "driver/i2c.h"

#include "log_wrapper.h"
#include "common.h"

#include "i2c_helper.h"

static const char* TAG = "i2c_helper";

void i2c_master_init()
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port,
                        conf.mode,
                        I2C_MASTER_RX_BUF_DISABLE,
                        I2C_MASTER_TX_BUF_DISABLE,
                        0);
}