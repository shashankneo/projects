#include <sys/reg.h>
uint64_t read_rsp()
{
    uint64_t rsp;
    __asm __volatile("movq %%rsp,%0" : "=r" (rsp));
    return rsp;
}

uint64_t read_ss()
{
    uint64_t ss;
    __asm __volatile("movq %%ss,%0" : "=r" (ss));
    return ss;
}

uint64_t read_rflags()
{
    uint64_t rflags;
    __asm __volatile("pushfq; popq %0" : "=r" (rflags));
    return rflags;
}

uint64_t read_cs()
{
    uint64_t cs;
    __asm __volatile("movq %%cs,%0" : "=r" (cs));
    return cs;
}

uint64_t read_fs()
{
    uint64_t fs;
    __asm __volatile("movq %%fs,%0" : "=r" (fs));
    return fs;
}

uint64_t read_gs()
{
    uint64_t gs;
    __asm __volatile("movq %%gs,%0" : "=r" (gs));
    return gs;
}

uint64_t read_ds()
{
    uint64_t ds;
    __asm __volatile("movq %%ds,%0" : "=r" (ds));
    return ds;
}

uint64_t read_es()
{
    uint64_t es;
    __asm __volatile("movq %%es,%0" : "=r" (es));
    return es;
}

uint64_t read_r15()
{
    uint64_t r15;
    __asm __volatile("movq %%r15,%0" : "=r" (r15));
    return r15;
}

uint64_t read_r14()
{
    uint64_t r14;
    __asm __volatile("movq %%r14,%0" : "=r" (r14));
    return r14;
}

uint64_t read_r13()
{
    uint64_t r13;
    __asm __volatile("movq %%r13,%0" : "=r" (r13));
    return r13;
}

uint64_t read_r12()
{
    uint64_t r12;
    __asm __volatile("movq %%r12,%0" : "=r" (r12));
    return r12;
}

uint64_t read_r11()
{
    uint64_t r11;
    __asm __volatile("movq %%r11,%0" : "=r" (r11));
    return r11;
}

uint64_t read_r10()
{
    uint64_t r10;
    __asm __volatile("movq %%r10,%0" : "=r" (r10));
    return r10;
}

uint64_t read_r9()
{
    uint64_t r9;
    __asm __volatile("movq %%r9,%0" : "=r" (r9));
    return r9;
}

uint64_t read_r8()
{
    uint64_t r8;
    __asm __volatile("movq %%r8,%0" : "=r" (r8));
    return r8;
}

uint64_t read_rdi()
{
    uint64_t rdi;
    __asm __volatile("movq %%rdi,%0" : "=r" (rdi));
    return rdi;
}

uint64_t read_rsi()
{
    uint64_t rsi;
    __asm __volatile("movq %%rsi,%0" : "=r" (rsi));
    return rsi;
}

uint64_t read_rbp()
{
    uint64_t rbp;
    __asm __volatile("movq %%rbp,%0" : "=r" (rbp));
    return rbp;
}

uint64_t read_rdx()
{
    uint64_t rdx;
    __asm __volatile("movq %%rdx,%0" : "=r" (rdx));
    return rdx;
}

uint64_t read_rcx()
{
    uint64_t rcx;
    __asm __volatile("movq %%rcx,%0" : "=r" (rcx));
    return rcx;
}

uint64_t read_rbx()
{
    uint64_t rbx;
    __asm __volatile("movq %%rbx,%0" : "=r" (rbx));
    return rbx;
}

uint64_t read_rax()
{
    uint64_t rax;
    __asm __volatile("movq %%rax,%0" : "=r" (rax));
    return rax;
}

__inline__ void write_rsp(uint64_t rsp)
{
    __asm__ __volatile__("movq %0,%%rsp" : :"r" (rsp));
}