#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>


#include "coordinator.h"
#include "mymqtt.h"
#include <sys/select.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>

void run_bridge();
void *read_database(void * p);
void *read_coordinator(void * p);
void *read_server(void * p);

void run_coordinator();
void* coordinator_tty_read(void*);
void* coordinator_pipe_read(void*);

void coordinator_terminate(int signo);
void coordinator_quit(int signo);
void coordinator_cleanup(void);



/*********************************************
             coordinator
**********************************************/

int up_stream_pipe[2];
int down_stream_pipe[2];

String co_inbuf;
String co_outbuf;
int tty_fds;

int COORDINATOR_WRITING;
int COORDINATOR_TERM;
int COORDINATOR_QUIT;

pid_t monitor_pid, bridge_pid, coordinator_pid, database_pid;
int childExitStatus;

//1 to restart -1 to kill default 0
int KILL_COORDINATOR_OR_RESTART;
int KILL_DATABASE_OR_RESTART;
int KILL_SERVER_OR_RESTART;

int main(){
	    
    pipe(up_stream_pipe);
	pipe(down_stream_pipe);
	
	/*
	int status_flag = fcntl(up_stream_pipe[0],F_GETFL);
	printf("%d\n",status_flag);
	fcntl(up_stream_pipe[0], F_SETFL, status_flag | O_NONBLOCK);
	printf("%d\n",status_flag);
	
	status_flag = fcntl(up_stream_pipe[1],F_GETFL);
	fcntl(up_stream_pipe[1], F_SETFL, status_flag | O_NONBLOCK);
	
	status_flag = fcntl(down_stream_pipe[0],F_GETFL);
	fcntl(down_stream_pipe[0], F_SETFL, status_flag | O_NONBLOCK);
	
	status_flag = fcntl(down_stream_pipe[1],F_GETFL);
	fcntl(down_stream_pipe[1], F_SETFL, status_flag | O_NONBLOCK); */
	
    bridge_pid = fork();

    if(bridge_pid == 0){
        run_bridge();
    }else{

        coordinator_pid = fork();
        if(coordinator_pid == 0){
            run_coordinator();
        }
    }
    
	waitpid(bridge_pid,&childExitStatus,0);
    waitpid(coordinator_pid,&childExitStatus,0);
   // waitpid(database_pid,&childExitStatus,0);

    return 1;
}

void run_bridge(){

    close(up_stream_pipe[1]);    
	close(down_stream_pipe[0]); 
	
	int status_flag = fcntl(up_stream_pipe[0],F_GETFL);
	printf("%d\n",status_flag);
    
    pthread_t pth_server;
    pthread_t pth_coordinator;
    pthread_t pth_database;

    int *p1, *p2, *p3;

    if(pthread_create(&pth_coordinator,NULL,read_coordinator,NULL)){
        perror("create thread unsuccessful\n");
    }  
    if(pthread_create(&pth_database,NULL,read_database,NULL)){
        perror("create thread unsuccessful\n");
    }
    if(pthread_create(&pth_server,NULL,read_server,NULL)){
        perror("create thread unsuccessful\n");
    }
    
    char c = 0;
	while(1) {
		
		read(STDIN_FILENO,&c,1);
		if(c=='q') {             //kill the child
			//kill(pid,SIGUSR1);
		}
		if(c=='s') {
			printf("%s %d\n","parent begin to sending hello...",down_stream_pipe[1]);
			int bytes = write(down_stream_pipe[1],"hello from parent",17);
			printf("parent write %d bytes\n",bytes);
		}
		if(c=='k') {
			//kill(pid,SIGSEGV);
		}
		
	}
    
	if(pthread_join(pth_coordinator, (void**)&p1)){
        perror("join thread unsuccessful\n");
    }
    if(pthread_join(pth_database, (void**)&p2)){
        perror("join thread unsuccessful\n");
    }
    if(pthread_join(pth_server, (void**)&p3)){
        perror("join thread unsuccessful\n");
    }
    
    exit(EXIT_FAILURE);
}

void *read_database(void * p){

    //while(1);
    
    
}

void *read_coordinator(void * p){
    
    	
	while(1) {
		printf("%s %d\n","bridge reading pipe: ",up_stream_pipe[0]);
		Byte bytes = 0;
		read(up_stream_pipe[0],&bytes,1);
		if(bytes<=sizeof(MQTT)) {
			printf("%s\n","mqtt data too short...");
			continue;
		} 
		printf("brdge is going to receive another %d bytes..",bytes);
		mqtt_ptr_t mqtt = malloc(sizeof(*mqtt));
		void* data =  malloc(bytes-sizeof(*mqtt));
		read(up_stream_pipe[0],mqtt,sizeof(*mqtt));
		read(up_stream_pipe[0], data, bytes-sizeof(*mqtt));
		mqtt->msg = data;
		
		printf("address: %ld  message: %s",mqtt->addr, (char*)mqtt->msg);
	}
}

void *read_server(void * p){
	return NULL;
}

void run_coordinator() {
	//SIGQUIT //CURRENT 
	//SIGTERM //write  TO ZIGBEE
	
	atexit(coordinator_cleanup);
	
	signal(SIGTERM,coordinator_terminate);
	signal(SIGQUIT,coordinator_quit);
	
	printf("%s\n","coordinator process running...");
	
	tty_serial_init();
	close(up_stream_pipe[0]);    
	close(down_stream_pipe[1]); 
	
	/*
	while(1) {
		
		struct timeval tv;
		fd_set readfds;
		
		FD_ZERO(&readfds);
		FD_SET(tty_fds,&readfds);
		FD_SET(down_stream_pipe[0],&readfds);
		
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		
		int ret = select(2,&readfds,NULL,NULL,&tv);
		if(ret == -1) {
			perror("select");
			exit(EXIT_FAILURE);
		} else if (ret==0) {
			printf("%s\n","5 seconds elapsed, no data available to read...");
			//sleep(5);
		} else {    //nr fds ready to read			
			if(FD_ISSET(tty_fds,&readfds))      coordinator_tty_read();
			if(FD_ISSET(down_stream_pipe[0],&readfds)) coordinator_pipe_read();
		}   
	}  */
	
	pthread_t pth_tty, pth_pipe;
	int err;
	void* ttyStatus;
	void* pipeStatus;
	
	if((err=pthread_create(&pth_tty,NULL,coordinator_tty_read,NULL))!=0) {
		printf("can't create tty thread: %s\n",strerror(err));
		exit(EXIT_FAILURE);
	}	
	if((err=pthread_create(&pth_pipe,NULL,coordinator_pipe_read,NULL))!=0) {
		printf("can't create pipe thread: %s\n",strerror(err));
		exit(EXIT_FAILURE);
	}
	
	pthread_join(pth_tty,&ttyStatus);	
	pthread_join(pth_pipe,&pipeStatus);
	
	exit(EXIT_FAILURE);
}	

void* coordinator_tty_read(void* arg) {
	
	printf("%s\n","coordinator_tty_read thread running...");
	
	while(1) {
		int bytes = 0;
		bytes = tty_serial_read(tty_fds,co_inbuf);
		printf("received frame length: %d %d %d %d %d\n\n",bytes,co_inbuf[2],co_inbuf[co_inbuf[2]+3],co_inbuf[5],co_inbuf[6]);
		
		/*
		mqtt_ptr_t mydata = xbee_data_read(&co_inbuf[3],bytes-4); //only the data section, excluding checksum
		if(mydata) {      
			Byte nbytes = sizeof(*mydata) + mydata->len;   //actual data bytes
			void* bytestream = malloc(nbytes+1);
			*(Byte*)bytestream = nbytes;		
			memcpy(bytestream+1, mydata, sizeof(*mydata));
			memcpy(bytestream+sizeof(*mydata)+1, mydata->msg, mydata->len);		
			write(up_stream_pipe[1], bytestream, nbytes+1);
			free(mydata->msg);
			free(mydata);
			free(bytestream);
		}  */

		mqtt_ptr_t mqtt = malloc(sizeof(*mqtt));
		mqtt->addr = 12345;
		mqtt->len = 6;
		mqtt->ts = time(NULL);
		mqtt->msg = malloc(6);
		*(Byte*)mqtt->msg = 6;
		memcpy(mqtt->msg+1, "hello", 5);
		
		Byte nbytes = sizeof(*mqtt) + mqtt->len;   //actual data bytes
		void* bytestream = malloc(nbytes+1);
		*(Byte*)bytestream = nbytes;		
		memcpy(bytestream+1, mqtt, sizeof(*mqtt));
		memcpy(bytestream+sizeof(*mqtt)+1, mqtt->msg, mqtt->len);
		printf("coordinator write %d bytes to pipe...\n",nbytes);	
		nbytes = write(up_stream_pipe[1], bytestream, nbytes+1);
		free(mqtt->msg);
		free(mqtt);
		free(bytestream);
		printf("coordinator write %d bytes to pipe...\n",nbytes);

	}
	/*
	int logfds = open("./rdesclog.txt", O_CREAT | O_RDWR | O_APPEND);
	int ret = 0, leng = bytes;
	String fbuf = co_inbuf;
	while (leng>0 && (ret=write(logfds,fbuf,leng))!=0) {
		if(ret==-1) {
			if(errno==EINTR)
				continue;
			perror("wrinting to log file");
		}
		leng -= ret;
		fbuf += ret;
	}
	if(close(logfds)==-1) perror("close log file");	
	*/
}

void* coordinator_pipe_read(void* arg) {
	
	printf("%s %d\n","coordinator_pipe_read thread read pipe", down_stream_pipe[0]);
	COORDINATOR_TERM = 0;
	
	while(1) {
		Byte bytes = 0;
		read(down_stream_pipe[0],&bytes,1);
		if(bytes<=sizeof(MQTT)) {
			printf("%s\n","mqtt data too short...");
			continue ;
		}
		mqtt_ptr_t mqtt = malloc(sizeof(*mqtt));
		void* data =  malloc(bytes-sizeof(*mqtt));
		read(down_stream_pipe[0],mqtt,sizeof(*mqtt));
		read(down_stream_pipe[0], data, bytes-sizeof(*mqtt));
		//mqtt->msg = data;
		
		if(mqtt->len != *(Byte*)data) {
			printf("%s\n","unmatched length field...");
			continue ;
		}
		
		COORDINATOR_WRITING = 1;
		
		uint64_t address = mqtt->addr;
		String mac_addr = malloc(8);
		int i;
		for(i=0; i<8; i++) {
			mac_addr[7-i] = address%256;
			address /= 256;
		}
		char ntw_addr[2] = {0xFF,0xFE};
		String RFData = (String)data;	
		//memcpy(RFData,mqtt->msg,mqtt->len);
		
		bytes = xbee_txReq_frame_assemble(mac_addr,ntw_addr,0,0,RFData,mqtt->len);

		/*
		int logfds = open("./wresclog.txt", O_CREAT | O_RDWR | O_APPEND);
		int ret = 0;
		leng = bytes;
		String fbuf = co_outbuf;
		while (leng>0 && (ret=write(logfds,fbuf,leng))!=0) {
			if(ret==-1) {
				if(errno==EINTR)
					continue;
				perror("wrinting to log file");
			}
			leng -= ret;
			fbuf += ret;
		}
		if(close(logfds)==-1) perror("close log file");	
		*/
		
		bytes = tty_serial_write(tty_fds,co_outbuf,bytes);
		printf("write %d bytes... %d\n",bytes,RFData[0]);
		
		free(data);
		free(mqtt);
		free(mac_addr);
		//free(RFData);
		
		COORDINATOR_WRITING = 0;
		
		if(COORDINATOR_TERM) exit(EXIT_SUCCESS);	
	}
}

void coordinator_terminate(int signo) {
	if(!COORDINATOR_WRITING) exit(EXIT_SUCCESS);
	COORDINATOR_TERM = 1;
}

void coordinator_quit(int signo) {
	exit(EXIT_SUCCESS);
}

void coordinator_cleanup() {
	free(co_inbuf);
	free(co_outbuf);	
}
