/*
* This module need to be compiled under 64-bit platform using GCC.
*/
#include <linux/module.h> /* module_init module_exit */
#include <linux/init.h> /* __init __exit */
#include <linux/syscalls.h> /* sys_close __NR_close __NR_mkdir */
#include <linux/delay.h> /* loops_per_jiffy */
#include <asm/bitops.h> /* set_bit clear_bit */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("shuo.d@outlook.com");
MODULE_DESCRIPTION("This module can hook the sys_mkdir function.");

static unsigned long **sys_call_table;
static asmlinkage long (*original_sys_mkdir)(const char __user *pathname, umode_t mode);

static void disable_write_protection(void);
static void enable_write_protection(void);

static unsigned long **find_sys_call_table(void);

/* We need to define a new sys_mkdir and use the new function to replace the original function. */
static asmlinkage long new_sys_mkdir(const char __user *pathname, umode_t mode);

static int __init hook_sys_mkdir_init(void);
static void __exit hook_sys_mkdir_exit(void);

module_init(hook_sys_mkdir_init);
module_exit(hook_sys_mkdir_exit);

static void disable_write_protection(void)
{
	unsigned long cr0 = read_cr0();
	clear_bit(16, &cr0);
	write_cr0(cr0);
}

static void enable_write_protection(void)
{
	unsigned long cr0 = read_cr0();
	set_bit(16, &cr0);
	write_cr0(cr0);
}

static unsigned long **find_sys_call_table(void)
{
	unsigned long vaddr_number;
	
	/* We want this pointer to point to the sys_call_table. */
	unsigned long **p;

	/*
	* It seems that the virtual address of the sys_call_table is always between
	* the virtual address of the sys_close and the virtual address of the loops_per_jiffy,
	* so we can search this area to find the sys_call_table.
	*/
	for (vaddr_number = (unsigned long)sys_close;
		vaddr_number < (unsigned long)&loops_per_jiffy;
	vaddr_number += sizeof(void *)) {
		p = (unsigned long **)vaddr_number;
		if (p[__NR_close] == (unsigned long *)sys_close)
			return p;
	}

	/* If this function cannot find the virtual address of sys_call_table, it will return NULL. */
	return NULL;
}

static asmlinkage long new_sys_mkdir(const char __user *pathname, umode_t mode)
{
	printk("<0>""calling sys_mkdir --hook_sys_mkdir\n");
	
	/* We still need to pass the parameters to the original sys_mkdir to complete the procedure of the system call. */
	return original_sys_mkdir(pathname, mode);
}

static int __init hook_sys_mkdir_init(void)
{
	sys_call_table = find_sys_call_table();

	if (!sys_call_table) {
		printk("<0>""The hook_sys_mkdir cannot find the sys_call_table.\n");
		return -1;
	}

	original_sys_mkdir = sys_call_table[__NR_mkdir];
	disable_write_protection();
	sys_call_table[__NR_mkdir] = new_sys_mkdir;
	enable_write_protection();

	return 0;
}

static void __exit hook_sys_mkdir_exit(void)
{
	disable_write_protection();
	sys_call_table[__NR_mkdir] = original_sys_mkdir;
	enable_write_protection();
        printk("<0>""rm hook success!\n");
}
