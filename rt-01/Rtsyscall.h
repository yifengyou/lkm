#ifndef __RT_SYSCALL__
#define __RT_SYSCALL__

#include "Rtonline.h"

extern unsigned long *sys_call_table;
extern unsigned int addr_do_fork;

unsigned long * find_sys_call_table_method_one(void);
unsigned long * find_sys_call_table_method_two(void);
unsigned int find_do_fork(void);

#endif // __RT_SYSCALL__
