#ifndef _K_STRING_H_
#define  _K_STRING_H_

#include <stdlib.h>

char* itoa(int v, char *cArray, int base);
char* xtoa(unsigned int v, char *cArray, int base, int caps);
char* ptoa(uint64_t v, char *cArray, int base, int caps);
int atoi(int *v, const char *cArray);
int atox(unsigned int *v, const char *cArray);
int atou(uint64_t *v, const char *cArray);
int atoo(uint64_t *v, const char *cArray);
int strcmp( const char *str1,  const char *str2);
int strncmp( char *str1,  char *str2, int len);
char* strncpy(char *dest,  const char *src, int len);
int strincmp( char *str1,  char *str2, int len);
char to_lower ( char ch );
void * memcpy ( void * destination,  const void * source, unsigned int num );
void * memset ( void * ptr, int value, unsigned int num );
char * strncat( char * destination,  char * source, unsigned int num );
int strlen(const char *str);
int strnlen(const char *str, uint64_t max_allowed_len);
char *strstr( char *s1,  char *s2);
int read_line(char *cmd_line, int fd);
char *strrev( char *s1);
#endif //_K_STRING_H_