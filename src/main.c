#include "factory.h"
int exitFds[2];
int cnt=0;

void cleanup(void* p)
{
    //printf("i am clean up\n");
    pthread_mutex_t* mutex=(pthread_mutex_t*)p;
    pthread_mutex_unlock(mutex);
}
void* threadFunc(void* p)
{
    pfactory_t pf=(pfactory_t)p;
    pque_t pq=&pf->que;
    pNode_t pdelete;
    while(1)
    {
        pthread_mutex_lock(&pq->mutex);
        pthread_cleanup_push(cleanup,&pq->mutex);
        if(pf->startFlag&&pq->que_size==0)//任务队列为空，则等待
        {
            pthread_cond_wait(&pf->cond,&pq->mutex);
        }
        if(!pf->startFlag)
        {
            pthread_exit(NULL);//触发清理函数
        }
        queGet(pq,&pdelete);//任务队列不为空，则获取任务
        pthread_mutex_unlock(&pq->mutex);
        pthread_cleanup_pop(0);
        char buf2[128]={0};
        Dir_t dir;
        int ID=pdelete->userID;
        dirInit(&dir,&(pdelete->newFd));
        chdir(dir.root);//切换到disk根目录
        Dir_t predir;
        Dir_t tmpdir;
        int dataLen;
        dirInit(&predir,&(pdelete->newFd));
        dirInit(&tmpdir,&(pdelete->newFd));
        memcpy(&predir,&dir,sizeof(Dir_t));
        int key,ret;
        time_t now;
        struct tm *p;
        char logbuf[128]={0};
        char buf1[10]={0};
        char timebuf[128]={0};
        while(1)
        {
            ret=recv(pdelete->newFd,&key,4,0);
            if(ret==0)
            {
                printf("%d connect\n",--cnt);
                free(pdelete);
                pdelete=NULL;
                break;
            }
            bzero(buf1,sizeof(buf1));
            printf("the command is=%d\n",key);
            switch (key){
                case 1:strcpy(buf1,"ls");break;
                case 2:strcpy(buf1,"pwd");break;
                case 3:strcpy(buf1,"cd");break;
                case 4:strcpy(buf1,"puts");break;
                case 5:strcpy(buf1,"gets");break;
                case 6:strcpy(buf1,"remove");break;
                case 7:strcpy(buf1,"mkdir");break;
                case 8:strcpy(buf1,"rmdir");break;
            }
            if(key>=3&&key<=8)
            {
                bzero(buf2,sizeof(buf2));
                recvCycle(pdelete->newFd,&dataLen,4);
                recvCycle(pdelete->newFd,buf2,dataLen);
                //printf("buf2=%s,key=%d\n",buf2,key);
            }
            bzero(timebuf,sizeof(timebuf));
            time(&now);
            p=gmtime(&now);
            sprintf(timebuf,"%04d-%02d-%02d %02d:%02d:%02d",p->tm_year+1900,p->tm_mon+1,p->tm_mday,p->tm_hour+8,p->tm_min,p->tm_sec);
            sprintf(logbuf,"%4d %-10s %-20s %s",ID,buf1,buf2,timebuf);
            int fd=open("log.txt",O_CREAT|O_WRONLY,0666);
            lseek(fd,0,SEEK_END);
            write(fd,logbuf,strlen(logbuf));
            write(fd,"\n",1);
            close(fd);
            if(key>0&&key<9)
            {
                commandFunc(key,pdelete->newFd,buf2,&dir,&predir,&tmpdir,ID);//选项操作函数
            }
            //sleep(1);
        }
    }
}

void factoryStart(pfactory_t pf)
{
    int i;
    if(!pf->startFlag)
    {
        pf->startFlag=1;
        for(i=0;i<pf->thread_num;++i)
        {
            pthread_create(pf->pthid+i,NULL,threadFunc,pf);
        }
    }
}

void sigFuncExit(int signum)
{
    printf("signal is send\n");
    write(exitFds[1],&signum,1);//监控进程往写端写入1个字节
    int status;
    wait(&status);
    if(WIFEXITED(status))
    {
        printf("exit value=%d\n",WEXITSTATUS(status));
    }
    exit(0);
}

int confhandle(int fd,char* ip,char* port,int* thread_num,int* capacity)
{
    char buf[128]={0};
    char thread_numbuf[128]={0};
    char capacitybuf[128]={0};
    int ret=read(fd,buf,sizeof(buf));
    ERROR_CHECK(ret,-1,"read");
    int i,j=0;
    for(i=0;i<(int)strlen(buf);i++)
    {
        while(buf[i++]!='\n')
        {
            ip[j++]=buf[i-1];
        }
        j=0;
        while(buf[i++]!='\n')
        {
            port[j++]=buf[i-1];
        }
        j=0;
        while(buf[i++]!='\n')
        {
            thread_numbuf[j++]=buf[i-1];
        }
        j=0;
        while(buf[i++]!='\n')
        {
            capacitybuf[j++]=buf[i-1];
        }
    }
    *thread_num=atoi(thread_numbuf);
    *capacity=atoi(capacitybuf);
    return 0;
}

int main(int argc,char* argv[])
{
    ARGC_CHECK(argc,2);
    pipe(exitFds);
    int sig=1;
    while(fork())
    {//父进程座位监控进程，主进程异常退出时，拉起主进程
        //父进程读端不关闭，但不能去读，只使用写端
        signal(SIGUSR1,sigFuncExit);//信号退出机制
        wait(NULL);
        printf("error quik %d time\n",sig++);
    }
    close(exitFds[1]);//子进程作为主线程，关闭主线程写端
    char ip[20]={0};
    char port[10]={0};
    int thread_num=0;
    int capacity=0;
    int fd=open(argv[1],O_RDONLY);
    ERROR_CHECK(fd,-1,"open");
    confhandle(fd,ip,port,&thread_num,&capacity);
    printf("%s %s %d %d\n",ip,port,thread_num,capacity);
    factory_t fac;
    factoryInit(&fac,thread_num,capacity);
    factoryStart(&fac);//创建线程
    int socketFd;
    tcpInit(&socketFd,ip,port);
    int epfd=epoll_create(1);
    struct epoll_event event,evs[2];
    event.events=EPOLLIN;
    event.data.fd=socketFd;
    epoll_ctl(epfd,EPOLL_CTL_ADD,socketFd,&event);
    event.data.fd=exitFds[0];
    epoll_ctl(epfd,EPOLL_CTL_ADD,exitFds[0],&event);
    int newFd,ret;
    int readyFdNum,i;
    Train_t train;
    int dataLen;
    char logbuf[128]={0};
    pque_t pq=&fac.que;
    while(1)
    {
        readyFdNum=epoll_wait(epfd,evs,2,-1);
        for(i=0;i<readyFdNum;++i)
        {
            if(evs[i].data.fd==socketFd)
            {
                Person_t person;
                int a;
                newFd=accept(socketFd,NULL,NULL);//用户连接后，验证用户名
            welcome:
                ret=recv(newFd,&a,4,0);
                if(ret>0)
                {
                    if(a==1)//登录
                    {
                    checkname:
                        personInit(&person);
                        bzero(logbuf,sizeof(logbuf));
                        recvCycle(newFd,&dataLen,4);//先接火车头里的数字
                        ret=recvCycle(newFd,logbuf,dataLen);//接文件内容
                        //printf("name=%s,ret=%d\n",logbuf,ret);
                        if(ret>0)
                        {
                            ret=checkname(logbuf,&person);//存在返回1，不存在返回0
                            //printf("ret=%d,ID=%d\n",ret,person.ID);
                            send(newFd,&ret,4,0);
                            if(ret==1)//存在该用户
                            {
                                int key=0,k=0;
                                char cryptcodebuf[200]={0};
                                train.dataLen=strlen(person.salt);
                                strcpy(train.buf,person.salt);
                                send(newFd,&train,4+train.dataLen,0);//发送盐值
                            cryptcode:
                                recv(newFd,&dataLen,4,0);
                                recv(newFd,cryptcodebuf,dataLen,0);//接收密文
                                if(strcmp(cryptcodebuf,person.cryptcode)==0)//密码正确
                                {
                                    key=1;
                                    send(newFd,&key,4,0);//发送密码正确信号
                                    //while(1);
                                    pNode_t pnew=(pNode_t)calloc(1,sizeof(Node_t));
                                    pnew->newFd=newFd;
                                    pnew->userID=person.ID;
                                    pthread_mutex_lock(&pq->mutex);
                                    queInsert(pq,pnew);//任务放入队列
                                    printf("%d connect\n",++cnt);
                                    pthread_mutex_unlock(&pq->mutex);
                                    pthread_cond_signal(&fac.cond);//唤醒一个子线程
                                }else{
                                    k++;
                                    if(k<3)
                                    {
                                        send(newFd,&key,4,0);//发送密码错误命令
                                        goto cryptcode;
                                    }
                                }
                            }else if(ret==0)//不存在该用户
                            {
                                //printf("2\n");
                                goto checkname;
                            }
                        }
                    }else if(a==2)//注册
                    {
                        registeruser(newFd,&person);
                        printf("insert success\n");
                        goto welcome;//注册后则跳转回登录位置
                    }
                }
            }
            if(evs[i].data.fd==exitFds[0])
            {
                printf("strat exit\n");
                fac.startFlag=0;
                for(i=0;i<fac.thread_num;++i)
                {
                    pthread_cond_signal(&fac.cond);
                }
                for(i=0;i<fac.thread_num;++i)
                {
                    pthread_join(fac.pthid[i],NULL);
                }
                exit(0);
            }
        }
    }
}
