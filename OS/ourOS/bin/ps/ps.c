#include<stdio.h>
#include<stdlib.h>
#include<our_string.h>

char proc_state[][20] = {"Created", "Running", "Runnable", "Waiting", "Zombie", "Unknown"};

int main ( int argc, char *argv[], char *envp[] )
{
    struct proc_struct proc;

    int rc = 0;

    if ( argc != 1 )
    {
        printf("Usage: ps\n");
        return -1;
    }
    memset(&proc, 0, sizeof(struct proc_struct));
    printf("PID     PPID       STATE       CMD\n");
    while (1)
    {
        
        rc = ps(&proc);
        if ( rc < 0 )
        {
            break;
        }
        if (proc.state != PROC_TERMINATED)
        {
            printf("%d       %d          %s      %s\n", proc.pid,proc.ppid, 
                proc_state[proc.state], proc.name);
        }
    }
    
    //printf("Number of processes in system = %d\n", ps(NULL));
    return 0;
}
