#include<stdio.h>
#include<string.h>
#include<stdlib.h>
int main()
{
char *pch;
char *str[20];
scanf("%s",&str);
pch=strtok(str," ,.-");
while(pch!=NULL)
{
printf("%s\n",pch);
pch=strtok(NULL," ,.-");
}
getchar();
return 0;
}
