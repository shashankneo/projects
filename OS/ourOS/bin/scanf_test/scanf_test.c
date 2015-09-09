#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[], char* envp[])
{
    char s1[100] = {1,2,65,66,67,68,69,70,71,72};
    int i,x;
    char c;
    printf("This is scanf test(i,x,c,s).. I am sleeping for 10 sec:\n");
    sleep(10);
    scanf("%d", &i);
    scanf("%x", &x);
    scanf("%c", &c);
    scanf("%s", s1);
    printf("Integer    : %d\n", i);
    printf("Hex Integer: %x\n", x);
    printf("character  : %c\n", c);
    printf("string     : %s\n", s1);
}
