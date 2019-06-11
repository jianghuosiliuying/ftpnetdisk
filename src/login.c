#include "mysql.h"

int query2(char* aim,char* table,char* condition,pFile_t* pphead,pFile_t* pptail)
{
    MYSQL *conn;
    MYSQL_RES *res;
    //MYSQL_RES *res1;
    MYSQL_ROW row;
    char* server="localhost";
    char* user="root";
    char* password="0220";
    char* database="Test";//要访问的数据库名称
    char query2[300]="select";
    File_t file;
    sprintf(query2,"%s %s from %s %s",query2,aim,table,condition);
    //puts(query2);
    unsigned int t;
    conn=mysql_init(NULL);
    if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))
    {
        printf("Error connecting to database:%s\n",mysql_error(conn));
        return -1;
    }else{
        //printf("Connected...\n");
    }
    t=mysql_query(conn,query2);
    if(t)
    {
        printf("Error making query2:%s\n",mysql_error(conn));
    }else{
        //printf("query2 made...\n");
        res=mysql_store_result(conn);
        int i=mysql_num_rows(res);
        //printf("the row=%lld\n",mysql_num_rows(res));//打印行数
        if(res)
        {
            //printf("welcome to ues yunpan\n");
            while((row=mysql_fetch_row(res))!=NULL)
            {
                memset(&file,0,sizeof(File_t));
                file.precode=atoi(row[0]);
                file.code=atoi(row[1]);
                strncpy(file.filename,row[2],strlen(row[2]));
                strncpy(file.filetype,row[3],strlen(row[3]));
                file.belong=atoi(row[4]);
                if(strcmp(file.filetype,"f")==0)
                {
                    strncpy(file.md5,row[5],strlen(row[5]));
                    file.size=atoi(row[6]);
                }
                pFile_t pnew=(pFile_t)calloc(1,sizeof(File_t));
                memcpy(pnew,&file,sizeof(File_t));
                if(*pphead==NULL)
                {
                    *pphead=pnew;
                    *pptail=pnew;
                }else{
                    (*pptail)->pNext=pnew;
                    *pptail=pnew;
                }
            }
            mysql_free_result(res);
            mysql_close(conn);
            return i;//返回查询结果行数
        }else{
            printf("Don't find data\n");
            mysql_free_result(res);
            mysql_close(conn);
            return 0;
        }
    }
    return -1;
}

int query1(char* aim,char* table,char* condition,Person_t** pperson)
{
    MYSQL *conn;
    MYSQL_ROW row;
    MYSQL_RES *res;
    char* server="localhost";
    char* user="root";
    char* password="0220";
    char* database="Test";//要访问的数据库名称
    char query[300]="select";
    sprintf(query,"%s %s from %s %s",query,aim,table,condition);
    //puts(query);
    unsigned int t;
    //int r=0,num;
    conn=mysql_init(NULL);
    if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))
    {
        printf("Error connecting to database:%s\n",mysql_error(conn));
        return -1;
    }else{
        //printf("Connected...\n");
    }
    t=mysql_query(conn,query);
    if(t)
    {
        printf("Error making query1:%s\n",mysql_error(conn));
    }else{
        //printf("query1 made...\n");
        res=mysql_store_result(conn);
        if(res)
        {
            //printf("welcome to ues yunpan\n");
            while((row=mysql_fetch_row(res))!=NULL)
            {
                (**pperson).ID=atoi(row[0]);
                strncpy((**pperson).name,row[1],strlen(row[1]));
                strncpy((**pperson).salt,row[2],strlen(row[2]));
                strncpy((**pperson).cryptcode,row[3],strlen(row[3]));
            }
            mysql_free_result(res);
            mysql_close(conn);
            return mysql_num_rows(res);//返回查询结果行数
        }else{
            printf("Don't find data\n");
            mysql_free_result(res);
            mysql_close(conn);
            return -1;
        }
    }
    return -1;//实际无论什么情况，都不会执行这一句
}

int checkname(char* logbuf,Person_t* pperson)
{
    char table[10]="person";
    char aim[20]="*";
    char condition[200]="where";
    sprintf(condition,"%s name='%s'",condition,logbuf);
    int ret=query1(aim,table,condition,&pperson);
    //printf("ret=%d,ID=%d\n",ret,(*pperson).ID);
    return ret;
}

