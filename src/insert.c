#include "mysql.h"

int fileInsert(char* table,char* aimthing,char* condition)
{
	MYSQL *conn;
	//MYSQL_RES *res;
	//MYSQL_ROW row;
	char* server="localhost";
	char* user="root";
	char* password="0220";
	char* database="Test";
	char query[200]="insert into";
    sprintf(query,"%s %s(%s)values(%s)",query,table,aimthing,condition);
	int t;
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))
	{
		printf("Error connecting to database:%s\n",mysql_error(conn));
	}else{
		printf("Connected...\n");
	}
	t=mysql_query(conn,query);
	if(t)
	{
		printf("Error making query:%s\n",mysql_error(conn));
	    mysql_close(conn);
	    return -1;
	}else{
        int row=(long)mysql_affected_rows(conn);
		printf("insert success,insert row=%d\n",row);
	    mysql_close(conn);
	    return 0;
	}
}
int personInsert(char* aimthing,char* table,char* condition)
{
	MYSQL *conn;
	//MYSQL_RES *res;
	//MYSQL_ROW row;
	char* server="localhost";
	char* user="root";
	char* password="0220";
	char* database="Test";
	char query[200]="insert into";
    sprintf(query,"%s %s(%s)values(%s)",query,table,aimthing,condition);
	puts(query);
    int t;
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))
	{
		printf("Error connecting to database:%s\n",mysql_error(conn));
	}else{
		printf("Connected...\n");
	}
	t=mysql_query(conn,query);
	if(t)
	{
		printf("Error making query:%s\n",mysql_error(conn));
	    mysql_close(conn);
	    return -1;
	}else{
        int row=(long)mysql_affected_rows(conn);
		printf("insert success,insert row=%d\n",row);
	    mysql_close(conn);
	    return 0;
	}
}
