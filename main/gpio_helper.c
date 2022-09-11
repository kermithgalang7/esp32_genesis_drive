/* GPIO Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
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

#define ESP_INTR_FLAG_DEFAULT   0

#define ILP_FREQ_HZ             5000
/**
 * NOTES on PWM
 *  we have max of 16 channels, 8 for high speed and 8 for low speed
 * 
 */
/**
 * Brief:
 * This test code shows how to configure gpio and how to use gpio interrupt.
 *
 * GPIO status:
 * GPIO18: output
 * GPIO19: output
 * GPIO4:  input, pulled up, interrupt from rising edge and falling edge
 * GPIO5:  input, pulled up, interrupt from rising edge.
 *
 * Test:
 * Connect GPIO18 with GPIO4
 * Connect GPIO19 with GPIO5
 * Generate pulses on GPIO18/19, that triggers interrupt on GPIO4/5
 *
 */

static const char* TAG = "GPIO Helper";

static xQueueHandle gpio_evt_queue = NULL;
int is_gpio_initialized = NO;
int (*ilp_gpio_isr_callback)(int gpio, int level) = NULL;

int is_gpio_pwm_initialized = NO;
ledc_timer_config_t ledc_timer = {
    .duty_resolution = LEDC_TIMER_13_BIT, 
    .freq_hz = ILP_FREQ_HZ,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .timer_num = LEDC_TIMER_0,
    .clk_cfg = LEDC_AUTO_CLK,
};
ledc_channel_config_t ledc_channel[ILP_PWM_MAX_CHANNEL];

static void IRAM_ATTR ilp_gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void ilp_gpio_task(void* arg)
{
    uint32_t io_num;
    for(;;)
    {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY))
        {
            ILP_LOGI(TAG, "GPIO[%d] intr, val: %d\n",
                io_num, 
                gpio_get_level(io_num)
            );
            if(ilp_gpio_isr_callback == NULL) 
            {
                ILP_LOGI(TAG, "Callback is NULL\n");
            }
            else
            {
                ilp_gpio_isr_callback(io_num, gpio_get_level(io_num));
            }            
        }
    }
}

/****** below are public functions **************/
void ilp_gpio_helper_init(void)
{
    if(is_gpio_initialized == NO)
    {
        gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

        xTaskCreate(ilp_gpio_task, 
            "ilp_gpio_task", 
            2048, 
            NULL, 
            10, 
            NULL
        );

        gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

        is_gpio_initialized = YES;
        ILP_LOGI(TAG, "GPIO Initialized\n");
    }
    else
    {
        ILP_LOGI(TAG, "GPIO Already Initialized\n");
    }
}

int ilp_gpio_config_powersave(int gpio)
{
    gpio_config_t io_conf;

    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL<<gpio);
    io_conf.pull_down_en = 1;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    return 0;
}
int ilp_gpio_config_output(int gpio)
{
    gpio_config_t io_conf;

    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL<<gpio);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    return 0;
}
int ilp_gpio_config_input(int gpio)
{
    gpio_config_t io_conf;

    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL<<gpio);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    return 0;
}
int ilp_gpio_config_interrupt(int gpio)
{
    gpio_config_t io_conf;

    // io_conf.intr_type = GPIO_PIN_INTR_ANYEDGE;
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL<<gpio);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);   

    return 0;
}

int ilp_gpio_set_high(int gpio)
{
    gpio_set_level(gpio, ILP_GPIO_HIGH);
    return 0;
}
int ilp_gpio_set_low(int gpio)
{
    gpio_set_level(gpio, ILP_GPIO_LOW);
    return 0;
}

int ilp_gpio_read(int gpio)
{
    return gpio_get_level(gpio);
}

//to follow ISR and ISR handler
int ilp_register_isr(int (*gpio_isr_callback)(int gpio, int level))
{
    ilp_gpio_isr_callback = gpio_isr_callback;

    return 0;
}
int ilp_register_gpio_interrupt(int gpio)
{
    gpio_isr_handler_add(
        gpio, 
        ilp_gpio_isr_handler, 
        (void*) gpio
    );

    return 0;
}

void ilp_pwm_helper_init(void)
{
    int channel;
    
    if(is_gpio_pwm_initialized == YES)
    {
        ILP_LOGI(TAG, "GPIO PWM Already Initialized\n");
        return;
    }

    ILP_LOGI(TAG, "GPIO PWM Initializing\n");

    //initialize HS timer first
    ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;
    ledc_timer.timer_num = LEDC_TIMER_0;
    ledc_timer_config(&ledc_timer);
    //initialize LS timer
    ledc_timer.speed_mode = LEDC_LOW_SPEED_MODE;
    ledc_timer.timer_num = LEDC_TIMER_1;
    ledc_timer_config(&ledc_timer);

    //initialize all channel
    for(channel = 0; channel < ILP_PWM_HS_MAX_CH; channel++)
    {
            ledc_channel[channel].channel    = LEDC_CHANNEL_0;
            ledc_channel[channel].duty       = 0;
            ledc_channel[channel].gpio_num   = 0;
            ledc_channel[channel].speed_mode = LEDC_HIGH_SPEED_MODE;
            ledc_channel[channel].hpoint     = 0;
            //note timer0 is configured as high speed, check above
            ledc_channel[channel].timer_sel  = LEDC_TIMER_0;
    }
    for(channel = 0; channel < ILP_PWM_LS_MAX_CH; channel++)
    {
            ledc_channel[channel].channel    = LEDC_CHANNEL_0;
            ledc_channel[channel].duty       = 0;
            ledc_channel[channel].gpio_num   = 0;
            ledc_channel[channel].speed_mode = LEDC_LOW_SPEED_MODE;
            ledc_channel[channel].hpoint     = 0;
            //note timer1 is configured as low speed, check above
            ledc_channel[channel].timer_sel  = LEDC_TIMER_1;
    }

    // Initialize fade service.
    ledc_fade_func_install(0);

    is_gpio_pwm_initialized = YES;
    ILP_LOGI(TAG, "GPIO PWM Initializing DONE\n");
}
int ilp_gpio_config_pwm_channel(int gpio, int channel)
{
    //channel 8 to 15 are high speed
    //note that there are 0 to 7 highspeed and 0 to 7 lowspeed
    //enum LEDC_CHANNEL_0=0
    if(channel >= 16)
    {
        ILP_LOGI(TAG, "Invalid Channel, 0~15 only\n");
        return 0;
    }
    if(channel >= 8)
        ledc_channel[channel].channel    = channel - 8;
    else
        ledc_channel[channel].channel    = channel;
    ledc_channel[channel].gpio_num   = gpio;

    ledc_channel_config(&ledc_channel[channel]);

    return 0;
}
int ilp_gpio_pwm_set_dutycycle(int channel, int dutycycle)
{
    ledc_set_duty(ledc_channel[channel].speed_mode, 
        ledc_channel[channel].channel, 
        dutycycle
    );
    ledc_update_duty(ledc_channel[channel].speed_mode, 
        ledc_channel[channel].channel
    );

    return 0;
}
