#include <sys/defs.h>
#include <sys/syscall.h>
#include <sys/sbunix.h>
#include <sys/process.h>
#include <sys/pagetable.h>
#include <sys/kstring.h>
#include <sys/process_mem_layout.h>
#include <error.h>
#include <sys/reg.h>
#include <sys/tarfs.h>
#include <sys/terminal.h>
#include <sys/kmalloc.h>
#include <sys/reg.h>
#include <stdio.h>

int64_t
system_call_write(int fd, const void *buf, size_t count)
{
    int rc;
    //If buffer is null return straight away. No playing games
    //anymore
    if( fd < 0 || fd >= MAX_FILE_DESCRIPTORS)
    {
        return -ERR_BADF;
    }
    
    if( count == 0)
    {
        return count;
    }
    
    rc = validate_user_buffer((uint64_t)buf, count, PTE_P|PTE_U);
    if (rc < 0)
    {
        return -ERR_INVAL;
    }

    uint8_t descriptor_type = curr_running_proc->listFileDesc[fd]->type;

    if( descriptor_type == STDOUT || descriptor_type == STDERR )
    {
        char *temp_buf = kmalloc(count+1);
        if (!temp_buf)
            return -ERR_UNSPECIFIED;

        memcpy(temp_buf, buf, count);
        temp_buf[count] = '\0';
        //Store the number of bytes written
        rc = user_printf(temp_buf);
        kfree(temp_buf);
        temp_buf = NULL;
    }
    else if( descriptor_type == PIPE_WRITE)
    {
        //Multiple processes think what to do
        //write data from pipe into buf
        fileDesc* currDesc = curr_running_proc->listFileDesc[fd] ;
        
        if(!currDesc)
       {
            return -ERR_BADF;
       }          
        pipes* vpipe = currDesc->vpipe;
        uint64_t* procBuf = currDesc->buffer;
        //rc contains bytes written
        rc = writeToPipe(vpipe,(uint64_t*) buf, procBuf, count);
       
        if (rc < 0)
        {
            // Cannot write everything for some reason
            #ifdef LOG
                printf(" Cannot write to pipe. Some problem \n");
            #endif
            return -ERR_PIPE;
        }
        
    }
    else if( descriptor_type == DESC_FILE)
    {
        //Write not permitted
        //return proper error code
        return -ERR_ACCES;
    }
    else
    {
        // rest all descriptors not allowed
        return -ERR_ACCES;
    }
    return rc;
}

int64_t
system_call_read(int fd, void *buf, size_t count)
{
    int64_t rc = 0;
    if( fd < 0 || fd >= MAX_FILE_DESCRIPTORS)
    {
        return -ERR_BADF;
    }
    
    if( count == 0)
    {
        return count;
    }
    if((rc = validate_user_buffer((uint64_t)buf, count, PTE_P|PTE_U|PTE_W))<0)
    {
        return -ERR_INVAL;;
    }
    
    if (!buf)
        return -ERR_INVAL;

   
    uint8_t descriptor_type = curr_running_proc->listFileDesc[fd]->type;
    //If file descriptor is from standard input
    if( descriptor_type == STDIN)
    {
        // confirm this is a foreground process
        if (curr_running_proc != foreground_proc && curr_running_proc->pid != 2)
        {
            setcr3(phys_kernel_level4);
            terminate_process(curr_running_proc, -1);
            process_schedule();
            // We should never reach here
            return -ERR_INVAL;
        }
        rc = getline(buf, count);
        if (rc < 0)
        {
            // No data available to read
            curr_running_proc->flags |= WAITING_FOR_READ;
            wait_in_kernel();
            rc = getline(buf, count);
             if (rc < 0)
             {
                //If for some reasons this happens then fail
                return -ERR_INVAL;
             }
        }
    }
    else if ( descriptor_type == DESC_FILE)
    {
        //Read data from file into buf
        fileDesc* currDesc = curr_running_proc->listFileDesc[fd] ;
        if(!currDesc)
        {
            return -ERR_BADF;
        } 
        //rc contains bytes read
        rc = getDataFromFile(currDesc,buf, count);
        if (rc < 0)
        {
            // No data available to read
            #ifdef LOG
                printf(" Cannot read from file. Some problem \n");
            #endif
            return -ERR_FILE_READ;
        }
    }
    else if ( descriptor_type == PIPE_READ)
    {
        //Multiple processes think what to do
        //write data from pipe into buf
        fileDesc* currDesc = curr_running_proc->listFileDesc[fd] ;
        if(!currDesc)
        {
            return -ERR_BADF;
        }   
        pipes* vpipe = currDesc->vpipe;
        uint64_t* procBuf = currDesc->buffer;
        //rc contains bytes written
        rc = readFromPipe(vpipe,(uint64_t*) buf, procBuf, count);
       
        if (rc < 0)
        {
            // Cannot write everything for some reason
            #ifdef LOG
                printf(" Cannot write to pipe. Some problem \n");
            #endif
            return -ERR_PIPE;
        }
        
    
    }
    
    return rc;
}
int64_t 
system_call_close(int fd)
{
    int rc = 0;
    if( fd < 0 || fd >= MAX_FILE_DESCRIPTORS)
    {
        return -ERR_BADF;
    }
    
    if (!curr_running_proc->listFileDesc[fd])
        return -ERR_BADF;
    pipes* common_pipe;   
    uint8_t  type =  curr_running_proc->listFileDesc[fd]->type;
    if(type ==PIPE_READ)
    {
        common_pipe = curr_running_proc->listFileDesc[fd]->vpipe;
        if (!common_pipe)
            return -ERR_BADF;
        rc = removePipeReadDesc( curr_running_proc->listFileDesc[fd]->vpipe,
                    curr_running_proc,fd);
        if( rc < 0)
        {
            return -ERR_UNSPECIFIED;
        }
        rc = wakeProcDueToPipe(curr_running_proc->listFileDesc[fd]->vpipe);
        if( rc < 0)
        {
            return -ERR_UNSPECIFIED;
        }
         if(common_pipe->readDesc==NULL && common_pipe->writeDesc==NULL)
        {
            if(curr_running_proc->listFileDesc[fd]->buffer)
            {
               kfree(curr_running_proc->listFileDesc[fd]->buffer);
            }
            kfree(common_pipe);
            curr_running_proc->listFileDesc[fd]->vpipe = NULL;
            kfree(curr_running_proc->listFileDesc[fd]);
         }
    }
    else if( type == PIPE_WRITE)
    {
        common_pipe = curr_running_proc->listFileDesc[fd]->vpipe;
        if (!common_pipe)
            return -ERR_BADF;
        rc = removePipeWriteDesc( curr_running_proc->listFileDesc[fd]->vpipe,
                    curr_running_proc,fd);
        if( rc < 0)
        {
            return -ERR_UNSPECIFIED;
        }
        
        rc = wakeProcDueToPipe(curr_running_proc->listFileDesc[fd]->vpipe);
        if( rc < 0)
        {
            return -ERR_UNSPECIFIED;
        }
        if(common_pipe->readDesc==NULL && common_pipe->writeDesc==NULL)
        {
            if(curr_running_proc->listFileDesc[fd]->buffer)
            {
               kfree(curr_running_proc->listFileDesc[fd]->buffer);
            }
            kfree(common_pipe);
            curr_running_proc->listFileDesc[fd]->vpipe = NULL;
            kfree(curr_running_proc->listFileDesc[fd]);
         }
    }
    else
    {
        kfree(curr_running_proc->listFileDesc[fd]);
    }
    curr_running_proc->listFileDesc[fd] = NULL;
    
    
    return 0;
}

void
system_call_exit(int return_value )
{
    // switch to kernel's cr3
    setcr3(phys_kernel_level4);

    /* exit can return only 8 bits of info. If the process has called exit()
     * it means that it is dying peacefully. Set the 9th bit of the exit_status
     * indicating the same. If the process has been killed because of other 
     * reasons, then the bit PEACEFUL_DEATH wont be set and the caller should
     * not read the exit status of the process
     */
    terminate_process(curr_running_proc, return_value | PEACEFUL_DEATH);

    process_schedule();
    return;
}

pid_t 
system_call_getpid( void )
{
    return curr_running_proc->pid;
}

pid_t 
system_call_getppid( void )
{
    return curr_running_proc->ppid;
}

int dummy()
{
    return 0;
}
void
wait_in_kernel()
{
    __label__ return_point;
    //void *ptr = &&return_point;
    uint64_t rsp;
    __asm __volatile("movq %%rsp,%0" : "=r" (rsp));
    /* Put the current kernel state into the process register state */
    curr_running_proc->state                = PROCESS_WAITING;
    curr_running_proc->registers.rip        = (uint64_t)&&return_point;
    curr_running_proc->registers.cs         = read_cs();
    curr_running_proc->registers.rflags     = read_rflags();
    curr_running_proc->registers.rsp        = rsp;
    curr_running_proc->registers.ss         = read_ss();
    curr_running_proc->registers.rax        = read_rax();
    curr_running_proc->registers.rbx        = read_rbx();
    curr_running_proc->registers.rcx        = read_rcx();
    curr_running_proc->registers.rdx        = read_rdx();
    curr_running_proc->registers.rbp        = read_rbp();
    curr_running_proc->registers.rsi        = read_rsi();
    curr_running_proc->registers.rdi        = read_rdi();
    curr_running_proc->registers.r8         = read_r8();
    curr_running_proc->registers.r9         = read_r9();
    curr_running_proc->registers.r10        = read_r10();
    curr_running_proc->registers.r11        = read_r11();
    curr_running_proc->registers.r12        = read_r12();
    curr_running_proc->registers.r13        = read_r13();
    curr_running_proc->registers.r14        = read_r14();
    curr_running_proc->registers.r15        = read_r15();
    curr_running_proc->registers.fs         = read_fs();
    curr_running_proc->registers.gs         = read_gs();
    curr_running_proc->registers.ds         = read_ds();
    curr_running_proc->registers.es         = read_es();
    switch_process_queue(curr_running_proc, PROCESS_WAITING_QUEUE);
    process_schedule();

return_point:   dummy();//printf("hello\n");//dummy();
                return;
}

pid_t
system_call_fork(int background_proc)
{
    process_t *proc = NULL;
    process_t *child_proc = NULL;
    pid_t new_pid; 
    int32_t ret=0;

    if(curr_running_proc == NULL  || curr_running_proc->pid == -1)
    {
        return -ERR_INVAL;
    }

    //Create a descriptor for new process
    proc = create_process_new(NULL,
                              curr_running_proc->pid,
                              -1, /* Generate a new pid by sending an invalid pid*/
                              curr_running_proc->name,
                              NULL,
                              0,
                              NULL,
                              0);
    if( proc == NULL)
    {
        #ifdef LOG
            printf(" Can't create child process \n");
        #endif
        return -ERR_CHILDFAIL;
    }

    new_pid = proc->pid;

    // Copy pages from parent to child
    ret = copy_pages_parent_child(curr_running_proc->pid, new_pid );
    if( ret < 0)
    {
        proc = get_proc_from_pid(new_pid);
        terminate_process(proc, -1);
        #ifdef LOG
            printf(" Can't copy pages from parent to child \n");
        #endif
        return -ERR_CHILDFAIL;
    }

    // Copy vmas from parent to child
    ret = copy_vmas_parent_child(curr_running_proc->pid, new_pid );
    if( ret < 0)
    {
        proc = get_proc_from_pid(new_pid);
        terminate_process(proc, -1);
        #ifdef LOG
            printf(" Can't copy vmas from parent to child \n");
        #endif
        return -ERR_CHILDFAIL;
    }

    child_proc = get_proc_from_pid(new_pid);
    proc = get_proc_from_pid(curr_running_proc->pid);

    if (!child_proc || !proc)
    {
        terminate_process(child_proc, -1);
        #ifdef LOG
            printf("Failed to get procs from pids\n");
        #endif
        return -ERR_CHILDFAIL;
    }

    //Copy file descriptors from parent to child
    ret = copy_filedesc_parent_child( curr_running_proc,child_proc,0);
    if( ret < 0)
    {
        proc = get_proc_from_pid(new_pid);
        terminate_process(proc, -1);
        #ifdef LOG
            printf(" Can't copy desc from parent to child \n");
        #endif
        return -ERR_CHILDFAIL;
    }
    //Copy parent process registers to child process
    memcpy(&child_proc->registers, &proc->registers, sizeof(isr_arguments_t));

    //Set the return value for child process as 0
    child_proc->registers.rax = 0; 

    //Set parent process id in new process
    child_proc->ppid = curr_running_proc->pid; 

    if (!background_proc &&
     foreground_proc == curr_running_proc)
    {
        child_proc->flags |= FOREGROUND_PROCESS;
        curr_running_proc->flags &= ~FOREGROUND_PROCESS;
        foreground_proc = child_proc;
    }
    //Return process process pid to parent
    //printf("Child pid = %d \n",new_pid);
    return new_pid;
}

pid_t
system_call_waitpid(pid_t pid, int *status, int options)
{
    process_t *proc = NULL;
    /* Currently I am ignoring options assuming we don't need to implement 
     * signals. If my assumption goes wrong, then the below code needs major  
     * re-work. ( brain implant?) 
     */
     
    /* Check if this parent really has its child in the process table */
    proc = get_proc_from_pid(pid);
    if (!proc || proc->ppid != curr_running_proc->pid)
    {
        return -ERR_CHILD;
    }

    /* If the child has already terminated, return immly with exit status */
    if ( proc->state == PROCESS_TERMINATED )
    {
        *status = proc->exit_status;
    }
    else
    {
        //Inform the child that its we(parent) are waiting for it, so that it can wake
        // us up on its death
        proc->flags |= PAPA_WAITING;
        wait_in_kernel();
    }

    *status = proc->exit_status;
    // The child has been reaped. Its not needed anymore
    proc = pop_process_from_queue(proc);

    kfree(proc);
    proc = NULL;

    return pid;
}

int64_t system_call_brk(size_t new_brk)
{
    vma_t *heap_vma = find_vma_by_flag(curr_running_proc, VMA_HEAP);
    vma_t *stack_vma = find_vma_by_flag(curr_running_proc, VMA_STACK);

    if (new_brk == 0)
    {
        return (heap_vma->vma_start + heap_vma->memsz);
    }

    if (!heap_vma || !stack_vma)
    {
        #ifdef LOG
            printf("Failed to get stack/heap vma\n");
        #endif
        return -ERR_NOMEM;
    }

    if (new_brk < heap_vma->vma_start)
    {
        #ifdef LOG
            printf("new brk is smaller than start\n");
        #endif
        return -ERR_NOMEM;
    }

    if (new_brk < (heap_vma->vma_start + heap_vma->memsz))
    {
        #ifdef LOG
            printf("new brk is smaller than previous\n");
        #endif
        return -ERR_NOMEM;
    }

    // stack_vma->vma_start is already page aligned
    // Also add one guard page
    if (new_brk >= (stack_vma->vma_start - PGSIZE))
    {
        #ifdef LOG
            printf("new brk:%p hitting stack\n", new_brk);
        #endif
        return -ERR_NOMEM;
    }

    heap_vma->filesz = new_brk - heap_vma->vma_start;
    heap_vma->memsz = new_brk - heap_vma->vma_start;

    return 0;
}

unsigned int 
system_call_sleep(struct timespec *sleeptime, struct timespec *time_remaining)
{
    curr_running_proc->flags |= I_AM_SLEEPING;
    curr_running_proc->sleep_ticks = sleeptime->tv_sec*100;
    wait_in_kernel();
    /* We are not implementing signals. So we can always return 0 */
    return 0;
}

// Working:
// 1. Release all resources of curr_running_proc
// 2. Overwrite it's Data and other VMA's with new VMA for the program to be loaded
// 3. Start executing new process
// Note: Check this-->
//          Fork - maps same physical page for child and parent, so when we
//          call terminate_process for say child then page should be kept intact 
//          for parent --> I think this will be handled by page_ref so no need to
//                         worry, but just making note of it in case we hit some
//                         corner case
int64_t system_call_execve(const char *filename,
                           char *const argv[],
                           char *const envp[])
{
    // We should never return unless it's a failure.
    // We directly schedule this process

    size_t i, j, argv_count, envp_count;
    int len;
    int rc = 0;
    process_t *proc = NULL;
    char interpreter_filename[MAX_STRING] = {0};
    char extracted_filename[MAX_STRING] = {0};

    rc = validate_user_buffer((uint64_t)filename, sizeof(extracted_filename), PTE_P|PTE_U);
    if (rc < 0)
    {
        return -ERR_FAULT;
    }

    len = strnlen(filename, sizeof(extracted_filename));

    if (len == 0)
    {
        return -ERR_NOENT;
    }
    else if (len < 0)
    {
        return -ERR_NAMETOOLONG;
    }

    strncpy(extracted_filename, filename, sizeof(extracted_filename));
    parse_path(extracted_filename);

    // envp is NULL then no envp is passed
    // Different behaviour for argv handling - exe vs script
    //      If exe then nothing is passed
    //      Else if Script then one entry is created for script-filename

    // if file is a script then replace argv[0] with it's name

    argv_count = 0;
    for (i = 0; (i < sizeof(execve_argv)/sizeof(execve_argv[0])) && argv; i++)
    {
        if (argv[i] == 0)
            break;

        rc = validate_user_buffer((uint64_t)argv[i], sizeof(execve_argv[i]), PTE_P|PTE_U);

        if (rc < 0)
        {
            return -ERR_FAULT;
        }

        // Check if we are trying to launch it as background
        if (strncmp(argv[i], "&", 2) == 0)
        {
            if (foreground_proc == curr_running_proc)
            {
                process_t *parent_proc = get_proc_from_pid(curr_running_proc->ppid);
                if (!parent_proc)
                {
                    #ifdef LOG
                        printf("Failed to get parent process for [%d]:%s\n",
                                curr_running_proc->pid, curr_running_proc->name);
                    #endif
                    break;
                }

                parent_proc->flags |= FOREGROUND_PROCESS;
                curr_running_proc->flags &= ~FOREGROUND_PROCESS;
                foreground_proc = parent_proc;
            }

            // No need to check for other arguments
            break;
        }

        strncpy(execve_argv[i], argv[i], sizeof(execve_argv[i]));
        argv_count++;
    }
    execve_argv[i][0] = 0;

    envp_count = 0;
    for (i = 0; (i < sizeof(execve_envp)/sizeof(execve_envp[0])) && envp; i++)
    {
        if (envp[i] == 0)
            break;

        rc = validate_user_buffer((uint64_t)envp[i], sizeof(execve_envp[i]), PTE_P|PTE_U);
        if (rc < 0)
        {
            return -ERR_FAULT;
        }

        strncpy(execve_envp[i], envp[i], sizeof(execve_envp[i]));
        envp_count++;
    }
    execve_envp[i][0] = 0;

    Tree *pTree = get_proc_binary(extracted_filename);
    if (pTree == NULL)
    {
        return -ERR_NOENT;
    }

    uint64_t chmod = 0;
    if (atou(&chmod, pTree->fileInfo->mode) < 0)
    {
        return -ERR_ACCES;
    }

    // chmod = 755, we need to check 1XX
    if (((chmod / 100 ) & 1) == 0)
    {
        return -ERR_ACCES;
    }

    void *binary = (void *)((char *)pTree->fileInfo+sizeof(struct posix_header_ustar));
    if (!is_elf(binary))
    {
        // check if it's a script then run interpreter and
        // pass argv[0] as script filename

        // Find the interpreter
        size_t file_size = 0;
        if (atoo(&file_size, pTree->fileInfo->size))
        {
            return -ERR_ACCES;
        }

        i = 0;
        char *file = (char *)binary;

        // Ignore white spaces
        while(i < file_size)
        {
            if (file[i] != ' ')
                break;
            i++;
        }

        //confirm #!
        if (i+1 >= file_size ||
            file[i] != '#' ||
            file[i+1] != '!')
        {
            return -ERR_ACCES;
        }
        i += 2;
        // Ignore white spaces
        while(i < file_size)
        {
            if (file[i] != ' ')
                break;
            i++;
        }
        
        // Get interpreter file name
        j = 0;
        while(i < file_size)
        {
            if (file[i] == ' '  ||
                file[i] == '\n' ||
                file[i] == '\t' ||
                file[i] == '\r')
            {
                break;
            }
            interpreter_filename[j++] = file[i];
            i++;
        }
        interpreter_filename[j] = 0;

        pTree = get_proc_binary(interpreter_filename);
        if (pTree == NULL)
        {
            return -ERR_NOENT;
        }

        chmod = 0;
        if (atou(&chmod, pTree->fileInfo->mode) < 0)
        {
            return -ERR_ACCES;
        }

        // chmod = 755, we need to check 1XX
        if (((chmod / 100 ) & 1) == 0)
        {
            return -ERR_NOEXEC;
        }

        binary = (void *)((char *)pTree->fileInfo+sizeof(struct posix_header_ustar));
        if (!is_elf(binary))
        {
            return -ERR_ACCES;
        }
        strncpy(execve_argv[0], pTree->name, sizeof(execve_argv[0]));
        strncpy(execve_argv[1], filename, sizeof(execve_argv[1]));
        argv_count = 2;
    }

    /* Observe that the new process's parent and the current process' parent
     * are the same because the current process is only an intermediate process
     * and to the user is really non-existent
     */
    proc = create_process_new(binary,
                            curr_running_proc->ppid,
                            curr_running_proc->pid,
                            pTree->name,
                            (char **const)execve_argv,
                            argv_count,
                            (char **const)execve_envp,
                            envp_count);
    
    if (proc == NULL)
    {
        
        return -ERR_ACCES;
    }

    //proc = get_proc_from_pid(rc);

    // COPY original process' data
    //if (curr_running_proc == foreground_proc)
    //{
      //  foreground_proc = proc;
    //}
    rc = copy_filedesc_parent_child( curr_running_proc, proc,1);

    if(rc < 0)
    {
        terminate_process(proc, -1);
        #ifdef LOG
            printf(" Can't copy desc from parent to child \n");
        #endif
        return -ERR_CHILDFAIL;
    }
    proc->pid = curr_running_proc->pid;

    proc->flags = curr_running_proc->flags;
    proc->currentDirectory = curr_running_proc->currentDirectory;
    proc->homeDirectory = curr_running_proc->homeDirectory;

    process_t *save_curr_proc = curr_running_proc;

    // switch to kernel's cr3
    setcr3(phys_kernel_level4);

    // Release its resource
    // Destroying these flags is important
    curr_running_proc->flags = 0;
    // Make its pid = -1, so that we can delete it from Terminated Queue
    curr_running_proc->pid = -1;
    terminate_process(curr_running_proc, 0);

    curr_running_proc = save_curr_proc;
    pop_process_from_queue(curr_running_proc);
    pop_process_from_queue(proc);

    // Overlay new process on to curr process 
    *curr_running_proc = *proc;
    if (curr_running_proc->flags & FOREGROUND_PROCESS)
        foreground_proc = curr_running_proc;
    add_process_to_queue(curr_running_proc, PROCESS_RUNNABLE_QUEUE);

    kfree(proc);
    proc = NULL;

    context_switch(curr_running_proc);

    // In theory we will never reach here
    return rc;
}

char*
truncate_forward_slashes(char *path_str)
{
    int i = strlen(path_str);
    if (!i)
        return path_str;
    while(path_str[--i] == '/')
        ;
    path_str[i+1] = '\0';
    return path_str;
}

char*
truncate_forward_slashes_except_one(char *path_str)
{
    int i = strlen(path_str);
    if (!i)
        return path_str;
    i--;
    while(path_str[i] == '/')
    {
        if (i-1 >= 0 && path_str[i-1] == '/')
            i--;
        else
            break;
    }
    path_str[i+1] = '\0';
    return path_str;
}

char*
truncate_previous_dir(char *path_str)
{
    int i = 0;
    truncate_forward_slashes(path_str);
    i = strlen(path_str);
    if (!i)
        return path_str;
    
    i--;
    while (i >= 0 && path_str[i] != '/')
        i--;
    i++;

    path_str[i] = '\0';
    return path_str;
}

char*
parse_path(char *path_str)
{
    char temp_path[MAX_STRING];
    int i = 0, j = 0, len = 0;

    // truncate multiple continuous // slashes
    j = 0;
    for (i = 0; path_str[i]; i++)
    {
        if (j > 0 &&
            path_str[i] == '/' &&
            path_str[j-1] == '/')
        {
            // No need to copy
        }
        else
        {
            path_str[j++] = path_str[i];
        }
    }
    path_str[j] = '\0';

    i = j = 0;
    if (strlen(path_str) == 1 && path_str[0] == '~')
    {
        // It's ~ and we need to go to home directory
        strncpy(path_str, curr_running_proc->homeDirectory->name, NAME_MAX);
        return path_str;
    }
    else if (path_str[0] != '/')
    {
        // It's not the absolute path, so lets include pwd
        strncpy(temp_path, curr_running_proc->currentDirectory->name, 
                 sizeof(temp_path));
        i = strlen(temp_path);
        if (temp_path[i-1] != '/')
        {
            temp_path[i++] = '/';
            temp_path[i] = '\0';
        }
    }

    truncate_forward_slashes_except_one(path_str);
    
    len = strlen(path_str);
    for (j = 0; j < len; )
    {
        // For the first . or later /.
        if ((j == 0 || path_str[j-1] == '/') &&
            path_str[j] == '.')
        {
            j++;
            if (j < len && path_str[j] == '.')
            {
                j++;
                if ((j < len && path_str[j] == '/') ||
                     j == len)
                {
                    truncate_previous_dir(temp_path);
                    i = strlen(temp_path);
                }
                else
                {
                    temp_path[i++] = '.';
                    temp_path[i++] = '.';
                    temp_path[i++] = path_str[j];
                    temp_path[i] = '\0';
                }
            }
            else if (j < len && path_str[j] != '/')
            {
                temp_path[i++] = '.';
                temp_path[i++] = path_str[j];
                temp_path[i] = '\0';
            }
        }
        else
        {
            temp_path[i++] = path_str[j];
            temp_path[i] = '\0';
        }
        j++;
    }
    
    // This situation will be for    ../../../../../../  where we will consume
    // all folders, so we should be at least at the root in such scenarios
    if (i == 0)
        temp_path[i++] = '/';
    
    temp_path[i] = '\0';
    strncpy(path_str, temp_path, NAME_MAX);
    return path_str;
}

int
system_call_open(char *given_path, int flags )
{

    if(!given_path)
    {
        return -ERR_INVAL;
    }
    if( flags & O_WRONLY || flags & O_CREAT )
    {
        return -ERR_ACCES;
    }
    char pathname[FILEPATH_LEN] = {0};
    //Load the current working directory for this process
    Tree* startNode = curr_running_proc->currentDirectory;
    fileDesc* newDesc;
    int fromRoot = 0;
    int retCode=0;

    if((retCode = validate_user_buffer((uint64_t)given_path, FILEPATH_LEN, PTE_P|PTE_U))<0)
    {
        return -ERR_FAULT ;
    }

    strncpy(pathname, given_path, sizeof(pathname));
    parse_path(pathname);

    //Look for path starting from root
    if(pathname[0]=='/')
    {
        startNode = root;
        fromRoot++;
    }
    
    //Find the tree Node in filesystem corresponding to the path
    Tree* foundNode = findNodeInFileSystembyPath(startNode, pathname+fromRoot);
    if(foundNode == NULL)
    {
        return -ERR_NOENT ;
    }
    if(foundNode->fileInfo)
    {
        //If we have passed a directory flag
         if( flags & O_DIRECTORY )
        {
            //But it's not a directory
            if(*foundNode->fileInfo->typeflag != DIRTYPE)
            {
                return -ERR_NOTDIR;
            }   
        }
        else
        {
            //If it's a directory
            if(*foundNode->fileInfo->typeflag == DIRTYPE)
            {
                //But not passed a directory flag
                return -ERR_ISDIR;
            }   
        }
    }
    int fd_index = findFirstFreeFDIndex(curr_running_proc);
    if( fd_index < 0)
    {
        return -ERR_NFILE;
    }
    
    //Assign a proper descriptor for file
    newDesc = (fileDesc* )kmalloc(sizeof(fileDesc));
    newDesc->offset = 0;
    newDesc->type = DESC_FILE;
    newDesc->file = foundNode;
    newDesc->buffer = (uint64_t*)((uint64_t)(foundNode->fileInfo)+
                           sizeof(struct posix_header_ustar));
    curr_running_proc->listFileDesc[fd_index] = newDesc;
    
    return fd_index;
    
}

int system_call_pipe(int filedes[2])
{
    int retCode;
    fileDesc* newDesc;
    if((retCode = validate_user_buffer((uint64_t)filedes,
                                        sizeof(fileDesc),
                                        PTE_P|PTE_U|PTE_W)) < 0)
    {
        return -ERR_FAULT;
    }
    
    //Find a free file descriptor for read end
    retCode = findFirstFreeFDIndex(curr_running_proc);
    if(retCode<0)
    {
        #ifdef LOG
            printf("Descriptor for pipe not found \n");
        #endif
        return -ERR_MFILE;
    }
    filedes[0] = retCode;
     //Both the read/write end share a common pipe strucutre
    //along with the buffer
    pipes* common_pipe =(pipes*)kmalloc(sizeof(pipes));
    common_pipe->size = PIPE_SIZE;
    common_pipe->read_index = 0;
    common_pipe->write_index = 0;
    common_pipe->total_bytes_written = 0;
    common_pipe->readDesc = NULL;
    common_pipe->writeDesc = NULL;
    //Contains the location where all content is read/written
    uint64_t* buffer = (uint64_t*)kmalloc(PIPE_SIZE);
    memset(buffer,0x00,PIPE_SIZE);
    
    //Assign a proper descriptor for pipe[0] 
    newDesc = (fileDesc*)kmalloc(sizeof(fileDesc));
    //Tells that this descriptor is of type PIPE_READ
    newDesc->type = PIPE_READ;
    newDesc->vpipe = common_pipe;
    //Contains the location where all content is read/written
    newDesc->buffer = buffer;
    curr_running_proc->listFileDesc[filedes[0]] = newDesc;
     
    //Update the list of Pipe read descriptors for this pipe
    addPipeReadDesc(common_pipe,curr_running_proc, filedes[0]);
    
    //Find a free file descriptor for write end
    retCode = findFirstFreeFDIndex(curr_running_proc);
    if(retCode<0)
    {
        #ifdef LOG
            printf("Descriptor for pipe not found \n");
        #endif
        return -ERR_MFILE;
    }
    filedes[1] = retCode;
        
    
    //Assign a proper descriptor for pipe[1] 
    newDesc = (fileDesc*)kmalloc(sizeof(fileDesc));
    //Tells that this descriptor is of type PIPE_READ
    newDesc->type = PIPE_WRITE;
    newDesc->vpipe = common_pipe;
    //Contains the location where all content is read/written
    newDesc->buffer = buffer;
    curr_running_proc->listFileDesc[filedes[1]] = newDesc;
    
   //Update the list of Pipe write descriptors for this pipe
    addPipeWriteDesc(common_pipe,curr_running_proc, filedes[1]);
    return 0;
}

int64_t system_call_alarm(uint64_t time_sec)
{
    // If time_sec is 0, a pending alarm request (if any) is cancelled.
    // Only one alarm request can be active at any given time.
    // New time_sec, reschedules the timer
    // Return:
    //  0 -  if there was no alarm
    //  time_remaing - if some alarm was present already

    int64_t rc = 0;
    if (curr_running_proc->flags & ALARM_ACTIVATED)
    {
        rc = curr_running_proc->alarm_ticks / 100;
    }

    if (time_sec == 0)
    {
        curr_running_proc->flags ^= (ALARM_ACTIVATED);
        curr_running_proc->alarm_ticks = 0;
    }
    else
    {
        curr_running_proc->flags |= ALARM_ACTIVATED;
        curr_running_proc->alarm_ticks = time_sec * 100;
    }

    return rc;
}

int system_call_getdents(unsigned int fd, 
                         struct dirent *dirp, 
                         unsigned int count)
{
    fileDesc *file         = NULL;
    Tree     *dir_node     = NULL;
    
    if(validate_user_buffer((uint64_t)dirp, count, PTE_U|PTE_W)<0)
    {
        return -ERR_FAULT;
    }
    file = fd_to_file(fd);
    
    if ( !file )
    {
        return -ERR_BADF;
    }
    if ( count < sizeof(struct dirent) )
    {
        return -ERR_INVAL;
    }
    if ( file->file->fileInfo->typeflag[0] != DIRTYPE )
    {
        return -ERR_NOTDIR;
    }
    dir_node = file->file;
    return get_children(dir_node, dirp, count, file);
}
int64_t system_call_getcwd(char *buf, size_t size)
{
    if(validate_user_buffer((uint64_t)buf, size, PTE_U|PTE_W)<0)
    {
        return -ERR_FAULT;
    }
    if ( size == 0 && buf != NULL )
    {
        return -ERR_INVAL;
    }
    if ( strlen(curr_running_proc->currentDirectory->name)+1 > size )
    {
        return -ERR_RANGE;
    }
    strncpy(buf, curr_running_proc->currentDirectory->name, NAME_MAX);
    
    return 0;
}
int64_t
system_call_chdir(const char *path)
{
    char abs_path[NAME_MAX];
    int len = 0;

    Tree* startNode = curr_running_proc->currentDirectory;
    int fromRoot = 0;

    if(validate_user_buffer((uint64_t)path, sizeof(abs_path), PTE_U|PTE_P)<0)
    {
        return -ERR_FAULT;
    }

    len = strnlen(path, sizeof(abs_path));
    if (len == 0)
    {
        return -ERR_NOENT;
    }
    else if (len < 0)
    {
        return -ERR_NAMETOOLONG;
    }

    strncpy(abs_path, path, sizeof(abs_path));
    parse_path(abs_path);

    if ( strlen(abs_path) > NAME_MAX )
    {
        return -ERR_NAMETOOLONG;
    }
    //Look for path starting from root
    if(path[0]=='/')
    {
        startNode = root;
        fromRoot++;
    }
    //Find the tree Node in filesystem corresponding to the path
    Tree* foundNode = findNodeInFileSystembyPath(startNode, (char*)path+fromRoot);
    if(foundNode == NULL || strlen(path) == 0)
    {
        return -ERR_NOENT;
    }
    if ( foundNode->fileInfo->typeflag[0] != DIRTYPE )
    {
        return -ERR_NOTDIR;
    }
    if ( fromRoot )
    {
        strncpy(curr_running_proc->currentDirectory->name, path, NAME_MAX);
    }
    else
    {
        if (strlen(curr_running_proc->currentDirectory->name) + strlen(path)+1 > NAME_MAX)
        {
            return -ERR_NAMETOOLONG;
        }
        strncat(curr_running_proc->currentDirectory->name, "/", sizeof("/"));
        strncat(curr_running_proc->currentDirectory->name, (char*)path, strlen(path)+1);
    }
    
    return 0;
}

off_t system_call_lseek(int fd, off_t offset, int whence)
{
    fileDesc *file         = fd_to_file(fd);
    uint64_t size=0;
    
    if (!file)
    {
        return -ERR_BADF;
    }

    atoo(&size, file->file->fileInfo->size);
    if ( file->offset + offset < 0 )
    {
        return -ERR_INVAL;
    }
    switch (whence)
    {
        case SEEK_SET:
            file->offset = offset;
            break;
        case SEEK_CUR:
            file->offset += offset;
            break;
        case SEEK_END:
            file->offset = (size +offset);
            break;
        default:
            return -ERR_INVAL;
    }
            
    return file->offset;
}
int system_call_dup2(int oldfd, int newfd)
{
    if(oldfd < 0 ||
       oldfd >= MAX_FILE_DESCRIPTORS ||
       newfd < 0 ||
       newfd >= MAX_FILE_DESCRIPTORS)
    {
        #ifdef LOG
            printf("Invalid old/new file descriptors sent \n");
        #endif
        return -ERR_BADF;
    }
    if(oldfd == newfd)
    {
        return newfd;
    }
    uint8_t  type =  curr_running_proc->listFileDesc[oldfd]->type;
    if(type ==PIPE_READ)
    {
          addPipeReadDesc(curr_running_proc->listFileDesc[oldfd]->vpipe,
            curr_running_proc,newfd);
    }
    else if( type == PIPE_WRITE)
    {
     
         addPipeWriteDesc(curr_running_proc->listFileDesc[oldfd]->vpipe,
            curr_running_proc,newfd);
    }
    curr_running_proc->listFileDesc[newfd] = curr_running_proc->listFileDesc[oldfd];
    return newfd;
}
int system_call_dup(int oldfd)
{
    if(oldfd < 0 || oldfd >= MAX_FILE_DESCRIPTORS)
    {
        #ifdef LOG
            printf("Invalid old file descriptor sent \n");
        #endif
        return -ERR_BADF;
    }
    int fd = findFirstFreeFDIndex(curr_running_proc);
    if( fd < 0)
    {
        #ifdef LOG
            printf("No free file descriptor left \n");
        #endif
        return -ERR_MFILE;
    }
     uint8_t  type =  curr_running_proc->listFileDesc[oldfd]->type;
    if(type ==PIPE_READ)
    {
          addPipeReadDesc(curr_running_proc->listFileDesc[oldfd]->vpipe,
            curr_running_proc,fd);
    }
    else if( type == PIPE_WRITE)
    {
     
         addPipeWriteDesc(curr_running_proc->listFileDesc[oldfd]->vpipe,
            curr_running_proc,fd);
    }
    curr_running_proc->listFileDesc[fd] = curr_running_proc->listFileDesc[oldfd];
    return fd;
}
int system_call_kill (pid_t pid)
{
    process_t *proc = get_proc_from_pid(pid);
    if ( pid == 1 )
    {
        #ifdef LOG
            printf("kill: %d: Cannot kill init process\n", pid);
        #endif
        return -ERR_RR_PERM;
    }
    if ( pid == 2 )
    {
        #ifdef LOG
            printf("kill: %d: Cannot kill root shell\n", pid);
        #endif
        return -ERR_RR_PERM;
    }
    return kill_process(proc);
}
int system_call_ps(struct proc_struct *proc)
{
    //return count_processes();

    int rc = 0;

    if((rc = validate_user_buffer((uint64_t)proc, sizeof(struct proc_struct), 
             PTE_P|PTE_U|PTE_W))<0)
    {
        return -ERR_FAULT;
    }
    
    return list_process(proc);


}
int64_t
syscall(uint64_t syscall_number, uint64_t param1, uint64_t param2,
        uint64_t param3, uint64_t param4, uint64_t param5)
{
    int64_t rc = 0;
    
    switch (syscall_number)
    {
        case SYS_exit:
            system_call_exit((int)param1); 
            break;
        case SYS_brk:
            rc = system_call_brk((size_t)param1);
            break;
        case SYS_fork:
            rc = system_call_fork(0);
            break;
        case SYS_background_fork:
            rc = system_call_fork(1);
            break;
        case SYS_getpid:
            rc = system_call_getpid();
            break;
        case SYS_getppid:
            rc = system_call_getppid();
            break;
        case SYS_execve:
            rc = system_call_execve((const char *)param1,
                                    (char **const)param2,
                                    (char **const)param3);
            break;
        case SYS_wait4:
            rc = system_call_waitpid((pid_t)param1, (int *)param2, (int)param3 );
            break;
        case SYS_nanosleep:
            rc = system_call_sleep((struct timespec *)param1,(struct timespec *)param2);
            break;
        case SYS_alarm:
            rc = system_call_alarm((uint64_t) param1);
            break;
        case SYS_getcwd:
            rc = system_call_getcwd((char *)param1, (size_t)param2);
            break;
        case SYS_chdir:
            rc = system_call_chdir((const char *)param1);
            break;
        case SYS_open:
            rc = system_call_open((char *)param1, (int)param2);
            break;
        case SYS_read:
            rc = system_call_read((int)param1, (void *)param2,
                                   (size_t)param3);
            break;
        case SYS_write:
            rc = system_call_write((int)param1, (const void *)param2,
                                   (size_t)param3);
            break;
        case SYS_lseek:
            rc = system_call_lseek((int)param1, (off_t) param2, (int) param3);
            break;
        case SYS_close:
             rc = system_call_close((int)param1);
            break;
        case SYS_pipe:
             rc = system_call_pipe((int*)param1);
            break;
        case SYS_dup:
            rc = system_call_dup((int)param1);
            break;
        case SYS_dup2:
            rc = system_call_dup2((int)param1,(int)param2);
            break;
        case SYS_getdents:
            rc = system_call_getdents((unsigned int )param1, 
                                 (struct dirent *)param2, 
                                 (unsigned int)param3);
            break;
        case SYS_kill:
            rc = system_call_kill((pid_t) param1 );
            break;
        case SYS_ps:
            rc = system_call_ps((struct proc_struct *)param1);
            break;
        default:
            #ifdef LOG
                printf("I don't recognize this syscall(no=%x)\n", syscall_number);
            #endif
            break;
    }
    return rc;
}
