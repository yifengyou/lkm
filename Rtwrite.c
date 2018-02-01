#include "Rtwrite.h"

unsigned int reg_cr0 = 0;

// clear WP bit of CR0
/**
 * 写保护指的是写入只读内存时出错。 
 * 这个特性可以通过CR0寄存器控制：开启或者关闭， 只需要修改一个比特，
 * 也就是从 0 开始数的第 16个比特
 */
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

