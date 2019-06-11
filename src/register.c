#include "factory.h"

int registeruser(int newFd,Person_t* pperson)
{
    char name[128]={0};
    char table[10]={0};
    char aim[20]={0};
    char condition[200]={0};
    int ret;
    int dataLen;
name:
    bzero(name,sizeof(name));
    recvCycle(newFd,&dataLen,4);
    ret=recvCycle(newFd,name,dataLen);
    if(ret>0)
    {
        strncpy(table,"person",6);
        strncpy(aim,"*",1);
        sprintf(condition,"where name='%s'",name);
        int r=query1(aim,table,condition,&pperson);
        printf("r=%d\n",r);
        send(newFd,&r,4,0);
        if(r==1)//已经存在该名字
        {
            goto name;
        }else if(r==0)//不存在同名，可以进行注册
        {
            char salt[128]={0};
            char cryptcode[200]={0};
            recvCycle(newFd,&dataLen,4);
            recvCycle(newFd,salt,dataLen);//接收盐值
            printf("salt=%s\n",salt);
            recvCycle(newFd,&dataLen,4);
            recvCycle(newFd,cryptcode,dataLen);//接收密文
            printf("cryptcode=%s\n",cryptcode);
            char aimthing[200]={0};
            bzero(table,sizeof(table));
            bzero(condition,sizeof(condition));
            strncpy(table,"person",6);
            strncpy(aimthing,"name,salt,cryptcode",19);
            sprintf(condition,"'%s','%s','%s'",name,salt,cryptcode);
            ret=personInsert(aimthing,table,condition);
            send(newFd,&ret,4,0);
        }
    }
    return 0;
}

