#include "head.h"

int main(int argc,char* argv[])
{
	ARGC_CHECK(argc,3);
	int socketFd=socket(AF_INET,SOCK_STREAM,0);
	ERROR_CHECK(socketFd,-1,"socket");
	struct sockaddr_in ser;
	bzero(&ser,sizeof(ser));
	ser.sin_family=AF_INET;
	ser.sin_port=htons(atoi(argv[2]));
	ser.sin_addr.s_addr=inet_addr(argv[1]);//点分十进制转为32位网络字节序
	int ret=connect(socketFd,(struct sockaddr*)&ser,sizeof(ser));
	ERROR_CHECK(ret,-1,"connect");
	int a;
    Train_t train;
    int dataLen;
    printf("connect success\n");
welcome:
    printf("****************************\n");
    printf("        1.log in\n");
    printf("        2.register\n");
    printf("        3.exit\n");
    printf("****************************\n");
    printf("choose:");
    setbuf(stdin,NULL);
    scanf("%d",&a);
    char logbuf[128]={0};
    send(socketFd,&a,4,0);
    if(a==1)//用户登录验证
    {
    namecheck:
        setbuf(stdin,NULL);
        memset(logbuf,0,sizeof(logbuf));
        printf("------------------------------------------\n");
        printf("enter your name:");
        scanf("%s",logbuf);//会自动忽略末尾的\n
        train.dataLen=strlen(logbuf);
        strcpy(train.buf,logbuf);
        int r=send(socketFd,&train,4+train.dataLen,0);//发送用户名
        ERROR_CHECK(r,-1,"send");
        recv(socketFd,&ret,4,0);//接收姓名验证
        //printf("name=%s,r=%d\n",logbuf,r);
        setbuf(stdin,NULL);
        if(ret==1)
        {
            int key;
            char saltbuf[200]={0};
            char cryptcodebuf[200]={0};
            recvCycle(socketFd,&dataLen,4);
            recvCycle(socketFd,saltbuf,dataLen);//接收盐值
            int k=0;
            char *passwd;
        passwd:
            passwd=getpass("enter your passwd:");
            strncpy(cryptcodebuf,crypt(passwd,saltbuf),strlen(crypt(passwd,saltbuf)));
            train.dataLen=strlen(cryptcodebuf);
            strcpy(train.buf,cryptcodebuf);
            send(socketFd,&train,4+train.dataLen,0);//发送密文
            recv(socketFd,&key,4,0);
            if(key==1)//密码正确
            {
                printf("login success\n");
                printf("------------------------------------------\n");
                commandFunc(socketFd);
            }else if(key==0)//密码错误
            {
                k++;
                printf("the %d time error passwd,try again\n",k);
                if(k==3)
                {
                    return 0;
                }
                goto passwd;
            }
        }else if(ret==0)
        {
            printf("don't find your name\n");
            goto namecheck;
        }
    }else if(a==2){//注册用户
        char name[128]={0};
        int key;
    name:
        printf("enter your name:");
        scanf("%s",name);
        printf("name=%s\n",name);
        train.dataLen=strlen(name);
        strcpy(train.buf,name);
        send(socketFd,&train,4+train.dataLen,0);
        recv(socketFd,&key,4,0);
        if(key==1)//已经存在该用户名
        {
            printf("there have a same name\n");
            goto name;
        }else if(key==0)//无同名，可以进行注册
        {
            char salt[128]={0};
            char saltbuf[128]={0};
            char cryptcode[200]={0};
            char *passwd1;
            getsalt(salt);//得到盐值随机数
            sprintf(saltbuf,"$6$%s",salt);
            //printf("salt=%s\n",saltbuf);
            passwd1=getpass("enter your passwd:");
            //printf("passwd1=%s\n",passwd1);
            train.dataLen=strlen(saltbuf);
            strcpy(train.buf,saltbuf);
            send(socketFd,&train,4+train.dataLen,0);//发送盐值
            strncpy(cryptcode,crypt(passwd1,saltbuf),strlen(crypt(passwd1,saltbuf)));
            //printf("cryptcode=%s\n",cryptcode);
            train.dataLen=strlen(cryptcode);
            strcpy(train.buf,cryptcode);
            send(socketFd,&train,4+train.dataLen,0);//发送密文
            recv(socketFd,&key,4,0);//接收添加成功命令
            printf("rigister success\n");
            goto welcome;
        }
    }else if(a==3){
        printf("byebye\n");
        return 0;
    }else{
        printf("error select\n");
        goto welcome;
    }
    close(socketFd);
	return 0;
}
