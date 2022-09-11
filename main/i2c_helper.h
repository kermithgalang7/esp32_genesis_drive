#ifndef __I2C_HELPER_H__
#define __I2C_HELPER_H__

// #define I2C_SDA     33
// #define I2C_SCL     36

// I2C CONFIG
#define I2C_MASTER_SCL_IO    GPIO_NUM_22         /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO    GPIO_NUM_21         /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM    I2C_NUM_1             /*!< I2C port number for master dev */
#define I2C_MASTER_TX_BUF_DISABLE   0           /*!< I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0           /*!< I2C master do not need buffer */
#define I2C_MASTER_FREQ_HZ    100000            /*!< I2C master clock frequency */

#define SSD1306_OLED_ADDR1  0x3C                /*!< slave address for ssd1306 oled display */
#define SSD1306_OLED_ADDR2  0x3D                /*!< slave address for ssd1306 oled display */
#define WRITE_BIT  I2C_MASTER_WRITE             /*!< I2C master write */
#define READ_BIT   I2C_MASTER_READ              /*!< I2C master read */
#define ACK_CHECK_EN   0x1                      /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS  0x0                      /*!< I2C master will not check ack from slave */
#define ACK_VAL    0x0                          /*!< I2C ack value */
#define NACK_VAL   0x1                          /*!< I2C nack value */

void i2c_master_init();

#endif //__I2C_HELPER_H__