#include "factory.h"

void factoryInit(pfactory_t pf,int thread_num,int capatory)
{
    memset(pf,0,sizeof(factory_t));
    pf->pthid=(pthread_t*)calloc(thread_num,sizeof(pthread_t));//给存放线程id的指针申请空间
    pf->thread_num=thread_num;
    pthread_cond_init(&pf->cond,NULL);
    queInit(&pf->que,capatory);
}

void personInit(Person_t* pperson)
{
    memset(pperson,0,sizeof(Person_t));
}
void dirInit(Dir_t* pdir,int* newFd)
{
    memset(pdir,0,sizeof(Dir_t));
    pdir->childFd=*newFd;
    strncpy(pdir->path,"/home/chengang/netdisk/disk/",28);
    strncpy(pdir->root,"/home/chengang/netdisk/disk/",28);
    pdir->code=0;
}

void fileInit(File_t* pfile)
{
    memset(pfile,0,sizeof(File_t));
}
