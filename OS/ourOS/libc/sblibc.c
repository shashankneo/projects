#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <syscall.h>
#include <error.h>

//wrapper for exit system call
void exit(int status)
{
    syscall_1(SYS_exit, (uint64_t)status);
}

int kill (pid_t pid)
{
    int32_t ret = syscall_1(SYS_kill, (uint64_t)pid);
    if( ret < 0 )
    {
        return -1;
    }
    else
    {
        return ret;
    }
}

int ps (struct proc_struct *proc)
{
    int32_t ret = syscall_1(SYS_ps, (uint64_t)proc);
    if( ret < 0 )
    {
        return -1;
    }
    else
    {
        return ret;
    }
}
//wrapper for fork system call
pid_t fork(void)
{
    pid_t ret=syscall_0(SYS_fork);
    if(ret<0)
    {
        return -1;
    }
    else
    {
        return ret;
    }
}

//CUSTOM: wrapper to create background process
pid_t background_fork(void)
{
    pid_t ret=syscall_0(SYS_background_fork);
    if(ret<0)
    {
        return -1;
    }
    else
    {
        return ret;
    }
}

//wrapper for getpid system call
pid_t getpid(void)
{
    return syscall_0(SYS_getpid);
}

//wrapper for getppid system call
pid_t getppid(void)
{
    return syscall_0(SYS_getppid);
}

//wrapper for execve system call
int execve(const char *filename, char *const argv[], char *const envp[])
{
    int64_t ret= syscall_3(SYS_execve, (uint64_t)filename, (uint64_t)argv, (uint64_t)envp);
    if(ret<0)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

//wrapper for waitpid system call
pid_t waitpid(pid_t pid, int *status, int options)
{
    pid_t ret;
    ret= syscall_3(SYS_wait4, (uint64_t)pid, (uint64_t)status, (uint64_t)options);
    if(ret<0)
    {
        return -1;
    }
    else
    {
        return ret;
    }
}

//wrapper for sleep system call
unsigned int sleep(unsigned int seconds)
{
    int64_t ret = 0;
    struct timespec  sleeptime, timeRemaining = {0};
    sleeptime.tv_sec=seconds;
    sleeptime.tv_nsec=0;
    ret = syscall_2(SYS_nanosleep, (uint64_t)&sleeptime,(uint64_t)&timeRemaining);
    if (ret < 0)
    {
        return (unsigned int)timeRemaining.tv_sec +
               sleeptime.tv_nsec/500000000L;
    }
    else
        return 0;
}

//wrapper for alarm system call
unsigned int alarm(unsigned int seconds)
{
    return syscall_1(SYS_alarm, (uint64_t)seconds);
}
//wrapper for getcwd system call
char *getcwd(char *buf, size_t size)
{
    //If the buffer passed is as NULL, then allocate memory using malloc with size as argument
    //If size is 0, then allocate size as necessary
    if(buf==NULL)
    {
        if(size==0)
        {
            size=MAX_PATH_LENGTH;
        }
        buf = malloc(size);
        if (!buf)
            return NULL;
    
    }
    if(syscall_2(SYS_getcwd, (uint64_t)buf, (uint64_t)size)<0)
    {
        return NULL;
    }

    return buf;
}
//wrapper for chdir system call
int chdir(const char *path)
{
    int64_t ret;
    ret= syscall_1(SYS_chdir, (uint64_t)path);
    if(ret<0)
        return -1;
    else
        return 0;
}
//wrapper for open system call
int open(const char *pathname, int flags)
{
    int64_t ret;
    ret=syscall_2(SYS_open, (uint64_t)pathname, (uint64_t)flags);
    if(ret<0)
        return -1;
    else
        return ret;
}

//wrapper for read system call
ssize_t read(int fd, void *buf, size_t count)
{
    ssize_t ret;
    ret= syscall_3(SYS_read, (uint64_t)fd, (uint64_t)buf, (uint64_t)count);
    if(ret<0)
        return -1;
    else
        return ret;
}

//wrapper for write system call
ssize_t write(int fd, const void *buf, size_t count)
{
    ssize_t ret= syscall_3(SYS_write, (uint64_t)fd, (uint64_t)buf, (uint64_t)count);
    if(ret<0)
        return -1;
    else
        return ret;
}
//wrapper for lseek system call
off_t lseek(int fildes, off_t offset, int whence)
{
    off_t ret;
    ret= syscall_3(SYS_lseek, (uint64_t)fildes, (uint64_t)offset, (uint64_t)whence);
    if(ret<0)
        return -1;
    else
        return ret;
}
//wrapper for close system call
int close(int fd)
{
    int ret;
    ret=syscall_1(SYS_close, (uint64_t)fd);
    if(ret<0)
        return -1;
    else
        return 0;
}
int pipe(int filedes[2])
{
    int ret;
    ret=syscall_1(SYS_pipe, (uint64_t)filedes);
    if(ret<0)
        return -1;
    else
        return ret;
}
int dup(int oldfd)
{
    int ret;
    ret= syscall_1(SYS_dup, (uint64_t)oldfd);
    if(ret<0)
        return -1;
    else
        return ret;
}
int dup2(int oldfd, int newfd)
{
    int ret;
    ret= syscall_2(SYS_dup2, (uint64_t)oldfd,(uint64_t)newfd);
    if(ret<0)
        return -1;
    else
        return ret;
}

int brk(void *end_data_segment)
{
    uint64_t curr_break = syscall_1(SYS_brk, 0);
    uint64_t ret        = syscall_1(SYS_brk, (uint64_t)end_data_segment);

    //printf("curr_break=%x. ret=%x\n", curr_break, ret);
    /* If SYS_brk returns the curr_break, it is a failure */
    if ( ret == curr_break)
    {
        errno = ERR_NOMEM;
        return -1;
    }
    else
    {
        return 0;
    }
}

void *sbrk(intptr_t increment)
{
    uint64_t curr_break;
    uint64_t ret; 

    curr_break = syscall_1(SYS_brk, 0);
    
    //printf("increment = %d\n", increment);
    
    if (increment == 0)
    {
        return (void *)curr_break;
    }
    ret = syscall_1(SYS_brk, curr_break+increment);
    if ( ret == curr_break )
    {
        errno = ERR_NOMEM;
        return (void *) -1;
    }
    else
    {
        return (void *) curr_break;
    }
}

void *
opendir( const char *name )
{
    struct DIR *dir = malloc (sizeof(struct DIR));
    if (dir == NULL)
        return NULL;
    
    dir->fd = open(name, O_RDONLY | O_DIRECTORY /*| O_NONBLOCK | O_CLOEXEC*/ );
    
    if ( dir->fd == -1 )
    {
        free(dir);
        dir = NULL;
        return NULL;
    }
    dir->pos = 0;
    dir->bytes_read = 0;
    dir->buf = NULL;
    
    return dir;
}

int closedir ( void *dirp)
{
    int rc = 0;
    struct DIR *dir = (struct DIR *) dirp;
    
    if (dir)
    {
        rc = close(dir->fd);
        free(dir->buf);
        free(dir);
    }
    return rc;
}

struct dirent *
readdir( void * dirp)
{
    struct DIR *dir = (struct DIR *) dirp;
    struct dirent *d = NULL;

    if ( !dir )
    {
        return NULL;
    }
    if ( dir->fd < 0 )
    {
        errno = ERR_BADF;
        return NULL;
    }
    if ( dir->pos > dir->bytes_read )
    {
        errno = ERR_NOENT;
        return NULL;
    }
    /* If there is no data in buffer or we are done reading all data, then
     * get more entries from the system by invoking getdents() again
     */
    if ( dir->pos == dir->bytes_read)
    {
        /* Reset buffer pointer to zero */
        dir->pos = 0;
        
        /* Read in multiples of 32k. Allocate buffer only once. We will keep
         * overwriting the buffer
         */
        if ( dir->buf == NULL )
        {
            dir->buf = malloc(32768);
            if (dir->buf == NULL)
            {
                return NULL;
            }
        }
        
        dir->bytes_read = syscall_3(SYS_getdents, (uint64_t)dir->fd, 
                                   (uint64_t)dir->buf, 32768);
        if ( dir->bytes_read < 0 )
        {
            /* getdents() has returned failure. Example, we may not have passed
             * a directory
             */
            return NULL;
        }
        if (dir->bytes_read == 0)
        {
            /* No more dirents present. Return NULL */
            return NULL;
        }
    }
    d = (struct dirent *)(dir->buf + dir->pos);
    dir->pos += d->d_reclen;
    
    return d;
}
