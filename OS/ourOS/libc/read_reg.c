#include <stdio.h>
#include <stdlib.h>

uint64_t read_rsp()
{
uint64_t rsp;
__asm __volatile("movq %%rsp,%0" : "=r" (rsp));
return rsp;
}
