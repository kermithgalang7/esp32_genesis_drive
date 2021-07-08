#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef CONFIG_IDF_TARGET_ESP32
#define CHIP_NAME "ESP32"
#endif

#ifdef CONFIG_IDF_TARGET_ESP32S2BETA
#define CHIP_NAME "ESP32-S2 Beta"
#endif

#define SUCCESS             0
#define FAIL                1

#define YES                 1
#define NO                  0

#define LED_TEST_GPIO       2

void ilp_esp32_specific_init(void);

void ilp_delay_in_millis(int millisec);
int ilp_random(int maxvalue);
int ilp_reset_device(void);
int ilp_create_thread(void* function_thread, char* thread_name);
int ilp_delete_thread(void* function_thread);


#endif //__COMMON_H__