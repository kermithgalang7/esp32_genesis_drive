#ifndef __GPIO_HELPER_H__
#define __GPIO_HELPER_H__

#define ILP_GPIO_ERROR          -1
#define ILP_GPIO_LOW            0
#define ILP_GPIO_HIGH           1

#define ILP_PWM_DUTY            (4000)
#define ILP_PWM_FADE_TIME       (3000)
//lets use 8 to 15 as high speed, and 0 to 7 as low speed channel
#define ILP_PWM_MAX_CHANNEL     16
#define ILP_PWM_HS_MAX_CH       8
#define ILP_PWM_LS_MAX_CH       8

void ilp_gpio_helper_init(void);
int ilp_gpio_config_output(int gpio);
int ilp_gpio_config_input(int gpio);
int ilp_gpio_config_interrupt(int gpio);

int ilp_gpio_set_high(int gpio);
int ilp_gpio_set_low(int gpio);

int ilp_gpio_read(int gpio);

//to follow ISR and ISR handler
int ilp_register_isr(int (*gpio_isr_callback)(int gpio, int level));
int ilp_register_gpio_interrupt(int gpio);

// gpio pwm function
void ilp_pwm_helper_init(void);
int ilp_gpio_config_pwm_channel(int gpio, int channel);
int ilp_gpio_pwm_set_dutycycle(int channel, int dutycycle);

#endif //__GPIO_HELPER_H__