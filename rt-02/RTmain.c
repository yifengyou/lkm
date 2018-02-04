#include "RTmain.h"


unsigned int rtpid = -1;
unsigned int rtfd = -1;

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




typedef struct {
    int pid;
    int count;
    long lastcall;

} PROC_RECORD;
PROC_RECORD pred[10];

void
del_lkm(const char *name) {
    struct module * pmod;
    struct list_head *plist;
    //struct list_head *plisthead;

    if (module_list_head == NULL) {
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

// int init_module(const char *name, struct module *image);

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

RT_SYSCALL_DEFINE(long, chdir, const char __user *filename) {
    int ret;
    ret = RT_SYSCALL_CALL(chdir, filename);
    return ret;
}

// sys_read

RT_SYSCALL_DEFINE(long, read, unsigned int fd,
        char __user *buf, size_t count) {
    int ret;
    ret = RT_SYSCALL_CALL(read, fd, buf, count);
    return ret;
}

/**
 * sys_open
 * 只检查打开的文件是否是 /proc/net/tcp 或 /proc/net/udp，否则调用正常中断
 */
RT_SYSCALL_DEFINE(long, open, const char __user *filename,
        int flags, int mode) {
    int ret;
    ret = RT_SYSCALL_CALL(open, filename, flags, mode);
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

RT_SYSCALL_DEFINE(long, kill, int pid, int sig) {
    long ret = 0;
    ret = RT_SYSCALL_CALL(kill, pid, sig);
    return ret;
}


static unsigned long **find_sys_call_table(void) {
    unsigned long vaddr_number;

    /* We want this pointer to point to the sys_call_table. */
    unsigned long **p;

    /*
     * It seems that the virtual address of the sys_call_table is always between
     * the virtual address of the sys_close and the virtual address of the loops_per_jiffy,
     * so we can search this area to find the sys_call_table.
     */
    for (vaddr_number = (unsigned long) sys_close;
            vaddr_number < (unsigned long) &loops_per_jiffy;
            vaddr_number += sizeof (void *)) {
        p = (unsigned long **) vaddr_number;
        if (p[__NR_close] == (unsigned long *) sys_close)
            return p;
    }

    /* If this function cannot find the virtual address 
     * of sys_call_table, it will return NULL. 
     */
    return NULL;
}

static int __init
ThisInit(void) {
    rtprint("init function");

    sys_call_table = find_sys_call_table();


    rtprint("sys_call_table = find_sys_call_table() finished!");

    rtprint("addr_do_fork = find_do_fork() finished!");


    rtprint("disable_write_protection");


    rtprint("enable_write_protection");
    rtprint("Leave init function");

    return 0;
}

static void __exit
ThisExit(void) {
    rtprint("Enter exit function");


    rtprint("disable_write_protection");

    rtprint("enable_write_protection");
    rtprint("Leave exit function");
}

MODULE_LICENSE("GPL");

module_init(ThisInit); //模块入口函数
module_exit(ThisExit); //模块出口函数


