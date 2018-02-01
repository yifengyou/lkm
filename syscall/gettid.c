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
int
main(int argc, char *argv[])
{
    pid_t tid;
    tid = syscall(SYS_gettid);
    printf("tid:%d\n",tid);

}





