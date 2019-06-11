#include "mysql.h"

int deletefile(char* table,char* comdition)
{
	MYSQL *conn;
	//MYSQL_RES *res;
	//MYSQL_ROW row;
	char* server="localhost";
	char* user="root";
	char* password="0220";
	char* database="Test";
	char query[200]="delete from ";
    sprintf(query,"%s %s %s",query,table,comdition);
	//puts(query);
    int t,r;
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
    	return 0;
	}else{
        r=(long)mysql_affected_rows(conn);
		printf("delete success,delete row=%d\n",r);
    	mysql_close(conn);
    	return r;
	}
}
