#include <sys/terminal.h>
#include <sys/process.h>
#include <sys/sbunix.h>
#include <sys/console.h>
#include <sys/pagetable.h>

char buf[PGSIZE] = {0};
int read_index = -1;
int write_index = -1;
int line_count = 0;

int
is_buffer_empty()
{
    return ( read_index == write_index)? 1:0;
}
int pushchar(char ch)
{
    if (((write_index + 1) % sizeof(buf)) == read_index)
    {
        // buffer overflow
        return -1;
    }

    if (ch == '\b')
    {
        if (write_index ==  read_index)
        {
            // It means we need to reset both indexes
            write_index = read_index = -1;
        }
        else if (write_index != -1)
        {
            // We need to move back the write pointer
            // if and only if we actually pushed something
            if (read_index ==  -1 && write_index == 0)
            {
                write_index = -1;
            }
            else
            {
                write_index = (write_index - 1);
                write_index = (write_index + sizeof(buf)) % sizeof(buf);
            }
        }
        return 0;
    }

    write_index = (write_index + 1) % sizeof(buf);
    buf[write_index] = ch;

    if (ch == '\n')
    {
        line_count++;
        wake_up_foreground_proc();
    }

    return 0;
}

int getchar(char *ch)
{
    if (!ch)
        return -1;

    // Nothing to read
    if (read_index == write_index)
        return 0;

    read_index = (read_index + 1) % sizeof(buf);
    *ch = buf[read_index];
    return 1;
}

// getline fills str[] till len chars/newline char
// It doesn't add null character BUT it adds newline character
int getline(char *str, size_t len)
{
    int rc = 0;
    size_t i = 0;
    if (line_count == 0)
    {
        return -1;
    }

    while(i < len)
    {
        rc = getchar(&str[i]);
        if (rc < 0)
        {
            // Failed to get character
            // In theory we should never reach here
            #ifdef LOG
                printf("Failed to get a character\n");
            #endif
            return -1;
        }
        else if(rc == 0)
        {
            //Nothing to read
            break;
        }

        i++;

        if (str[i-1] == '\n')
        {
            // We reached a newline character
            line_count--;
            break;
        }
    }

    return i;
}