#include<stdio.h>
#include<stdlib.h>

int main (int argc, char *argv[], char *envp[])
{
     int prev_fd[2] = {-1, -1}, next_fd[2] = {-1, -1}, pipefd[2];
    int i = 0;
    char buf[100];
    int pid,bytes_read,fd=1;
    //int wait_process[NUM_PROCESS]={};
    int num_process=-1;
    
    int cmd_count = 5;
    for(i=0;i<cmd_count;i++)
    {    
        if (i != (cmd_count-1))
        {
            pipe(next_fd);
        }
        pid=fork();
        if(pid==0)
        {
            //If i is equal to 0 then it takes from the stdin
            //Else it takes from previous pipe
            if(i==0)
            {
                //Close read end
                close(next_fd[0]);
                while((bytes_read=read(fd,buf,100))>0)
                {
                     write(next_fd[1],buf,bytes_read);
                }   
                //Close write end after using it
                close(pipefd[1]);
            }
            if( i != 0 )
            {
                dup2(prev_fd[0],0);
                close(prev_fd[0]);
                close(prev_fd[1]);
            }
            if( i != cmd_count-1 )
            {
                dup2(next_fd[1],1);
                close(next_fd[0]);
                close(next_fd[1]);
            }
           
            exit(1);
        }
        else
        {
            num_process++;
            //wait_process[num_process]=pid;
            if( i != 0 )
            {
                close(prev_fd[0]);
                close(prev_fd[1]);
            }
            prev_fd[0]=next_fd[0];
            prev_fd[1]=next_fd[1];
        }
    }
    for(i=0; i <= num_process; i++)
    {
     //   waitpid(wait_process[i],&status,0);
    }
    return 0;
}

