#include <my_global.h>
#include <mysql.h>
#include "config.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>


#define host     "studev.groept.be"
#define username "a12_syssoft"
#define password "a12_syssoft"
#define database "a12_syssoft"
#define filename "myfile.data"




int main(int argc, char **argv)
{

		MYSQL         	*conn;
        FILE          	*fp;
        sensor_data_t 	*buf;
        sensor_data_t   data;
        int              n,i;
        char 	        array[300];
        char			idbuf[30];
        char			valuebuf[30];
        char			tsbuf[30];
        //init  
		conn = mysql_init(NULL);

		if (conn == NULL) {
			printf("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
			exit(1);
		}
        //connect the database
		if (mysql_real_connect(conn, host, username, password, database, 0, NULL, 0) == NULL) {
			printf("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
			exit(1);
		}
        //creat the database
		/*if (mysql_query(conn, "create database Database")) {
			printf("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
			exit(1);
		}*/
         
        if ((buf = (sensor_data_ptr_t)malloc(sizeof(sensor_data_t)) )== NULL) {
                perror("malloc buf" );
                exit(1);
        }

        if ((fp = fopen(filename, "rb" )) == NULL) {
                perror("fopen file" );
                exit(1);
        }

        
        //creat table bingtao and copy value from file to sql
        if (mysql_query(conn, "CREATE TABLE bingtao(id int not null primary key auto_increment,sensor_id int,sensor_value int,timestamp TIMESTAMP)")) {
			
			printf("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
		}
        //read the value from file
        for(i=0;i<10;i++){                 
			n = fread(buf,sizeof(sensor_data_t),1,fp);
			
			if (n==0) {        
                perror("fread file" );
                exit(1);
			}
			data=*buf;
			//sprintf(idbuf,"%d",(int)data.id);
			//sprintf(valuebuf,"%d",(int)data.value);
			//sprintf(tsbuf,"%ld",(long int)data.ts);
			sprintf(array,"INSERT INTO bingtao (null,sensor_id,sensor_value,timestamp) values(null,%d,%d,%ld)",(int)data.id,(int)data.value,(long int)data.ts);
			//copy the string to array
			//strcpy(array,"INSERT INTO bingtao (null,sensor_id,sensor_value,timestamp) values(null,idbuf,valuebuf,tsbuf)");
			mysql_query(conn,array);
			
			memset(array,0,300);
			//mysql_query(conn,"insert into sensor(sensor_id sensor_value timestamp) values((int)*(n->id),(int)*(n->value),(time_t)(*(n->ts))");
            
			
			if ((mysql_real_query(conn, "SELECT sensor_id FROM bingtao WHERE sensor_value=(int)(buf->value)",(int)(buf->id))) != 0) {
                printf("insert failed, %s\n", mysql_error(conn));
                exit(1);
			}
			
			
		}

        fclose(fp);
        mysql_close(conn);
        
        return 0;


void copy_fun(MYSQL *conn,sensor_data_t data){
    sprintf(idbuf,"%d",(int)data.id);
    sprintf(valuebuf,"%d",(int)data.value);
    sprintf(tsbuf,"%ld",(long int)data.ts);
	//copy the string to array
	strcpy(array,"INSERT INTO bingtao (null,sensor_id,sensor_value,timestamp) values(array,idbuf,valuebuf,tsbuf)");
	mysql_query(conn,array);
}


}

