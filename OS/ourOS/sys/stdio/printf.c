#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/kstring.h>
#include <sys/console.h>

static int get_formated_output(char *output, const char *format, va_list _args)
{
    va_list args;
    uint64_t printed = 0;
    char c[256] = {0};
    char *pStr = NULL;
    int iVal = 0;
    unsigned int uVal = 0;
    uint64_t u64 = 0;
    va_copy(args, _args);

    while(*format)
    {
        // Handle special symbols
        if (*format == '\\')
        {
            switch (*(format + 1))
            {
                case 'n':
                    strncat(output, (char *)format, 2);
                    format++;
                    break;
                case 't':
                    strncat(output, (char *)format, 2);
                    format++;
                    break;
                default:
                    strncat(output, (char *)format, 1);
                    break;
            }
            printed++;
        }
        // Handle other characters
        else if (*format != '%')
        {
            strncat(output, (char *)format, 1);
            printed++;
        }
        // Handle format specifiers
        else
        {
            format++;
            switch (*format)
            {
                case 'c':
                    c[0] = va_arg(args, int);
                    c[1] = '\0';
                    strncat(output, c, 1);
                    printed++;
                    break;
                case 's':
                    pStr = va_arg(args, char *);
                    strncat(output, pStr, strlen(pStr));
                    printed += strlen(pStr);
                    break;
                case 'd':
                    iVal = va_arg(args, int);
                    pStr = itoa(iVal, c, 10);
                    strncat(output, pStr, strlen(pStr));
                    printed += strlen(pStr);
                    break;
                case 'x':
                    uVal = va_arg(args, unsigned int);
                    pStr = xtoa(uVal, c, 16, 0);
                    strncat(output, pStr, strlen(pStr));
                    printed += strlen(pStr);
                    break;
                case 'X':
                    uVal = va_arg(args, unsigned int);
                    pStr = xtoa(uVal, c, 16, 1);
                    strncat(output, pStr, strlen(pStr));
                    printed += strlen(pStr);
                    break;
                case 'p':
                    u64 = (uint64_t)va_arg(args, void *);
                    pStr = ptoa(u64, c, 16, 0);
                    strncat(output, pStr, strlen(pStr));
                    printed += strlen(pStr);
                    break;
                default:
                    strncat(output, (char *)format, 1);
                    printed++;
                    break;
            }
        }
        format++;
    }
    va_end(args);
    return printed;
}

int sprintf(char *output, const char *format, ...)
{
    int printed;
    va_list args;
    va_start(args, format);
    *output = 0;
    printed = get_formated_output(output, format, args);
    va_end(args);
    return printed;
}

// Will support %c, %x, %d, %s, %p
int printf(const char *format, ...)
{
    int printed;
    va_list args;
    char output[MAX_STRING];
    memset(output, 0, sizeof(output));
    va_start(args, format);
    printed = get_formated_output(output, format, args);
    kprintf(output);
    va_end(args);
    return printed;
}

int user_printf(char *output)
{
    int printed;
    kprintf(output);
    printed = strlen(output);
    return printed;
}
