#include "RTsyscalltable.h"

unsigned int addr_do_fork = 0;

unsigned int find_do_fork(void) {
    unsigned int ret;
    unsigned char *p;
    unsigned char *psyscall = NULL;
    unsigned char *psysvfork = NULL;

    if (NULL == sys_call_table) {
        DLog("sys_call_table == NULL");
        return 0;
    }

    psyscall = (unsigned char *) sys_call_table[__NR_vfork];

    if (NULL == psyscall) {
        DLog("sys_call_table[__NR_vfork] == NULL");
        return 0;
    }

    for (p = psyscall; p < psyscall + 0x10;) {
        if (*p++ == (unsigned char) (0xE9)) {
            ret = *((unsigned int *) p);
            psysvfork = p + 4 + ret;
            break;
        }
    }

    if (NULL == psysvfork) {
        DLog("psysvfork == NULL");
        return 0;
    }

    ret = 0;

    for (p = psysvfork; p < psysvfork + 0x40;) {
        if (*p++ == (unsigned char) (0xE8)) {
            ret = *((unsigned int *) p);
            ret = (unsigned int) p + 4 + ret;
            break;
        }
    }

    return ret;
}



