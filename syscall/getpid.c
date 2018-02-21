/*************************************************************************
> File Name: getpid.c
> Author: 
> Mail: 
> Created Time: 2018年01月22日 星期一 18时17分23秒
************************************************************************/

#include<stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <errno.h>

int
main(int argc, char *argv[])
{
    pid_t pid;
    //pid = syscall(SYS_getpid);
    //pid = syscall(172);
    pid = syscall(__NR_getpid);
    printf("errno:%d\n",errno);
    printf("pid:%d\n",pid);

}





