#ifndef __HEAD_H__
#define __HEAD_H__
#include <errno.h>
#include <sys/uio.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/msg.h>
#include <pthread.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <syslog.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/sendfile.h>

#define ARGC_CHECK(argc,val) {if(argc!=val){printf("error argc\n");return -1;}}
#define ERROR_CHECK(ret,retval,funcName) {if(ret==retval){printf("%d:",__LINE__);fflush(stdout);perror(funcName);return -1;}}
#define THREAD_ERROR_CHECK(ret,funcName) {if(ret!=0){printf("%s:%s\n",funcName,strerror(ret));return -1;}}
#define FILENAME "file"
typedef struct{
    int dataLen;
    char buf[4096];
}Train_t;
typedef struct{
    int childFd;
    char path[128];
    char root[128];
    int code;
}Dir_t,*pDir_t;
typedef struct{
    int ID;
    char name[200];
    char salt[200];
    char cryptcode[200];
}Person_t,*pPerson_t;
typedef struct file{
    int precode;
    int code;
    char filename[128];
    char filetype[1];
    int belong;
    char md5[50];
    int size;
    struct file* pNext;
}File_t,*pFile_t;
typedef struct{
    char filename[128];
    char filetype[1];
    int size;
}ls_t,*pls_t;
typedef struct{
    int key;
    char buf2[128];
}childthread,*pchildthread;
int recvCycle(int,void*,int);
#endif
