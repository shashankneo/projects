#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[], char* envp[])
{
    int i;
    printf("In argv_test.c\n");
    for (i = 0; i < argc; i++)
    {
        printf("Argc %d: %s\n", i+1, argv[i]);
    }

    for (i = 0; envp[i]; i++)
    {
        printf("envp %d: %s\n", i+1, envp[i]);
    }
}
