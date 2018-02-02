/*
 * This module need to be compiled under 64-bit platform using GCC.
 */
#include <linux/module.h> /* module_init module_exit */
#include <linux/init.h> /* __init __exit */
#include <linux/syscalls.h> /* sys_close __NR_close __NR_mkdir */
#include <linux/delay.h> /* loops_per_jiffy */
#include <asm/bitops.h> /* set_bit clear_bit */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("youyifeng");
MODULE_DESCRIPTION("This module can hook the sys_* function.");

static unsigned long **sys_call_table;

static asmlinkage long (*original_sys_mkdir)(const char __user *pathname, umode_t mode);
static asmlinkage long (*original_sys_chdir)(const char __user *pathname);

static void disable_write_protection(void);
static void enable_write_protection(void);

static unsigned long **find_sys_call_table(void);

/* We need to define a new sys_mkdir and use the new 
 * function to replace the original function. 
 */
static asmlinkage long new_sys_mkdir(const char __user *pathname, umode_t mode);
static asmlinkage long new_sys_chdir(const char __user *pathname);

static int __init hook_sys_mkdir_init(void);
static void __exit hook_sys_mkdir_exit(void);

module_init(hook_sys_mkdir_init);
module_exit(hook_sys_mkdir_exit);

static void disable_write_protection(void) {
    unsigned long cr0 = read_cr0();
    clear_bit(16, &cr0);
    write_cr0(cr0);
}

static void enable_write_protection(void) {
    unsigned long cr0 = read_cr0();
    set_bit(16, &cr0);
    write_cr0(cr0);
}

static unsigned long **find_sys_call_table(void) {
    unsigned long vaddr_number;

    /* We want this pointer to point to the sys_call_table. */
    unsigned long **p;

    /*
     * It seems that the virtual address of the sys_call_table is always between
     * the virtual address of the sys_close and the virtual address of the loops_per_jiffy,
     * so we can search this area to find the sys_call_table.
     */
    for (vaddr_number = (unsigned long) sys_close;
            vaddr_number < (unsigned long) &loops_per_jiffy;
            vaddr_number += sizeof (void *)) {
        p = (unsigned long **) vaddr_number;
        if (p[__NR_close] == (unsigned long *) sys_close)
            return p;
    }

    /* If this function cannot find the virtual address 
     * of sys_call_table, it will return NULL. 
     */
    return NULL;
}

static asmlinkage long new_sys_mkdir(const char __user *pathname, umode_t mode) {
    /*
    功能：用于将用户空间的数据传送到内核空间。
   unsigned long copy_from_user(void * to, const void __user * from, unsigned long n)
   第一个参数to是内核空间的数据目标地址指针，
   第二个参数from是用户空间的数据源地址指针，
   第三个参数n是数据的长度。
   如果数据拷贝成功，则返回零；否则，返回没有拷贝成功的数据字节数。
   此函数将from指针指向的用户空间地址开始的连续n个字节的数据产送到to指针指向的内核空间地址
     */
    char pathName[1020];
    memset(pathName, 0, 1020);
    if (!copy_from_user(pathName, pathname, strnlen_user(pathname, 1020))) {
        printk("mkdir:pathName-mod:[%s]-[%d]\n", pathName, mode);
    } else {
        printk("mkdir:copy_from_user failure~~\n");
    }

    /* We still need to pass the parameters to the original sys_mkdir 
     * to complete the procedure of the system call. 
     */
    return original_sys_mkdir(pathname, mode);
}

static asmlinkage long new_sys_chdir(const char __user *pathname) {
    char pathName[1020];
    memset(pathName, 0, 1020);
    if (!copy_from_user(pathName, pathname, strnlen_user(pathname, 1020))) {
        printk("chdir:pathName-mod:[%s]\n", pathName);
    } else {
        printk("chdir:copy_from_user failure~~\n");
    }
    return original_sys_chdir(pathname);
}

static int __init hook_sys_mkdir_init(void) {
    sys_call_table = find_sys_call_table();
    if (!sys_call_table) {
        printk("<0>""The hook_sys_mkdir cannot find the sys_call_table.\n");
        return -1;
    }
    original_sys_mkdir = sys_call_table[__NR_mkdir];
    original_sys_chdir = sys_call_table[__NR_chdir];

    disable_write_protection();

    sys_call_table[__NR_mkdir] = new_sys_mkdir;
    sys_call_table[__NR_chdir] = new_sys_chdir;

    enable_write_protection();
    return 0;
}

static void __exit hook_sys_mkdir_exit(void) {
    disable_write_protection();

    sys_call_table[__NR_mkdir] = original_sys_mkdir;
    sys_call_table[__NR_chdir] = original_sys_chdir;

    enable_write_protection();
    printk("<0>""rm hook success!\n");
}




