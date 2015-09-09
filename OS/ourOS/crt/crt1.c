#include <stdlib.h>

#include <stdio.h>

// .
// .
// envp    <-- stack[3 + argc + 1]
// 0       <-- stack[3 + argc]
// .       
// .
// argv    <-- stack[3]
// argc    <-- stack[2]
// 0       <-- stack[1]
// 0       <-- stack[0]
static int ret = 0;
void _start(void)
{
    uint64_t stack[1] = {0};

    ret = main(stack[2],
              (char **)&stack[3],
              (char **)&stack[3 + stack[2] + 1]);
    
    exit(ret);
}
