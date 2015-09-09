#ifndef __SBUNIX_H
#define __SBUNIX_H

#include <sys/defs.h>

//#define LOG 1

int sprintf(char *output, const char *format, ...);
int printf(const char *fmt, ...);
int user_printf(char *output);

#endif
