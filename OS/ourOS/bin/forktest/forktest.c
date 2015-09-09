#include <stdio.h>
#include <stdlib.h>
#include <our_string.h>

int main(int argc, char* argv[], char* envp[])
{
    pid_t pid = fork();
    while(1)
    {
        pid = fork();
        if ( pid < 0 )
            exit(1);
        //malloc(10*4096);
    }
}
/*
int main(int argc, char* argv[], char* envp[])
{
    int count = 0;
    int rc = 0;
    //pid_t parent_pid = getpid();
    
    //while(1)
    {
        //if ( parent_pid == getpid() )
        for (int i=0; i< 5; i++)
        {
            printf(" Count is %d\n", count++);
            rc = fork();
            if ( rc < 0 )
            {
                print_errno(errno, "fork");
                printf("Number of processes in system = %d\n", ps(NULL));
                exit(1);
            }
        }
    }

    return 0;
}   
*/