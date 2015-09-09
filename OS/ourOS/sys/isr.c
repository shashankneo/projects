#include <sys/isr.h>
#include <sys/sbunix.h>
#include <sys/kbd.h>
#include <sys/kstring.h>
#include <sys/console.h>
#include <sys/process.h>
#include <sys/pagetable.h>
#include <sys/syscall.h>
#include <sys/reg.h>
#include <sys/process_mem_layout.h>
#include <sys/console_driver.h>
#include <sys/assembly_utility.h>
#include <sys/kmalloc.h>
#include <stdio.h>

extern void divide_zero_isr(void);
extern void timer_interrupt_isr(void);
extern void kbd_interrupt_isr(void);
extern void general_prot_fault_isr(void);

extern void double_fault_isr(void);
extern void invalid_task_switch_isr(void);
extern void seg_not_present_isr(void);
extern void stack_exception_isr(void);
extern void general_prot_fault_isr(void);
extern void page_fault_isr(uint64_t error_code);
extern void debug_isr(void);
extern void nmi_isr(void);
extern void brk_isr(void);
extern void overflow_isr(void);
extern void bound_check_isr(void);
extern void illigal_opcode_isr(void);
extern void device_not_available_isr(void);
extern void floating_point_error_isr(void);
extern void alignment_error_isr(void);
extern void machine_check_isr(void);
extern void simd_error_isr(void);

extern void system_call_isr(void);

struct idt_entry_t idt[256];

static struct idtr_t idtr = {
    sizeof(idt) - 1,
    (uint64_t)idt,
};

int timer=0;
//pre_seconds store the time in seconds before timer was
//reset to prevent overflow
int pre_seconds=0;

void init_idt()
{
    IDT_ENTRY(&idt[0], divide_zero_isr, 0x8, 0xE, 0, 1);
    IDT_ENTRY(&idt[TIMER_INTERRUPT], timer_interrupt_isr, 0x8, 0xE, 0, 1);
    IDT_ENTRY(&idt[KEYBOARD_INTERRUPT], kbd_interrupt_isr, 0x8, 0xE, 0, 1);
    IDT_ENTRY(&idt[SYSCALL_INTERRUPT], system_call_isr, 0x8, 0xE, 3, 1);
    
    IDT_ENTRY(&idt[8], double_fault_isr, 0x8, 0xE, 0, 1);
    IDT_ENTRY(&idt[10], invalid_task_switch_isr, 0x8, 0xE, 0, 1);
    IDT_ENTRY(&idt[11], seg_not_present_isr, 0x8, 0xE, 0, 1);
    IDT_ENTRY(&idt[12], stack_exception_isr, 0x8, 0xE, 0, 1);
    IDT_ENTRY(&idt[13], general_prot_fault_isr, 0x8, 0xE, 0, 1);
    IDT_ENTRY(&idt[14], page_fault_isr, 0x8, 0xE, 0, 1);
    IDT_ENTRY(&idt[1], debug_isr, 0x8, 0xE, 0, 1);
    IDT_ENTRY(&idt[2], nmi_isr, 0x8, 0xE, 0, 1);
    IDT_ENTRY(&idt[3], brk_isr, 0x8, 0xE, 0, 1);
    IDT_ENTRY(&idt[4], overflow_isr, 0x8, 0xE, 0, 1);
    IDT_ENTRY(&idt[5], bound_check_isr, 0x8, 0xE, 0, 1);
    IDT_ENTRY(&idt[6], illigal_opcode_isr, 0x8, 0xE, 0, 1);
    IDT_ENTRY(&idt[7], device_not_available_isr, 0x8, 0xE, 0, 1);
    IDT_ENTRY(&idt[16], floating_point_error_isr, 0x8, 0xE, 0, 1);
    IDT_ENTRY(&idt[17], alignment_error_isr, 0x8, 0xE, 0, 1);
    IDT_ENTRY(&idt[18], machine_check_isr, 0x8, 0xE, 0, 1);
    IDT_ENTRY(&idt[19], simd_error_isr, 0x8, 0xE, 0, 1);
    
    
    __asm__ __volatile__ (  "lidt (%0)\n"
                            :
                            :"r"(&idtr)
                            :
                            );
    
    uint16_t tss_selector = 0x28;
    __asm __volatile("ltr %0" : : "r" (tss_selector));
}

void pic_eoi(uint64_t interrupt_number)
{
    if (interrupt_number >= 32 && interrupt_number <= 47)
    {
        if(interrupt_number >= 40)
        {
            outb(0xA0,0x20);
        }
        outb(0x20,0x20);
    }
}

int32_t
pagefault_handler(isr_arguments_t *p_isr_arg)
{
    uint64_t fault_addr;
    uint32_t pgfault_type = p_isr_arg->error_code & 0x7;
    int32_t ret_code = 0;
    uint64_t *faulting_pte;
    uint64_t *proc_pte = NULL;
    uint64_t req_perm = 0;

    __asm__ __volatile__("mov %%cr2,%0":"=r"(fault_addr)::);
    //printf("Fault @ %p\n", fault_addr);

    req_perm |= (p_isr_arg->rip == fault_addr) ? VMA_PERM_EXEC : 0;

    switch(pgfault_type)
    {
        case USER_READ_NO_PTE_FAULT:
            ret_code = user_vma_update( fault_addr,
                                        req_perm | VMA_PERM_READ,
                                        p_isr_arg->rsp);
            break;

        case USER_WRITE_NO_PTE_FAULT:
            ret_code = user_vma_update( fault_addr,
                                        req_perm | VMA_PERM_WRITE,
                                        p_isr_arg->rsp);
            break;

        case USER_READ_PROT_FAULT:
            #ifdef LOG
                printf("User level read protection fault %d\n", pgfault_type);
            #endif
            ret_code = -1;
            break;
        case USER_WRITE_PROT_FAULT:
            //Look up faulting page table entry in curr proc page table
            faulting_pte = page_lookup( PHYS_TO_VA(
                                        curr_running_proc->page_table_base)
                                        ,fault_addr);

            //We haven't checked for PTE_P because then USER_WRITE_NO_PTE_FAULT
            // would have happened
            //Check for Whether the faulting PTE has PTE_COW bit set
            if( *faulting_pte &  PTE_COW  )
            {
               //Find the shared page 
               Pages* shared_page = PHYS_TO_PAGE(*faulting_pte);
               //Roundown address for faulting shit
               uint64_t* fault_rdown_addr =(uint64_t*) ROUNDDOWN(fault_addr,
                    PGSIZE, uint64_t);
                    
               // Check the reference count for this faulting page
               if( shared_page->ref == 1)
               {
                   //This means no other process is referencing this page
                   //So simply make the bit PTE_COW as PTE_W
                   //Map this faulting pte entry with appropriate permissions
                    proc_pte = page_lookup(PHYS_TO_VA( curr_running_proc->page_table_base),
                                           (uint64_t)fault_rdown_addr);
                    if (proc_pte)
                    {
                        *proc_pte = (*proc_pte & MASK_PAGE_PERM) | ( PTE_U | PTE_W | PTE_P);
                    }

                    //Invalidate the tlb entry
                    invalidate_tlb((uint64_t)fault_addr);
               }
               else
               {
                    //Get new physical page for faulting page. Cow write happening
                    uint64_t* new_page = page_alloc();
                    if( new_page == NULL)
                    {
                        #ifdef LOG
                            printf(" No more physical memory available for COW \n");
                        #endif
                        ret_code = -ERR_NOMEM;
                        break;
                    }
                    
                    //Copy memory from faulting page to our COW page in kernel
                    memcpy((uint64_t*)KERNEL_COW_START, fault_rdown_addr,PGSIZE);

                    proc_pte = page_lookup(PHYS_TO_VA( curr_running_proc->page_table_base),
                                           (uint64_t)fault_rdown_addr);
                    if (proc_pte)
                    {
                        *proc_pte = ((uint64_t)new_page & MASK_PAGE_PERM) | ( PTE_U | PTE_W | PTE_P);
                    }

                    //Invalidate the tlb entry
                    invalidate_tlb((uint64_t)fault_addr);

                    //Copy memory from COW page to our faulting page
                    memcpy(fault_rdown_addr,(uint64_t*)KERNEL_COW_START,
                    PGSIZE);
                    
                    //Decrease the shared page ref
                    shared_page->ref--;
               }
               return 0;
            }
            else
            {
                #ifdef LOG
                    printf("ULPF by [%d]:%s @addr:%p rip:%p\n",
                            curr_running_proc->pid,
                            curr_running_proc->name,
                            fault_addr,
                            p_isr_arg->rip
                            );
                #endif
                ret_code = -1;
            }
           
            break;
        
        case KERNEL_READ_NO_PTE_FAULT:
        case KERNEL_WRITE_NO_PTE_FAULT:
        case KERNEL_READ_PROT_FAULT:
        case KERNEL_WRITE_PROT_FAULT:
            #ifdef LOG
                printf("Kernel level protection fault %d\n", pgfault_type);
            #endif
            ret_code = -1;

            break;
    }

    if (ret_code < 0)
    {
        if ( ret_code == -ERR_NOMEM )
        {
            printf("No memory left: Unable to demand page. Killing process\n");
        }
        else
        {
            printf("Segmentation fault\n");
        }
        kill_process(curr_running_proc);
    }

    if (curr_running_proc == NULL)
    {
        #ifdef LOG
            printf("We have no user process running and we hit a page fault... BAD!!\n");
        #endif
        return -1;
    }

    // WE SHOULD REACH HERE ONLY FOR USER MODE PROCESS

    ret_code = user_demand_paging_handler(fault_addr);
    if ( ret_code < 0)
    {
        if ( ret_code == -ERR_NOMEM )
        {
            printf("No memory left: Unable to demand page. Killing process\n");
        }
        else if (ret_code == -ERR_FAULT)
        {
            printf("Segmentation Fault\n");
        }
        kill_process(curr_running_proc);
    }

    return ret_code;
}

void gpfault_handler(void)
{
    printf("General protection fault\n");
    kill_process(curr_running_proc);
}


void generic_handler(isr_arguments_t *p_isr_arg)
{
    //If we have a current running process then save its register
    //state in process descriptors
    if (p_isr_arg->cs == GDT_USER_CS &&
        curr_running_proc != NULL )
    {
        curr_running_proc->registers = *p_isr_arg;
    }
    
    switch(p_isr_arg->interrupt_number)
    {
        case TIMER_INTERRUPT:
            timer_interrupt_handler();
            //Call the process schedule thing
            pic_eoi(p_isr_arg->interrupt_number);
            handle_waiting_processes();

            // Free previous process' kernel stack which we have killed
            kfree(delete_prev_stack);
            delete_prev_stack = NULL;

            process_schedule();
           break;
        case KEYBOARD_INTERRUPT:
            pic_eoi(p_isr_arg->interrupt_number);
            keyboard_handler();
            break;
        case 13:
            gpfault_handler();
            break;
        case PAGEFAULT_HANDLER:
            pagefault_handler(p_isr_arg);
            break;
        case SYSCALL_INTERRUPT:
            curr_running_proc->registers.rax = syscall(p_isr_arg->rax,
                                                       p_isr_arg->rdi,
                                                       p_isr_arg->rsi,
                                                       p_isr_arg->rdx,
                                                       0,
                                                       0);
            break;
        default:
            #ifdef LOG
                printf("\nHit fault %d\n", p_isr_arg->interrupt_number);
            #endif
            break;
    }

    if (p_isr_arg->cs == GDT_USER_CS &&
        curr_running_proc != NULL )
    {
        p_isr_arg->rax=curr_running_proc->registers.rax;
    }
}

//Handler for timer interrupt
void timer_interrupt_handler()
{
    char temp_string[MAX_STRING];
    int seconds=-1;
    timer=timer+10;
    if(timer%1000==0)
    {   
        
        seconds=timer/1000+pre_seconds;
        if(timer==TIMER_MS_OVERFLOW)
        {
            pre_seconds=pre_seconds+timer/1000;
            timer=0;
        }
        itoa(seconds, temp_string, 10);
        print_at(temp_string, 71, STATUS_BAR);

        //sprintf(temp_string,"%d Free", total_free_pages);
        //print_at(temp_string, 50, STATUS_BAR);
    }
}
