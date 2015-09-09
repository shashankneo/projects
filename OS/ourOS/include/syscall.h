#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <sys/defs.h>
#include <sys/syscall.h>

//test
static __inline int64_t syscall_0(uint64_t n) 
{
	int64_t ret;    
    __asm__ __volatile__  ("int $0x80\n"
                            :"=a"(ret)
                            :"a"(n)
                            :"rcx", "rbx", "rdx", "rsi", "rdi", "memory", "cc"
                            );  
	if(ret<0)
	{
		errno=-ret;
	}
    return ret;
}

static __inline int64_t syscall_1(uint64_t n, uint64_t a1) 
{
	int64_t ret;
    __asm__ __volatile__  ("int $0x80\n"
                            :"=a"(ret)     
                            :"a"(n),"D"(a1)
                            :"rcx", "rbx", "rdx", "rsi", "memory", "cc"
                            );
                            
    if(ret<0)
	{
		errno=-ret;
	}
	return ret;
}

static __inline int64_t syscall_2(uint64_t n, uint64_t a1, uint64_t a2) 
{
	int64_t ret;
    __asm__ __volatile__  ("int $0x80\n"
                            :"=a"(ret)     
                            :"a"(n),"D"(a1),"S"(a2)
                            :"rcx", "rbx", "rdx", "memory", "cc"
                            );    
                            
    if(ret<0)
	{
		errno=-ret;
	}
	return ret;
}

static __inline int64_t syscall_3(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3) 
{
	int64_t ret;
    __asm__ __volatile__  ("int $0x80\n"
                            :"=a"(ret)     
                            :"a"(n),"D"(a1),"S"(a2),"d"(a3)
                            :"rcx", "rbx", "memory", "cc"
                            );    
                            
    if(ret<0)
	{
		errno=-ret;
	}
	return ret;
}
void wait_in_kernel();
#endif
