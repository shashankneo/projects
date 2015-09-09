#ifndef __PROCESS__H__ 
#define __PROCESS__H__

#include<stdlib.h>
#include<sys/isr.h>
#include <sys/env_variable.h>
#include<sys/file.h>

#define INITIAL_STACK_SIZE 4096*4096

#define EI_MAG0 0
#define EI_MAG1 1
#define EI_MAG2 2
#define EI_MAG3 3
#define PT_LOAD 1

#define VMA_PERM_EXEC   0x1
#define VMA_PERM_WRITE  0x2
#define VMA_PERM_READ   0x4

#define GDT_USER_CS   ((3 << 3) | 3)
#define GDT_USER_DS   ((4 << 3) | 3)

#define GDT_KERNEL_CS   (1 << 3)
#define GDT_KERNEL_DS   (2 << 3)


#define MAX_VMAS 64 
#define MAX_FILE_DESCRIPTORS    32
#define INITIAL_STACK_PAGES     1
#define INITIAL_HEAP_PAGES      1

#define USER_KERNEL_STACK_SIZE  PGSIZE

#define VMA_STACK               1
#define VMA_HEAP                2
#define VMA_ENV_VARIABLE        4
#define VMA_ARGV                8

#define MAX_PROCESSES 5

#define MAX_FILENAME_SIZE   256

/* 32-bit ELF base types. */
typedef uint32_t   Elf32_Addr;
typedef uint16_t   Elf32_Half;
typedef uint32_t   Elf32_Off;
typedef int32_t    Elf32_Sword;
typedef uint32_t   Elf32_Word;

/* 64-bit ELF base types. */
typedef uint64_t   Elf64_Addr;
typedef uint16_t   Elf64_Half;
typedef int16_t    Elf64_SHalf;
typedef uint64_t   Elf64_Off;
typedef int32_t    Elf64_Sword;
typedef uint32_t   Elf64_Word;
typedef uint64_t   Elf64_Xword;
typedef int64_t    Elf64_Sxword;

typedef struct
{
    unsigned char e_ident[16]; /* ELF identification */
    Elf64_Half e_type; /* Object file type */
    Elf64_Half e_machine; /* Machine type */
    Elf64_Word e_version; /* Object file version */
    Elf64_Addr e_entry; /* Entry point address */
    Elf64_Off e_phoff; /* Program header offset */
    Elf64_Off e_shoff; /* Section header offset */
    Elf64_Word e_flags; /* Processor-specific flags */
    Elf64_Half e_ehsize; /* ELF header size */
    Elf64_Half e_phentsize; /* Size of program header entry */
    Elf64_Half e_phnum; /* Number of program header entries */
    Elf64_Half e_shentsize; /* Size of section header entry */
    Elf64_Half e_shnum; /* Number of section header entries */
    Elf64_Half e_shstrndx; /* Section name string table index */
} Elf64_Ehdr;

typedef struct
{
    Elf64_Word p_type; /* Type of segment */
    Elf64_Word p_flags; /* Segment attributes */
    Elf64_Off p_offset; /* Offset in file */
    Elf64_Addr p_vaddr; /* Virtual address in memory */
    Elf64_Addr p_paddr; /* Reserved */
    Elf64_Xword p_filesz; /* Size of segment in file */
    Elf64_Xword p_memsz; /* Size of segment in memory */
    Elf64_Xword p_align; /* Alignment of segment */
} Elf64_Phdr;

typedef struct
{
    Elf64_Word sh_name; /* Section name */
    Elf64_Word sh_type; /* Section type */
    Elf64_Xword sh_flags; /* Section attributes */
    Elf64_Addr sh_addr; /* Virtual address in memory */
    Elf64_Off sh_offset; /* Offset in file */
    Elf64_Xword sh_size; /* Size of section */
    Elf64_Word sh_link; /* Link to other section */
    Elf64_Word sh_info; /* Miscellaneous information */
    Elf64_Xword sh_addralign; /* Address alignment boundary */
    Elf64_Xword sh_entsize; /* Size of entries, if section has table */
} Elf64_Shdr;

typedef enum process_state
{
    PROCESS_CREATED=0,
    PROCESS_RUNNING,
    PROCESS_RUNNABLE,
    PROCESS_WAITING,
    PROCESS_TERMINATED,
    PROCESS_NONE
}process_state_t;

typedef struct vma
{
    uint64_t    vma_start;
    uint64_t    filesz;
    uint64_t    memsz;
    uint64_t    perm;
    struct vma  *vma_next;
    uint64_t    file_ptr;
    uint64_t    flags;
}vma_t;

typedef struct mm_struct
{
    vma_t       *vma_list;
    //uint64_t    map_count;
}mm_struct_t;

enum 
{
    PAPA_WAITING            = (1 << 0),
    I_AM_SLEEPING           = (1 << 1),
    PEACEFUL_DEATH          = (1 << 8),
    WAITING_FOR_PIPEREAD    = (1 << 13),
    WAITING_FOR_PIPEWRITE   = (1 << 14),
    FOREGROUND_PROCESS      = (1 << 16),
    WAITING_FOR_READ        = (1 << 17),
    ALARM_ACTIVATED         = (1 << 18),
};

typedef enum
{
    PROCESS_WAITING_QUEUE,
    PROCESS_RUNNABLE_QUEUE,
    PROCESS_TERMINATED_QUEUE,
} QueueType;

/* Return true if process exited normally */
#define WIFEXITED(status) (status & PEACEFUL_DEATH)
/* The below macro needs to be called only if prev macro returned true */
#define WEXITSTATUS(status) (status & 0xFF)

typedef struct _process_t_
{
    char               name[50];
    pid_t              pid; /* Process ID */
    pid_t              ppid; /* Parent Process ID */
    uint64_t           *page_table_base; /* Physical Base address of the process' page table */
    char               *kernel_stack; /* kernel stack */
    process_state_t    state; /* Current state of the process */
    isr_arguments_t    registers;
    uint64_t           *entry_point; /*Entry point for the process*/
    mm_struct_t        mm;
    struct _process_t_ *next;
    int64_t            exit_status;
    uint64_t           flags;
    uint64_t           sleep_ticks;
    uint64_t           alarm_ticks;
    fileDesc*           listFileDesc[MAX_FILE_DESCRIPTORS];
    Tree*               currentDirectory;
    Tree*               homeDirectory;    
}process_t;

extern char stack[INITIAL_STACK_SIZE];
extern char *delete_prev_stack;

extern process_t * curr_running_proc;
extern process_t * foreground_proc;

extern char execve_envp[MAX_ENV_COUNT][MAX_ENV_SIZE];
extern char execve_argv[MAX_ARGV_COUNT][MAX_ARGV_SIZE];
extern char execve_filename[MAX_FILENAME_SIZE];

process_t* create_process_new(void* binary, pid_t ppid,  int new_pid, 
                       const char *proc_name,
                       char * const argv[],
                       size_t argv_count,
                       char * const envp[],
                       size_t envp_count);

void context_switch_reg(isr_arguments_t    registers);
void context_switch(process_t* new_process);
int32_t init_vmas_proc(process_t *proc,
                       void* binary,
                       size_t argv_count,
                       size_t envp_count);
int launch_process(char *proc_name);
void init_process_desc();
void process_schedule();
int setup_registers_proc( process_t *proc );
int terminate_process(process_t *proc, int return_value);

int32_t handle_waiting_processes();

void create_kernel_process(uint64_t* proc_addr);
int32_t copy_pages_parent_child( pid_t curr_proc_pid, pid_t new_pid );
int32_t copy_vmas_parent_child( pid_t curr_proc_pid, pid_t new_pid );
int32_t copy_vmas_parent_child(pid_t curr_proc_pid, pid_t new_pid );

int32_t copy_filedesc_parent_child( process_t* curr_proc, process_t* new_proc, int execv);

vma_t* find_vma_by_vaddr (process_t *proc, uint64_t fault_addr);
vma_t* find_vma_by_flag(process_t *proc, uint64_t flag);
int32_t increase_vma_size(vma_t *vma, size_t size);
int findFirstFreeFDIndex(process_t*  proc);
uint64_t min(uint64_t a, uint64_t b);
uint64_t max(uint64_t a, uint64_t b);
uint64_t num_of_free_pages();

int wake_up_foreground_proc();

Tree* get_proc_binary(const char *proc_name);


void free_vma ( vma_t *vma );
uint64_t generate_pid();
process_t* get_proc_from_pid(uint64_t pid);

void zero_process_structure(process_t *proc, int new_pid);
process_t* allocate_process(int new_pid);
int32_t reclaim_process_resources(process_t *proc);
int32_t add_process_to_queue(process_t *proc, QueueType qType);
process_t* pop_process_from_queue(process_t *proc_to_be_popped);
int32_t switch_process_queue(process_t *proc, QueueType to_q);
int32_t remove_process(process_t *proc_to_be_removed);
void free_filedesc(process_t* proc);

int is_elf(const void* binary);

int kill_process(process_t *proc);
int list_process (struct proc_struct *proc_details);
int count_processes (void);
int reparent_children(process_t *proc);

#endif /* #ifndef __PROCESS__H__ */ 

