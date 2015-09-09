#ifndef _ASSEMBLY_UTILITY_H_
#define _ASSEMBLY_UTILITY_H_

#define KEYBOARD_COMMAND_PORT 0x64
#define KEYBOARD_DATA_PORT 0x60

static __inline void outb(int port, unsigned char data)
{
    __asm __volatile("outb %%al, (%%dx)" : : "a" (data), "d" (port));
}

static __inline unsigned char inb(int port)
{
    unsigned char data;
    __asm __volatile("inb (%%dx), %%al" : "=a" (data) : "d" (port));
    return data;
}

#endif //_ASSEMBLY_UTILITY_H_