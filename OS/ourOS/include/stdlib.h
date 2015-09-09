#ifndef _STDLIB_H
#define _STDLIB_H

#include <sys/defs.h>
#include <sys_reg.h>
#include <shell.h>
#include <error.h>

extern __thread int errno;

int main(int argc, char* argv[], char* envp[]);
void exit(int status);

typedef int64_t intptr_t;
typedef int64_t time_t;
typedef uint64_t size_t;

// memory
void *malloc(size_t size);
void free(void *ptr);
int brk(void *end_data_segment);
void *sbrk(intptr_t increment);

// processes
typedef int32_t pid_t;
pid_t fork(void);
pid_t background_fork(void);
pid_t getpid(void);
pid_t getppid(void);
int execve(const char *filename, char *const argv[], char *const envp[]);
pid_t waitpid(pid_t pid, int *status, int options);
unsigned int sleep(unsigned int seconds);
unsigned int alarm(unsigned int seconds);

// paths
char *getcwd(char *buf, size_t size);
int chdir(const char *path);

// files
typedef int64_t ssize_t;

enum { O_RDONLY = 0, O_WRONLY = 1, O_RDWR = 2, O_CREAT = 0x40, O_DIRECTORY = 0x10000 };
int open(const char *pathname, int flags);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
enum { SEEK_SET = 0, SEEK_CUR = 1, SEEK_END = 2 };
typedef uint64_t off_t;
off_t lseek(int fildes, off_t offset, int whence);
int close(int fd);
int pipe(int filedes[2]);
int dup(int oldfd);
int dup2(int oldfd, int newfd);
int kill (pid_t pid);

typedef enum proc_state
{
    PROC_CREATED=0,
    PROC_RUNNING,
    PROC_RUNNABLE,
    PROC_WAITING,
    PROC_TERMINATED,
    PROC_NONE
}proc_state_t;

// directories
#define NAME_MAX 256
struct dirent
{
	long d_ino;
	off_t d_off;
	unsigned short d_reclen;
    char           d_type;
	char d_name [NAME_MAX];
};

struct DIR
{
    int fd; /* File descriptor to the open directory */
    int pos; /* Position index into buffer */
    int bytes_read; /* Number of bytes read from getdents() */
    char *buf; /* Buffer into which we read dentries */
};
struct timespec {
    time_t tv_sec;        /* seconds */
    long   tv_nsec;       /* nanoseconds */
};

struct proc_struct
{
    char        name[256];
    pid_t       pid;
    pid_t       ppid;
    uint32_t    state;
    off_t       offset;
};

int ps (struct proc_struct* proc);
void *opendir(const char *name);
struct dirent *readdir(void *dir);
int closedir(void *dir);

#endif
