#ifndef _ENV_VARIABLE_H_
#define _ENV_VARIABLE_H_

#include <sys/defs.h>

///////Attention:
// Before even planning to change these values,
//  1. check process_mem_layout.h and update USER_ENV_VARIABLE_START accordingly
//  2. Based on these values we know that we need only 64*8 Bytes on stack for envp[]
//     and only one stack page is more than enough, if you modify this then increase
//     allocated stack size
#define     MAX_ENV_SIZE        256
#define     MAX_ENV_COUNT       64

#define     MAX_ARGV_SIZE       256
#define     MAX_ARGV_COUNT      64

extern char env_variables[MAX_ENV_COUNT][MAX_ENV_SIZE];
extern size_t kern_env_count;

#endif // _ENV_VARIABLE_H_