#include <stdio.h>
#include <stdlib.h>
int main(int argc, char* argv[], char* envp[]) 
{
int a=0,b=0,c=4,d=4;
char str[20]="sdsds";
scanf("%d %d %d %d",&a,&b,&c,&d);
printf("numbers=%d %d %d %d %x %x %s\n",a,b,c,d,str,str,str+2);
return 0;

}
