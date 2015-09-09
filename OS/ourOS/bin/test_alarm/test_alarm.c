#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[], char* envp[])
{
    int rc = 0;
    rc = alarm(10);
    printf("First alarm: %d\n", rc);
    sleep(5);
    rc = alarm(10);
    printf("Second alarm: %d  [Expected value around 5 secs]\n", rc);
    int i = 1;
    while(1)
    {
        sleep(1);
        printf("Infinite loop [%d]  [max around 10]\n", i++);
    }
}
