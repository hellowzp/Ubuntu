#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

/***********************************
            Zigbee
************************************/
#define ZIGBEE_START_DELIMITER 0x7E

#define AT_COMMAND             0x08
#define AT_COMMAND_QUEUE       0x09
#define TRANSMIT_REQUEST       0x10
#define REMOTE_AT              0x17
#define CREATE_SOURCE_ROUTE    0x21
#define REG_JOINING_DEVICE     0x24
#define AT_RESPONSR            0x88
#define MODEM_STATUS           0x8A
#define TX_STATUS              0x8B
#define RECEIVE_PACKET         0x90
#define EXPLICIT_RX_INDICATOR  0x91
#define REMOTE_RESPONSE        0x97

#define PARAMETER_ERROR(fun) do{ printf("%s() error: invalid parameter...\n",fun); \
                                 exit(EXIT_FAILURE); \
                               } while(0)

typedef unsigned char Byte;
typedef char* String;

typedef struct {
	String name;
	String value;
} Map, *map_ptr_t;

typedef struct {
	char* address;     // mac address
	map_ptr_t payload; // waspmote sensor field
} Data, *data_ptr_t; 

typedef struct {
	void* msg;   //mqtt message
	char* addr;
} MQTT, *mqtt_ptr_t;

Byte xbee_set_checksum(String xbee_data, int len);
int xbee_txReq_data_assemble(String mac_addr,String ntwk_addr,char radius,char option,String RFData,int len,String* retStr);
int xbee_txReqst_frame_assemble(String mac_addr,String ntwk_addr,char radius,char option,String RFData,int len);
//read the data section of the zigbee frame and call the corresponding sub-read function according to the frame type
data_ptr_t xbee_data_read(String xbee_data,int len);
//read ZigBee Receive Packet(0x90) data section, return the desired data structure pointer 
data_ptr_t xbee_rx_data_read(String xbee_data,int len);
data_ptr_t xbee_exrx_data_read(String xbee_data,int len);
int xbee_remoteAT_frame_assemble(String mac_addr,String ntwk_addr,char option,String cmd, char para);
int xbee_localAT_frame_assemble(String cmd,char value);
int xbee_mqtt_wframe_assenble(mqtt_ptr_t mqtt);
int xbee_frame_escape(String buf,int len);  //length of buffer(outbuf)
int xbee_frame_descape(String buf,int len); //length of buffer(inbuf)

/***********************************
            Waspmote
************************************/
#define MAX_FRAME_SIZE   200
#define ALLOW_EMPTY_FIELD

String wframe_get_data_at_del( char* pkt, int n, char del);
//get data section between two dels including the ending del but excluding the beginning del
//if the beginning del is <=0 get before the second del
//if the ending is too large get after the first
String wframe_get_data_between_del( char* pkt, int len, int bn, int en, char del);
int wframe_assemble(Byte frame_type, Byte fields, char* serial_id,Byte seq,map_ptr_t payload,char** wframe);

/***********************************
            ttyUSB
************************************/

#ifdef RD
#define MODEM_DEVICE "/dev/ttyUSB0"
#endif
#ifdef WR
#define MODEM_DEVICE "/dev/ttyUSB0"
#endif

int tty_open(char* tty_name);
void tty_serial_config(int fds);
void tty_serial_init();
int tty_serial_read(int fds,String buf);
int tty_serial_write(int fds,String buf,int len);


/***********************************
            mystring
************************************/

#define FREE_STRING_ARRAY(sarray) do{ int i=0;  \
	while(*(sarray+i)) {                        \
		free(*(sarray+i));                      \
		*(sarray+i) = NULL;                     \
		i++;                                    \
	}                                           \
	free(sarray);                               \
	sarray = NULL;                              \
} while(0)

int int_to_str(int num,char** str);
String hex_to_str(String hexstr,int len);
int str_length( char* str);
long power(char a, unsigned char b);
int string_to_int(String s, int base);
char* str_get_between_index( char* str, int bn, int en);
//concatenate variable length of strings(sensor field)
//if allow empty field, treat NULL field as empty and also add it
//else exit with error
char* str_multi_cat(int num,  char del, ...);
int str_split( char* str, char del,char*** retArray);
map_ptr_t str_to_map( char* str, char fdel, char sdel);
int map_cat(map_ptr_t map,  char fdel,  char sdel, char** str);


/***********************************
          global variables
************************************/
String inbuf;
String outbuf;
int tty_fds;
int waspmote_seq;


/***********************************
            Zigbee
************************************/

Byte xbee_set_checksum(String xbee_data, int len) {
    if(!xbee_data) PARAMETER_ERROR("xbee_set_chechsum");
    int sum = 0;
    int i;
    for(i=0; i<len; i++) {   //can't use strlen() because the data may contain 0x00
        sum += xbee_data[i];
        sum = (sum>255)?(sum-256):sum;
    }
    return (Byte)(0xFF-sum); 
}

int xbee_txReq_data_assemble(String mac_addr,String ntwk_addr,char radius,char option,String RFData,int len,String* retStr) {
    if(!mac_addr || !ntwk_addr || !RFData || !retStr)
        PARAMETER_ERROR("xbee_txReq_data_assemble");
    //int len = strlen(RFData);
    String str = malloc(len+15);
    str[0] = 0x10;
    str[1] = 0x01;
    str[2] = 0;
    strncat(str,mac_addr,8);
    strncat(str,ntwk_addr,2);
    str[12] = radius;
    str[13] = option;
    str[14] = 0;
    strncat(str,RFData,len);
    *retStr = str;
    return len+14;
}

int xbee_txReqst_frame_assemble(String mac_addr,String ntwk_addr,char radius,char option,String RFData,int len) {
	if(!mac_addr || !ntwk_addr || !RFData)
        PARAMETER_ERROR("xbee_txReq_frame_assemble");
    //int len = strlen(RFData) + 14;  //frame length field, not the total length   
    len += 14;
    outbuf[0] = ZIGBEE_START_DELIMITER;
    outbuf[1] = len/256;
    outbuf[2] = len%256;
    outbuf[3] = 0x10;  //frame type
    outbuf[4] = 0x01;  //frame id
    int i;
    for(i=0; i<8; i++) outbuf[5+i] = mac_addr[i];		
	outbuf[13] = ntwk_addr[0];
    outbuf[14] = ntwk_addr[1];
    outbuf[15] = radius;
    outbuf[16] = option;
    for(i=0; i<len-14; i++) outbuf[17+i] = RFData[i];
    outbuf[len+3] = xbee_set_checksum(&outbuf[3],len);
    outbuf[len+4] = 0;
    return len+4;   //return total length
}    

int xbee_remoteAT_frame_assemble(String mac_addr,String ntwk_addr,char option,String cmd, char para) {
	if(!mac_addr || !ntwk_addr || !cmd)
        PARAMETER_ERROR("xbee_remoteAT_frame_assemble");
    
    outbuf[0] = ZIGBEE_START_DELIMITER;
    outbuf[1] = 00;
    //outbuf[2] = len%256;
    outbuf[3] = 0x17;  //frame type
    outbuf[4] = 0x01;  //frame id
    int i;
    for(i=0; i<8; i++) outbuf[5+i] = mac_addr[i];		
	outbuf[13] = ntwk_addr[0];
    outbuf[14] = ntwk_addr[1];
    outbuf[15] = option;
    outbuf[16] = cmd[0];
    outbuf[17] = cmd[1];
    
    if(para=='Q') { //query, no para
		outbuf[2] = 15;
		outbuf[18] = xbee_set_checksum(&outbuf[3],15);
		return 19;
	} else {
		outbuf[2] = 15;
		outbuf[18] = para;
		outbuf[19] = xbee_set_checksum(&outbuf[3],16);
		return 20;
    }   
}

int xbee_localAT_frame_assemble(String cmd, char para) {
	if(!cmd) PARAMETER_ERROR("xbee_localAT_frame_assemble");
	outbuf[0] = ZIGBEE_START_DELIMITER;
    outbuf[1] = 00;
    //outbuf[2] = len%256;
    outbuf[3] = 0x08;  //frame type
    outbuf[4] = 0x01;  //frame id
    outbuf[5] = cmd[0];
    outbuf[6] = cmd[1];
    
    if(para=='Q') { //query, no para
		outbuf[2] = 4;
		outbuf[7] = xbee_set_checksum(&outbuf[3],4);
		return 8;
	} else {
		outbuf[2] = 5;
		outbuf[7] = para;
		outbuf[8] = xbee_set_checksum(&outbuf[3],5);
		return 9;
    }   
}

int xbee_mqtt_wframe_assenble(mqtt_ptr_t mqtt) {
	if(!mqtt) PARAMETER_ERROR("xbee_mqtt_Wframe_assenble");
	int len = *(Byte*)(mqtt->msg+1) + 14;  //LENGTH FIELD not the total length
    outbuf[0] = ZIGBEE_START_DELIMITER;
    outbuf[1] = len/256;
    outbuf[2] = len%256;
    outbuf[3] = 0x10;  //frame type
    outbuf[4] = 0x01;  //frame id
    String mac_addr = mqtt->addr;
    int i;
    for(i=0; i<8; i++) outbuf[5+i] = mac_addr[i];		
	outbuf[13] = 0xFF;
    outbuf[14] = 0XFE;
    outbuf[15] = 0;
    outbuf[16] = 0;
    String RFData = (String)mqtt->msg;
    for(i=0; i<len-14; i++) outbuf[17+i] = RFData[i];
    outbuf[len+3] = xbee_set_checksum(&outbuf[3],len);
    return len+4;   //return total length
}

int xbee_frame_escape(int len) {
	int i,j, leng=len;
	char esc[MAX_FRAME_SIZE];
	esc[0] = 0; //save the number of escape characters
	for(i=3; i<len-1; i++) {  //just escape the data section
		char c = buf[i];
		if(c==0x11 || c==0x13 || c==0x7D || c==0x7E) {
			esc[0]++;
			esc[esc[0]] = i;
			leng++;
		}
	}
	
	//insert algorithm
	esc[esc[0]+1] = len;
	for(i=esc[0];i>0;i--) {
		for(j=esc[i+1]-1;j>=esc[i];j--) {
			buf[i+j] = buf[j];			
		}
		//printf("%d %d %d ",i,j,outbuf[i+j+1]);
		buf[i+j+1] ^= 0x20;   //XOR
		//printf("%d\n",outbuf[i+j+1]);
		buf[i+j] = 0x7D;
	}	
	//outbuf[2] += esc[0];	//change length	
	return leng;
}

int xbee_frame_descape(String buf, int len) {
	if(!buf) PARAMETER_ERROR("xbee_frame_descape");
	int i,j, leng=len;
	char esc[MAX_FRAME_SIZE];
	esc[0] = 0; //save the number of escape characters
	for(i=3; i<len-1; i++) {  //just escape the data section
		if(buf[i]==0x7D) {
			esc[0]++;
			esc[esc[0]] = i;
			leng--;
		}
	}
	
	//insert algorithm
	esc[esc[0]+1] = len;
	for(i=1;i<=esc[0];i++) {
		for(j=esc[i]+1;j<esc[i+1];j++) {
			buf[j-i] = buf[j];			
		}
		buf[esc[i]+1-i] ^= 0x20; 
	}	
	
	return leng;
}
        
//read ZigBee Receive Packet(0x90) data section including the checksum, return the desired data structure pointer 
data_ptr_t xbee_rx_data_read(String xbee_data,int len) {
	if(!xbee_data) PARAMETER_ERROR("xbee_rx_data_read");
	//int len = sizeof(xbee_data)-1;    //ignore the ending teminator; and don't use strlen(), will be stopped counting by the terminating 0 in the string
	String mac_addr = str_get_between_index(xbee_data,1,8);
	String wframe = str_get_between_index(xbee_data,12,len-2);
	
	//FIRST GET THE PAYLOAD STRING and then convert it to map format 
	/*String payload_str = NULL;
	map_ptr_t payload_map = NULL;
	int fields = *(wframe+4); //the fifth char, expressed as 0x03(char with asc value 3), so no need to convert (from asc) to number
	payload_str = wframe_get_data_between_del(wframe,4,fields+4,'#');
	payload_map = str_to_map(payload_str,'#',':');
	printf("%c %c %d\n",wframe[0],wframe[1],fields);
	
	data_ptr_t mydata = (data_ptr_t)malloc(sizeof(Data));
	mydata->address = mac_addr;
	mydata->payload = payload_map;
	free(payload_str);
	free(wframe);
	return mydata; */
	
	mqtt_ptr_t mqtt = malloc(sizeof(MQTT));
	mqtt->addr = mac_addr;
	String s = wframe_get_data_between_del(wframe,len-13,1,2,'#');
	mqtt->msg = str_get_between_index(s,2,len-14);
	free(s);
	
	//int bytes = xbee_mqtt_wframe_assenble(mqtt);
	//tty_serial_write(tty_fds,outbuf,bytes);
	printf("%s\n",(char*)(mqtt->msg));
	return NULL;
}	

//read Explicit Rx_indicator(0x91) data section, return the desired data structure pointer 
data_ptr_t xbee_exrx_data_read(String xbee_data,int len) {
	if(!xbee_data) PARAMETER_ERROR("xbee_exrx_data_read");
	String mac_addr = str_get_between_index(xbee_data,2,9);
	String wframe = str_get_between_index(xbee_data,18,len-2);
	
	//FIRST GET THE PAYLOAD STRING and then convert it to map format
	String payload_str = NULL;
	map_ptr_t payload_map = NULL;
	int fields = *(wframe+4); //the fifth char, expressed as 0x03(char with asc value 3), so no need to convert (from asc) to number
	payload_str = wframe_get_data_between_del(wframe,len-19,1,2,'#');
	payload_map = str_to_map(payload_str,'#',':');
	printf("%d %d %d\n",wframe[0],wframe[1],fields);
	
	data_ptr_t mydata = (data_ptr_t)malloc(sizeof(Data));
	mydata->address = mac_addr;
	mydata->payload = payload_map;
	free(payload_str);
	free(wframe);
	return mydata;
}	

data_ptr_t xbee_data_read(String xbee_data,int len) {
	if(!xbee_data) PARAMETER_ERROR("xbee_data_read");
	Byte frame_type = xbee_data[0];
	printf("received frame type: %d\n",(int)frame_type);
	
	data_ptr_t mydata = NULL;
	switch(frame_type) {		
		case 0x97 :    //remote AT response
		case 0x90 :    //received an RF packet
			mydata = xbee_rx_data_read(xbee_data,len);
			break;
		case 0x91 :    //explicit rx indicator
			mydata = xbee_exrx_data_read(xbee_data,len);
			break;
		case 0x8A :    //modem status
			//printf("");
		case 0x8B :    //transmit status
			
		default:
			break;
	}
	return mydata;
}


/***********************************
            ttyUSB
************************************/

int tty_open(char* tty_name) {
	int tty_fds = open(tty_name,O_RDWR | O_NOCTTY | O_NONBLOCK);
	if(tty_fds<0) {
        perror("open tty error");
        exit(EXIT_FAILURE);
	} 
	return tty_fds;
}

void tty_serial_config(int fds) {
	/*
	 * http://www.opensource.apple.com/source/xnu/xnu-1456.1.26/bsd/sys/termios.h
	 * typedef unsigned long	tcflag_t;
	   typedef unsigned char	cc_t;
	   typedef unsigned long	speed_t;

	   struct termios {
			tcflag_t	c_iflag;	input flags 
			tcflag_t	c_oflag;	output flags 
			tcflag_t	c_cflag;	control flags 
			tcflag_t	c_lflag;	local flags 
			cc_t		c_cc[NCCS];	control chars 
			speed_t		c_ispeed;	input speed 
			speed_t		c_ospeed;	output speed 
        };   
     */
	struct termios tio_old, tio_new;
	tcgetattr(fds,&tio_old); /* save the current serial port settings */
	//bzero(&newtio, sizeof(newtio));
	tio_new=tio_old;
	
	//input option:ignore parity check, map CR(Enter,carriage return,\r) to NL
	tio_new.c_iflag |= (IGNPAR | ICRNL);

	//output option: raw output
	tio_new.c_oflag &= ~OPOST;
	
	//control flags
	//8N1:8 bits, no parity check, 1 stop bit 
	tio_new.c_cflag &= ~PARENB;
	tio_new.c_cflag &= ~CSTOPB;
	tio_new.c_cflag &= ~CSIZE;
	tio_new.c_cflag |= CS8;
	//disable hardware control
	tio_new.c_cflag &= ~CRTSCTS;
	
	tio_new.c_cflag |= (CLOCAL | CREAD);	
	
	//line option: raw input mode
	tio_new.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	
	//special control characters
	tio_new.c_cc[VMIN]  = 1;
	tio_new.c_cc[VTIME] = 0;
	
	//baud rate
	cfsetispeed(&tio_new, B115200);
	cfsetospeed(&tio_new, B115200);
	
	tcsetattr(fds, TCSANOW, &tio_new);//Make changes now without waiting for data to complete
}	

void tty_serial_init() {
	tty_fds = tty_open(MODEM_DEVICE);
	tty_serial_config(tty_fds);
	waspmote_seq = 0;
	
	if((inbuf = malloc(MAX_FRAME_SIZE)) == NULL ) {
		printf("%s\n","fail to allocate memory for inbuf...");
		exit(EXIT_FAILURE);
	} else {
		memset(inbuf,0,MAX_FRAME_SIZE);
	}

	if((outbuf = malloc(MAX_FRAME_SIZE)) == NULL ) {
		printf("%s\n","fail to allocate memory for outbuf...");
		exit(EXIT_FAILURE);
	} else {
		memset(outbuf,0,MAX_FRAME_SIZE);
	}	
}

//read one total frame and load the data section(except for the start three bytes) into buf
//and return the length of it(including the checksum bit)
int tty_serial_read(int fds, String tbuf) {
	if(fds<0 || !tbuf) PARAMETER_ERROR("tty_serial_read");
	if(tbuf[0]) memset(tbuf,0,MAX_FRAME_SIZE);
	
	while(tbuf[0]!='~') {
		if(read(fds,tbuf,1)<0) {
			perror("reading ttyUSB0");
			sleep(2);
		}
	}
	tbuf++; //point to the new position
		
	ssize_t ret;
	int i,bytes=1,leng = 2; //bytes:bytes already read, length: bytes to read
	while (leng!=0 && (ret=read(fds,tbuf,leng))!=0) {
		if(ret==-1) {
			if(errno==EINTR)
				continue;
			perror("read the starting 3 bytes");  //no data available in non-blocking mode or other errors
			sleep(2);
			return -1;
		}
		leng -= ret;
		tbuf += ret;
	}
	bytes += 2;
	
	leng = inbuf[1]*256 + inbuf[2] +1;  //including the checksum bit, be careful to use inbuf cauze tbuf is moving!!
	printf("received data section length including the checksum: %d\n",leng);
	
	//continue reading the followint bytes of length leng and check escape
	int nr_escapes = leng;
	do {
		leng = nr_escapes;
		while (leng!=0 && (ret=read(fds,tbuf,leng))!=0) {
			if(ret==-1) {
				if(errno==EINTR)
					continue;
				perror("read the data section error");
				sleep(1);
				return -1;
			}
			leng -= ret;
			tbuf += ret;
		}
		bytes += nr_escapes;
		
		int i, leng = nr_escapes;
		nr_escapes = 0;
		for(i=bytes-leng;i<bytes;i++) {  //check the newly read bytes
			if(inbuf[i]==0x7D) {
				printf("the byte after escape: %d %d\n",i, inbuf[i+1]);
				//int c = tbuf[1+i];
				//if(c==0x7E || c==0x11 || c==0x13 || c==0x7D) {
				nr_escapes++;
			}
		} 
	} while(nr_escapes);
	
	#ifndef DEBUG
	leng = bytes;
	String fbuf = inbuf;
	int logfds = open("./esclog.txt", O_CREAT | O_RDWR | O_APPEND);
	while (leng!=0 && (ret=write(logfds,fbuf,leng))!=0) {
		if(ret==-1) {
			if(errno==EINTR)
				continue;
			perror("wrinting to log file");
		}
		leng -= ret;
		fbuf += ret;
	}
	if(close(logfds)==-1) perror("close esclog file");
	#endif
	
	//descape and check checksum
	bytes = xbee_frame_descape(inbuf,bytes);		
	if(xbee_set_checksum(&inbuf[3],bytes-3)!=0) {
		printf("%s\n","checksum error...");
		//memset(inbuf,0,bytes);
		return 0;
	} else {
		printf("%s total bytes after descape: %d\n","checksum perfect!",bytes);
	}
	
	#ifndef DEBUG
	leng = bytes;
	fbuf = inbuf;
	int dlogfds = open("./esclog.txt", O_CREAT | O_RDWR | O_APPEND);
	while (leng!=0 && (ret=write(dlogfds,fbuf,leng))!=0) {
		if(ret==-1) {
			if(errno==EINTR)
				continue;
			perror("wrinting to dlog file");
		}
		leng -= ret;
		fbuf += ret;
	}
	if(close(dlogfds)==-1) perror("close desclog file");
	#endif
	
	return bytes;
}

int tty_serial_write(int fds, String buf, int len) {
	if(fds<0 || !buf) PARAMETER_ERROR("tty_serial_write");
	
	ssize_t ret;
	int bytes = 0;
	while (len!=0 && (ret=write(fds,buf,len))!=0) {
		if(ret==-1) {
			if(errno==EINTR)
				continue;
			perror("read the starting 3 bytes");
			return 0;
		}
		len -= ret;
		buf += ret;
		bytes+=ret;
	}
	memset(buf,0,MAX_FRAME_SIZE);
	return bytes;
}

/***********************************
            Waspmote
************************************/

String wframe_get_data_at_del( char* pkt, int n,  char del) {
	if(!pkt || n<=0) {
		printf("%s\n","wframe_get_data_at_del(): invalid parameter...");
		return NULL;
	}
	int i,j,cnt = 0;
	int len = str_length(pkt);
	for(j=0; j<len; j++) {
		if(*(pkt+j)==del) {
			cnt++;
			if(cnt==n-1) i=j;
			if(cnt==n) break;
		}
	}
	if(n==1) return str_get_between_index(pkt, 0, j-1); //char at j is the del, remove it
	if(i+1<j-1) return str_get_between_index(pkt, i+1, j-1);
	else if(i+1==j-1) {    //between the 2 dels contains only one char
		char* c = (char*)malloc(2);
		*c = *(pkt+i+1);
		*(c+1) = 0;
		return c;
	} else {               //the 2 dels are next to each other
		char* c = (char*)malloc(1);
		*c = 0;
		return c;
	}
}

String wframe_get_data_between_del( char* pkt, int len, int bn, int en,  char del) {
	if(!pkt || bn<0 || en<bn) {
		printf("%s\n","wframe_get_data_between_del(): invalid parameter...");
	}
	int i,j,cnt = 0;
	for(j=0; j<len; j++) {
		if(*(pkt+j)==del) {
			cnt++;
			if(cnt==bn) i=j;
			if(cnt==en) break;
		}
	}
	if(bn==0) return str_get_between_index(pkt, 0, j);  //get before the second
	
	if(j==len) j -= 1;
	return str_get_between_index(pkt, i+1, j);
}

int wframe_assemble(Byte frame_type, Byte fields, char* serial_id,Byte seq,map_ptr_t payload,char** wframe) {
	if(!payload || !wframe) PARAMETER_ERROR("wframe_assemble");
	char* seq_str = NULL;
	char* pld_str = NULL;
	int_to_str(seq,&seq_str);
	map_cat(payload,'#',':',&pld_str);
	char* start = malloc(6); //first 5 bytes
	*start = 60;
	*(start+1) = 61;
	*(start+2) = 62;
	*(start+3) = (char)frame_type;
	*(start+4) = (char)fields;
	*(start+5) = 0;
	
	*wframe = str_multi_cat(5,'#',start,serial_id,"WASPMOTE_XBEE",seq_str,pld_str);  
	free(start);
	free(seq_str);
	free(pld_str);
	return str_length(*wframe);
}	

/***********************************
            mystring
************************************/
int int_to_str(int num,char** str) {
	int bits = 1;
	int temp = num;
	while(num = num/10) {
		bits++;
		//num = num/10;
	}
	*str = malloc(bits+1);
	int i;
	for(i=0; i<bits; i++) {
		*(*str+bits-1-i) = temp%10 + 48;
		temp = temp/10;
	}
	*(*str+bits) = 0;
	return bits;
}

String hex_to_str(String hex,int len) {
	//int len = strlen(hex);
	if(len!=4 || len!=16) {
		printf("%s\n","invalid address string");
		return NULL;
	} 
	int i, l = len/2;
	String s = malloc(l+1);
	for(i=0; i<l; i++) {
		int msb = hex[2*i];
		int lsb = hex[2*i+1];
		if(48<=msb && msb<=57) {
			msb -= 48;         //numbers
		} else if(65<=msb && msb<=70) {
			msb -= 55;         //upper-case letters
		} else if(97<=msb && msb<=102) {
			msb -= 87;         //lower-case letters
		} else {
			printf("%s\n","invalid address..");
			exit(EXIT_FAILURE);
		}
		if(48<=lsb && lsb<=57) {
			lsb -= 48;         //numbers
		} else if(65<=lsb && lsb<=70) {
			lsb -= 55;         //upper-case letters
		} else if(97<=lsb && lsb<=102) {
			lsb -= 87;         //lower-case letters
		} else {
			printf("%s\n","invalid address..");
			exit(EXIT_FAILURE);
		}
		s[i] = msb*16 + lsb;
	}
	s[l] = 0;
	return s;
}		

int str_length( char* str) {
	if(!str) return -1;
	int len = 0;
	while(*(str+len)) len++;
	return len;
}

long power(char a, unsigned char b) {
	unsigned char i;
	long pow = 1;
	if(a>=48) a-=48;  //like if a='1' instead of a=1
	for(i=0; i<b; i++) {
		pow *= a;
	}
	return pow;
}

String str_get_between_index( char* str, int bn, int en) {
	if(!str || bn<0 || bn>en) {
		printf("%s\n","str_get_between_index(): invalid parameter...");
		return NULL;
	}
	//int len = str_length(str);
	//if(en>=len) return NULL;
	char* data = (char*)malloc(en-bn+2);
	int i;
	for(i=bn; i<=en; i++) {
		*(data+i-bn) = *(str+i);
	}
	*(data+en-bn+1) = 0;
	return data;
}

int str_split( char* str, char del,char*** retArray) {
	if(!str) {
		printf("%s\n","str_split(): invalid parameter...");
		return -1;
	}
	int len = str_length(str);
	int i,j=0;
	int index[len];  //save the index of delimiters
	for(i=0; i<len; i++) {
		if(*(str+i)==del) {
			index[j]=i;
			j++;
		}
	}

	char** str_array = NULL;
	int size = (j+1)*sizeof(*str_array);  //one extra space for indicating the end in order to free later
	str_array = (char**)malloc(size);
	//printf("%d %d\n",j,size);
	for(i=0; i<j; i++) {  //fill the array row by row
		int k;
		if(i==0) {
			*(str_array+i) = (char*)malloc(index[i]+1);
			for(k=0; k<index[i]; k++) {
				*(*(str_array+i)+k) = *(str+k);
			}
			*(*(str_array+i)+k) = 0; //terminator \0
			//printf("%s\n",*(str_array+i));
		} else {
			*(str_array+i) = (char*)malloc(index[i]-index[i-1]);
			for(k=0; k<index[i]-index[i-1]-1; k++) {
				*(*(str_array+i)+k) = *(str + index[i-1] + k +1);
			}
			*(*(str_array+i)+k) = 0; //terminator \0
			//printf("%s\n",*(str_array+i));
		}
	}
	*(str_array+j) = 0;
	*retArray = str_array;
	return j;
}

char* str_multi_cat(int num,  char del, ...) {
	char* str = (char*)malloc(MAX_FRAME_SIZE);
    va_list argmt;
    va_start(argmt,del);
    int i,j,index=0;
    for(i=0; i<num; i++) {
        char* sf = (char *)va_arg(argmt,char*);
        int len = str_length(sf);
        if(len<=0) {  //empty(0) or NULL(-1)
            //first complete the current loop then point to the new position
            // cuz each time index points to the beginning position of the loop
            #ifdef ALLOW_EMPTY_FIELD
            *(str+index) = del;
            index++;
            #else
            printf("%s\n","str_multi_cat(): empty sensor field not allowed.");
            #endif
        } else {
			for(j=0; j<len; j++) {
				*(str+index+j) = *(sf+j);
			}
			*(str+index+len) = del;
			index += (len+1);
        }
	}
    va_end(argmt);
    *(str+index) = 0;
    return str;
}

//can be used to convert the waspmote sensor payload to map
map_ptr_t str_to_map( char* str, char fdel, char sdel) {
    char** payload = NULL;
    int fields = str_split(str,fdel,&payload);  //frame delimiter
    //printf("%p %s\n",*(payload),*(payload+1));
    if(fields<=0) {
        printf("%s\n","str_to_map(): invalid parameter...");
		return NULL;
    }
    map_ptr_t map = (map_ptr_t)malloc((fields+1)* sizeof(Map));
    int i=0;
    int j=0;
    for(i=0; i<fields; i++) {
        //printf("%d %p %s\n",i,*(payload),*(payload+i));
        int len = str_length(*(payload+i));
        for(j=0; j<len; j++) {
			if(*(*(payload+i)+j) == sdel) {
				(map+i)->name = str_get_between_index(*(payload+i),0,j-1);
				(map+i)->value = str_get_between_index(*(payload+i),j+1,len-1);
				break;
			}
		}
	}
    FREE_STRING_ARRAY(payload);
    (map+fields)->name = NULL;  //in oder to calculate the size
    return map;
}

int map_to_str(map_ptr_t map,  char del, char** str) {
	if(!map || !str) PARAMETER_ERROR("map_to_str");
	int nlen = str_length(map->name);
	int vlen = str_length(map->value);
	*str = (char*)malloc(nlen+vlen+2);
	int i;
	for(i=0; i<nlen; i++) {
		*(*str+i) = *(map->name+i);
	}
	*(*str+nlen) = del;
	for(i=0; i<vlen; i++) {
		*(*str+nlen+1+i) = *(map->value+i);
	}
	*(*str+nlen+vlen+1) = 0;
	return (nlen+vlen+1);
}

int map_cat(map_ptr_t map,  char fdel,  char sdel, char** str) {
	if(!map) PARAMETER_ERROR("map_cat");
	/*
	int fields = map_get_size(map);
	char** sensors = (char**)malloc((fields+1)*sizeof(char*));
	int i,len=0;
	for(i=0; i<fields; i++) {
		len += map_to_str(map+i,sdel,&(*(sensor+i)));
	}
	*(sensors+fields) = NULL;
	*str = (char*)str_multi_cat(fields,);
	*/
	char* str_temp = (char*)malloc(MAX_FRAME_SIZE); //initialize the memory block so str_length() returns 0
	memset(str_temp,0,MAX_FRAME_SIZE);
	int i = 0;
	int len = 0;
    while(((map+i)->name)) {
        char* sensor = NULL;
        //len += str_cat((map+i)->name,(map+i)->value,sdel,&sensor); str_cat() will add sdel in the end!!
        len += map_to_str(&map[i],sdel,&sensor);
        strncat(str_temp,sensor,len);   //will not change len 
        *(str_temp+len+i) = fdel;
        free(sensor);
        i++;
    }
    if(len+i != str_length(str_temp)) {
		printf("%d %d %s %s\n",len+i,str_length(str_temp),str_temp,"algorithm error in map_cat()...");
		exit(EXIT_FAILURE);
	}
    *str = str_get_between_index(str_temp,0,len+i-2);  //remove the last del otherwise an extra del will appear when passed as an parameter to str_multi_cat()
    free(str_temp);
	return (len+i-1);
}

int map_get_size(map_ptr_t map) {
    if(!map) PARAMETER_ERROR("map_get_size");
    int i = 0;
    while(((map+i)->name)) {
        i++;
    }
    return i;
}

char* map_get_value(map_ptr_t map,char* name) {
	if(!map || !name) PARAMETER_ERROR("map_get_value");
    int i = 0;
    while(!((map+i)->name)) {
        if(strcmp((map+i)->name,name)==0)
			return (map+i)->value;
        i++;
    } 
    printf("%s\n","the map does not contain the iven name..");
    return NULL;
}

void map_destroy(map_ptr_t map) {
	if(!map) PARAMETER_ERROR("map_destroy");
    int i = 0;
    while(!((map+i)->name)) {
        free((map+i)->name);
        (map+i)->name = NULL;
        free((map+i)->value);
        (map+i)->value = NULL;
        i++;
    } 
    free(map);
    map = NULL;
}

void data_destroy(data_ptr_t dptr) {
	if(!dptr) PARAMETER_ERROR("data_destroy");
	free(dptr->address);
	dptr->address = NULL;
	map_destroy(dptr->payload);
	free(dptr);
	dptr = NULL;
}

void data_print(data_ptr_t dptr) {
	if(!dptr) PARAMETER_ERROR("data_print");
	printf("receive sensor data @ address=%s\n",dptr->address);
	printf("%s\n","\tname\tvalue");
	int i = 0;
	while((dptr->payload+i)->name != NULL) {
		printf("\t%s\t%s\n",(dptr->payload+i)->name,(dptr->payload+i)->value);
		i++;
	}
}

int main() {
    tty_serial_init();
    
    #ifdef RD
    #ifdef AU
    while(1) {
		int bytes = 0;
		while(bytes<=0) {
			bytes = tty_serial_read(tty_fds,inbuf);
			//printf("received data length: %d\n",bytes);
		}
		printf("received frame length: %d %d %d %d %d\n",bytes,inbuf[2],inbuf[inbuf[2]+3],inbuf[5],inbuf[6]);
		data_ptr_t mydata = xbee_data_read(&inbuf[3],bytes-3);
		if(mydata) {      
			data_print(mydata);
			free(mydata);
		}
	}	
		    
    #else
    int bytes = 0;
    while(bytes<=0) {			
		memset(inbuf,0,MAX_FRAME_SIZE);
		bytes = read(tty_fds,inbuf,MAX_FRAME_SIZE);
		printf("received data length: %d %d %d %d %d\n",bytes,inbuf[2],inbuf[inbuf[2]+4+2],inbuf[5],inbuf[6]);
		sleep(3);
	}
			
	bytes = xbee_frame_descape(inbuf,bytes);
	printf("received data length: %d %d %d %d %d\n",bytes,inbuf[2],inbuf[inbuf[2]+3],inbuf[5],inbuf[6]);
	if(xbee_set_checksum(&inbuf[3],inbuf[2]+1)) printf("%s\n","checksum error");
	
	int logfds = open("./esclog.txt", O_CREAT | O_RDWR | O_APPEND);
	int ret = 0, leng = bytes;
	String fbuf = inbuf;
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
		
	data_ptr_t mydata = xbee_data_read(&inbuf[3],bytes-3);
	if(mydata) {      
		data_print(mydata);
		free(mydata);
	}
	#endif
	
	if(close(tty_fds)==-1) perror("close ttyUSB file");
    #endif 
    
    #ifdef WR
    /* The coordinator can be addressed by either setting the 64-bit address 
    to all 0x00s and the 16-bit address to 0xFFFE, 
    OR by setting the 64-bit address to the coordinator's 64-bit address 
    and the 16-bit address to 0x0000. 
    Set to the 16-bit address of the destination device, if known. 
	Set to 0xFFFE if the address is unknown or if sending a broadcast */ 
    //String mac_addr = hex_to_str("0013A200407A247A"); //0013A20040795BCE
    //String ntw_addr = hex_to_str("FFFE");  
    //char mac_addr[9] = {0x00,0x13,0xA2,0x00,0x40,0x7A,0x24,0x7A};
    #ifndef AP
    char mac_addr[9] = {0x00,0x13,0xA2,0x00,0x40,0x79,0x5B,0xCE};
    char ntw_addr[3] = {0xFF,0xFE};
    String RFData = NULL;
    map_ptr_t payload = str_to_map("Temp:35#GPS:31.200;42.100#DATE:12-01-01#",'#',':');
    int fields = map_get_size(payload);
	int len = wframe_assemble(0x80,fields,"356906593",56,payload,&RFData); //356906593#WASPMOTE_XBEE
    int bytes = xbee_txReqst_frame_assemble(mac_addr,ntw_addr,0,0,RFData,len); 
    
    //free(mac_addr);
    //free(ntw_addr);
    map_destroy(payload);
    free(RFData);
    
    printf("frame length: %d RF length: %d\n",bytes,len);
    //bytes = xbee_frame_escape(bytes);   
      
    int logfds = open("./esclog.txt", O_CREAT | O_RDWR | O_APPEND);
	int ret = 0, leng = bytes;
	String fbuf = outbuf;
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
	
	#else
	
	int bytes = xbee_localAT_frame_assemble("AP",AP);
	printf("frame length: %d\n",bytes);
    //bytes = xbee_frame_escape(bytes);	
	
	#endif
	
	tty_serial_write(tty_fds,outbuf,bytes); 
    if(close(tty_fds)==-1) perror("close tty file");  
    
    /*
    outbuf[0] = 0x7E;
    outbuf[1] = 0x00;
    outbuf[2] = 0x13;
    outbuf[3] = 0x10;
    outbuf[4] = 0x01;
    outbuf[5] = 0x00;
    outbuf[6] = 0x13;
    outbuf[7] = 0xA2;
    outbuf[8] = 0x00;
    outbuf[9] = 0x40;
    outbuf[10] = 0x7A;
    outbuf[11] = 0x24;
    outbuf[12] = 0x7A;
    outbuf[13] = 0xFF;
    outbuf[14] = 0xFE;
    outbuf[15] = 0x00;
    outbuf[16] = 0x00;
    outbuf[17] = 0x68;
    outbuf[18] = 0x65;
    outbuf[19] = 0x6C;
    outbuf[20] = 0x6C;
    outbuf[21] = 0x6F;
    outbuf[22] = 0xD0; 
    
    int bytes = 23;
    //bytes = xbee_frame_escape(23);
    
    int logfds = open("./esclog.txt", O_CREAT | O_RDWR | O_APPEND);
	int ret = 0, leng = bytes;
	String fbuf = outbuf;
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
	
    //tty_serial_write(tty_fds,outbuf,bytes);
    printf("write bytes: %d\n",bytes); 
    
    //FILE* tty_stm = fdopen(tty_fds,"w");
    //fflush(tty_stm);
    //if(fclose(tty_stm)==-1) perror("close ttyUSB file");
    if(close(tty_fds)==-1) perror("close tty file"); */
    #endif
    
    return 0;
}

