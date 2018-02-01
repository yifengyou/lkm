#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>
MODULE_LICENSE("GPL");

void **sys_call_table;

asmlinkage static int (*original_call) (const char __user *pathname,
        umode_t mode);

asmlinkage static int our_sys_mkdir(const char __user *pathname, umode_t mode)
{
   printk("!!!!call mkdir\n");
   return original_call(pathname,mode);
}

static int init_module2(void)
{
    // sys_call_table address in System.map
    sys_call_table = (void*)0xc1175c10;
    original_call = sys_call_table[__NR_mkdir];

    // Hook: Crashes here
    sys_call_table[__NR_mkdir] = our_sys_mkdir;
    return 0;
}

static void cleanup_module2(void)
{
   // Restore the original call
   sys_call_table[__NR_mkdir] = original_call;
}

module_init(init_module2);
module_exit(cleanup_module2);
