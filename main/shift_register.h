#ifndef __SHIFT_REGISTER_H__
#define __SHIFT_REGISTER_H__


#define ILP_SHIFT_REG_HAS_LATCH     1

#define ILP_DEFAULT_SHIFTREG_COUNT  2

#define ILP_SHIFT_REG_CLK           21
#define ILP_SHIFT_REG_DAT           23
#ifdef ILP_SHIFT_REG_HAS_LATCH
#define ILP_SHIFT_REG_LATCH         22
#endif

int ilp_init_shift_register(void);
int ilp_deinit_shift_register(void);

int ilp_flush_shift_reg(void);

int ilp_set_shift_register_count(int numberofshiftregister);
int ilp_set_8bit_value(char data8bit);
int ilp_set_bit_value(int bitcount, int valuehighlow);

#endif //__SHIFT_REGISTER_H__