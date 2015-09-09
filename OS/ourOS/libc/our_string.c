#include "our_string.h"
#include <stdio.h>
#include <stdlib.h>
#include "shell.h"

char* itoa(int v, char *cArray, int base)
{
    int negative = 0;
    int rem;
    char *buf = cArray;
    
    if (!cArray)
        return NULL;
    
    if(v < 0)
    {
        negative = 1;
        v = -v;
    }
    
    do
    {
        rem = v % base;
        v /= base;
        *buf = (rem < 10) ? (rem + '0') : ('A' + rem - 10);
        buf++;
    } while (v);

    if (negative)
    {
        *buf = '-';
        buf++;
    }

    *buf = '\0';
    return strrev(cArray);
}

char* xtoa(unsigned int v, char *cArray, int base, int caps)
{
    int rem;
    char *buf = cArray;

    if (!cArray)
        return NULL;
    
    do
    {
        rem = v % base;
        v /= base;
        *buf = (rem < 10) ? (rem + '0') : ((caps ? 'A' : 'a') + rem - 10);
        buf++;
    } while (v);

    *buf = '\0';
    return strrev(cArray);
}

char* ptoa(uint64_t v, char *cArray, int base, int caps)
{
    int rem;
    char *buf = cArray;

    if (!cArray)
        return NULL;

    do
    {
        rem = v % base;
        v /= base;
        *buf = (rem < 10) ? (rem + '0') : ((caps ? 'A' : 'a') + rem - 10);
        buf++;
    } while (v);

    *buf = 'x';
    buf++;
    *buf = '0';
    buf++;    
    *buf = '\0';
    return strrev(cArray);
}

// On invalid input it returns -1
int atoi(int *v, const char *cArray)
{
    int negative = 0;
    int start_base = 1;
    int len = strlen(cArray);
    int endIndex = len - 1;
    int startIndex = 0;

    if (!v || !cArray)
        return -1;

    if(cArray[0] == '-')
    {
        negative = 1;
        startIndex = 1;
    }

    *v = 0;
    while (endIndex >= startIndex)
    {
        if ('0' <= cArray[endIndex] && cArray[endIndex] <= '9')
        {
            *v += ((cArray[endIndex] - '0') * start_base);
            start_base *= 10;
        }
        else
        {
            *v = 0;
            return -1;
        }
        endIndex--;
    };

    if (negative)
    {
        *v = -*v;
    }

    return 0;
}

// On invalid input it returns -1
int atox(unsigned int *v, const char *cArray)
{
    int negative = 0;
    int start_base = 1;
    int len = strlen(cArray);
    int endIndex = len - 1;
    int startIndex = 0;
    char ch;

    if (!v || !cArray)
        return -1;

    if(cArray[0] == '-')
    {
        negative = 1;
        startIndex = 1;
    }

    *v = 0;
    while (endIndex >= startIndex)
    {
        ch = to_lower(cArray[endIndex]);
        if ('0' <= ch && ch <= '9')
        {
            *v += ((ch - '0') * start_base);
        }
        else if ('a' <= ch && ch <= 'f')
        {
            *v += ((ch - 'a' + 10) * start_base);
        }
        else
        {
            *v = 0;
            return -1;
        }
        start_base *= 16;
        endIndex--;
    };

    if (negative)
    {
        *v = -*v;
    }

    return 0;
}

int atou(uint64_t *v, const char *cArray)
{
    int negative = 0;
    int start_base = 1;
    int len = strlen(cArray);
    int endIndex = len - 1;
    int startIndex = 0;

    if (!v || !cArray)
        return -1;

    if(cArray[0] == '-')
    {
        negative = 1;
        startIndex = 1;
    }

    *v = 0;
    while (endIndex >= startIndex)
    {
        if ('0' <= cArray[endIndex] && cArray[endIndex] <= '9')
        {
            *v += ((cArray[endIndex] - '0') * start_base);
            start_base *= 10;
        }
        else
        {
            *v = 0;
            return -1;
        }
        endIndex--;
    };

    if (negative)
    {
        *v = -*v;
    }

    return 0;
}

int atoo(uint64_t *v, const char *cArray)
{
    int negative = 0;
    int start_base = 1;
    int len = strlen(cArray);
    int endIndex = len - 1;
    int startIndex = 0;

    if (!v || !cArray)
        return -1;

    if(cArray[0] == '-')
    {
        negative = 1;
        startIndex = 1;
    }

    *v = 0;
    while (endIndex >= startIndex)
    {
        if ('0' <= cArray[endIndex] && cArray[endIndex] <= '7')
        {
            *v += ((cArray[endIndex] - '0') * start_base);
            start_base *= 8;
        }
        else
        {
            *v = 0;
            return -1;
        }
        endIndex--;
    };

    if (negative)
    {
        *v = -*v;
    }

    return 0;
}

bool
is_comment ( char *line)
{
    while (*line == ' ')    line++;
    return *line == '#'? true:false; 
}

int read_line(char *cmd_line, int fd)
{
    char ch = 0;
    unsigned int count =0;
    int rc = 0;
    while ( (rc = read(fd, &ch, sizeof(ch))) >0)
    {
        if (ch == '\n')
        {
            cmd_line[count] = ch;
            break;
        }
        cmd_line[count] = ch;
        count++;
    }

    if(count > 0 &&
       cmd_line[count-1] == '\r' &&
       cmd_line[count] == '\n')
    {
        cmd_line[count-1] = 0;
    }

    cmd_line[count] = 0;
    
    return rc < 0? rc : count;  
}
char 
to_lower ( char ch )
{
    if ( ch >= 'A' && ch <= 'Z' )
    {
        ch+= 32;
    }
    return ch;
}
int strcmp( const char *str1, const char *str2)
{
    int i;
    for (i = 0; str1[i] && str2[i]; i++)
    {
        if (str1[i] < str2[i])
            return -1;
        else if (str1[i] > str2[i])
            return 1;
    }
    return (!str1[i] && !str2[i]) ? 0 : (!str1[i] ? -1 : 1);
}

int strncmp( char *str1,  char *str2, int len)
{
    int i;
    for (i = 0; str1[i] && str2[i] && (i < len); i++)
    {
        if (str1[i] < str2[i])
            return -1;
        else if (str1[i] > str2[i])
            return 1;
    }
    return ((!str1[i] && !str2[i]) || (i == len)) ? 0 : (!str1[i] ? -1 : 1);
}

int strincmp( char *str1,  char *str2, int len)
{
    int i;
    for (i = 0; str1[i] && str2[i] && (i < len); i++)
    {
        if (to_lower(str1[i]) < to_lower(str2[i]))
            return -1;
        else if (to_lower(str1[i]) > to_lower(str2[i]))
            return 1;
    }
    return ((!str1[i] && !str2[i]) || (i == len)) ? 0 : (!str1[i] ? -1 : 1);
}

char * strncpy(char *dest,  const char *src, int len)
{
    int i;
    for (i = 0; i < len && src[i]; i++)
    {
        dest[i] = src[i];
    }
    dest[i] = '\0';
    
    return dest;
}

char * strncat ( char * destination,  char * source, unsigned int num )
{
    int i = 0,
        j = 0;
    
    for (i =0; destination[i];i++);
    
    for ( j = 0; source[j] && j < num ; j++, i++ )
    {
        destination[i] = source[j];
    }
    destination[i] = '\0';
    
    return destination;
}
void * memcpy ( void * destination,  const void * source, unsigned int num )
{
    unsigned int i = 0;
    
    for (i=0; i < num; ++i)
    {
        ((char*)destination)[i] = ((char*)source)[i];
    }
    return destination;
}

void * memset ( void * ptr, int value, unsigned int num )
{
    unsigned int  i = 0;
    
    for (i=0; i < num; ++i)
    {
        ((char*)ptr)[i] = value;
    }
    return ptr;
}

int strlen(const char *str)
{
    unsigned int i;
    for (i = 0; str[i]; i++)
        ;
    return i;
}

char *strstr( char *s1,  char *s2)
{
    unsigned   len1 = strlen(s1);
    unsigned   len2 = strlen(s2);
    int i =0;
    
    if (!s1 || !s2 )
    {
        return NULL;
    }
    if ( len2 > len1 )
    {
        return NULL;
    }
    
    for ( i =0; i < len1-len2+1; i++, s1++)
    {
        if (strncmp(s1, s2, len2) == 0)
        {
            return (char *)s1;
        }
    }

    return NULL;
}

char *strrev(char *s1)
{
    int i, temp;
    int j = strlen(s1) - 1;
    
    for(i = 0; i < j; i++, j--)
    {
        temp = s1[i];
        s1[i] = s1[j];
        s1[j] = temp;
    }
    
    return s1;
}