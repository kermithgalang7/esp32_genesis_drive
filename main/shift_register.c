/*  Shift Register helper
 *  NOTE: this shift register helper needs a new revision
 *      will be independent and modularized, removing fix
 *      header #define that forces to change the file itself
 *      maybe a config is much better before init
 * 
 *  NOTE: maybe its better if this is a new thread instead
 *      of blocking function, especially when sending 
 *      different bit commands
 *
 */

#include <string.h>

#include "gpio_helper.h"
#include "shift_register.h"

#include "log_wrapper.h"
#include "common.h"

static const char* TAG = "ShiftReg";

char* shift_reg_data = NULL;
int max_shift_reg = 0;

/**********************************************/

int ilp_init_shift_register(void)
{
    ILP_LOGI(TAG, "Initializing Shift Register GPIO\n");
#ifdef ILP_SHIFT_REG_HAS_LATCH
    ILP_LOGI(TAG, "with Latch\n");
#endif
    ilp_gpio_config_output(ILP_SHIFT_REG_DAT);
    ilp_gpio_config_output(ILP_SHIFT_REG_CLK);
    ilp_gpio_config_output(ILP_SHIFT_REG_LATCH);
    ilp_gpio_set_low(ILP_SHIFT_REG_DAT);
    ilp_gpio_set_low(ILP_SHIFT_REG_CLK);
    ilp_gpio_set_low(ILP_SHIFT_REG_LATCH);
    
    ilp_set_shift_register_count(ILP_DEFAULT_SHIFTREG_COUNT);

    ilp_set_8bit_value(0xFF);
    ilp_set_8bit_value(0xFF);
    // ilp_set_8bit_value(0x00);
    // ilp_set_8bit_value(0x00);
    // ilp_set_8bit_value(0x00);
    // ilp_set_8bit_value(0x00);
    // ilp_set_bit_value(14, 1);    //For testing only
    // ilp_set_bit_value(1, 0);
    return 0;
}
int ilp_deinit_shift_register(void)
{
    if(max_shift_reg != 0)
    {
        free(shift_reg_data);
        return 0;
    }
    ILP_LOGE(TAG, "Not yet Initialized\n");
    return -1;
}

int ilp_flush_shift_reg(void)
{
    int i;
    int j;

    if(max_shift_reg == 0)
    {
        ILP_LOGE(TAG, "Shift Reg Count not set\n");
        return -1;
    }

    for(j = 0; j < max_shift_reg; j++)
    {
        // ILP_LOGI(TAG, "ShiftReg [0x%x]\n", shift_reg_data[j]);
        for(i = 0; i < 8 ; i++)
        {
            if((shift_reg_data[j] & (1 << i)) > 0 )            
                ilp_gpio_set_high(ILP_SHIFT_REG_DAT);
            else
                ilp_gpio_set_low(ILP_SHIFT_REG_DAT);
            
            ilp_gpio_set_low(ILP_SHIFT_REG_CLK);
            ilp_gpio_set_high(ILP_SHIFT_REG_CLK);
        }
    }

#ifdef ILP_SHIFT_REG_HAS_LATCH
    ilp_gpio_set_low(ILP_SHIFT_REG_LATCH);
    ilp_gpio_set_high(ILP_SHIFT_REG_LATCH);
#endif

    return 0;
}

int ilp_set_shift_register_count(int numberofshiftregister)
{
    if(max_shift_reg == 0)
    {
        shift_reg_data = malloc(sizeof(char) * numberofshiftregister);
        max_shift_reg = numberofshiftregister;
    }
    else
    {
        shift_reg_data = realloc(shift_reg_data, sizeof(char) * numberofshiftregister);
        memset(shift_reg_data, 0, numberofshiftregister);
        max_shift_reg = numberofshiftregister;
    }

    memset(shift_reg_data, 0, numberofshiftregister);

    return 0;
}

int ilp_set_8bit_value(char data8bit)
{
    int i;

    if(max_shift_reg == 0)
    {
        ILP_LOGE(TAG, "Shift Reg Count not set\n");
        return -1;
    }

    for(i = 1; i < max_shift_reg; i++)
    {
        shift_reg_data[i] = shift_reg_data[i-1];
    }
    shift_reg_data[0] = data8bit;

    ilp_flush_shift_reg();
    return 0;
}

int ilp_set_bit_value(int bitcount, int valuehighlow)
{
    char bittarget = 0;

    if(max_shift_reg == 0)
    {
        ILP_LOGE(TAG, "Shift Reg Count not set\n");
        return -1;
    }
    if(bitcount >= (max_shift_reg * 8))
    {
        ILP_LOGE(TAG, "Bit outside of range\n");
        return -1;
    }

    bittarget = bitcount % 8;
    // ILP_LOGI(TAG, "Bit to change %d\n", bittarget);
    // ILP_LOGI(TAG, "Bit to change 0x%x\n", shift_reg_data[bitcount / 8]);
    if(valuehighlow == 1)
        shift_reg_data[bitcount / 8] |= 1 << bittarget;
    else
        shift_reg_data[bitcount / 8] &= ~(1 << bittarget);

    // ILP_LOGI(TAG, "Bit to change 0x%x\n", shift_reg_data[bitcount / 8]);

    ilp_flush_shift_reg();
    return 0;
}
