#ifndef __MAIN__
#define __MAIN__

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

struct linux_dirent {
	unsigned long d_ino;
	unsigned long d_off;
	unsigned short d_reclen;
	char d_name[1];
};

#ifndef __AVOID_NULL__
#define __AVOID_NULL__
#define AvoidNull(call, args1) {\
if(args1) call(args1);\
}
#endif 



#ifdef DEBUG  
# define DLog(fmt, ... ) printk(("Rootkit:%s-[%s]:%d " fmt "\n"), __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);  
#else  
# define DLog(...);  
#endif  



// maximum length of the path
#define MAX_PATH 260
//#define NAME_MAX        255    /* # �ļ�������ַ��� */
//#define PATH_MAX        4096    /* # ���·��������ַ��� */

// /proc/net/tcp �ļ�ÿ��150���ַ���ּ�ڹ���ÿһ��TCP��Ϣ
#define TCP_SZ 150

// /proc/net/udp �ļ�ÿ��128���ַ���ּ�ڹ���ÿһ��UDP��Ϣ
#define UDP_SZ 128


/* maximized args number that audit_socketcall can process */
#define AUDITSC_ARGS		6


#endif // __MAIN__
