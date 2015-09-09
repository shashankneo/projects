#ifndef _IDT_H_
#define _IDT_H_

#include <sys/defs.h>
//Macros for all interrupts
#define GENERAL_PROT_FAULT_HANDLER 13
#define PAGEFAULT_HANDLER 14
#define TIMER_INTERRUPT 32
#define KEYBOARD_INTERRUPT 33
#define SYSCALL_INTERRUPT 128

#define FLAG_IF 0x200

#define TIMER_MS_OVERFLOW 10000000
#define IDT_ENTRY(idt_entry, func_addr, sel, t, d, pst) \
    {(idt_entry)->offset_part_1 = ((uint64_t)func_addr & 0xFFFF); \
     (idt_entry)->selector = (sel & 0xFFFF); \
     (idt_entry)->unused = 0; \
     (idt_entry)->type = t; \
     (idt_entry)->s = 0; \
     (idt_entry)->dpl = (d & 0x3); \
     (idt_entry)->p = pst; \
     (idt_entry)->offset_part_2 = ((((uint64_t)func_addr >> 16) & 0xFFFF)); \
     (idt_entry)->offset_part_3 = ((((uint64_t)func_addr >> 32) & 0xFFFFFFFF)); \
     (idt_entry)->unused2 = 0;}

struct idtr_t {
    uint16_t size;
    uint64_t addr;
}__attribute__((packed));

struct idt_entry_t
{
    uint16_t offset_part_1;
    uint16_t selector;
    uint8_t  unused;
    uint64_t type : 4;
    uint64_t s : 1;
    uint64_t dpl : 2;
    uint64_t p : 1;
    uint16_t offset_part_2;
    uint32_t offset_part_3;
    uint32_t unused2;
}__attribute__((packed));


typedef struct _isr_arguments_
{
    // uint16_t fs_padding1;
    // uint16_t fs_padding2;
    uint64_t fs;

    uint64_t gs;
    // uint16_t gs_padding1;
    // uint32_t gs_padding2;

    uint64_t ds;
    // uint16_t ds_padding1;
    // uint32_t ds_padding2;

    uint64_t es;
    // uint16_t es_padding1;
    // uint32_t es_padding2;

    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;

 
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rbp;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    uint64_t interrupt_number;
    uint64_t error_code;
    /*http://pdos.csail.mit.edu/6.097/lec/l8.html*/
    /* Below are pushed automatically by the user to kernel switch*/
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;

}__attribute__((packed)) isr_arguments_t;

extern struct idt_entry_t idt[];
void init_idt();
void generic_handler(isr_arguments_t *p_isr_arg);
void timer_interrupt_handler();


#endif
