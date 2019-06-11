#include <stdio.h>
#include <string.h>
#include <openssl/md5.h>
#include "head.h"

int compute_file_md5(FILE *fp,char* file_md5)
{
    MD5_CTX ctx;
    int len;
    unsigned char buffer[1024]={0};
    unsigned char digest[16]={0};
    MD5_Init(&ctx);
    while((len=fread(buffer,1,1024,fp))>0)
    {
        MD5_Update(&ctx,buffer,len);
    }
    MD5_Final(digest,&ctx);
    fclose(fp);
    int i=0;
    char tmp[3]={0};
    for(i=0;i<16;i++)
    {
        sprintf(tmp,"%02x",digest[i]);
        strcat(file_md5,tmp);
    }
    return 0;
}

