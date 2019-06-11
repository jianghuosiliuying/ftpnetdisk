#include "head.h"

int recvCycle(int newFd,void* p,int len)
{
    int ret,total=0;
    char *pStart=(char*)p;
    while(total<len)
    {
        ret=recv(newFd,pStart+total,len-total,0);
        if(ret==0)//服务器断开，ret等于0
        {
            return -1;
        }
        total+=ret;
    }
    return total;
}

int sendCycle(int newFd,void* p,int len)
{
    int ret,total=0;
    char *pStart=(char*)p;
    while(total<len)
    {
        ret=send(newFd,pStart+total,len-total,0);
        ERROR_CHECK(ret,-1,"send");
        total+=ret;
    }
    return 0;
}
