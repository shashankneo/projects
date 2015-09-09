#ifndef __SYS_SYSCALL_H
#define __SYS_SYSCALL_H

#include<sys/defs.h>
#include<stdlib.h>

#define SYS_exit       60
#define SYS_brk        12
#define SYS_fork       57
#define SYS_getpid     39
#define SYS_getppid   110
#define SYS_execve     59
#define SYS_wait4      61
#define SYS_nanosleep  35
#define SYS_alarm      37
#define SYS_getcwd     79
#define SYS_chdir      80
#define SYS_open        2
#define SYS_read        0
#define SYS_write       1
#define SYS_lseek       8
#define SYS_close       3
#define SYS_pipe       22
#define SYS_dup        32
#define SYS_dup2       33
#define SYS_getdents   78
#define SYS_kill       62
#define SYS_ps         100 // Random number chosen
#define SYS_background_fork     101 // Random number chosen

int64_t
syscall(uint64_t syscall_number, uint64_t param1, uint64_t param2,
        uint64_t param3, uint64_t param4, uint64_t param5);
        
int
system_call_open(char *pathname, int flags );
int64_t
system_call_write(int fd, const void *buf, size_t count);

int64_t 
system_call_close(int fd);

void
system_call_exit(int return_value );

pid_t 
system_call_getpid( void );

pid_t 
system_call_getppid( void );

pid_t
system_call_waitpid(pid_t pid, int *status, int options);


void
wait_in_kernel();

unsigned int 
system_call_sleep(struct timespec *sleeptime, struct timespec *time_remaining);

pid_t
system_call_fork();

int system_call_getdents(unsigned int fd, struct dirent *dirp,
                         unsigned int count);

char *getcwd(char *buf, size_t size);

int64_t
system_call_chdir(const char *path);

off_t system_call_lseek(int fd, off_t offset, int whence);

int system_call_kill (pid_t pid);

int system_call_ps(struct proc_struct *proc);

#endif
