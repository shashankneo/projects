#include<stdio.h>
#include<stdlib.h>
#include<our_string.h>

int main ( int argc, char *argv[], char *envp[] )
{
    int pid = 0;
    int rc  = 0;
    char err_string[64]={0};

    if ( argc != 2)
    {
        printf("Usage: kill pid\n");
        return -1;
    }
    rc = atoi(&pid, argv[1]);
    if ( rc < 0 )
    {
        printf("%s:%s:Invalid PID\n", argv[1]);
        return -1;
    }
    rc = kill(pid);
    if ( rc < 0 )
    {
        sprintf(err_string, "%s:(%d)", "kill", pid);
        print_errno(errno, err_string);
        return rc;
    }
    return 0;
}

