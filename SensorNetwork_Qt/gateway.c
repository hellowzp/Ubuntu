#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <time.h>

#include "config.h"
#include "list.h"
#include "queue.h"
#include "sensor_db.h"
#include "tcpsocket.h"
#include "util.h"

#define FIFO_NAME "temp/logfifo"
#define LOG_FILE "temp/gateway.log"

#define POINTER_CHECK(ptrcheck,fun) do{ if(ptrcheck) printf("%s() error: invalid parameter...\n",fun); } while(0)

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)  \
       fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __FUNCTION__, ##args)
#else
#define DEBUG_PRINT(fmt, args...)    /* Don't do anything in non-debug mode */
#endif

const unsigned int QUEUE_ELEMENT_SIZE = sizeof(sensor_data_t);

int port;
int fifo_fds;
float min_tmp;
float max_tmp;
queue_ptr_t sensor_queue;
list_ptr_t sensor_list;
pthread_mutex_t list_mutex;
pthread_mutex_t fifo_mutex;

static int sem_id = 0;
union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *arry;
};

void list_element_copy(element_ptr_t* des, element_ptr_t src) {
    assert(des && src);
    MALLOC(*des, sizeof(list_element_t));
    memcpy(*des, src, sizeof(list_element_t));

}

void list_queue_element_free(element_ptr_t* element){
    DEBUG_PRINT("free element @%p\n", element);
}

int list_element_compare_by_id(element_ptr_t x, element_ptr_t y) {
    sensor_id_t x_id = ((list_element_ptr_t) x )->sensor_id;
    sensor_id_t y_id = ((list_element_ptr_t) y )->sensor_id;
    return x_id > y_id ?  1 :
           x_id < y_id ? -1 : 0;
}

void list_element_print(element_ptr_t element) {
    if(element) {
        list_element_ptr_t e = (list_element_ptr_t)element;
        printf("list element: id(%d) temperature(%2.4f) ts(%s)",
               e->sensor_id, e->avg_tmp, asctime(localtime(&e->ts)));
    }
}

void queue_element_copy(element_ptr_t* des, element_ptr_t src) {
    assert(des && *des && src);
    memcpy(*des, src, QUEUE_ELEMENT_SIZE);
}

int  queue_element_compare(element_ptr_t x, element_ptr_t y) {
    sensor_id_t x_id = ((sensor_data_ptr_t) x )->id;
    sensor_id_t y_id = ((sensor_data_ptr_t) y )->id;
    return x_id > y_id ?  1 :
           x_id < y_id ? -1 : 0;
}

static int set_semvalue();
static void del_semvalue();
static int semaphore_p();
static int semaphore_v();

void* tcp_connect(void* arg);
void client_cleanup_if_stopped(void* arg);
void* client_response(void* client);
void* data_manage(void* arg);
void* storage_manage(void* arg);
void child_cleanup(int signo); 
void send_log_msg(int fds, const char* info);

int main(int argv, char* args[]) {
	if(argv!=2) {  
		printf("usage: %s %s\n",args[0],"port");
		exit(EXIT_FAILURE);
	} 

	#ifdef SET_MIN_TEMP
    min_tmp = (float)SET_MIN_TEMP;
	
	#ifdef SET_MAX_TEMP
    max_tmp = (float)SET_MAX_TEMP;
	#else 
	#error "SET_MIN_TEMP and SET_MAX_TEMP should be given as preprocessor directives."
	#endif
	
	#else 
	#error "SET_MIN_TEMP and SET_MAX_TEMP should be given as preprocessor directives."
	#endif
	
	port = (int)strtol(args[1],NULL,10);  //server port
	if(port <= 0 || port > 65535){
       printf("invalid port\n");
       exit(-2);
    }
    
    sensor_queue = queue_create();
    sensor_list = list_create( &list_element_copy, &list_queue_element_free,
                               &list_element_compare_by_id, &list_element_print);
	
	list_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	fifo_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	
	sem_id = semget((key_t)1234, 1, 0666 | IPC_CREAT);	
	if(!set_semvalue()) {
		fprintf(stderr, "Failed to initialize semaphore\n");
		exit(EXIT_FAILURE);
	}

	if( access(FIFO_NAME,F_OK) != 0) {   //SEE MAN 2 ACCESS
        printf("access fifo test: creating new fifo\n");
		int ret = mkfifo(FIFO_NAME,S_IRWXU);
		if(ret==-1){
			perror("mkfifo");
			exit(EXIT_FAILURE);
		}
	}
	
//	pid_t pid = fork();
//	if(pid>0) {
//		signal(SIGCHLD,child_cleanup);
		
//		sleep(5);   //let the child open fifo for read first
//		int pfds = open(FIFO_NAME, O_WRONLY);
//		while(pfds<0) {
//			perror("parent open fifo for writing error");
//			if(errno==ENXIO) {
//				sleep(5);
//				pfds = open(FIFO_NAME, O_WRONLY);
//			} else {
//				exit(EXIT_FAILURE);
//			}
//		}
//		fifo_fds = pfds;
		
//		pthread_t pth_tcp_con, pth_data_man, pth_strg_man;
//		int err;
//		void* retStatus;
		
//		if((err=pthread_create(&pth_tcp_con,NULL,tcp_connect,NULL))!=0) {
//			printf("can't create tcp connection thread: %s\n",strerror(err));
//			exit(EXIT_FAILURE);
//		}
		
//		if((err=pthread_create(&pth_data_man,NULL,data_manage,NULL))!=0) {
//			printf("can't create data management thread: %s\n",strerror(err));
//			exit(EXIT_FAILURE);
//		}
		
//		if((err=pthread_create(&pth_strg_man,NULL,storage_manage,NULL))!=0) {
//			printf("can't create staorage management thread: %s\n",strerror(err));
//			exit(EXIT_FAILURE);
//		}
		
//		waitpid(pid, NULL, 0);
		
//		pthread_join(pth_tcp_con,&retStatus);
//		printf("tcp connection thread terminated with status: %s\n",(char*)retStatus);
//		pthread_join(pth_data_man,&retStatus);
//		printf("data management thread terminated with status: %s\n",(char*)retStatus);
//		pthread_join(pth_strg_man,&retStatus);
//		printf("staorage management thread terminated with status: %s\n",(char*)retStatus);
		
//	} else if (pid==0) {
//		int cfds = open(FIFO_NAME, O_RDONLY);
//		if(cfds<0) {
//			perror("child open fifo for reading error");
//			exit(EXIT_FAILURE);
//		}
		
//		int log_fds = open("gateway.log",O_CREAT | O_WRONLY | O_APPEND, S_IRWXU | S_IRWXG);
//		if(log_fds<0) {
//			perror("child open log file error");
//			exit(EXIT_FAILURE);
//		}
		
//		char buf[100];
//		memset(buf,0,100);
//		while(1) {
//			int bytes = read(cfds,buf,100);
//			if(bytes==-1) perror("reading fifo");
//			else if(bytes>0) write(log_fds,buf,bytes);
////			sleep(60);
//		}
			
//	} else {
//		perror("fork");
//		exit(EXIT_FAILURE);
//	}
	
	return 0;
}

void* tcp_connect(void* arg) {
	Socket server = tcp_passive_open(port);		
    while( 1 ) {	
		pthread_t resp;
		Socket client = tcp_wait_for_connection(server);
		if(client) {
			pthread_create(&resp,NULL,client_response,(void*)client);  //new thread need the client as the parameter
			//void* cltStatus; //returned client status if client connection is closed
			//pthread_join(resp,&cltStatus);	//will block here..
			//printf("%s\n",(char*)cltStatus);
		}
    }

    tcp_close( &server );
    pthread_exit((void*)"tcp connection thread return successfully...");
}

void client_cleanup_if_stopped(void* arg) {
	printf("%s\n","client clean up...");
	wait(NULL);
}

void* client_response(void* client) {
	DEBUG_PRINT("%s\n","new client thread created, ");
	send_log_msg(fifo_fds,"new sensor node connected");
	//pthread_cleanup_push(client_cleanup_if_stopped,NULL);
	
	//Socket client = (Socket)arg;
	int sid;
	while(1) {
//        tcp_packet_ptr_t ppt = malloc(sizeof(*ppt));
//		int bytes = tcp_receive( client, (void*)ppt, sizeof(*ppt) );  //will only block if connected
//		if(bytes==0) break;   //client has stopped
		
//		time_t tm = time(NULL);
//		printf("received message of %d bytes, id: %d temp: %d %s", bytes, ppt->id, ppt->tem, ctime(&tm)); //ctime() string will end with new line
//		if( bytes==4 && sensor_check_parity(ppt)==0 ) {
//			//printf("%s\n","parity ok...");
//            tcp_packet_list_ptr_t tpt = malloc(sizeof(*tpt));
//			tpt->pkt = *ppt;
//			tpt->tms = tm;
//			sid = ppt->id;
			
//			char found = 0;
//			pthread_mutex_lock(&list_mutex);
//            list_ptr_t lptr = sensor_list;
//			while(lptr) {
//				if( ((xlist_ptr_t)(lptr->data))->id == sid ) {
//					found = 1;
//					list_ptr_t list = ((xlist_ptr_t)(lptr->data))->lptr;
//					int size = ((xlist_ptr_t)(lptr->data))->size;
//					list_insert_at_index(list, (void*)tpt, size+1); //append sensor data to the list
//					((xlist_ptr_t)(lptr->data))->size ++;
					
//					if(size>=MAX_LIST_SIZE)    //delete the first item in the data list to save memory
//						((xlist_ptr_t)(lptr->data))->lptr = list_free_at_index(list,0);
//					break;
//				} else {
//					lptr = lptr->next;
//				}
//			}
			
//			if(!found) {
//                if( ((xlist_ptr_t)(sensor_list->data))->id == 0 ) {  //just initialized, id = 0
//                    ((xlist_ptr_t)(sensor_list->data))->id = sid;
//                    ((xlist_ptr_t)(sensor_list->data))->size = 1;
//                    ((xlist_ptr_t)(sensor_list->data))->lptr->data = tpt;
//				} else {  //first create a sensor data extended list structure and then append it to the sensor array
					
//					xlist_ptr_t sensor_data = malloc(sizeof(extended_list_t));
//					sensor_data->id = sid;
//					sensor_data->size = 1;
//					sensor_data->lptr = list_alloc();
//					sensor_data->lptr->data = tpt;
						
//                    int size = list_size(sensor_list);
//                    sensor_list = list_insert_at_index(sensor_list, sensor_data, size+1);
//				}
//			}
//			pthread_mutex_unlock(&list_mutex);
			
//			//don't free tpt, the content is also pointed to by sensor_array now
//			free(ppt);  //pkt in tpt is not a pointer, so ppt can be freed
				
//		} else {
//			free(ppt);
//			send_log_msg(fifo_fds,"even parity check failed");
//		}
		//sleep(5);
	}
	
	tcp_close( &client );
	
	send_log_msg(fifo_fds,"sensor node disconnected");
	
	pthread_mutex_lock(&list_mutex);
//    list_ptr_t sensor_list = sensor_list;
//	while(sensor_list) {
//		if( ((xlist_ptr_t)(sensor_list->data))->id == sid ) {
//			//free that sensor list
//			list_ptr_t prev = sensor_list->prev;
//			list_ptr_t next = sensor_list->next;
			
//			if(prev)  prev->next = next;
//			if(next)  next->prev = prev;
			
//			list_free_all( &( ((xlist_ptr_t)(sensor_list->data))->lptr) );
//			free(sensor_list->data);
//			free(sensor_list);
			
//			break;
//		} else {
//			sensor_list = sensor_list->next;
//		}
//	}
	pthread_mutex_unlock(&list_mutex);
	
	printf("client thread with sensor ID %d stopped...\n",sid);
	pthread_exit((void*)("client closed...")); //better to describe client exit status
}


void* data_manage(void* arg) {
	#ifdef DEBUG
	#define CHECK_SIZE 5
	#else
	#define CHECK_SIZE 10
	#endif

//	unsigned char position[1024];
//	memset(position,CHECK_SIZE,1024*sizeof(Byte));  //be sure to init array before use
//	list_ptr_t sensor_list;
	
//	while(1) {
//		pthread_mutex_lock(&list_mutex);
//        sensor_list = sensor_list;
//		while(sensor_list) {
//			uint32_t id = ((xlist_ptr_t)(sensor_list->data))->id;
//			DEBUG_PRINT("%d %d %d\n",id,((xlist_ptr_t)(sensor_list->data))->size,position[id]);
//			if( ((xlist_ptr_t)(sensor_list->data))->size >= position[id] ) {
				
//				sensor_data_ptr_t sdp = malloc(sizeof(sensor_data_t));
//				sdp->id = id;
//                sdp->tmp = 0;
//				sdp->ts = time(NULL);
				
//				//first get tcp packet, then data packet and temperature
//				int j;
//				//long long unsigned int time = 0;
//				for(j=0; j<CHECK_SIZE; j++) {
//					list_ptr_t list = ((xlist_ptr_t)(sensor_list->data))->lptr;
//					data_ptr_t data = list_get_data_at_index(list,position[id]-j); //tcp packet
//					int sign = ( ((LIST_DATA_TYPE*)data)->pkt.sign > 0 ) ? -1 : 1;
//                    sdp->tmp += ((LIST_DATA_TYPE*)data)->pkt.tem * sign;
//					//time += ((LIST_DATA_TYPE*)data)->tms;   //timy overflow, use bigger type first
//                    DEBUG_PRINT("%d %d %ld %s\n",sign,sdp->tmp,((LIST_DATA_TYPE*)data)->tms, ctime(&(sdp->ts)));
//				}
//                sdp->tmp /= 10;
//				//sdp->ts = time/10;
				
//				position[id] += CHECK_SIZE;
//				//DEBUG_PRINT("%s\n","enqueueing...");
//                Enqueue(sensor_queue,sdp);
//				semaphore_v();
//				//DEBUG_PRINT("%s\n","enqueueing finished...");
				
//                if( (float)sdp->tmp / 100.0f >= max_tmp) {
//                    DEBUG_PRINT("too hot: %4.2f degrees\n",(float)sdp->tmp/100.0f);
//					send_log_msg(fifo_fds,"too hot");
//                } else if ((float)sdp->tmp / 100.0f <= min_tmp) {
//                    DEBUG_PRINT("too cold: %4.2f degrees\n",(float)sdp->tmp/100.0f);
//					send_log_msg(fifo_fds,"too cold");
//				} else {
//                    DEBUG_PRINT("normal temperature: %4.2f degrees\n",(float)sdp->tmp/100.0f);
//				}
//			}
			
//			sensor_list = sensor_list->next;
//		}
//		pthread_mutex_unlock(&list_mutex);
//		sleep(30);
//	}
	
	printf("%s","data management thread stopped...\n");
	pthread_exit((void*)("data management thread stopped...")); //better to describe thread exit status
}


void* storage_manage(void* arg) {
	DEBUG_PRINT("%s\n","mysql thread created..");
	MYSQL* mysql;
	int i = 5; //AUTO-RECONNECTION
	do{
		mysql = init_connection(1);
		sleep(30);
	}while(!mysql && --i);

	if(!mysql) {
		send_log_msg(fifo_fds,"mysql server connection failed");
		exit(EXIT_FAILURE);
	} else {
		DEBUG_PRINT("%s\n","mysql connection created..");
	}
	
	while(1) {
		semaphore_p();
		DEBUG_PRINT("%s\n","queue top...");
        sensor_data_ptr_t sdp = (sensor_data_ptr_t)queue_top(sensor_queue);
		DEBUG_PRINT("%s %p\n","queue top finished...",sdp);
		if(sdp) {
            queue_dequeue(sensor_queue);
            DEBUG_PRINT("%d %d %ld\n",sdp->id,sdp->tmp,sdp->ts);
            if(insert_sensor(mysql, sdp->id, sdp->tmp, sdp->ts)==0) {
				printf("%s\n","write to mysql successfully...");
				
				MYSQL_RES* res = find_sensor_all(mysql);
				print_result(res);
				free_result(res);
			}
		}
		//sleep(60);
	}	
	
	printf("%s","storage management thread stopped...\n");
	pthread_exit((void*)("storage management thread stopped...")); //better to describe thread exit status
}

void child_cleanup(int signo) {
	printf("signal %d %s\n",signo,"child begin to exit...");
	exit(EXIT_FAILURE);
}

void send_log_msg(int fds, const char* info) {
	char* msg = malloc(100);
	static unsigned int logseq = 0;
	logseq++;
	time_t t = time(NULL);
	int bytes = snprintf(msg,100,"EVENT %d: %s @ %s", logseq,info,ctime(&t));
	pthread_mutex_lock(&fifo_mutex);
	write(fds,msg,bytes);
	pthread_mutex_unlock(&fifo_mutex);
	free(msg);
}

static int set_semvalue()
{
	union semun sem_union;

	sem_union.val = 0;
	if(semctl(sem_id, 0, SETVAL, sem_union) == -1)
		return 0;
	return 1;
}

static void del_semvalue()
{
	union semun sem_union;

	if(semctl(sem_id, 0, IPC_RMID, sem_union) == -1) 
		fprintf(stderr, "Failed to delete semaphore\n");
}

static int semaphore_p()
{
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = -1;//P()
	sem_b.sem_flg = SEM_UNDO;
	if(semop(sem_id, &sem_b, 1) == -1)
	{
		fprintf(stderr, "semaphore_p failed\n");
		return 0;
	}
	return 1;
}

static int semaphore_v()
{
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = 1;//V()
	sem_b.sem_flg = SEM_UNDO;
	if(semop(sem_id, &sem_b, 1) == -1)
	{
		fprintf(stderr, "semaphore_v failed\n");
		return 0;
	}
	return 1;
}

