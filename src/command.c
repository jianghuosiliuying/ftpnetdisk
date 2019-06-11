#include "factory.h"

int commandFunc(int key,int newFd,char* buf2,Dir_t* pdir,Dir_t* ppredir,Dir_t* ptmpdir,int ID)
{
    //printf("buf2=%s,key=%d,userID=%d\n",buf2,key,ID);
    char table[10]={0};
    char aim[128]={0};
    char aimthing[128]={0};
    char condition[200]={0};
    char md5[50]={0};
    int ret,dataLen;
    Train_t train;
    pFile_t phead=NULL;
    pFile_t ptail=NULL;
    if(key==1)//ls
    {
        printf("enter select 1\n");
        strncpy(table,"file",4);
        strncpy(aim,"*",1);
        sprintf(condition,"where precode=%d and belong=%d",(*pdir).code,ID);//返回链表
        ret=query2(aim,table,condition,&phead,&ptail);
        //printf("ret=%d\n",ret);
        int r1=send(newFd,&ret,4,0);//发送查询结果命令
        ERROR_CHECK(r1,-1,"send");
        //printf("send over,r1=%d\n",r1); 
        if(ret>0)
        {
            ls_t file[ret];
            bzero(file,sizeof(file));
            int i=0;
            //printf("ret=%d,i=%d,sizeof(file)=%ld\n",ret,i,sizeof(file));
            while(phead)
            {
                strncpy(file[i].filetype,phead->filetype,1);
                strncpy(file[i].filename,phead->filename,strlen(phead->filename));
                file[i].size=phead->size;
                i++;
                phead=phead->pNext;
            }
            //i=0;
            //while(i<ret)
            //{
            //    printf("%s %s %d\n",file[i].filetype,file[i].filename,file[i].size);
            //    i++;
            //}
            //printf("ret=%d,i=%d,sizeof(file)=%ld\n",ret,i,sizeof(file));
            recv(newFd,&key,4,0);//接收客户端准备就绪命令
            send(newFd,file,sizeof(file),0);//发送ls结果
            recv(newFd,&ret,4,0);//等待客户端完成接收
        }
    }else if(key==2)//pwd
    {
        printf("enter select 2\n");
        train.dataLen=strlen((*pdir).path);
        strcpy(train.buf,(*pdir).path);
        send(newFd,&train,4+train.dataLen,0);
    }else if(key==3)//cd
    {
        printf("enter select 3\n");
        if(strcmp("..",buf2)==0||strcmp("../",buf2)==0)
        {
            if((*pdir).code==0)
            {
                int i=1;
                send(newFd,&i,4,0);
                train.dataLen=strlen((*pdir).path);
                strcpy(train.buf,(*pdir).path);
                send(newFd,&train,4+train.dataLen,0);
            }
            else{
                strncpy(table,"file",4);
                strncpy(aim,"*",1);
                sprintf(condition,"where code=%d",(*pdir).code);//找到code，既是上一级
                ret=query2(aim,table,condition,&phead,&ptail);
                if(ret==1)//查询结果只可能有一种，因为code唯一，不重复
                {
                    bzero((*ppredir).path,sizeof((*ppredir).path));
                    memcpy(ppredir,pdir,sizeof(Dir_t));
                    int i,k=0;
                    for(i=strlen((*pdir).path)-1;i>27;--i)//27为disk根目录
                    {
                        if((*pdir).path[i]=='/'&&k!=0)
                        {
                            break;
                        }
                        k++;
                    }//将路径后的一个dir去除，如/dir1/--->/
                    bzero((*pdir).path,sizeof((*pdir).path));
                    strncpy((*pdir).path,(*ppredir).path,strlen((*ppredir).path)-k);
                    (*pdir).code=phead->precode;
                    send(newFd,&ret,4,0);
                    train.dataLen=strlen((*pdir).path);
                    strcpy(train.buf,(*pdir).path);
                    send(newFd,&train,4+train.dataLen,0);
                }
            }
        }else if(strcmp("-",buf2)==0)
        {
            bzero((*ptmpdir).path,sizeof((*ptmpdir).path));//code和FD直接覆盖即可
            memcpy(ptmpdir,pdir,sizeof(Dir_t));//将当前赋给临时
            bzero((*pdir).path,sizeof((*pdir).path));//code和FD直接覆盖即可
            memcpy(pdir,ppredir,sizeof(Dir_t));//将过去赋给当前
            bzero((*ppredir).path,sizeof((*ppredir).path));
            memcpy(ppredir,ptmpdir,sizeof(Dir_t));
            int i=1;
            send(newFd,&i,4,0);
            train.dataLen=strlen((*pdir).path);
            strcpy(train.buf,(*pdir).path);
            send(newFd,&train,4+train.dataLen,0);
        }else if(strcmp("~",buf2)==0||strcmp("/",buf2)==0||strcmp("~/",buf2)==0)
        {
            int i=1;
            bzero((*ppredir).path,sizeof((*ppredir).path));
            memcpy(ppredir,pdir,sizeof(Dir_t));
            (*pdir).code=0;
            bzero((*pdir).path,sizeof((*pdir).path));
            strncpy((*pdir).path,(*pdir).root,strlen((*pdir).root));
            send(newFd,&i,4,0);
            train.dataLen=strlen((*pdir).path);
            strcpy(train.buf,(*pdir).path);
            send(newFd,&train,4+train.dataLen,0);
        }else if(strcmp(".",buf2)==0)
        {
            int i=1;
            send(newFd,&i,4,0);
            train.dataLen=strlen((*pdir).path);
            strcpy(train.buf,(*pdir).path);
            send(newFd,&train,4+train.dataLen,0);
        }
        else{
            strncpy(table,"file",4);
            strncpy(aim,"*",1);
            sprintf(condition,"where filetype='d' and filename='%s' and precode=%d and belong=%d",buf2,(*pdir).code,ID);//待改进
            ret=query2(aim,table,condition,&phead,&ptail);
            if(ret==0)//不存在的文件或文件名，不能cd
            {
                send(newFd,&ret,4,0);
            }else if(ret==1)//查询结果只可能有一种
            {
                bzero((*ppredir).path,sizeof((*ppredir).path));
                memcpy(ppredir,pdir,sizeof(Dir_t));
                sprintf((*pdir).path,"%s%s/",(*pdir).path,buf2);
                (*pdir).code=phead->code;
                send(newFd,&ret,4,0);
                train.dataLen=strlen((*pdir).path);
                strcpy(train.buf,(*pdir).path);
                send(newFd,&train,4+train.dataLen,0);
            }
        }
    }else if(key==4)//客户端传到服务器
    {
        printf("enter select 4\n");
        bzero(table,sizeof(table));
        bzero(aim,sizeof(aim));
        bzero(condition,sizeof(condition));
        strncpy(table,"file",4);
        strncpy(aim,"*",1);
        sprintf(condition,"where precode=%d and filename='%s' and filetype='f' and belong=%d",(*pdir).code,buf2,ID);//查询是否存在同样的文件
        ret=query2(aim,table,condition,&phead,&ptail);//查看是否有同名文件
        //printf("queryfile ret=%d\n",ret);
        send(newFd,&ret,4,0);
        if(ret==1)//存在同名文件，不能上传
        {
        }else if(ret==0)//允许上传，先接受md5，查找md5是否存在于文件池
        {
            recv(newFd,&ret,4,0);//接收客户端发送md5指令
            if(ret==-100)//防止客户端打开文件挂掉
            {
                bzero(table,sizeof(table));
                bzero(aim,sizeof(aim));
                bzero(condition,sizeof(condition));
                bzero(md5,sizeof(md5));
                recvCycle(newFd,&dataLen,4);
                recvCycle(newFd,md5,dataLen);
                strncpy(table,"file",4);
                strncpy(aim,"*",1);
                sprintf(condition,"where md5='%s'",md5);//查询是否存在同样md5码的文件
                ret=query2(aim,table,condition,&phead,&ptail);//文件只有一份，数据库中MD5可以有多个
                send(newFd,&ret,4,0);
                if(ret>0)//秒传
                {
                    bzero(table,sizeof(table));
                    bzero(aimthing,sizeof(aimthing));
                    bzero(condition,sizeof(condition));
                    strncpy(table,"file",4);
                    strncpy(aimthing,"precode,filename,filetype,belong,md5,size",41);
                    sprintf(condition,"%d,'%s','f',%d,'%s',%d",(*pdir).code,buf2,ID,md5,phead->size);//插入文件
                    ret=fileInsert(table,aimthing,condition);//插入成功返回0
                }else if(ret==0){//不存在相同md5码的文件，普通上传
                    int i=1;
                    send(newFd,&i,4,0);//开始接收
	                int fd=open(md5,O_CREAT|O_RDWR,0666);
                    ERROR_CHECK(fd,-1,"open");
                    struct stat file;
                    fstat(fd,&file);
                    off_t truesize=file.st_size;
                    int size=download(newFd,fd,truesize);//服务器接收文件
                    //printf("download over,size=%d\n",size);
                    bzero(table,sizeof(table));
                    bzero(aimthing,sizeof(aimthing));
                    bzero(condition,sizeof(condition));
                    strncpy(table,"file",4);
                    strncpy(aimthing,"precode,filename,filetype,belong,md5,size",41);
                    sprintf(condition,"%d,'%s','%s',%d,'%s',%d",(*pdir).code,buf2,"f",ID,md5,size);//插入文件
                    ret=fileInsert(table,aimthing,condition);//插入成功返回0
                    send(newFd,&ret,4,0);//完成接收
                }
            }
        }
    }else if(key==5)//服务器发给客户端
    {
        printf("enter select 5\n");
        strncpy(table,"file",4);
        strncpy(aim,"*",1);
        sprintf(condition,"where filetype='f' and filename='%s' and precode=%d and belong=%d",buf2,(*pdir).code,ID);//判断是否存在该file
        ret=query2(aim,table,condition,&phead,&ptail);//同一份文件MD5，数据库可以存多次，此处严格查询，结果唯一
        send(newFd,&ret,4,0);
        //printf("gets file ret=%d,md5=%s\n",ret,phead->md5);
        //printf("pwd=%s\n",getcwd(NULL,0));
        if(ret==0)//数据库没有该文件
        {
        }else if(ret==1)
        {
            strncpy(md5,phead->md5,strlen(phead->md5));
            tranFile(newFd,buf2,md5);//发文件
            //printf("tranFile over\n");
        }
    }else if(key==6)//remove file
    {
        printf("enter select 6\n");
        strncpy(table,"file",5);
        strncpy(aim,"*",1);
        sprintf(condition,"where filetype='f' and filename='%s' and precode=%d and belong=%d",buf2,(*pdir).code,ID);//查文件是否存在
        ret=query2(aim,table,condition,&phead,&ptail);
        send(newFd,&ret,4,0);
        if(ret==0)//不存在该文件
        {
        }else if(ret==1)//查询结果只可能有一种
        {
            int r=ret;
            bzero(table,sizeof(table));
            bzero(condition,sizeof(condition));
            strncpy(table,"file",5);
            sprintf(condition,"where code=%d",phead->code);//删除文件
            ret=deletefile(table,condition);//删除数据库该文件记录
            //printf("delete row=%d\n",ret);
            bzero(table,sizeof(table));
            bzero(aim,sizeof(aim));
            bzero(condition,sizeof(condition));
            strncpy(table,"file",5);
            strncpy(aim,"*",1);
            sprintf(condition,"where md5='%s'",phead->md5);
            ret=query2(aim,table,condition,&phead,&ptail);//搜索该文件引用数目
            if(ret==0)
            {
                unlink(phead->md5);//解除硬链接
            }
            send(newFd,&r,4,0);//发送完成删除信号
        }
    }else if(key==7)//创建目录mkdir
    {
        printf("enter select 7\n");
        bzero(table,sizeof(table));
        bzero(aim,sizeof(aim));
        bzero(condition,sizeof(condition));
        strncpy(table,"file",5);
        strncpy(aim,"*",1);
        sprintf(condition,"where filetype='d' and filename='%s' and precode=%d and belong=%d",buf2,(*pdir).code,ID);//查文件是否存在
        ret=query2(aim,table,condition,&phead,&ptail);//查询是否存在该目录
        send(newFd,&ret,4,0);
        if(ret>0)//存在该目录
        {
        }else if(ret==0)
        {
            bzero(table,sizeof(table));
            bzero(aimthing,sizeof(aimthing));
            bzero(condition,sizeof(condition));
            strncpy(table,"file",5);
            strncpy(aimthing,"precode,filename,filetype,belong",32);
            sprintf(condition,"%d,'%s','%s',%d",(*pdir).code,buf2,"d",ID);//插入文件
            ret=fileInsert(table,aimthing,condition);
            send(newFd,&ret,4,0);
        }
    }else if(key==8)//rmdir
    {
        printf("enter select 8\n");
        bzero(table,sizeof(table));
        bzero(aim,sizeof(aim));
        bzero(condition,sizeof(condition));
        strncpy(table,"file",5);
        strncpy(aim,"*",1);
        sprintf(condition,"where filetype='d' and filename='%s' and precode=%d and belong=%d",buf2,(*pdir).code,ID);//查文件是否存在
        ret=query2(aim,table,condition,&phead,&ptail);
        send(newFd,&ret,4,0);
        if(ret==0)//不存在该目录
        {
        }else if(ret==1)//查询结果只可能有一种
        {
            int code=phead->code;
            bzero(table,sizeof(table));
            bzero(aim,sizeof(aim));
            bzero(condition,sizeof(condition));
            strncpy(table,"file",5);
            strncpy(aim,"*",1);
            sprintf(condition,"where precode=%d and belong=%d",code,ID);
            int r=query2(aim,table,condition,&phead,&ptail);//查询该目录是否还有文件
            send(newFd,&r,4,0);
            if(r>0)
            {
                //printf("not empty dir,find %d file or dir\n",r);
            }else if(r==0)
            {
                bzero(table,sizeof(table));
                bzero(condition,sizeof(condition));
                strncpy(table,"file",5);
                sprintf(condition,"where code=%d",code);//只删空目录
                r=deletefile(table,condition);
                //printf("delete row=%d\n",ret);
                send(newFd,&r,4,0);
            }
        }
    }else{
        printf("error command key=%d\n",key);
    }
    return 0;
}

