#ifndef __NON_VOLATILE_MEM_H__
#define __NON_VOLATILE_MEM_H__


int ilp_set_string(char* label, char* value);
int ilp_get_string(char* label, char* buffer, int len);
int ilp_set_int(char* label, int value);
int ilp_get_int(char* label, int* buffer);
int ilp_erase_all_storage(void);

#endif //__NON_VOLATILE_MEM_H__