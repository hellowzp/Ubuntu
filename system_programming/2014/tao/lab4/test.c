#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>




//unsigned char *Converter(char *);//in this method you can get a
 
/*------------------------------------------------------------------------------
		global variable declarations
------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
		implementation code
------------------------------------------------------------------------------*/


//----------------------------------------------------------------------------//
int main ( int argc,  char *argv[])
//----------------------------------------------------------------------------//
{	

	int number;
	int fullLines;
	int bytesInLastLine;
	int i, j;
	int sign;
	unsigned char *startPtr;

	printf("FIY the address of the main function is %p\n", &main);
	printf("FIY the address of the first address in main() is %p\n", &number);

        if (argc > 1)
	{

		startPtr = argv[1];
		number = atoi(argv[2]);
	}
	else if (argc == 1)
	{
		printf("Enter the start address you want to begin with\n");
		scanf("%p", &startPtr);

 		printf("Enter the number of address(s) you want to prinf\n");
		scanf("%d", &number);
	}
 	
	sign = number/abs(number);


	fullLines = abs(number)/10;
	bytesInLastLine = abs(number)%10;

	

	printf("Address        Bytes                          Chars\n");
	printf("-------------------------------------------------------------\n");

	for (i = 0; i < fullLines; i++)
		{
			
			printf("%p\t",startPtr);
			for (j = 0; j < 10; j++)
			{
				if (*(startPtr + sign*j) < 16)
				printf("0");
				printf("%x ", *(startPtr + sign*j));

				
			}				
			printf("\t");
			for (j = 0; j < 10; j++)
			{
				if (isprint(*(startPtr + sign*j)))
				{
					printf("%c", *(startPtr + sign*j));
				}
				else
				{
					printf(".");
				}
			}
			printf("\n");
			startPtr = startPtr + sign*10;
		}
	if (bytesInLastLine != 0)
	{

		printf("%p\t",startPtr);
		for(i = 0; i < 10; i++)
		{
			if (i < bytesInLastLine)
			{
				if (*(startPtr + sign*j) < 16)
				printf("0");
				printf("%x ", *(startPtr + sign*j));
 			}
			else 
			{
				printf("   ");
			}
		}
		printf("\t");
		for(i = 0; i < 10; i++)
		{
			if (i < bytesInLastLine)
			{
				if (isprint(*(startPtr + sign*j)))
				{
					printf("%c", *(startPtr + sign*j));
				}
				else
				{
					printf(".");
				}
			}
			else
			{
				printf(" ");
			}
		}
		printf("\n");
	}	
	
	return 0;
}
