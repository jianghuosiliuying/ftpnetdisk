#include "head.h"

int commandFunc(int socketFd)
{
    Train_t train;
    int ret,dataLen;
    //printf("welcome to ues mynetdisk!\n");
    char opbuf[128]={0};
    char buf1[20]={0};
    char buf2[128]={0};
command:
    printf("enter your command:");
    bzero(opbuf,sizeof(opbuf));
    bzero(buf1,sizeof(buf1));
    bzero(buf2,sizeof(buf2));
    setbuf(stdin,NULL);
    fgets(opbuf,sizeof(opbuf),stdin);//包含末尾的\n
    //printf("opbuf=%s",opbuf);//opbuf里包含\n
    ret=anasy(opbuf,strlen(opbuf)-1,buf1,buf2);//返回buf1对应功能的数字
    //printf("ret=%d,buf1=%s,buf2=%s\n",ret,buf1,buf2);
    if(ret>0&&ret<9)//正确命令才发送
    {
        send(socketFd,&ret,4,0);//发送ret
        if(ret>=3&&ret<=8)//ls1和pwd1不发送buf2,不正确命令9也忽略
        {
            train.dataLen=strlen(buf2);
            strcpy(train.buf,buf2);
            send(socketFd,&train,4+train.dataLen,0);//发送文件名或路径
        }
        if(ret==1)//ls
        {
            printf("------------------------------------------1\n");
            int r1,r,i=0;
            r=recv(socketFd,&r1,4,0);//接收ls查询结果多少行
            ERROR_CHECK(r,-1,"recv");
            //printf("recv over,r=%d\n",r);
            if(r>0)
            {
                //printf("r1=%d\n",r1);
                if(r1==0)//空文件夹
                {
                    printf("empty dir\n");
                    printf("------------------------------------------1\n");
                    goto command;
                }
                else if(r1>0)
                {
                    ls_t file[r1];
                    bzero(file,sizeof(file));
                    //printf("ret=%d,sizeof(file)=%ld\n",ret,sizeof(file));
                    send(socketFd,&ret,4,0);//发送准备接收命令
                    recv(socketFd,file,sizeof(file),0);//接收ls查询结果
                    printf("    type  name       size\n");
                    while(i<r1)
                    {
                        printf("     %s    %-10s %d\n",file[i].filetype,file[i].filename,file[i].size);
                        i++;
                    }
                    send(socketFd,&r1,4,0);//完成接收命令
                    printf("------------------------------------------1\n");
                    goto command;
                }
            }
        }
        if(ret==2)//pwd
        {
            printf("------------------------------------------2\n");
            char tmppath[128]={0};
            char path[128]={0};
            recvCycle(socketFd,&dataLen,4);
            recvCycle(socketFd,tmppath,dataLen);
            int j=0;
            for(int i=27;i<(int)strlen(tmppath);++i)
            {
                path[j++]=tmppath[i];
            }//将地址前的27个字节去除,只留/ 
            puts(path);
            printf("------------------------------------------2\n");
            goto command;
        }
        if(ret==3)//cd
        {
            printf("------------------------------------------3\n");
            char tmppath[128]={0};
            char path[128]={0};
            int key;
            recv(socketFd,&key,4,0);
            if(key==1)//存在该地址
            {
                recvCycle(socketFd,&dataLen,4);
                recvCycle(socketFd,tmppath,dataLen);//接收结果路径
                int j=0;
                for(int i=27;i<(int)strlen(tmppath);++i)
                {
                    path[j++]=tmppath[i];
                }//将地址前的27个字节去除,只留/
                puts(path);
            }else if(key==0)
            {
                printf("error path\n");
            }
            printf("------------------------------------------3\n");
            goto command;
        }
        if(ret==4)//上传
        {
            printf("------------------------------------------4\n");
            FILE *fp = fopen(buf2,"rb");
            if(fp==NULL)//检查本地是否有该文件名
            {
                printf("file open fail,check the file name\n");
                printf("------------------------------------------4\n");
                goto command;
            }
            int key;
            recv(socketFd,&key,4,0);
            if(key==1)
            {
                printf("threre have a same name file\n");
                printf("------------------------------------------4\n");
                goto command;
            }else if(key==0)
            {
                printf("seaching,please wait...\n");
                char file_md5[128] = {0};
                int r=-100;
                send(socketFd,&r,4,0);
                compute_file_md5(fp, file_md5);
                train.dataLen=strlen(file_md5);
                strcpy(train.buf,file_md5);
                send(socketFd,&train,4+train.dataLen,0);
                fclose(fp);
                recv(socketFd,&r,4,0);
                if(r>0)
                {
                    printf("quikly tranfile success\n");
                    printf("------------------------------------------4\n");
                    goto command;
                }else if(r==0)
                {
                    recv(socketFd,&r,4,0);//开始上传
                    printf("begin common tranfile\n");
                    tranFile(socketFd,buf2);
                    recv(socketFd,&r,4,0);//完成上传
                    printf("\ncommon tranfile success\n");
                    printf("------------------------------------------4\n");
                    goto command;
                }
            }
        }
        if(ret==5)//下载
        {
            printf("------------------------------------------5\n");
            int key;
            recv(socketFd,&key,4,0);
            if(key==0)
            {
                printf("dont't find %s in dir\n",buf2);
                printf("------------------------------------------5\n");
                goto command;
            }else if(key==1)
            {
                download(socketFd);
                //printf("download success\n");
                printf("------------------------------------------5\n");
                goto command;
            }
        }
        if(ret==6)//remove
        {
            printf("------------------------------------------6\n");
            int key;
            recv(socketFd,&key,4,0);
            if(key==0)
            {
                printf("don't find %s in dir\n",buf2);
                printf("------------------------------------------6\n");
                goto command;
            }else if(key==1)
            {
                recv(socketFd,&key,4,0);
                printf("remove success\n");
                printf("------------------------------------------6\n");
                goto command;
            }
        }
        if(ret==7)//mkdir
        {
            printf("------------------------------------------7\n");
            int key;
            recv(socketFd,&key,4,0);
            if(key>0)
            {
                printf("there have a same dir\n");
                printf("------------------------------------------7\n");
                goto command;
            }else if(key==0)
            {
                recv(socketFd,&key,4,0);
                printf("mkdir success\n");
                printf("------------------------------------------7\n");
                goto command;
            }
        }
        if(ret==8)//rmdir
        {
            printf("------------------------------------------8\n");
            int key;
            recv(socketFd,&key,4,0);
            if(key==0)
            {
                printf("don't find %s in path\n",buf2);
                printf("------------------------------------------8\n");
                goto command;
            }else if(key==1)
            {
                int r;
                recv(socketFd,&r,4,0);
                if(r>0)
                {
                    printf("not emptry,can't delete\n");
                    printf("------------------------------------------8\n");
                    goto command;
                }else if(r==0)
                {
                    recv(socketFd,&r,4,0);
                    printf("rmdir success\n");
                    printf("------------------------------------------8\n");
                    goto command;
                }
            }
        }
    }else if(ret==9)
    {
        printf("------------------------------------------9\n");
        printf("error command\n");
        printf("------------------------------------------9\n");
        goto command;
    }
    return 0;
}
