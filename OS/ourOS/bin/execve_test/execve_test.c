#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[], char* envp[])
{
    char name[50] = "bin/argv_test";
    char *argv_temp[] = {"hello", "how", "are", "you", ""};
    argv_temp[4] = NULL;
    /*
    int i;
    
    printf("In execve_test.c\n");
    for (i = 0; i < argc; i++)
    {
        printf("Argc %d: %s\n", i+1, argv[i]);
    }

    for (i = 0; envp[i]; i++)
    {
        printf("envp %d: %s\n", i+1, envp[i]);
    }

    */
    execve(name, argv_temp, envp);
    printf("execve failed with %d\n", errno);
}
