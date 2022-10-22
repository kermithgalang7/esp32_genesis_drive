#ifndef __EMMY_TEST_FW_H__
#define __EMMY_TEST_FW_H__

#define STAT_UNKNOWN            -1
#define STAT_IDLE               0
#define STAT_WIFI_WAIT          1
#define STAT_WIFI_DISCON        2
#define STAT_WIFI_ERR           3
#define STAT_TEST_RUNNING       4
#define STAT_TEST_ERROR         5
#define STAT_TEST_HOLD          6
#define STAT_TEST_CONF          7

void app_emmy_main(void* param);


#endif //__EMMY_TEST_FW_H__