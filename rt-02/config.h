#ifndef __CONFIG__
#define __CONFIG__


#define DEBUG
#define MAX_PATH 1020

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