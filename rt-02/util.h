#ifndef __UTIL__
#define __UTIL__
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
#include "main.h"

unsigned int atoi(char *str);
const char *get_fullpath(char *fullpath, int size, struct dentry *pd);
const char *get_path(char *fullpath, int size, unsigned int fd);

#endif // __UTIL__

