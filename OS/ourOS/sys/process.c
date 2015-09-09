#include<sys/process_mem_layout.h>
#include<sys/tarfs.h>
#include<sys/defs.h>
#include<sys/process.h>
#include<sys/sbunix.h>
#include<sys/kstring.h>
#include<sys/pagetable.h>
#include<sys/gdt.h>
#include<sys/kmalloc.h>

char stack[INITIAL_STACK_SIZE];

char *delete_prev_stack = NULL;

process_t * curr_running_proc=NULL;
process_t * foreground_proc = NULL;

process_t *process_waiting_queue    = NULL;
process_t *process_runnable_queue   = NULL;
process_t *process_terminated_queue = NULL;

char execve_envp[MAX_ENV_COUNT][MAX_ENV_SIZE] = {{0}};
char execve_argv[MAX_ARGV_COUNT][MAX_ARGV_SIZE] = {{0}};
char execve_filename[MAX_FILENAME_SIZE] = {0};

char dummyStack[4096] = {0};

static int reparent_queue(process_t *proc, process_t *parent)
{
    while ( proc )
    {
        if ( proc->ppid == parent->pid )
        {
            proc->ppid = 1;
        }
        proc = proc->next;
    }
    return 0;
}
int reparent_children(process_t *proc)
{

    /* Search for my children */
    // 1. In Runnable Queue
    reparent_queue(process_runnable_queue, proc);

    // 2. The Waiting Queue
    reparent_queue(process_waiting_queue, proc);
    
    // 3. Terminated Queue
    reparent_queue(process_terminated_queue, proc);
    return 0;
}
int list_process (struct proc_struct *proc_details)
{
    process_t *proc = NULL;
    unsigned count = 0;

    if ( proc_details->offset < 0 )
    {
        return -1;
    }
    
    proc = process_runnable_queue;
    
    /* Print all Running/Runnable process */
    for ( ; (count < proc_details->offset) && proc;  count++ )
    {
        proc = proc->next;
    }
    if ( proc )
    {
        strncpy(proc_details->name, proc->name, sizeof(proc->name));
        proc_details->pid = proc->pid;
        proc_details->ppid = proc->ppid;
        proc_details->state = proc->state;
        proc_details->offset++;
        return 0;
    }
    proc = process_waiting_queue;
    
    /* Print all Running/Runnable process */
    for (; (count < proc_details->offset) && proc;  count++ )
    {
        proc = proc->next;
    }
    if ( proc )
    {
        strncpy(proc_details->name, proc->name, sizeof(proc->name));
        proc_details->pid = proc->pid;
        proc_details->ppid = proc->ppid;
        proc_details->state = proc->state;
        proc_details->offset++;
        return 0;
    }
    
    proc = process_terminated_queue;
    
    /* Print all Running/Runnable process */
    for (; (count < proc_details->offset) && proc;  count++ )
    {
        proc = proc->next;
    }
    if ( proc )
    {
        strncpy(proc_details->name, proc->name, sizeof(proc->name));
        proc_details->pid = proc->pid;
        proc_details->ppid = proc->ppid;
        proc_details->state = proc->state;
        proc_details->offset++;
        return 0;
    }
    proc_details->offset = -1;
    return -1;
}
process_t* get_proc_from_pid(uint64_t pid)
{
    process_t * proc;
    
    proc = process_waiting_queue;
    while(proc)
    {
        if (proc->pid == pid)
        {
            return proc;
        }
        proc = proc->next;
    }

    proc = process_runnable_queue;
    while(proc)
    {
        if (proc->pid == pid)
        {
            return proc;
        }
        proc = proc->next;
    }

    proc = process_terminated_queue;
    while(proc)
    {
        if (proc->pid == pid)
        {
            return proc;
        }
        proc = proc->next;
    }

    return NULL;
}

void zero_process_structure(process_t *proc, int new_pid)
{
    if (!proc)
        return;

    proc->name[0] = 0;

    if ( new_pid < 0 )
        proc->pid = generate_pid();
    else
        proc->pid = new_pid;

    proc->ppid = -1;
    proc->page_table_base = NULL;
    proc->state = PROCESS_NONE;
    memset(&proc->registers, 0, sizeof(proc->registers));

    proc->next = NULL;
    proc->mm.vma_list = NULL;
    proc->kernel_stack = NULL;
    proc->currentDirectory = root;
    proc->homeDirectory = root;
}
int count_processes (void)
{
    uint64_t count = 0;
    process_t *proc = NULL;
    
    proc = process_runnable_queue;
    while (proc)
    {
        count++;
        proc = proc->next;
    }
    
    proc = process_waiting_queue;
    while (proc)
    {
        count++;
        proc = proc->next;
    }
    proc = process_terminated_queue;
    while (proc)
    {
        count++;
        proc = proc->next;
    }
    return count;
}
process_t* allocate_process(int new_pid)
{
    int i;
    process_t *proc = kmalloc(sizeof(process_t));
    if(!proc)
    {
        return NULL;
    }
    zero_process_structure(proc, new_pid);

    //Create file descriptor for stdout
    fileDesc* newDesc;
    newDesc = (fileDesc* )kmalloc(sizeof(fileDesc));
    if(!newDesc)
    {
        return NULL;
    }
    newDesc->type = STDIN;
    proc->listFileDesc[0] = newDesc;
    
    //Create file descriptor for stdin
    newDesc = (fileDesc* )kmalloc(sizeof(fileDesc));
    if(!newDesc)
    {
        return NULL;
    }
    newDesc->type = STDOUT;
    proc->listFileDesc[1] = newDesc;
    
    //Create file descriptor for stderr
    newDesc = (fileDesc* )kmalloc(sizeof(fileDesc));
    if(!newDesc)
    {
        return NULL;
    }
    newDesc->type = STDERR;
    proc->listFileDesc[2] = newDesc;
    
    //No need to create file descriptors for others right now
    //Assign them to NULL
    //NULL is a marker that they are free for now
    for(i=3; i< MAX_FILE_DESCRIPTORS; i++)
    {
        proc->listFileDesc[i] = NULL;
    }

    return proc;
}

int32_t reclaim_process_resources(process_t *proc)
{
    if (!proc)
        return -1;

    reclaim_pages(proc);
    free_vma(proc->mm.vma_list);
    proc->mm.vma_list = NULL;

    //// Why we are not freeing: Since we will be running as current running proc,
    //// and we don't wanna lose stack on which we are still working,
    //// so we will hanlding this by deleting it using    delete_prev_stack
    // kfree(proc->kernel_stack);
    // proc->kernel_stack = NULL;

    free_filedesc(proc);

    return 0;
}

int32_t add_process_to_queue(process_t *proc, QueueType qType)
{
    if (!proc)
        return -1;

    if (qType == PROCESS_WAITING_QUEUE)
    {
        proc->state = PROCESS_WAITING;
        proc->next = process_waiting_queue;
        process_waiting_queue = proc;
    }
    else if (qType == PROCESS_RUNNABLE_QUEUE)
    {
        proc->state = PROCESS_RUNNABLE;
        proc->next = process_runnable_queue;
        process_runnable_queue = proc;
    }
    else if (qType == PROCESS_TERMINATED_QUEUE)
    {
        proc->state = PROCESS_TERMINATED;
        proc->next = process_terminated_queue;
        process_terminated_queue = proc;
    }
    return 0;
}

process_t* pop_process_from_queue(process_t *proc_to_be_popped)
{
    process_t *proc = NULL;
    process_t *prev_proc = NULL;

    // lets scan through runnable process list
    proc = process_runnable_queue;
    prev_proc = NULL;
    while(proc)
    {
        if (proc ==  proc_to_be_popped)
        {
            if (prev_proc != NULL)
            {
                prev_proc->next = proc->next;
            }

            if (proc == process_runnable_queue)
            {
                process_runnable_queue = proc->next;
            }

            proc->next = NULL;
            return proc;
        }
        prev_proc = proc;
        proc = proc->next;
    }

    // lets scan through process_waiting_queue process list
    proc = process_waiting_queue;
    prev_proc = NULL;
    while(proc)
    {
        if (proc ==  proc_to_be_popped)
        {
            if (prev_proc != NULL)
            {
                prev_proc->next = proc->next;
            }

            if (proc == process_waiting_queue)
            {
                process_waiting_queue = proc->next;
            }

            return proc;
        }
        prev_proc = proc;
        proc = proc->next;
    }

    // lets scan through process_terminated_queue process list
    proc = process_terminated_queue;
    prev_proc = NULL;
    while(proc)
    {
        if (proc ==  proc_to_be_popped)
        {
            if (prev_proc != NULL)
            {
                prev_proc->next = proc->next;
            }

            if (proc == process_terminated_queue)
            {
                process_terminated_queue = proc->next;
            }

            return proc;
        }
        prev_proc = proc;
        proc = proc->next;
    }

    return NULL;
}

int32_t switch_process_queue(process_t *proc, QueueType to_q)
{
    process_t *proc_to_switch = pop_process_from_queue(proc);
    if (!proc_to_switch)
        return -1;

    if (curr_running_proc == proc_to_switch)
    {
        curr_running_proc =  NULL;
    }

    add_process_to_queue(proc_to_switch, to_q);
    return 0;
}

void print_vmas(process_t *proc)
{
    vma_t *vma = proc->mm.vma_list;
    int i = 0;
    printf("VMA for [%d]:%s:\n", proc->pid, proc->name);
    while ( vma )
    {
        printf("VMA %d:  ", ++i);
        printf("START:%p msz:%x fsz:%x f:%x p:%x\n", vma->vma_start, 
               vma->memsz,
               vma->filesz,
               vma->flags,
               vma->perm);
        vma = vma->vma_next;
    }
}

int add_vma_to_proc (uint64_t va_start,
                     uint64_t memsz,
                     uint64_t filesz,
                     process_t *proc,
                     uint64_t perm,
                     uint64_t file_ptr,
                     uint64_t flags)
{
    vma_t *vma_ptr     = NULL;

    vma_ptr            = kmalloc(sizeof(vma_t));
    if ( vma_ptr == NULL )
    {
        #ifdef LOG
            printf("add_vma_to_proc: Ran out of system memory\n");
        #endif
        return -1;
    }
    vma_ptr->vma_start = va_start;
    vma_ptr->memsz     = memsz;
    vma_ptr->filesz    = filesz;
    vma_ptr->perm      = perm;
    vma_ptr->file_ptr  = file_ptr;
    vma_ptr->flags     = flags;
    vma_ptr->vma_next  = proc->mm.vma_list;
    proc->mm.vma_list = vma_ptr;

    return 0;
}

uint64_t generate_pid()
{
    static uint64_t pid = 0x1;
    return pid++;
}

int setup_pagetable_proc(process_t *proc)
{
    uint64_t* level4_table_base = NULL;
    uint64_t index_to_copy_from_kernel_table=0;
    proc->page_table_base = page_alloc();
    int i;

    if (!proc->page_table_base)
    {
        #ifdef LOG
            printf("Failed to allocate memory for process [level 4]");
        #endif
        return -1;
    }

    level4_table_base = PHYS_TO_VA(proc->page_table_base);
    index_to_copy_from_kernel_table = FOURTH_LEVEL_TABLE_INDEX(kern_virtual_start);
    uint64_t *va_kernel_level4 = PHYS_TO_VA(phys_kernel_level4);
    for( i = index_to_copy_from_kernel_table ; i < MAX_PT_ENTRY ; i++)
    {
        level4_table_base[i] = va_kernel_level4[i];
    }

    return 0;
}

int setup_registers_proc( process_t *proc )
{
    proc->registers.rsp = USER_STACK_TOP - 8; 
    proc->registers.rip = (uint64_t) proc->entry_point;
    proc->registers.fs = GDT_USER_DS;
    proc->registers.ds = GDT_USER_DS;
    proc->registers.es = GDT_USER_DS;
    proc->registers.gs = GDT_USER_DS;
    proc->registers.ss = GDT_USER_DS;
    proc->registers.cs = GDT_USER_CS;
    proc->registers.rflags = FLAG_IF;
    return 0;
}

int setup_stack(process_t *proc,
                size_t argv_count,
                size_t envp_count)
{
    int ret_code = 0;
    int i;
    uint64_t save_cr3 = getcr3();
    setcr3((uint64_t)proc->page_table_base);

    // First we need to push envps 
    // which will be stored at USER_ENV_VARIABLE_START using demand paging
    // Last entry of envp should be NULL
    proc->registers.rsp -= 8;
    *(uint64_t*)(proc->registers.rsp) = 0;

    for (i = envp_count - 1; i >= 0; i--)
    {
        proc->registers.rsp -= 8;
        *(uint64_t*)(proc->registers.rsp) = USER_ENV_VARIABLE_START + (MAX_ENV_SIZE * i);
    }

    // Second we need to push argvs
    proc->registers.rsp -= 8;
    *(uint64_t*)(proc->registers.rsp) = 0;

    for (i = argv_count - 1; i >=0; i--)
    {
        proc->registers.rsp -= 8;
        *(uint64_t*)(proc->registers.rsp) = USER_ARGV_START + (MAX_ARGV_SIZE * i);
    }

     // Third argc
     proc->registers.rsp -= 8;
    *(uint64_t*)(proc->registers.rsp) = argv_count;

    setcr3(save_cr3);
    return ret_code;
}


//Initializes a process with its binary its argument
process_t* create_process_new(void* binary, pid_t ppid, int new_pid, 
                              const char *proc_name,
                              char * const argv[],
                              size_t argv_count,
                              char * const envp[],
                              size_t envp_count)
{
    uint64_t old_cr3 = 0;
    int32_t    rc   = 0;

    process_t *proc = allocate_process(new_pid);
    if (!proc)
    {
        #ifdef LOG
            printf("Failed to allocate memory for process\n");
        #endif
        return NULL;
    }
    
    // First process will be foreground... Need to change this logic
    // May be this will work fine when we will be running just shell
    if (foreground_proc == NULL)
    {
        proc->flags |= FOREGROUND_PROCESS;
        foreground_proc = proc;
    }

    strncpy(proc->name, proc_name, sizeof(proc->name));

    proc->kernel_stack = kmalloc(USER_KERNEL_STACK_SIZE);
    if(proc->kernel_stack == NULL)
    {
        reclaim_process_resources(proc);
        kfree(proc);
        #if LOG
            printf("failed to allocate memory for user-kern stack\n");
        #endif
        
        return NULL;
    }

    rc = setup_pagetable_proc(proc);
    if (rc)
    {
        reclaim_process_resources(proc);
        kfree(proc);
        #ifdef LOG
            printf("setup_pagetable_proc failed\n");
        #endif
        return NULL;
    }
    
    if (binary != NULL)
    {
        //If the process is child of some parent
        //So we don't need any binary
        rc = init_vmas_proc(proc, binary, argv_count, envp_count);
    }

    if (rc)
    {
        reclaim_process_resources(proc);
        kfree(proc);
        #ifdef LOG
            printf("init_vmas_proc failed\n");
        #endif
        return NULL;
    }

    //Setup registers for the process
    setup_registers_proc(proc);

    // Allocate a page for stack
    // Why we are not going for demand paging?
    //  We need to push env variables' addresses on stack while we are in kernel 
    //  and it's not a good idea to have a page fault in kernel
    if (binary != NULL)
    {   //Don't allocate page for stack because it's a child.
        allocate_memory((uint64_t)proc->page_table_base,
                    USER_STACK_TOP - PGSIZE,
                    PGSIZE,
                    PTE_P | PTE_U | PTE_W);
    }
    // This function should be last in this sequence since it uses rsp 
    // which we set in setup_registers_proc
    if (binary != NULL)
    {

        if (argv_count > 0)
        {
            allocate_memory((uint64_t)proc->page_table_base,
                            USER_ARGV_START,
                            argv_count * sizeof(execve_argv[0]),
                            PTE_P | PTE_U | PTE_W);
            old_cr3 = getcr3();
            setcr3((uint64_t)proc->page_table_base);
            memcpy((void *)USER_ARGV_START,
                    argv,
                    argv_count * sizeof(execve_argv[0]));
            setcr3(old_cr3);
        }

        if (envp_count > 0)
        {
            allocate_memory((uint64_t)proc->page_table_base,
                            USER_ENV_VARIABLE_START,
                            envp_count * sizeof(execve_envp[0]),
                            PTE_P | PTE_U | PTE_W);
            old_cr3 = getcr3();
            setcr3((uint64_t)proc->page_table_base);
            memcpy((void *)USER_ENV_VARIABLE_START,
                    envp,
                    envp_count * sizeof(execve_envp[0]));
            setcr3(old_cr3);
        }

        setup_stack(proc, argv_count, envp_count);
    }

    add_process_to_queue(proc, PROCESS_RUNNABLE_QUEUE);
    proc->ppid = ppid;
    //printf("New process PID = %d\n", proc->pid);

    return proc;
}

int32_t copy_pages_parent_child( pid_t parent, pid_t child_pid )
{
    if(parent < 0 || child_pid < 0)
    {
        return -1;
    }
    vma_t    *vma        = NULL;
    process_t* par_proc =  get_proc_from_pid(parent);
    process_t* child_proc = get_proc_from_pid(child_pid);

    if (!par_proc || !child_proc)
    {
        #ifdef LOG
            printf("Failed to get child/parent pid\n");
        #endif
        return -1;
    }

    uint64_t *parent_table_base = PHYS_TO_VA(par_proc->page_table_base);
    uint64_t *parent_proc_pte;
    uint64_t *child_proc_pte;
    
    if ( curr_running_proc->page_table_base != NULL  
        && child_proc->page_table_base != NULL )
    {
        
        /* For the fourth level page table, we need to copy only the user level
        * entries. Above 'kernel_start', shared kernel level page tables exist
        * which are not supposed to be copied. Hence check from the parent process
        vma */
        vma = par_proc->mm.vma_list;
        while ( vma )
        {
            uint64_t va_start = ROUNDDOWN(vma->vma_start, PGSIZE, uint64_t);
            uint64_t va_end = ROUNDUP(vma->vma_start + vma->memsz, PGSIZE, uint64_t);
            for ( ; va_start < va_end; va_start += PGSIZE )
            {
                //Retrieve parent process page table entry for this vma_start
                //value
                parent_proc_pte = page_lookup( parent_table_base , va_start);
               
                if( parent_proc_pte == NULL)
                {
                    //Entry is not mapped in parent vma
                    //Move to next entry.
                    continue;
                }
               
                //Increase the reference for shared page between
                // parent and child
                Pages* shared_page = PHYS_TO_PAGE(*parent_proc_pte);
                shared_page->ref++;

                uint64_t perm = PTE_U | PTE_P;
                if ( *parent_proc_pte & PTE_W || *parent_proc_pte & PTE_COW)
                {
                    perm |= PTE_W;
                }

                //Map this value of va_start in child page table with
                //appropriate permissions
                map_range_va2pa(PHYS_TO_VA(child_proc->page_table_base),
                                va_start,
                                va_start + PGSIZE,
                                *parent_proc_pte,
                                perm);

                //If parent write bit is set, then change parents pte to cow
                //Also test for cow bit in parent in case of multiple child
                //Because parent cow bit might already have been set
                //Due to a earlier child
                if ( *parent_proc_pte & PTE_W || *parent_proc_pte & PTE_COW)
                {
                    //Set the parent process page table entry bit to COW
                    *parent_proc_pte = (( *parent_proc_pte & MASK_PAGE_PERM) 
                                    | (( PTE_U | PTE_COW | PTE_P) & 0xFFF) );
                    invalidate_tlb(va_start);

                    //This permission ends being used for children

                    child_proc_pte = page_lookup(PHYS_TO_VA(child_proc->page_table_base),
                                                 va_start);
                    if (child_proc_pte)
                    {
                        *child_proc_pte = (*child_proc_pte & MASK_PAGE_PERM) | PTE_U | PTE_P | PTE_COW;
                    }
                }

            } /* for ( uint64_t va_start = ...) */
            vma = vma->vma_next;
        }
    }
    else
    {
        #ifdef LOG
            printf(" Some problem occured while copying of pages from parent to child \n");
        #endif
        return -1;
    }
    return 0;
}

int32_t copy_vmas_parent_child( pid_t curr_proc_pid, pid_t new_pid)
{
    process_t* curr_proc = get_proc_from_pid(curr_proc_pid);
    process_t* new_proc = get_proc_from_pid(new_pid);
    
    if (!curr_proc || !new_proc)
    {
       #ifdef LOG
            printf("Failed to get child/parent pid\n");
       #endif
       return -1;
    }
    
    vma_t    *curr_vma       = curr_proc->mm.vma_list;
    vma_t    *new_vma   = NULL;
    while (curr_vma)
    {
        new_vma = kmalloc(sizeof(vma_t));
        if ( new_vma == NULL )
        {
            #ifdef LOG
                printf("copy_vmas_parent_child: Ran out of system memory\n");
            #endif
            return -1;
        }
        *new_vma = *curr_vma;
        new_vma->vma_next = new_proc->mm.vma_list;
        new_proc->mm.vma_list = new_vma;
        curr_vma = curr_vma->vma_next;
    }
    return 0;
}


int32_t copy_filedesc_parent_child( process_t* curr_proc, process_t* new_proc, int execv)
{
    int rc;
    if(!curr_proc || !new_proc)
    {
        return -1;
    }
    pipes* common_pipe;
    fileDesc* newDesc;
    for(int i = 0; i < MAX_FILE_DESCRIPTORS; i++)
    {
        if(curr_proc->listFileDesc[i] == NULL)
        {
             new_proc->listFileDesc[i] = NULL;
             continue;        
        }
        //Pipe for this file descriptor index in parent process
        common_pipe =  curr_proc->listFileDesc[i]->vpipe;
        
      
        //Update the pipe's reverse mapping for read/write
        if(curr_proc->listFileDesc[i]->type == PIPE_READ)
        {
              if(!common_pipe)
            {
                return -1;
            }
            if(execv)
            {
                rc = addPipeReadDesc(common_pipe,curr_proc,i);
                if(rc < 0)
                {
                    #ifdef LOG
                    printf("addPipeReadDesc failed \n");
                    #endif
                    return -1;
                }
            }
            else
            {
                rc = addPipeReadDesc(common_pipe,new_proc,i);
                 if(rc < 0)
                {
                    #ifdef LOG
                    printf("addPipeReadDesc failed \n");
                    #endif
                    return -1;
                }
            }
            
        }
        else if(curr_proc->listFileDesc[i]->type == PIPE_WRITE)
        {
             if(!common_pipe)
            {
                return -1;
            }
            if(execv)
            {
                rc = addPipeWriteDesc(common_pipe,curr_proc,i);
                 if(rc < 0)
                {
                    #ifdef LOG
                    printf("addPipeWriteDesc failed \n");
                    #endif
                    return -1;
                }
            }
            else
            {
                rc = addPipeWriteDesc(common_pipe,new_proc,i);
                 if(rc < 0)
                {
                    #ifdef LOG
                    printf("addPipeReadDesc failed \n");
                    #endif
                    return -1;
                }
            }
        }
        //Assign a proper descriptor for file
        //Copy parent info to child structures
        newDesc = (fileDesc* )kmalloc(sizeof(fileDesc));
        if(!newDesc)
        {
            return -1;
        }
        newDesc->offset = curr_proc->listFileDesc[i]->offset;
        newDesc->type = curr_proc->listFileDesc[i]->type;
        newDesc->buffer = curr_proc->listFileDesc[i]->buffer;
        newDesc->file = curr_proc->listFileDesc[i]->file;
        newDesc->vpipe = curr_proc->listFileDesc[i]->vpipe;
        new_proc->listFileDesc[i] = newDesc;
    }
    return 0;
}

void
free_vma ( vma_t *vma )
{
    while (vma)
    {
        vma_t *tmp = vma;
        vma = vma->vma_next;
        kfree(tmp);
    }
}

void free_filedesc(process_t* proc)
{
    pipes* common_pipe;
    for(int i=0; i<MAX_FILE_DESCRIPTORS;i++)
    {
        fileDesc* fileDesc = proc->listFileDesc[i];
        //If one is mapped to this space
        if(fileDesc)
        {
            uint8_t  type =  fileDesc->type;
            if(type >=STDIN && type <= DESC_FILE)
            {
                kfree(fileDesc);
                proc->listFileDesc[i] = NULL;
            }
            else 
            {//PIPE case
                common_pipe =  proc->listFileDesc[i]->vpipe;
            
                //pipe is already freed
                if(common_pipe == NULL)
                    continue;
                
                
                //Free PipeDesc reverse mapping for this file descriptor
                if(type == PIPE_READ)
                {
                    removePipeReadDesc(common_pipe ,proc,i);
                }
                else
                {
                   // printf("Removed in proc is %s and id=%d \n", proc->name,proc->pid);
                    removePipeWriteDesc(common_pipe ,proc,i);
                }
               
                //Wake up anything blocked on this pipe
                wakeProcDueToPipe(common_pipe);
               //If readDesc and writeDesc are NULL free pipe
               if(common_pipe->readDesc==NULL && common_pipe->writeDesc==NULL)
               {
                    //printf("Pipe free \n");
                    if(proc->listFileDesc[i]->buffer)
                    {
                        kfree(proc->listFileDesc[i]->buffer);
                    }
                    
                    kfree(common_pipe);
                    proc->listFileDesc[i]->vpipe = NULL;
               }
               
                kfree(fileDesc);
                proc->listFileDesc[i] = NULL;
            }
        }
    
    }
}

uint64_t min(uint64_t a, uint64_t b)
{
    return a < b ? a : b;
}

uint64_t max(uint64_t a, uint64_t b)
{
    return a > b ? a : b;
}

void 
pop_regs ( process_t *proc )
{
    uint64_t regs = (uint64_t)(&proc->registers);
   
    __asm __volatile("movq %0,%%rsp\n"
                     "pop %%rax\n"
                     "mov %%rax, %%fs\n"

                     "pop %%rax\n"
                     "mov %%ax, %%gs\n"

                     "pop %%rax\n"
                     "mov %%ax, %%ds\n"

                     "pop %%rax\n"
                     "mov %%ax, %%es\n"

                     "pop %%r15\n"
                     "pop %%r14\n"
                     "pop %%r13\n"
                     "pop %%r12\n"
                     "pop %%r11\n"
                     "pop %%r10\n"
                     "pop %%r9\n"
                     "pop %%r8\n"

                     "pop %%rdi\n"
                     "pop %%rsi\n"
                     "pop %%rbp\n"
                     "pop %%rdx\n"
                     "pop %%rcx\n"
                     "pop %%rbx\n"
                     "pop %%rax\n"

                     "add $16, %%rsp\n"

                     "iretq\n"
                     ::"r"(regs): "memory");
}

//Runs process with process_t as its argument. Context switch
void context_switch(process_t* new_process)
{
    if(curr_running_proc && (curr_running_proc->state == PROCESS_RUNNING))
    {
        curr_running_proc->state = PROCESS_RUNNABLE;
    }
    tss.rsp0 = (uint64_t)(new_process->kernel_stack) 
                + USER_KERNEL_STACK_SIZE
                - 8;

    new_process->state=PROCESS_RUNNING;
    curr_running_proc=new_process;
    setcr3((uint64_t)new_process->page_table_base);
    pop_regs(new_process);
    return;
}

//Schedules the current batch of processes
void process_schedule()
{
    process_t* start;
    if (curr_running_proc != NULL )
    {
        start = curr_running_proc;
        //In round-robin our aim is to find next runnable process, and make it running
        //First run the list from currently running process till the end of allocated processes
        while(start != NULL)
        {
            if(start->state==PROCESS_RUNNABLE)
            {
                //just for fun check the pid
                if(start->pid==-1)
                {
                    #ifdef LOG
                        printf("Run for your lives. Process is runnable with -1 process id \n");
                    #endif
                    return;
                }
                context_switch(start);
                return;
            }
            start = start->next;
        }
    }
   
     //Now run the list from start of allocated processes till currently running process
     start = process_runnable_queue;
     while(start && start != curr_running_proc)
     {
        if (start->state==PROCESS_RUNNABLE)
        {
            //just for fun check the pid
            if(start->pid==-1)
            {
                #ifdef LOG
                    printf("Run for your lives. Process is runnable with -1 process id \n");
                #endif
                return;            
            }
            context_switch(start);
            return;
        }
        start = start->next;
     }
     //If we have reached this point then no other process is in dire need of scheduling.Hence return
     return;
}

int is_elf(const void* binary)
{
    Elf64_Ehdr *hdr = (Elf64_Ehdr *)binary;
    if ( hdr->e_ident[EI_MAG0 ] != 0x7F || hdr->e_ident[EI_MAG1] != 'E' || 
         hdr->e_ident[EI_MAG2] != 'L'|| hdr->e_ident[EI_MAG3] != 'F')
    {
        //printf("ELF Magic Number not Found\n");
        return 0;
    }
    return 1;
}

/* http://downloads.openwatcom.org/ftp/devel/docs/elf-64-gen.pdf */
int32_t init_vmas_proc(process_t *proc,
                       void* binary,
                       size_t argv_count,
                       size_t envp_count)
{
    Elf64_Ehdr *hdr  = NULL;
    Elf64_Phdr *phdr = NULL;
    uint64_t   va    = 0;
    uint64_t   memsz   = 0;
    uint64_t   filesz  = 0;
    int32_t    rc    = 0;
    uint64_t   env_variable_size = 0;
    size_t     argv_size = 0;

    hdr = (Elf64_Ehdr *)binary;
    phdr = (Elf64_Phdr *)( (uint64_t)binary + hdr->e_phoff);
    
    if (!is_elf(binary))
    {
        #ifdef LOG
            printf("ELF Magic Number not Found\n");
        #endif
        return -1;
    }
    //printf("%d program headers are present \n",hdr->e_phnum);
    for ( int i =0; i < hdr->e_phnum; i++ )
    {
        if ( phdr[i].p_type != PT_LOAD )
        {
            continue;
        }
        if ( phdr[i].p_filesz > phdr[i].p_memsz )
        {
            /* http://www.daemon-systems.org/man/elf.5.html */
            #ifdef LOG
                printf("ERROR: ELF File too big to fit into memory\n");
            #endif
            return -1;
        }
        else
        {
            //va  = ROUNDDOWN(phdr[i].p_vaddr, PGSIZE, uint64_t);
            va  = (uint64_t)phdr[i].p_vaddr;
            memsz = phdr[i].p_memsz;
            filesz = phdr[i].p_filesz;
            rc  = add_vma_to_proc(va,
                                  memsz,
                                  filesz,
                                  proc,
                                  phdr[i].p_flags,
                                  ((uint64_t)binary+ phdr[i].p_offset),
                                  0);
            if ( rc )
            {
                #ifdef LOG
                    printf("ERROR: Unable to allocate VMAs for process %d\n",
                       proc->pid);
                #endif
                return rc;
            }
        }
    }
    /* Add a VMA for stack */
    rc = add_vma_to_proc(USER_STACK_TOP- INITIAL_STACK_PAGES*PGSIZE,
                         INITIAL_STACK_PAGES*PGSIZE,
                         INITIAL_STACK_PAGES*PGSIZE,
                         proc,
                         (VMA_PERM_WRITE | VMA_PERM_READ),
                         0,
                         VMA_STACK);
    if (rc)
    {
        #ifdef LOG
            printf("Unable to add VMA for stack region\n");
        #endif
        return rc;
    }

    /* Add a VMA for heap */
    rc = add_vma_to_proc(USER_HEAP_START,
                         0,
                         0,
                         proc, 
                         (VMA_PERM_WRITE | VMA_PERM_READ),
                         0,
                         VMA_HEAP);
    if (rc)
    {
        #ifdef LOG
            printf("Unable to add VMA for heap region\n");
        #endif
        return rc;
    }

    /* Add a VMA for environment variables */
    env_variable_size = envp_count ? (envp_count * sizeof(execve_envp[0])) 
                                   : sizeof(env_variables);
    rc = add_vma_to_proc(USER_ENV_VARIABLE_START,
                         env_variable_size,
                         env_variable_size,
                         proc, 
                         (VMA_PERM_WRITE | VMA_PERM_READ),
                         envp_count ? 0 : (uint64_t)&env_variables,
                         VMA_ENV_VARIABLE);
    if (rc)
    {
        #ifdef LOG
            printf("Unable to add VMA for heap region\n");
        #endif
        return rc;
    }

    argv_size = argv_count * sizeof(execve_argv[0]);
    /* Add a VMA for argv */
    rc = add_vma_to_proc(USER_ARGV_START,
                         argv_size,
                         argv_size,
                         proc,
                         (VMA_PERM_WRITE | VMA_PERM_READ),
                         0,
                         VMA_ARGV);
    if (rc)
    {
        #ifdef LOG
            printf("Unable to add VMA for heap region\n");
        #endif
        return rc;
    }

    proc->entry_point = (uint64_t*)hdr->e_entry;
    //print_vmas(proc);
    return rc;
}

Tree* get_proc_binary(const char *proc_name)
{
    Tree *pTree = NULL;
    if (!curr_running_proc ||
        proc_name[0] == '/')
    {
        pTree = findNodeInFileSystembyPath(root, (char *)proc_name);
    }
    else
    {
        pTree = findNodeInFileSystembyPath(curr_running_proc->currentDirectory,
                                           (char *)proc_name);
    }

    return pTree;
}

int launch_process(char *proc_name)
{
    Tree *pTree = get_proc_binary(proc_name);
    void *binary = (void *)((char *)pTree->fileInfo+sizeof(struct posix_header_ustar));

    process_t *proc = NULL;

    if (binary)
    {
        proc = create_process_new(binary, 0, -1, /* generate new pid */
                                pTree->name,
                                NULL,
                                0,
                                (char **const)env_variables,
                                kern_env_count);
    }
    return proc == NULL? -1: proc->pid;
}

vma_t *
find_vma_by_vaddr (process_t *proc, uint64_t fault_addr)
{
    uint64_t va_start;
    uint64_t va_end;
    vma_t   *vma = proc->mm.vma_list;

    while (vma)
    {
        va_start = vma->vma_start;
        va_end = va_start + vma->memsz;

        if (va_start <= fault_addr &&
            fault_addr <= va_end)
        {
            return vma;
        }
        vma = vma->vma_next;
    }
    return NULL;
}

vma_t *
find_vma_by_flag(process_t *proc, uint64_t flag)
{
    vma_t   *vma = proc->mm.vma_list;
    while (vma)
    {
        if ( (vma->flags & flag) == flag)
        {
            return vma;
        }
        vma = vma->vma_next;
    }
    return NULL;
}

int32_t
increase_vma_size(vma_t *vma, size_t size)
{
    if ( vma->flags == VMA_STACK )
    {
        vma->vma_start -= size;
    }
    vma->memsz += size;
    return 0;
}

void create_kernel_process(uint64_t *proc_addr)
{
    process_t *proc = allocate_process(-1);
    if (!proc)
    {
        #ifdef LOG
            printf("Error: No free slot in process list \n");
        #endif
        return;
    }

    proc->page_table_base = (uint64_t*)phys_kernel_level4;

    proc->registers.rsp = (uint64_t)dummyStack + PGSIZE - 8;
    proc->registers.rip = (uint64_t)proc_addr;
    proc->registers.fs = GDT_KERNEL_DS;
    proc->registers.ds = GDT_KERNEL_DS;
    proc->registers.es = GDT_KERNEL_DS;
    proc->registers.gs = GDT_KERNEL_DS;
    proc->registers.ss = GDT_KERNEL_DS;
    proc->registers.cs = GDT_KERNEL_CS;
    proc->registers.rflags = FLAG_IF;

    proc->entry_point = proc_addr;

}
int findFirstFreeFDIndex(process_t*  proc)
{
    int i; 
    for(i=3; i< MAX_FILE_DESCRIPTORS; i++)
    {
       if( proc->listFileDesc[i] == NULL)
          return i;
    }
    return -1;

}
int wake_up_foreground_proc()
{
    if (foreground_proc)
    {
        if (foreground_proc->state == PROCESS_WAITING &&
            (foreground_proc->flags & WAITING_FOR_READ))
        {
            foreground_proc->flags &= ~WAITING_FOR_READ;
            switch_process_queue(foreground_proc, PROCESS_RUNNABLE_QUEUE);
        }
    }
    return 0;
}

int kill_process(process_t *proc)
{
    if ( proc == NULL || proc->state == PROCESS_TERMINATED)
    {
        return -ERR_SRCH;
    }
    if ( proc == curr_running_proc )
    {
        setcr3(phys_kernel_level4);
    }
    terminate_process(proc, 0);
    if ( curr_running_proc == NULL )
    {
        process_schedule();
    }
    return 0;
}

int32_t handle_waiting_processes()
{
    process_t* start = process_waiting_queue;
    while(start != NULL)
    {
        if (start->flags & I_AM_SLEEPING )
        {
            if ( start->sleep_ticks == 0 )
            {
                start->flags ^= I_AM_SLEEPING;
                switch_process_queue(start, PROCESS_RUNNABLE_QUEUE);
            }
            else
            {
                start->sleep_ticks--;
            }
        }

        if (start->flags & ALARM_ACTIVATED)
        {
            if ( start->alarm_ticks == 0 )
            {
                if (start == curr_running_proc)
                {
                    // switch to kernel's cr3
                    setcr3(phys_kernel_level4);
                }
                terminate_process(start, 0);
                start->flags ^= ALARM_ACTIVATED;
            }
            else
            {
                start->alarm_ticks--;
            }
        }
        start = start->next;
    }

    // Handle Alarm which is with runnable processes
    start = process_runnable_queue;
    while(start != NULL)
    {
        if (start->flags & ALARM_ACTIVATED)
        {
            if ( start->alarm_ticks == 0 )
            {
                if (start == curr_running_proc)
                {
                    // switch to kernel's cr3
                    setcr3(phys_kernel_level4);
                }
                terminate_process(start, 0);
                start->flags ^= ALARM_ACTIVATED;
            }
            else
            {
                start->alarm_ticks--;
            }
        }
        start = start->next;
    }

     return 0;
}

int terminate_process(process_t *proc, int return_value)
{
    if (!proc)
        return -1;

    reparent_children(proc);
    
    //printf("Killing process [%d]:%s \n", proc->pid, proc->name);
    if (proc == foreground_proc)
    {
        // Make parent of current process as foreground
        process_t *parent_proc = get_proc_from_pid(proc->ppid);
        if (parent_proc)
        {
            /* No point in making init the foreground proc. Make root
             * shell the BOSS
             */
            if ( parent_proc->pid == 1 )
            {
                parent_proc = get_proc_from_pid(2);
            }
            parent_proc->flags |= FOREGROUND_PROCESS;
            proc->flags &= ~FOREGROUND_PROCESS;
            foreground_proc = parent_proc;
        }
        else
        {
            /* Set it to root shell */
            foreground_proc = get_proc_from_pid(2);
        }
    }

    // Just deallocate resources but keep the process structure
    // and move process to TERMINATED QUEUE
    proc = pop_process_from_queue(proc);

    if (curr_running_proc == proc)
    {
        curr_running_proc =  NULL;
    }

    reclaim_process_resources(proc);

    kfree(delete_prev_stack);
    delete_prev_stack = proc->kernel_stack;
    proc->kernel_stack = NULL;

    add_process_to_queue(proc, PROCESS_TERMINATED_QUEUE);

    proc->exit_status = (return_value & 0xff);
    // If I am being waited upon by my parent, wake them up
    if (proc->flags & PAPA_WAITING)
    {
        process_t *parent_proc = get_proc_from_pid(proc->ppid);
        if (!parent_proc)
        {
            #ifdef LOG
                printf("Failed to get parent process during termination\n");
            #endif
            return -1;
        }
        switch_process_queue(parent_proc, PROCESS_RUNNABLE_QUEUE);
    }

    return 0;
}
