#include "head.h"
#define SPLICEBLOCK 4096

int tranFile(int newFd,char* buf2)//发送端
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
    int fd=open(buf2,O_RDONLY);
    ERROR_CHECK(fd,-1,"open");
    //printf("filename=%s\n",filename);
    //发送文件大小
    //printf("send file size\n");
    struct stat buf;
    fstat(fd,&buf);
    int fileSize=0,uploadsize=0,oldSize=0,sliceSize;
    fileSize=buf.st_size;
    train.dataLen=sizeof(buf.st_size);
    memcpy(train.buf,&buf.st_size,train.dataLen);
    ret=send(newFd,&train,4+train.dataLen,0);
    ERROR_CHECK(ret,-1,"send");
    //接收服务器已缓存大小
    off_t pos;
    recvCycle(newFd,&dataLen,4);
    recvCycle(newFd,&pos,dataLen);
    //printf("filesize=%ld\n",buf.st_size);
    //发文件内容
    //printf("send file content\n");
    struct timeval start,end;
    long t;
    lseek(fd,pos,SEEK_SET);
    gettimeofday(&start,NULL);
    uploadsize=pos;
    sliceSize=fileSize/10000;
    if(fileSize>100<<20)
    {
        char* pMap=(char*)mmap(NULL,fileSize-pos,PROT_READ,MAP_SHARED,fd,0);
        ret=send(newFd,pMap,fileSize-pos,0);//直接全部发送
        ERROR_CHECK(ret,-1,"send");
        gettimeofday(&end,NULL);
        uploadsize+=ret;
        t=(end.tv_sec-start.tv_sec)*1000000+end.tv_usec-start.tv_usec;
        printf("\ruploadloadfile<----%s  %5.2f%%    %5.2f M/s  ",filename,(float)uploadsize/fileSize*100,(float)((fileSize-pos)/t));
        //发送结束标志
        train.dataLen=0;
        ret=send(newFd,&train,4,0);
        ERROR_CHECK(ret,-1,"send");
        munmap(pMap,fileSize-pos);
    }else if(fileSize>=0&&fileSize<=100<<20){
        while((train.dataLen=read(fd,train.buf,sizeof(train.buf))))
        {
            uploadsize+=train.dataLen;
            if(uploadsize-oldSize>sliceSize)
            {
                gettimeofday(&end,NULL);
                t=(end.tv_sec-start.tv_sec)*1000000+end.tv_usec-start.tv_usec;
                printf("\ruploadloadfile<----%s  %5.2f%%    %5.2f M/s  ",filename,(float)uploadsize/fileSize*100,(float)(uploadsize/t));
                fflush(stdout);
                oldSize=uploadsize;
            }
            ret=send(newFd,&train,4+train.dataLen,0);
            ERROR_CHECK(ret,-1,"send");
        }
        //发送结束标志
        ret=send(newFd,&train,4,0);
        ERROR_CHECK(ret,-1,"send");
    }
    close(fd);
    return 0;
}

int download(int psocketFd)//接收端
{
    int dataLen;
    Train_t train;
	char buf[4096]={0};
    char filename[128]={0};
    //接收文件名
    recvCycle(psocketFd,&dataLen,4);
    recvCycle(psocketFd,buf,dataLen);
    bzero(filename,sizeof(filename));
    strncpy(filename,buf,strlen(buf));
	int fd=open(filename,O_CREAT|O_WRONLY,0666);
    ERROR_CHECK(fd,-1,"open");
    //接收文件大小
    off_t fileSize=0,oldSize=0,downloadSize=0,sliceSize;
    recvCycle(psocketFd,&dataLen,4);
    recvCycle(psocketFd,&fileSize,dataLen);
    //发送本地实际大小
    struct stat file;
    fstat(fd,&file);
    train.dataLen=sizeof(file.st_size);
    memcpy(train.buf,&file.st_size,train.dataLen);
    send(psocketFd,&train,4+train.dataLen,0);
    //接收文件内容
    int ret;
    int fds[2];
    pipe(fds);
    struct timeval start,end;
    long t;
    oldSize=downloadSize=file.st_size;
    lseek(fd,0,SEEK_END);
    gettimeofday(&start,NULL);
    sliceSize=fileSize/10000;
    if(fileSize>100<<20)
    {
        while(downloadSize+4096<fileSize)
        {
            ret=splice(psocketFd,NULL,fds[1],NULL,SPLICEBLOCK,SPLICE_F_NONBLOCK|SPLICE_F_MORE);
            ERROR_CHECK(ret,-1,"splice1");
            ret=splice(fds[0],NULL,fd,NULL,SPLICEBLOCK,SPLICE_F_NONBLOCK|SPLICE_F_MORE);
            ERROR_CHECK(ret,-1,"splice2");
            downloadSize+=ret;
            if(downloadSize-oldSize>sliceSize)
            {
                gettimeofday(&end,NULL);
                t=(end.tv_sec-start.tv_sec)*1000000+end.tv_usec-start.tv_usec;
                printf("\rdownloadfile---->%s  %5.2f%%     %5.2f M/s ",filename,(float)downloadSize/fileSize*100,(float)(downloadSize/t));
                fflush(stdout);
                oldSize=downloadSize;
            }
        }
        dataLen=fileSize-downloadSize;//接不足一整车的货物
        ret=splice(psocketFd,NULL,fds[1],NULL,dataLen,SPLICE_F_NONBLOCK|SPLICE_F_MORE);
        ERROR_CHECK(ret,-1,"splice3");
        ret=splice(fds[0],NULL,fd,NULL,dataLen,SPLICE_F_NONBLOCK|SPLICE_F_MORE);
        ERROR_CHECK(ret,-1,"splice4");
        printf("\rdownloadfile---->%s  100.00%%    %5.2f M/s    \n",filename,(float)(downloadSize/t));//最后一车的货物，忽略不计入下载速度
        printf("splice download success\n");
        ret=recvCycle(psocketFd,buf,4);//接收结束指令
        if(-1==ret)
        {
            printf("error recv\n");
        }
    }else if(fileSize>=0&&fileSize<=100<<20)
    {
        while(1)
        {
            ret=recvCycle(psocketFd,&dataLen,4);
            if(ret==-1)
            {
               printf("server is update\n");
               break;
            }
            if(dataLen>0)
            {
                ret=recvCycle(psocketFd,buf,dataLen);
                if(ret==-1)
                {
                    break;
                }
                write(fd,buf,dataLen);
                downloadSize+=dataLen;
                if(downloadSize-oldSize>sliceSize)
                {
                    gettimeofday(&end,NULL);
                    t=(end.tv_sec-start.tv_sec)*1000000+end.tv_usec-start.tv_usec;
                    printf("\rdownloadfile---->%s  %5.2f%%    %5.2fM/s",filename,(float)downloadSize/fileSize*100,(float)(downloadSize/t));
                    fflush(stdout);
                    oldSize=downloadSize;
                }
            }else{
                printf("\rdownloadfile---->%s  100.00%%    %5.2fM/s    \n",filename,(float)(downloadSize/t));
                printf("train download success\n");
                break;
            }
        }
    }
    close(fd);
    return 0;
}
