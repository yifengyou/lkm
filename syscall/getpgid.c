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
    pid_t pgid;
    pid_t currentid;
    currentid = getpid();
    //pid = syscall(SYS_getpid);
    //pid = syscall(172);
    printf("currentid:%d\n",currentid);
    pgid = syscall(__NR_getpgid,currentid);
    printf("syscall(__NR_getpgid,0):%d\n",syscall(__NR_getpgid,0));
    printf("errno:%d\n",errno);
    printf("pgid:%d\n",pgid);

}





