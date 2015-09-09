#include <stdio.h>
#include <stdlib.h>

void rec(int i)
{
    char str[256000];
    static char a = 'a';
    str[0] = a++;
    str[1] = 0;
    if (i <= 0)
        return;
    printf("Calling from recursive  %d %s\n", i, str);
    //sleep(1);
    rec(--i);
}

int main(int argc, char* argv[], char* envp[])
{
    printf("Hello from recursive\n");
    rec(20000);
    return 0;
}
