#include "head.h"

int anasy(char* opbuf,int len,char* buf1,char* buf2)
{
    //printf("len=%d,opbuf=%s,buf1=%s,buf2=%s\n",len,opbuf,buf1,buf2);
    int i,j=0,k=0;
    for(i=0;i<len;++i)
    {
        if(opbuf[i]!=' ')
        {
            buf1[j++]=opbuf[i];
            k++;
            continue;
        }else if(opbuf[i]==' '&&k==0)
        {
            continue;
        }
        break;
    }
    k=i;
    i=j=0;
    for(;k<len;++k)
    {
        if(opbuf[k]!=' '&&opbuf[k]!='\n')
        {
            buf2[j++]=opbuf[k];
            i++;
            continue;
        }else if(opbuf[k]==' '&&i==0)
        {
            continue;
        }
        break;
    }
    //printf("len=%d,opbuf=%s,buf1=%s,buf2=%s\n",len,opbuf,buf1,buf2);
    if(strcmp(buf1,"ls")==0){
        return 1;
    }else if(strcmp(buf1,"pwd")==0){
        return 2;
    }else if(strcmp(buf1,"cd")==0){
        return 3;
    }else if(strcmp(buf1,"puts")==0){
        return 4;
    }else if(strcmp(buf1,"gets")==0){
        return 5;
    }else if(strcmp(buf1,"remove")==0){
        return 6;
    }else if(strcmp(buf1,"mkdir")==0){
        return 7;
    }else if(strcmp(buf1,"rmdir")==0){
        return 8;
    }
    return 9;
}
