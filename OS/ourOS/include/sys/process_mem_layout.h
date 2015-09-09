/*
Kernel virtual address space starts at 0xffffffff80000000
Kernel stack is already present in kernel virtual address space
*/

//Supports only 64 ENVs of size 256 bytes each
#define KERNEL_COW_START                0xFFFFFFFF7FFFF000
#define USER_ENV_VARIABLE_START         0x00006FFFFFFEF000

#define USER_ARGV_START                 0x00006FFFFFFEE000

#define USER_STACK_TOP                  0x00006FFFFFFED000
#define USER_HEAP_START                 0x00005FFFFFFFF000
