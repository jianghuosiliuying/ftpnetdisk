#include "head.h"

int tranFile(int newFd,char* buf2,char* md5)//发送端
{
    Train_t train;
    int dataLen;
    char filename[128]={0};
    strncpy(filename,buf2,strlen(buf2));
    //printf("filename=%s\n",filename);
    //发送文件名
    //printf("send file name\n");
    train.dataLen=strlen(filename);
    strcpy(train.buf,filename);
    int ret=send(newFd,&train,4+train.dataLen,0);//火车头整型数占4个字节
    ERROR_CHECK(ret,-1,"send");
    int fd=open(md5,O_RDONLY);//实际文件存储其MD5码
    ERROR_CHECK(fd,-1,"open");
    //发送文件大小
    //printf("send file size\n");
    struct stat buf;
    fstat(fd,&buf);
    int fileSize=buf.st_size;
    train.dataLen=sizeof(buf.st_size);
    memcpy(train.buf,&buf.st_size,train.dataLen);
    ret=send(newFd,&train,4+train.dataLen,0);
    ERROR_CHECK(ret,-1,"send");
    //接收客户端本地已缓存大小
    off_t pos;
    recvCycle(newFd,&dataLen,4);
    recvCycle(newFd,&pos,dataLen);
    lseek(fd,pos,SEEK_SET);
    //发文件内容
    //printf("send file content\n");
    if(fileSize>100<<20)//超过100M使用sendfile
    {
        ret=sendfile(newFd,fd,NULL,fileSize-pos);//大货车一次全部发出
        ERROR_CHECK(ret,-1,"sendfile");
    }else if(fileSize>=0&&fileSize<=100<<20){
        while((train.dataLen=read(fd,train.buf,sizeof(train.buf))))
        {
            ret=send(newFd,&train,4+train.dataLen,0);
            ERROR_CHECK(ret,-1,"send");
        }
    }
    //发送结束标志
    //printf("send file end\n");
    ret=send(newFd,&train,4,0);
    ERROR_CHECK(ret,-1,"send");
    close(fd);
    return 0;
}

int download(int socketFd,int fd,off_t truesize)//接收方
{
    int dataLen;
    Train_t train;
	char buf[4096]={0};
    char filename[128]={0};
    //接收文件名
    //printf("begin recv name\n");
    recvCycle(socketFd,&dataLen,4);//火车长度
    recvCycle(socketFd,buf,dataLen);//火车内容
    bzero(filename,sizeof(filename));
    strncpy(filename,buf,strlen(buf));
    //printf("filename=%s\n",filename);
    //接收文件大小
    //printf("begin recv size\n");
    off_t fileSize=0;
    recvCycle(socketFd,&dataLen,4);
    recvCycle(socketFd,&fileSize,dataLen);
    //printf("filesize=%ld\n",fileSize);
    //发送服务器已缓存大小
    train.dataLen=sizeof(truesize);
    memcpy(train.buf,&truesize,train.dataLen);
    int ret=send(socketFd,&train,4+train.dataLen,0);
    ERROR_CHECK(ret,-1,"send");
    lseek(fd,truesize,SEEK_SET);
    //接收文件内容
    //printf("begin recv content\n");
    if(fileSize>100<<20)
    {
        ftruncate(fd,fileSize-truesize);
        char* pMap=(char*)mmap(NULL,fileSize-truesize,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
        ret=recvCycle(socketFd,pMap,fileSize-truesize);
        if(ret==-1)
        {
            printf("recv error\n");
        }
        ret=recvCycle(socketFd,buf,4);//接收结束标志
        if(ret==-1)
        {
            printf("recv error\n");
        }
        munmap(pMap,fileSize-truesize);
    }else if(fileSize>=0&&fileSize<=100<<20)
    {
        while(1)
        {
            ret=recvCycle(socketFd,&dataLen,4);
            if(ret==-1)
            {
               printf("server is update\n");
               break;
            }
            if(dataLen>0)
            {
                ret=recvCycle(socketFd,buf,dataLen);
                if(ret==-1)
                {
                    break;
                }
                write(fd,buf,dataLen);
            }else{
                break;
            }
        }
    }
    //printf("recv over\n");
    return fileSize;
}
