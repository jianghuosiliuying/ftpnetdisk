#include "head.h"
#define STR_LEN 8

int getsalt(char* salt)
{
    int i,flag;
    srand(time(NULL));
    for(i=0;i<STR_LEN;i++)
    {
        flag=rand()%3;
        switch(flag)
        {
        case 0:salt[i]=rand()%26+'a';break;
        case 1:salt[i]=rand()%26+'A';break;
        case 2:salt[i]=rand()%10+'0';break;
        }
    }
    //printf("%s\n",salt);
    return 0;
}

