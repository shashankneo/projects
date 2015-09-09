#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <our_string.h>

enum ReadState
{
    READING_LEADING_WS_START,
    READING_LEADING_WS_DONE
};

static char scanf_buf[MAX_STRING] = {0};
static int read_index = 0;
static int end_index = 0;

int is_whitespace(char c)
{
    if (c == ' ' || c == '\t' || c == '\n')
        return 1;
    return 0;
}

int read_without_whitespace(char *buf, size_t size)
{
    enum ReadState rs = READING_LEADING_WS_START;
    int i = 0;
    char c;
    
    if (!buf || size == 0)
        return -1;

    while(i < (size -1) && (end_index - read_index) > 0)
    {
        if (!is_whitespace(c = scanf_buf[read_index++]))
        {
            if (rs == READING_LEADING_WS_START)
                rs = READING_LEADING_WS_DONE;
            buf[i] = c;
            i++;
        }
        else
        {
            // It's a space after non-whitespace 
            if (rs != READING_LEADING_WS_START)
                break;
        }
    }
    buf[i] = '\0';
    return i;
}

int read_variable(int fd, char *buf, size_t size)
{
    int ret = 0;

    if (!buf || size == 0)
        return -1;

    do
    {
        while (end_index - read_index <= 0)
        {
            ret = read(fd, &scanf_buf, sizeof(scanf_buf));
            if (ret < 0)
                return -1;

            scanf_buf[(ret > 0) ? (ret - 1) : 0] = '\0';

            //if read() returns 0 then it means it was interrupted by some event
            if (ret == 0)
                return 0;
            else if (ret > 0)
            {
                read_index = 0;
                end_index = ret-1;
            }
        }
        // If ret is 0 then it means buffer had whitespaces only
        ret = read_without_whitespace(buf, size);
    } while (!ret);

    return ret;
}

// returns number of items successfully read
int scanf(const char *format, ...) {
    va_list val;
    int scanned = 0;
    int64_t read_chars = 0;
    
    char c[MAX_STRING] = {0};
    char *pStr = NULL;
    char *char_addr = NULL;
    unsigned int *hex_addr = NULL;
    int *int_addr = NULL;
    
    va_start(val, format);

    while(*format)
    {
        if (*format == '%')
        {
            format++;
            switch (*format)
            {
                case 'c':
                    char_addr = va_arg(val, char *);
                    read_chars = read_variable(0, c, 2);

                    if (read_chars <= 0)
                        return read_chars;

                    *char_addr = c[0];
                    scanned++;
                    break;
                case 's':
                    pStr = va_arg(val, char *);
                    read_chars = read_variable(0, c, MAX_STRING);

                    if (read_chars <= 0)
                        return read_chars;

                    strncpy(pStr, c, MAX_STRING);
                    scanned++;
                    break;
                case 'd':
                    int_addr = va_arg(val, int *);
                    read_chars = read_variable(0, c, MAX_STRING);

                    if (read_chars <= 0)
                        return read_chars;

                    if (atoi(int_addr, c) < 0)
                        return -1;
                    scanned++;
                    break;
                case 'x':
                case 'X':
                    hex_addr = va_arg(val, unsigned int *);
                    read_chars = read_variable(0, c, MAX_STRING);

                    if (read_chars <= 0)
                        return read_chars;

                    if (atox(hex_addr, c) < 0)
                        return -1;
                    scanned++;
                    break;
                default:
                    // Not supported... Ignore
                    break;
            }
        }
        ++format;
    }

    va_end(val);
    return scanned;
}
