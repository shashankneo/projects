#include <sys/env_variable.h>

size_t kern_env_count = 5;

// Last entry should be 0
char env_variables[MAX_ENV_COUNT][MAX_ENV_SIZE] = 
{
    "HOME=/bin",
    "PATH=/bin",
    "OS=TheBestOSEver",
    "PS1=sbush:",
    "PWD=/",
    {0}
};
