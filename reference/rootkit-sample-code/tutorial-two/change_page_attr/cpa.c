#include <linux/kernel.h>  
#include <linux/module.h>  
#include <linux/moduleparam.h>  
#include <linux/list.h>  
#include <linux/init.h>  
#include <linux/sched.h>  
#include <asm/unistd.h>  
#include <linux/dirent.h>  
#include <linux/stat.h>  
#include <linux/fs.h>  
#include <linux/proc_fs.h>  
#include <asm/uaccess.h>  

#include <net/sock.h>  
#include <net/netlink.h>  

#define CALLOFF 100  
#define SP_INTERCEPT_SOCKET_NETLINK 28  
//define idtr and idt struct  

char psname[10] = "hello";
char *processname = psname;

static char *mod_name = "hook";
module_param(mod_name, charp, 0);

struct {
    unsigned short limit;
    unsigned int base;
} __attribute__ ((packed))idtr;

struct {
    unsigned short off_low;
    unsigned short sel;
    unsigned char none;
    unsigned char flags;
    unsigned short off_high;
} __attribute__ ((packed))*idt;

struct _idt {
    unsigned short offset_low, segment_sel;
    unsigned char reserved, flags;
    unsigned short offset_high;
};

/*unsigned long *getscTable() 
{ 
    unsigned char idtr[6] = {0}, *shell = NULL, *sort = NULL; 
    struct _idt *idtLong = NULL; 
    unsigned long system_call = 0, sct = 0; 
    unsigned short offset_low = 0, offset_high = 0; 
    char *p = NULL; 
    int i = 0; 
 
    __asm__("sidt %0" : "=m" (idtr)); 
 
    idtLong=(struct _idt*)(*(unsigned long*)&idtr[2]+8*0x80); 
    offset_low = idtLong->offset_low; 
    offset_high = idtLong->offset_high; 
    system_call = (offset_high<<16)|offset_low; 
 
    shell=(char *)system_call; 
    sort="\xff\x14\x85"; 
 
    for(i=0;i<(100-2);i++) 
    { 
        if(shell[i] == sort[0] && shell[i+1] == sort[1] && shell[i+2] == sort[2]) 
    { 
        break; 
    } 
    } 
 
    p = &shell[i]; 
    p += 3; 
 
    sct=*(unsigned long*)p; 
    return (unsigned long*)(sct); 
}*/

//define function, Point to the system being hijacked  

struct linux_dirent {
    unsigned long d_ino;
    unsigned long d_off;
    unsigned short d_reclen;
    char d_name[1];
};

//asmlinkage long (*orig_getdents)(unsigned int fd, struct linux_dirent __user *dirp, unsigned int count);  

/* 
struct sockaddr 
{ 
    unsigned short sa_family; 
    char sa_data[14]; 
}; 
 
struct in_addr  
{ 
    unsigned long s_addr; 
}; 
 */
//struct sockaddr_in  
//{  
//  short int sin_family; /* Internet地址族*/  
//  unsigned short int sin_port; /* 端口号*/  
//  struct in_addr sin_addr; /* Internet地址*/  
//  unsigned char sin_zero[8]; /* 填充0（为了保持和struct sockaddr一样大小）*/  
//};  

asmlinkage long (*orig_getdents)(int fd, struct sockaddr __user *dirp, int addrlen);

//int orig_cr0 = 0;  
unsigned long *sys_call_table = NULL;

//add by liangz 2016-06-13  
void spinfo_destroy_netlink();
static struct mutex ply_cs_mutex;
static struct sock *nl_sk = NULL;
//policy list  

typedef struct _NetworkCtrl_ {
    // 是否启用  
    bool bEnable;
    //ip:port  
    char website[2048];

} NetworkCtrl, *PNetworkCtrl;

PNetworkCtrl sp_ply_info = NULL;

//get function system_call addr  
/*void* get_system_call(void) 
{ 
    printk(KERN_ALERT "start get_system_call...\n"); 
    void * addr = NULL; 
    asm("sidt %0":"=m"(idtr)); 
    idt = (void*) ((unsigned long*)idtr.base); 
    addr = (void*) (((unsigned int)idt[0x80].off_low) | (((unsigned int)idt[0x80].off_high)<<16 )); 
    return addr; 
}*/

//find sys_call_table  

char* findoffset(char *start) {
    printk(KERN_ALERT "start findoffset...\n");
    char *p = NULL;
    int i = 0;
    /*for(p=start; p < start + CALLOFF; p++) 
    { 
        if(*(p+0) == '\xff' && *(p+1) == '\x14' && *(p+2) == '\x85') 
        { 
            return p; 
        } 
    }*/

    p = start;
    for (i = 0; i < (100 - 2); i++, p++) {
        if (*(p + 0) == '\xff' && *(p + 1) == '\x14' && *(p + 2) == '\xc5') {
            printk(KERN_ALERT "p: 0x%x\n", p);
            return p;
        }
    }
    return NULL;
}

//get sys_call_table addr  

/*void** get_system_call_addr(void) 
{ 
    printk(KERN_ALERT "start get_system_call_addr.../n"); 
    unsigned long sct = 0; 
    char *p = NULL; 
    unsigned long addr = (unsigned long)get_system_call(); 
    if((p=findoffset((char*) addr))) 
    { 
        sct = *(unsigned long*)(p + 3); 
        printk(KERN_ALERT "find sys_call_addr: 0x%x\n", (unsigned int)sct); 
    } 
    return ((void**)sct); 
}*/

//clear and return cr0  

unsigned int clear_and_return_cr0(void) {
    printk(KERN_ALERT "start clear_and_return_cr0...\n");
    unsigned int cr0 = 0;
    unsigned int ret = 0;

    asm volatile ("movq %%cr0, %%rax" : "=a"(cr0));

    ret = cr0;

    cr0 &= 0xfffffffffffeffff;

    asm volatile ("movq %%rax, %%cr0"
                :
                : "a"(cr0)
                );

    return ret;
}

//ser cr0  

void setback_cr0(unsigned int val) {
    printk(KERN_ALERT "start setback_cr0...\n");

    asm volatile ("movq %%rax, %%cr0"
                :
                : "a"(val)
                );
}

//char* to int  

/*int atoi(char *str) 
{ 
    int res = 0; 
    int mul = 1; 
    char *ptr = NULL; 
    for(ptr = str + strlen(str)-1; ptr >= str; ptr--) 
    { 
        if(*ptr < '0' || *ptr > '9') 
        { 
            return -1; 
        } 
        res += (*ptr - '0') * mul; 
        mul *= 10; 
    } 
    return res; 
}*/

//check if process whose pid equals 'pid' is set to hidden  

/*int ishidden(pid_t pid) 
{ 
    if(pid < 0) 
    { 
        return 0; 
    } 
    struct task_struct * task = NULL; 
    task = find_task_by_pid(pid); 
    printk(KERN_ALERT "pid:%d,hide:%d/n", pid, task->hide); 
    if(NULL != task && 1 == task->hide) 
    { 
        return 1; 
    } 
    return 0; 
}*/

int myatoi(char *str) {
    int res = 0;
    int mul = 1;
    char *ptr = NULL;

    for (ptr = str + strlen(str) - 1; ptr >= str; ptr--) {
        if (*ptr < '0' || *ptr > '9') {
            return -1;
        }
        res += (*ptr - '0') * mul;
        mul *= 10;
    }
    if (res > 0 && res < 9999) {
        //printk(KERN_INFO "pid = %d\n",res);  
    }
    return res;
}

struct task_struct *get_task(pid_t pid) {
    //printk(KERN_INFO "start get_task.\n");  

    struct task_struct *p = get_current(), *entry = NULL;

    list_for_each_entry(entry, &(p->tasks), tasks) {
        if (entry->pid == pid) {
            //printk("pid found = %d\n",entry->pid);  
            return entry;
        }
        else {
            //printk(KERN_INFO "pid = %d not found\n",pid);  
        }
    }

    //printk(KERN_INFO "end get_task.\n");  

    return NULL;
}

static inline char *get_name(struct task_struct *p, char *buf) {
    int i = 0;
    char *name = NULL;
    name = p->comm;

    if (!name) {
        printk(KERN_INFO " process name is null!\n");
        return buf;
    }

    i = sizeof (p->comm);
    do {
        unsigned char c = *name;
        name++;
        i--;
        *buf = c;
        if (!c) {
            break;
        }
        if ('\\' == c) {
            buf[1] = c;
            buf += 2;
            continue;
        }
        if ('\n' == c) {
            buf[0] = '\\';
            buf[1] = 'n';
            buf += 2;
            continue;
        }
        buf++;
    } while (i);

    *buf = '\n';
    return buf + 1;
}

int get_process(pid_t pid) {
    struct task_struct *task = NULL;
    task = get_task(pid);
    char buffer[64] = {0};
    if (task) {
        get_name(task, buffer);

        if (0 == strlen(buffer)) {
            return 0;
        }
        //if(pid > 0 && pid < 9999)  
        //{  
        //printk(KERN_INFO "task name = %s\n",*buffer);  
        //}  
        //printk(KERN_INFO "pid = 1554,task name buffer = %s\n", buffer);  

        if (strstr(buffer, "SpinfoDepClient") || strstr(buffer, "SpinfoUsbCtrl") || strstr(buffer, "SpinfoPrintCtrl") || strstr(buffer, "SpinfoNetWorkCt")) {
            printk(KERN_INFO "task name = %s\n", buffer);
            return 1;
        }
        else {
            return 0;
        }
    }
    else {
        printk(KERN_INFO "get_task is null.\n");
    }
    return 0;
}

int my_inet_ntoa(struct in_addr ina, char* ipBuff) {
    unsigned char *ucp = (unsigned char *) &ina;
    sprintf(ipBuff, "%d.%d.%d.%d", ucp[0] & 0xff, ucp[1] & 0xff, ucp[2] & 0xff, ucp[3] & 0xff);
    printk("ipBuff = %s\n", ipBuff);

    return 0;
}

int DottedDecimal(unsigned long ulAddr, char* szAddr) {
    unsigned long ulMask[4] = {0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000};
    int i = 0;
    for (; i < 4; i++) {
        long l = (ulAddr & ulMask[i]) >> (i * 8);
        sprintf((szAddr + strlen(szAddr)), "%ld", l);
        if (i != 3) {
            strcat(szAddr, ".");
        }
    }
    return 0;
}

//the hacked sys_getdents64  

asmlinkage long hacked_getdents(int fd, struct sockaddr __user* dirp, int addrlen) {
    //add by liangz 2016-03-14  
    long modifyBufLength = 0;
    int value = 0;
    unsigned short len = 0;
    unsigned short tlen = 0;

    printk(KERN_ALERT "hide process start call __NR_connect...\n");

    //value = (*orig_getdents) (fd, dirp, addrlen);  
    //  
    /* 
    struct sockaddr* dirp2 = NULL; 
    dirp2 = (struct sockaddr*)kmalloc(sizeof(struct sockaddr), GFP_KERNEL); 
    if(!dirp2) 
    { 
        printk(KERN_ERR "kmalloc mail..."); 
        return 111; 
    } 
 
    if(copy_from_user(dirp2, dirp, sizeof(struct sockaddr))) 
    { 
        printk(KERN_ERR "fail to copy dirp to dirp2...\n"); 
    kfree(dirp2); 
    dirp2 = NULL; 
    return 111; 
    }*/

    char addrdata1[15] = {0};
    memcpy(addrdata1, dirp->sa_data, 14);

    unsigned short output;
    unsigned int ipInt;

#if BYTE_ORDER == LITTLE_ENDIAN  
    ((unsigned char*) &output)[0] = addrdata1[1];
    ((unsigned char*) &output)[1] = addrdata1[0];

    ((unsigned char*) &ipInt)[0] = addrdata1[2];
    ((unsigned char*) &ipInt)[1] = addrdata1[3];
    ((unsigned char*) &ipInt)[2] = addrdata1[4];
    ((unsigned char*) &ipInt)[3] = addrdata1[5];
#else  
#endif  

    //struct sockaddr_in *addrdata = NULL;  
    //addrdata = (struct sockaddr_in*)dirp;  

    char ipBuff[16] = {0};

    printk("port = %d\n", output);

    DottedDecimal(ipInt, ipBuff);

    printk("ip str = %s\n", ipBuff);

    //read policy info ip port  
    //  
    mutex_lock(&ply_cs_mutex);

    if (sp_ply_info && sp_ply_info->bEnable) {
        char cPort[5] = {0};
        snprintf(cPort, 5, "%d", output);

        char ipPort[32] = {0};
        strncat(ipPort, ipBuff, 15);
        strncat(ipPort, ":", 1);
        strncat(ipPort, cPort, 5);

        printk("ipPort = %s\n", ipPort);

        if (strstr(sp_ply_info->website, ipPort)) {
            printk("ip = %s:port = %d is illegal!\n", ipBuff, output);
            //copy_to_user (dirp, dirp2, sizeof(struct sockaddr));  
            //kfree(dirp2);  
            //dirp2 = NULL;  
            mutex_unlock(&ply_cs_mutex);
            return 111;
        }
    }

    mutex_unlock(&ply_cs_mutex);

    //copy_to_user (dirp, dirp2, sizeof(struct sockaddr));  

    //kfree(dirp2);  
    //dirp2 = NULL;  
    printk("end call __NR_connect...\n");
    //value = (*orig_getdents) (fd, dirp, addrlen);  
    /* 
    modifyBufLength = value; 
    return modifyBufLength; 
     */
    return 0;
}

static void *memmem(const void *haystack, size_t haystack_len, const void *needle, size_t needle_len) {
    const char *begin = NULL;
    const char *const last_possible = (const char *) haystack + haystack_len - needle_len;
    if (needle_len == 0) {
        printk(KERN_ALERT "needle_len == 0\n");
        return (void*) haystack;
    }

    if (__builtin_expect(haystack_len < needle_len, 0)) {
        return NULL;
    }

    for (begin = (const char *) haystack; begin <= last_possible; ++begin) {
        if (begin[0] == ((const char *) needle)[0]
                && !memcmp((const void *) &begin[1],
                (const void *) ((const char *) needle + 1),
                needle_len - 1)) {
            return (void*) begin;
        }
    }
    return NULL;
}

static unsigned long get_sct_addr(void) {
#define OFFSET_SYSCALL 200  

    unsigned long syscall_long, retval;
    char sc_asm[OFFSET_SYSCALL] = {0};

    rdmsrl(MSR_LSTAR, syscall_long);
    memcpy(sc_asm, (char *) syscall_long, OFFSET_SYSCALL);

    retval = (unsigned long) memmem(sc_asm, OFFSET_SYSCALL, "\xff\x14\xc5", 3);

    if (retval != 0) {
        retval = (unsigned long) (* (unsigned long *) (retval + 3));
    }
    else {
        printk("long mode : memmem found nothing, returning NULL");
        retval = 0;
    }
#undef OFFSET_SYSCALL  
    return retval;
    //unsigned sys_call_off = 0;  
    /*void *sys_call_off = NULL; 
    unsigned sct = 0; 
    char *p = NULL; 
 
    unsigned char idtrT[6] = {0}, *shell = NULL, *sort = NULL; 
    struct _idt *idtT; 
    unsigned long system_call = 0;// sct = 0; 
 
    asm("sidt %0":"=m"(idtrT)); 
    //idt = (void *) (idtr.base + 8 * 0x80); 
    printk(KERN_ALERT "=== 1 ===\n"); 
    //idt = (void*) ((unsigned long*)idtr.base); 
    idtT = (struct _idt*)(*(unsigned long*)&idtrT[2]+8*0x80); 
    //idt = (struct _idt*) 
    //sys_call_off = (idt->off_high << 16) | idt->off_low; 
    printk(KERN_ALERT "=== 2 ===\n"); 
    //sys_call_off = (void*) (((unsigned int)idt[0x80].off_low) | (((unsigned int)idt[0x80].off_high)<<16 )); 
    system_call = (idtT->offset_high<<16) | idtT->offset_low; 
 
    printk(KERN_ALERT "=== 3 ===\n"); 
 
    if ((p = findoffset((char *) system_call))) 
    { 
        sct = *(unsigned *) (p + 3); 
    } 
    else 
    { 
          printk(KERN_ALERT " findoffset fail...\n"); 
    } 
    return ((void **)sct);*/
}

int hideModule(void) {
    list_del(&THIS_MODULE->list); //lsmod,/proc/modules  
    kobject_del(&THIS_MODULE->mkobj.kobj); // /sys/modules  
    list_del(&THIS_MODULE->mkobj.kobj.entry); // kobj struct list_head entry  

    return 0;
}

static inline void rootkit_protect(void) {
    try_module_get(THIS_MODULE); // count++  
    //module_put(THIS_MODULE);//count--  
}

void nl_receive_policy(struct sk_buff *__skb) {
    struct sk_buff *skb = NULL;
    struct nlmsghdr *nlh = NULL;

    int len = 0;
    len = sizeof (NetworkCtrl);

    if (NULL == sp_ply_info) {
        sp_ply_info = kmalloc(sizeof (NetworkCtrl), GFP_KERNEL);
        if (!sp_ply_info) {
            printk(KERN_ERR "nl_receive_policy() : kmalloc() %d\n", -ENOMEM);
            return;
        }
    }
    else {
        printk("sp_ply_info is not NULL, memset sp_ply_info...\n");
        memset(sp_ply_info, 0, sizeof (NetworkCtrl));
    }

    skb = skb_get(__skb);
    if (skb->len >= NLMSG_SPACE(0)) {
        nlh = nlmsg_hdr(skb);

        mutex_lock(&ply_cs_mutex);

        memcpy(sp_ply_info, NLMSG_DATA(nlh), len);

        printk("bEnable = %d\n", sp_ply_info->bEnable);
        printk("website = %s\n", sp_ply_info->website);

        mutex_unlock(&ply_cs_mutex);
    }
    kfree_skb(skb);
}

int spinfo_init_netlink() {
    int err = 0;
    printk("spinfo_init_netlink...\n");

    mutex_init(&ply_cs_mutex);

    nl_sk = netlink_kernel_create(&init_net, SP_INTERCEPT_SOCKET_NETLINK, 1, nl_receive_policy, NULL, THIS_MODULE);
    if (!nl_sk) {
        printk(KERN_ERR "spinfo_init_netlink() : create netlink socket error.\n");
        err = -1;
    }

    return err;
}

static int __init hook_init(void) {
    //printk(KERN_ALERT"[insmod module] name:%s state:%d refcnt:%u \n",THIS_MODULE->name,THIS_MODULE->state,module_refcount(THIS_MODULE));      

    //rootkit_protect();  

    printk("init policy...\n");


    if (spinfo_init_netlink()) {
        return -1;
    }

    printk(KERN_ALERT"[insmod module] name:%s state:%d refcnt:%u \n", THIS_MODULE->name, THIS_MODULE->state, module_refcount(THIS_MODULE));

    /*int retHide = 0; 
    retHide = hideModule(); 
    if(0 == retHide) 
    { 
        printk(KERN_ALERT "hide module success...\n"); 
    }*/
    /*if(0 == THIS_MODULE->state) 
    { 
        THIS_MODULE->state = 1; 
    }*/

    printk(KERN_ALERT "start hook_init\n");
    unsigned long orig_cr0 = 0; //clear_and_return_cr0();  

    sys_call_table = (unsigned long*) get_sct_addr();
    sys_call_table = (unsigned long) sys_call_table | 0xffffffff00000000;

    if (!sys_call_table) {
        printk(KERN_ALERT "=== get_sct_addr fail ===\n");
        return -EFAULT;
    }
    else if (sys_call_table[__NR_connect] != hacked_getdents) {
        printk(KERN_ALERT "start __NR_connect ...");
        //printk(KERN_ALERT "sct:0x%x\n", (unsigned long)sys_call_table);  
        printk(KERN_ALERT "sct:0x%x,hacked_getdents:0x%x\n", (unsigned long) sys_call_table[__NR_connect], (unsigned long) hacked_getdents);

        orig_cr0 = clear_and_return_cr0();
        orig_getdents = sys_call_table[__NR_connect];
        printk(KERN_ALERT "old:0x%x, new:0x%x\n", (unsigned long) orig_getdents, (unsigned long) hacked_getdents);
        printk(KERN_ALERT "end __NR_connect ...");

        if (hacked_getdents != NULL) {
            printk(KERN_ALERT "call hacked_getdents...\n");

            sys_call_table[__NR_connect] = hacked_getdents;
        }

        setback_cr0(orig_cr0);

        printk(KERN_INFO "module loaded...\n");
        /*printk(KERN_ALERT"[insmod module] name:%s state:%d\n",THIS_MODULE->name,THIS_MODULE->state); 
        if(0 == THIS_MODULE->state) 
        { 
            THIS_MODULE->state = 1; 
        }*/
        return 0;
    }
    else {
        printk(KERN_ALERT "system_call_table_long[__NR_getdents64] == hacked_getdents\n");
        return -EFAULT;
    }
}

static int __exit unhook_exit(void) {
    printk(KERN_ALERT "start unhook_exit\n");

    spinfo_destroy_netlink();

    unsigned long orig_cr0 = clear_and_return_cr0();

    if (sys_call_table) {
        sys_call_table[__NR_connect] = orig_getdents;

        setback_cr0(orig_cr0);

        printk(KERN_ALERT "unhook_exit success...\n");
        return 0;
    }
    printk(KERN_ALERT "unhook_exit fail...\n");
    return -EFAULT;
}

void spinfo_destroy_netlink() {
    printk("spinfo_destroy_netlink...\n");

    if (nl_sk) {
        sock_release(nl_sk->sk_socket);
    }

    if (sp_ply_info) {
        kfree(sp_ply_info);
        sp_ply_info = NULL;
    }
}


MODULE_AUTHOR("zhao liang. Halcrow <mhalcrow@us.ibm.com>");
MODULE_DESCRIPTION("interceptSocket");
MODULE_LICENSE("GPL");

module_init(hook_init)
module_exit(unhook_exit)