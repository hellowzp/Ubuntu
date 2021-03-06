#include <stdio.h>
#include <stdlib.h>

char* byte_to_bstring(unsigned char num) {
	char* str = malloc(9);
	int i;
	for(i=0; i<8; i++) {
		str[8-1-i]=num%2+48;
		num = num/2;
	}
	str[8] = 0;
	return str;
}


int main() {
	void* p;
	int a = 1;
	struct s {    //change the order of elements, defferent results 
		char c; 
		int b;
		char e;
	};

	struct s d;
	d.b = 2;
	d.c = 3;
	d.e = 4;

	int size = sizeof(a)+(int)sizeof(d);	
	p = malloc(size);
	*(int*)p = a;
	*(struct s*)(p+sizeof(a)) = d;

	int i;
	printf("%d %ld %d\n",sizeof(a),sizeof(d),size);
	for(i=0;i<size;i++) { 
		char * s = byte_to_bstring(*(unsigned char*)(p+i));		
		printf("%d %s\n",(int)(*(unsigned char*)(p+i)), s);
		free(s);
	}
	free(p);

	return 0;
}
