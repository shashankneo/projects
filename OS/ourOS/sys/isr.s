#include <sys/isr.h>

.text

.globl divide_zero_isr
.align 8
divide_zero_isr:
    push $0 #error code
    push $0 #interrupt number
    jmp generic_isr

.globl timer_interrupt_isr
.align 8
timer_interrupt_isr:
    push $0 #error code
    push $32 #interrupt number
    jmp generic_isr

.globl kbd_interrupt_isr
.align 8
kbd_interrupt_isr:
    push $0 #error code
    push $33 #interrupt number
    jmp generic_isr

.global double_fault_isr
.align 8
double_fault_isr:
    push $8 #interrupt number
    jmp generic_isr

.global invalid_task_switch_isr
.align 8
invalid_task_switch_isr:
    push $10 #interrupt number
    jmp generic_isr

.global seg_not_present_isr
.align 8
seg_not_present_isr:
    push $11 #interrupt number
    jmp generic_isr

.global stack_exception_isr
.align 8
stack_exception_isr:
    push $12 #interrupt number
    jmp generic_isr

.global general_prot_fault_isr
.align 8
general_prot_fault_isr:
    push $13 #interrupt number
    jmp generic_isr

.global page_fault_isr
.align 8
page_fault_isr:
    push $14 #interrupt number
    jmp generic_isr

.globl debug_isr
.align 8
debug_isr:
    push $0 #error code
    push $1 #interrupt number
    jmp generic_isr


.globl nmi_isr
.align 8
nmi_isr:
    push $0 #error code
    push $2 #interrupt number
    jmp generic_isr

.globl brk_isr
.align 8
brk_isr:
    push $0 #error code
    push $3 #interrupt number
    jmp generic_isr

.globl overflow_isr
.align 8
overflow_isr:
    push $0 #error code
    push $4 #interrupt number
    jmp generic_isr

.globl bound_check_isr
.align 8
bound_check_isr:
    push $0 #error code
    push $5 #interrupt number
    jmp generic_isr

.globl illigal_opcode_isr
.align 8
illigal_opcode_isr:
    push $0 #error code
    push $6 #interrupt number
    jmp generic_isr

.globl device_not_available_isr
.align 8
device_not_available_isr:
    push $0 #error code
    push $7 #interrupt number
    jmp generic_isr

.globl floating_point_error_isr
.align 8
floating_point_error_isr:
    push $0 #error code
    push $16 #interrupt number
    jmp generic_isr

.globl alignment_error_isr
.align 8
alignment_error_isr:
    push $0 #error code
    push $17 #interrupt number
    jmp generic_isr

.globl machine_check_isr
.align 8
machine_check_isr:
    push $0 #error code
    push $18 #interrupt number
    jmp generic_isr

.globl simd_error_isr
.align 8
simd_error_isr:
    push $0 #error code
    push $19 #interrupt number
    jmp generic_isr

// This will be called only for SYSCALL instruction
.globl syscall_isr
.align 8
syscall_isr:
    // SYSCALL have its own way of passing arguments
    //      Like: RCX has RIP, 
    //            R11 has RFLAG
    //            RSP is pointing to USER stack only [need to check again??]

    // RSP
    movq %rsp, -16(%rsp)

    // SS
    movq $0x2B, -8(%rsp)

    sub $16, %rsp

    // Rflag
    push %r11

    // CS
    push $0x23
    
    // RIP
    push %rcx
    
    push $0 #error code
    push $128 #interrupt number
    
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rbp
    push %rsi
    push %rdi
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15

    mov %es, %rax
    push %rax

    mov %ds, %rax
    push %rax

    mov %gs, %rax
    push %rax

    mov %fs, %rax
    push %rax

    # pass stack pointer (where we stored all these registers) as a argument
    # to generic_handler
    mov %rsp, %rdi
    call generic_handler
    pop %rax
    mov %rax, %fs

    pop %rax
    mov %ax, %gs

    pop %rax
    mov %ax, %ds

    pop %rax
    mov %ax, %es

    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8

    pop %rdi
    pop %rsi
    pop %rbp
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax

    # skip error_code and interrupt_number 16B
    add $16, %rsp

    # skip cs, ip, rflag, rsp, ss
    add $40, %rsp

    sysret


// This will be called only for int 0x80 instruction
.globl system_call_isr
.align 8
system_call_isr:
    push $0 #error code
    push $128 #interrupt number
    jmp generic_isr

generic_isr:
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rbp
    push %rsi
    push %rdi
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15

    mov %es, %rax
    push %rax

    mov %ds, %rax
    push %rax

    mov %gs, %rax
    push %rax

    mov %fs, %rax
    push %rax

    # pass stack pointer (where we stored all these registers) as a argument
    # to generic_handler
    mov %rsp, %rdi
    call generic_handler
    pop %rax
    mov %rax, %fs

    pop %rax
    mov %ax, %gs

    pop %rax
    mov %ax, %ds

    pop %rax
    mov %ax, %es

    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8

    pop %rdi
    pop %rsi
    pop %rbp
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax

    # skip error_code and interrupt_number 16B
    add $16, %rsp

    iretq
