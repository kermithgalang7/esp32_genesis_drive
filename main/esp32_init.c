
#include "log_wrapper.h"
#include "common.h"
#include "gpio_helper.h"
#include "rest_api_handler.h"

#include "app_ble_fw.h"
#include "ble_helper.h"

static const char* TAG = "ESP32_INIT";

void ilp_esp32_custom_init(void)
{
    ILP_LOGI(TAG, "ESP32 Custom Init\n");

    //set all gpio to ouput and low
#if 0       //need to test
    ilp_gpio_config_powersave(0);
    ilp_gpio_set_low(0);
#endif
#if 0       //this is for UART0
    ilp_gpio_config_powersave(1);
    ilp_gpio_set_low(1);
#endif
    ilp_gpio_config_powersave(2);
    ilp_gpio_set_low(2);
#if 0       //this is for UART0
    ilp_gpio_config_powersave(3);
    ilp_gpio_set_low(3);
#endif
    ilp_gpio_config_powersave(4);
    ilp_gpio_set_low(4);

    ilp_gpio_config_powersave(5);
    ilp_gpio_set_low(5);
#if 0
    ilp_gpio_config_powersave(6);
    ilp_gpio_set_low(6);

    ilp_gpio_config_powersave(7);
    ilp_gpio_set_low(7);

    ilp_gpio_config_powersave(8);
    ilp_gpio_set_low(8);

    ilp_gpio_config_powersave(9);
    ilp_gpio_set_low(9);

    ilp_gpio_config_powersave(10);
    ilp_gpio_set_low(10);

    ilp_gpio_config_powersave(11);
    ilp_gpio_set_low(11);
#endif
    ilp_gpio_config_powersave(12);
    ilp_gpio_set_low(12);

    ilp_gpio_config_powersave(13);
    ilp_gpio_set_low(13);

    ilp_gpio_config_powersave(14);
    ilp_gpio_set_low(14);

    ilp_gpio_config_powersave(15);
    ilp_gpio_set_low(15);

    ilp_gpio_config_powersave(16);
    ilp_gpio_set_low(16);

    ilp_gpio_config_powersave(17);
    ilp_gpio_set_low(17);

    ilp_gpio_config_powersave(18);
    ilp_gpio_set_low(18);

    ilp_gpio_config_powersave(19);
    ilp_gpio_set_low(19);
#if 0       //NOT VALID GPIO???? Really!!!
    ilp_gpio_config_powersave(20);
    ilp_gpio_set_low(20);
#endif
    ilp_gpio_config_powersave(21);
    ilp_gpio_set_low(21);

    ilp_gpio_config_powersave(22);
    ilp_gpio_set_low(22);

    ilp_gpio_config_powersave(23);
    ilp_gpio_set_low(23);
#if 0       //NOT VALID GPIO???? Really!!!
    ilp_gpio_config_powersave(24);
    ilp_gpio_set_low(24);
#endif
    ilp_gpio_config_powersave(25);
    ilp_gpio_set_low(25);

    ilp_gpio_config_powersave(26);
    ilp_gpio_set_low(26);

    ilp_gpio_config_powersave(27);
    ilp_gpio_set_low(27);
#if 0       //NOT VALID GPIO???? Really!!!
    ilp_gpio_config_powersave(28);
    ilp_gpio_set_low(28);

    ilp_gpio_config_powersave(29);
    ilp_gpio_set_low(29);

    ilp_gpio_config_powersave(30);
    ilp_gpio_set_low(30);

    ilp_gpio_config_powersave(31);
    ilp_gpio_set_low(31);
#endif
    ilp_gpio_config_powersave(32);
    ilp_gpio_set_low(32);

    ilp_gpio_config_powersave(33);
    ilp_gpio_set_low(33);
#if 0       //NOT VALID OUTPUT, INPUT ONLY
    ilp_gpio_config_powersave(34);
    ilp_gpio_set_low(34);

    ilp_gpio_config_powersave(35);
    ilp_gpio_set_low(35);

    ilp_gpio_config_powersave(36);
    ilp_gpio_set_low(36);
#endif
#if 0       //NOT VALID GPIO???? Really!!!
    ilp_gpio_config_powersave(37);
    ilp_gpio_set_low(37);

    ilp_gpio_config_powersave(38);
    ilp_gpio_set_low(38);
#endif
#if 0       //NOT VALID OUTPUT, INPUT ONLY
    ilp_gpio_config_powersave(39);
    ilp_gpio_set_low(39);
#endif
}
