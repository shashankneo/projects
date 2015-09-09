#include <stdio.h>
#include <stdlib.h>
int main(int argc, char* argv[], char* envp[]) {
int ret;
int pid=fork();
if( pid == 0)
{
  //  alarm(3);
    ret=sleep(5);
    printf("sleep in child=%d parent=%d for 5sec..ret code=%d \n",getpid(),getppid(),ret);
    ret=sleep(4);
    printf("sleep in child=%d parent=%d for 4sec..ret code=%d \n",getpid(),getppid(),ret);
}
else
{
  // alarm(7);
    ret=sleep(3);
    printf("sleep in parent=%d and its parentid=%d for 3sec..ret code=%d \n",getpid(),getppid(),ret);
    ret=sleep(9);
     printf("sleep in parent=%d and its parentid=%d for 9sec..ret code=%d \n",getpid(),getppid(),ret);
 
}


}
