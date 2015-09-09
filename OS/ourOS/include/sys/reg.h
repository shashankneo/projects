#include <sys/defs.h>
uint64_t read_rsp();
uint64_t read_ss();
uint64_t read_rflags();
uint64_t read_cs();
uint64_t read_fs();
uint64_t read_gs();
uint64_t read_ds();
uint64_t read_es();
uint64_t read_r15();
uint64_t read_r14();
uint64_t read_r13();
uint64_t read_r12();
uint64_t read_r11();
uint64_t read_r10();
uint64_t read_r9();
uint64_t read_r8();
uint64_t read_rdi();
uint64_t read_rsi();
uint64_t read_rbp();
uint64_t read_rdx();
uint64_t read_rcx();
uint64_t read_rbx();
uint64_t read_rax();

extern void write_rsp(uint64_t rsp);
