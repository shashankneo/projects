#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <sys/defs.h>

int pushchar(char ch);
int getline(char *str, size_t len);
int
is_buffer_empty();
#endif//_TERMINAL_H_