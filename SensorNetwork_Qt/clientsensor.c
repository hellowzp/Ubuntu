#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "tcpsocket.h"
#include "gateway.h"

#define BUFSIZE 1024
#define OFF_NUM 100000  //let sensor only send limited data

//attention: here for req 2
//for req 2, the SET_ID check the make file ,there is a $(SET_ID), this is the one used after"make    ./sensorNode 127.0.0.1 1234"
	  //#ifndef SET_ID
	  //{
		  //printf("sensor id(SET_ID) isn't defined\n");
		  //return 0;

	  //}
	  //#endif
	  
//attention: here for req 3
#ifndef SET_FREQUENCY
#define SET_FREQUENCY 5
#endif

/*------------------------------------------------------------------------------
		definitions 
------------------------------------------------------------------------------*/
unsigned char buffer[BUFSIZE];

char * sensorFileGeneration(int sensorID);

/*------------------------------------------------------------------------------
		main
------------------------------------------------------------------------------*/
int main( int argc, char *argv[] ) 
{
	  Socket client;
      int ret,dataSample,MYPORT,off,fd,SET_ID;   //tcp_receive
      char *data = (char *)malloc(sizeof(char)*BUFSIZE);
	  char SERVER_IP[24];
	  long int dataHigh=0;
	  long int dataLow=0;
	  srand( time( NULL ) );
	  char * finalFile;
	    	  	    
//attention: here for req 4, two argments
      if(argc != 4)
      {
         printf("Incorrect number of args\n");
         exit(-1);
      }
      if(sscanf(argv[1],"%s", SERVER_IP)!=1)
      {
         printf("IP address setting failed.\n");
         exit(-2);
      }
      if (sscanf(argv[2],"%d", &MYPORT) != 1)
      {
         printf("PORT setting failed.\n");
         exit(-3);
      }

    if (sscanf(argv[3],"%d", &SET_ID) != 1)
      {
         printf("SET_ID setting failed.\n");
         exit(-3);
      }

    // log<Sensor_ID>.msg file
    finalFile=sensorFileGeneration(SET_ID%30);  //for testing:limit the sensor id from 0 to 29
    fd = open( finalFile, O_CREAT | O_APPEND| O_WRONLY, S_IRWXU);
    if ( fd == -1 ) {
        #ifdef DEBUG
        perror("File open failed: ");
        #endif
    }

    // open TCP connection to the server; server is listening to SERVER_IP and PORT
    client = tcp_active_open( MYPORT, SERVER_IP );

    ret=1;
    off=0;

    while(ret>0 && off<OFF_NUM)
	{ 
        //generate data from sensors
        dataSample=rand();            //generate 32 bits random number

        dataHigh=SET_ID<<20;
        dataLow=dataSample&1048575;   //Binary(number of 1: 20): 11111111111111111111-->Decimal:1048575
        dataSample=dataHigh|dataLow;  //set the id of the sensor

        printf("\ndata collected is %d\n",dataSample);

        packet_ptr_t sensor_packet_temp = (packet_ptr_t)&dataSample;
        printf("sensor id: %d sensor data id: %d \n",SET_ID,sensor_packet_temp->id);

        // The C library function int sprintf(char *str, const char *format, ...)
        // sends formatted output to a string pointed to, by str.
        sprintf(data,"%d",dataSample);

        tcp_send( client, (void *)data, strlen(data) );
        printf("clent sent tcp data succeed: \%s !", data);

//        // get reply from server
//        printf("\nanswer from server: ");
//        ret = tcp_receive (client, buffer, BUFSIZE);
//        printf("%s\n", buffer);

        char dataToFile[200];
        sprintf(dataToFile,"\nSensor id: %ld, temperature value: %ld, time: %ld\n",(long int)sensor_packet_temp->id,(long int)sensor_packet_temp->value,(long int)time(NULL));

        //attention: here for req 6
        ret = write(fd, dataToFile, sizeof(dataToFile));

        if ( ret == -1 )
        {
            #ifdef DEBUG
            perror("File write failed: ");
            #endif
        } else {
            printf("sensor data written to file: length %d %s\n", ret, dataToFile);
        }

        sleep(SET_FREQUENCY);

        off++;
	
    }
	
    printf("sensor client connection closed..\n");
    tcp_close( &client );
    free(data);
    close(fd);
    return 1;
}

//for generating names of the log<Sensor_ID>.msg files in the specific folder
char * sensorFileGeneration(int sensorID)
{
	  char* dataFile="log";
	  char* fileFormat=".msg";
	  char* parentFolder="data-generated/";
	
	  char fileId[12];
	  sprintf(fileId,"%d",sensorID);  
 
	  int len= strlen(dataFile)+strlen(fileFormat) + strlen(fileId);
	  char *finalFileName  = malloc(len);

	  strncpy(finalFileName, dataFile, len);
  	  strncat(finalFileName, fileId, len);
  	  strncat(finalFileName, fileFormat, len);

	  int len1= strlen(parentFolder) + strlen(finalFileName);
	  char *finalFile= malloc(len1);
		
	  strncpy(finalFile, parentFolder, len1);
      strncat(finalFile, finalFileName, len1);
	  //printf("/n%d ================   %s\n", sensorID,finalFile);
	  return  finalFile;
}