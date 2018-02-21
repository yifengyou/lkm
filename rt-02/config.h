#ifndef __CONFIG__
#define __CONFIG__


#define DEBUG

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


#endif