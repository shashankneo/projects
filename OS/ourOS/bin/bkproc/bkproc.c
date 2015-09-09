#include <stdio.h>
#include <stdlib.h>
#include <our_string.h>

int main(int argc, char* argv[], char* envp[])
{
    int i = 0;
    printf("I am background process and running infinitely in background\n");
    while (argv[i])
    {
        printf("\targv[%d]:%s\n", i, argv[i]);
        i++;
    }
    while(1);
}
