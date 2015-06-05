#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "tcpsocket.h"
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> 
#include <time.h>
#include <sys/wait.h>

#define BUFSIZE 1024
#define PORT 1234
#define PORT_1 2345
#define PORT_2 3456
#define PORT_3 4567
#define TRUE 1
#define FALSE 0
#define FILE_NAME_MAX_SIZE 512

unsigned char buffer[BUFSIZE];

void Doc_Server(Socket client,char file_name[]);

int main(void)
{
	int bytes=0;
	int addrlen , new_socket=0 , client_socket[30] , max_clients = 30 , activity, i , valread , s,sd_1,sd_2,sd_3;
	int child_pid_1,child_pid_2,child_pid_3,pid,status;
	struct sockaddr_in address;
	time_t rawtime;
    struct tm * timeinfo;
	char buffer[1024]; //data buffer of 1K
//	char file_name[FILE_NAME_MAX_SIZE+1];
	Socket serv1,serv2,serv3,client;
	
	int array[100]; 
    FILE *fp;
	//set of socket descriptors
	fd_set readfds;
	//a message
	char *message = "Hello New Client!!! \r\n";
	
	
	serv1 = tcp_passive_open( PORT_1 );
	sd_1 = get_socket_descriptor(serv1);

	serv2 = tcp_passive_open( PORT_2 );
	sd_2 = get_socket_descriptor(serv2);

	serv3 = tcp_passive_open( PORT_3 );
	sd_3 = get_socket_descriptor(serv3);

	
	
	//initialise all client_socket[] to 0 so not checked
	for (i = 0; i < max_clients; i++) 
	{
		client_socket[i] = 0;
	}

	//accept the incoming connection
	addrlen = sizeof(address);  
	puts("Waiting for connections...");
	
	while(TRUE) 
	{
		
		for(i=0;i<100;i++){
			
			pid=array[i];
			if(waitpid(pid,&status,WNOHANG)==pid){
				
			//	if(WIFEXITED(status))
			   // 	printf("");
			   
			    
			}
		}
		//clear the socket set
		FD_ZERO(&readfds);
		//add master socket to set
		FD_SET( sd_1 , &readfds);
		FD_SET( sd_2 , &readfds);
		FD_SET( sd_3 , &readfds);

	
		//wait for an activity on one of the sockets , timeout is NULL 
		activity = select( max_clients + 1 , &readfds , NULL , NULL , NULL);
		if ((activity < 0) && (errno!=EINTR)) 
		{
			printf("select error");
		}
		//If something happened on the master socket , then its an incoming connection
		if (FD_ISSET(sd_1, &readfds)) 
		{
        
			client = tcp_wait_for_connection( serv1 );
			child_pid_1=fork();
			array[0]=child_pid_1;
			if(child_pid_1==0){
			
				tcp_close(&serv1);
				//inform user of socket number - used in send and receive commands
				printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , sd_1 , get_ip_addr(client ) , get_port( client));
				//send new connection greeting message
				tcp_send( client, &message, bytes );
			/*	if( send(sd_1, message, strlen(message), 0) != strlen(message) ) 
				{
					perror("send");
				}*/
		        
			/*	send( sd_1 , buffer , strlen(buffer) , 0 );
				puts("Welcome message sent successfully");
		
		
				//echo function
				bytes = recv(sd_1, buffer, BUFSIZE, 0);
				printf("received message of %d bytes: %s\n", bytes, buffer );
				// echo msg back to client
				send( sd_1 , buffer , strlen(buffer) , 0 );*/
				
				
				bytes = tcp_receive( client, buffer, BUFSIZE ); 
				printf("received message of %d bytes: %s\n", bytes, buffer );

				// echo msg back to client
				tcp_send( client, (void*)buffer, bytes );

				//sleep(1);	/* to allow socket to drain */
				tcp_close( &client );
				exit(0);
			}
		
		}
	
		else if (FD_ISSET(sd_2, &readfds)){
		
			client = tcp_wait_for_connection( serv2 );
			child_pid_2=fork();
			array[1]=child_pid_2;
		
			if(child_pid_2==0){
			
				tcp_close(&serv2);
				//inform user of socket number - used in send and receive commands
				printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , sd_2 , get_ip_addr(client ) , get_port( client));
				//send new connection greeting message
			/*	if( send(sd_2, message, strlen(message), 0) != strlen(message) ) 
				{
					perror("send");
				}
		
				send( sd_2 , buffer , strlen(buffer) , 0 );
				puts("Welcome message sent successfully");*/
			
				//echo back the Date
				time ( &rawtime );
				timeinfo = localtime ( &rawtime );
				printf ( "\007The current date/time is: %s", asctime (timeinfo) );
				strcpy(buffer,(asctime(timeinfo)));
				tcp_send( client, (void*)buffer, BUFSIZE );
			//	send( sd_2 , buffer , strlen(buffer)+1 , 0 );
				exit(0);
			}
		}
	
		else if (FD_ISSET(sd_3, &readfds)){
		
			client = tcp_wait_for_connection( serv3 );
			child_pid_3=fork();
			array[2]=child_pid_3;
		
			if(child_pid_3==0){
			
				tcp_close(&serv3);
				//inform user of socket number - used in send and receive commands
				printf("New connection , socket fd is %d , ip is : %s , port : %d \n" ,sd_3 , get_ip_addr(client ) , get_port( client));
	
				//bzero(file_name,sizeof(file_name));
				bytes = tcp_receive( client, buffer, BUFSIZE ); 
                
		
				printf("You want to save %s \n",buffer);
				fflush(stdout);
                
                //tell client server has recieved the file name
                tcp_send( client, buffer, BUFSIZE);
		
		   
				//just create a file
				fp = fopen(buffer,"w");
		    
				if (fp == NULL)
				{
					printf("File:\t%s Can Not Open To Write!!!!!\n", buffer);
					exit(1);
				}
				fclose(fp); 
				Doc_Server(client,buffer);
				exit(0);
			}
		}
		else
		{exit(0);}
		 
	
		//add new socket to array of sockets
		for (i = 0; i < max_clients; i++) 
		{
			s=sd_1;
			if (s == 0)
			{	
				client_socket[i] = sd_1;
				printf("Adding to list of sockets as %d\n" , i);
				i = max_clients;
			}
			s=sd_2;
			if (s == 0)
			{	
				client_socket[i+1] = sd_2;
				printf("Adding to list of sockets as %d\n" , i+1);
				i = max_clients;
			}
			s=sd_3;
			if (s == 0)
			{	
				client_socket[i+2] = sd_3;
				printf("Adding to list of sockets as %d\n" , i+2);
				i = max_clients;
			}
			s = client_socket[i+3];
			if (s == 0)
			{	
				client_socket[i+3] = new_socket;
				printf("Adding to list of sockets as %d\n" , i+3);
				i = max_clients;
			}
		
		}
	}
	//else its some IO operation on some other socket :)
	for (i = 0; i < max_clients; i++) 
	{
		
		s = client_socket[i];
		if (FD_ISSET( s , &readfds)) 
		{
			//Check if it was for closing , and also read the incoming message
			if ((valread = read( s , buffer, 1024)) == 0)
			{
			//Somebody disconnected , get his details and print
			getpeername(s , (struct sockaddr*)&address , (socklen_t*)&addrlen);
			printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
			//Close the socket and mark as 0 in list for reuse
			close( s );
			client_socket[i] = 0;
			}
		//Echo back the message that came in
		else
		{
			//set the terminating NULL byte on the end of the data read
			buffer[valread] = '\0';
			send( s , buffer , strlen(buffer) , 0 );
		}
		}
	}
	
	return 0;
}

void Doc_Server(Socket client,char file_name[]){
	
	int bytes;
	FILE *fp;
    fflush(stdout);
    printf("%s",file_name);
	
	bzero(buffer, sizeof(buffer));
	fp = fopen(file_name,"w");
	do{
 
    //open a file
    
    
     bytes = tcp_receive( client, buffer, BUFSIZE ); 


    if (fp == NULL)
	{
		printf("File:\t%s Can Not Open To Write!\n", file_name);
		exit(1);
	}
    if (bytes < 0)
	{
		printf("Recieve Data From Client Failed!\n");
		break;
	}
    printf("received message of %d bytes: %s\n", bytes, buffer );
    printf("The server has finished written file.\n");
     
    fwrite(buffer,sizeof(char),bytes,fp);
    

	}while(bytes>0);
	fclose(fp);
	
}	
