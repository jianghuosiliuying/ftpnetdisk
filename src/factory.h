#ifndef __FACTORY_H__
#define __FACTORY_H__
#include "head.h"
#include "work_que.h"
typedef struct{
    pthread_t *pthid;//存储线程ID的起始地址
    int thread_num;//要创建的线程数目
    pthread_cond_t cond;//条件变量
    que_t que;//生产者和消费者操作队列
    short startFlag;//工厂启动标志
}factory_t,*pfactory_t;

int registeruser(int,Person_t*);
void factoryInit(pfactory_t,int,int);
void factoryStart(pfactory_t);
void queInit(pque_t,int);
int tcpInit(int*,char*,char*);
int sendCycle(int,void*,int);
int recvCycle(int,void*,int);
int tranFile(int,char*,char*);
int checkname(char*,Person_t*);
void personInit(Person_t*);
int checkpasswd(char*);
int query1(char*,char*,char*,Person_t**);
int query2(char*,char*,char*,pFile_t*,pFile_t*);
int commandFunc(int,int,char*,Dir_t*,Dir_t*,Dir_t*,int);
void dirInit(Dir_t*,int*);
void fileInit(File_t*);
int deletefile(char*,char*);
int fileInsert(char*,char*,char*);
int download(int,int,off_t);
int personInsert(char*,char*,char*);
#endif
