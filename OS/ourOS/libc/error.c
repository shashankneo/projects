#include <error.h>
#include <our_string.h>
#include <stdlib.h>

char *error_strings[] =
{
    "",
    "This operation is not permitted\n",
    "No such file or directory exists\n",
    "No such process exists\n",
    "System call is interrupted\n",
    "Input/output error: might be physical read or write error\n",
    "errno:6\n",
    "Too long argument list passed\n",
    "Invalid exe file format\n",
    "Invalid file descriptor\n",
    "There are no child processes\n",
    "errno:11\n",
    "Memory not available\n",
    "Permission denied or file/directory not found\n",
    "Invalid address\n",
    "Block device required\n",  //15
    "Resource is busy\n",
    "File already exists\n",
    "errno:18\n",
    "Unsupported device\n",
    "It's not a directory\n",
    "It is a directory\n",
    "Invalid argument\n",
    "Too many open files by the process\n",
    "Too many distinct open files in the system\n",
    "errno:25\n",
    "Can't execute a busy text file\n",
    "File too large\n",
    "No space left on device\n",
    "Illegal seek operation\n",
    "Operation not allowed on Read-only file system\n",
    "Too many links\n",
    "Pipe is broken\n",
    "Numerical argument out of domain\n",
    "Result out of range\n",
    "Command Not Found\n",
    "Error occured while reading from file\n",
    "Unspecified error\n",
    "Child cannot be created\n",
    "File descriptors limit reached\n",   // 39
    "errno:40\n",
    "errno:41\n",
    "errno:42\n",
    "errno:43\n",
    "errno:44\n",
    "errno:45\n",
    "errno:46\n",
    "errno:47\n",
    "errno:48\n",
    "errno:49\n",
    "errno:50\n",
    "errno:51\n",
    "errno:52\n",
    "errno:53\n",
    "errno:54\n",
    "errno:55\n",
    "errno:56\n",
    "errno:57\n",
    "errno:58\n",
    "errno:59\n",
    "errno:60\n",
    "errno:61\n",
    "errno:62\n",
    "Name too long\n", // 63
};

void print_errno(int err, char *cmd)
{
    char err_string[1024] = {0};
    strncpy(err_string, cmd, sizeof(err_string));
    strncat(err_string, ": ", strlen(": "));
    
    print_error(err_string);
    print_error((err >= 0 && err < ERR_LAST) ? 
                error_strings[err] : 
                "Unknown error\n");
}

void print_error(void *str)
{
    write(STD_ERR, str, strlen(str));
}
