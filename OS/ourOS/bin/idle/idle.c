#include <stdio.h>
#include <stdlib.h>
#include <our_string.h>

int reap_zombies()
{
    struct proc_struct proc;

    int rc = 0;
    int status = 0;
    memset(&proc, 0, sizeof(struct proc_struct));
    while (1)
    {
        rc = ps(&proc);
        if ( rc < 0 )
        {
            break;
        }
        if ( proc.ppid == getpid() && proc.state == PROC_TERMINATED)
        {
            waitpid(proc.pid, &status, 0);
            #if LOG
            printf("Successfully Reaped Zombie with pid %d\n", proc.pid);
            #endif
        }
    }
    return 0;
}

int main(int argc, char* argv[], char* envp[])
{
    pid_t pid = fork();

    uint64_t count = 0;
    
    if ( pid == 0 )
    {
        /* Child. Start the shell */
        execve("/bin/sbush", argv, envp);
        printf("Failed to start shell\n");
        exit(-1);
    }

    while (1)
    {
        count++;
        if ( (count % 214748364) == 0)
        {
            //printf("Reaping Zombies from init\n");
            reap_zombies();
        }
        //sleep(0);
    }
    return 0;
}
