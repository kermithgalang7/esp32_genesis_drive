#ifndef __APP_BLE_FW_H__

#define __APP_BLE_FW_H__

#define WROOM_BLUE_LED_GPIO         2

#define BLE_FW_STATUS_UNKNOWN           0
#define BLE_FW_STATUS_ERROR             1
#define BLE_FW_STATUS_DISABLED          2
#define BLE_FW_STATUS_ENABLED           3
#define BLE_FW_STATUS_IDLE              4
#define BLE_FW_STATUS_SEARCHING         5
#define BLE_FW_STATUS_CONNECTED         6

#define GPIO_LED0                       4
#define GPIO_LED1                       16
#define GPIO_LED2                       17

void app_ble_fw(void* param);
void cmd_from_user_to_esp32(int cmd);
int status_from_esp32_to_user(int param);


#endif //__APP_BLE_FW_H__