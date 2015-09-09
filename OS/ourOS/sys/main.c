#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>
#include <sys/isr.h>
#include <sys/pagetable.h>
#include <sys/kstring.h>
#include <sys/process.h>
#include <sys/kmalloc.h>
 
extern void remap_pic(void);
extern void set_timer(void);
extern void syscall_isr(void);

uint32_t* loader_stack;
extern char kernmem, physbase;
struct tss_t tss;


void kernel_process()
{
	launch_process("bin/hello");
    launch_process("bin/idle");
	while(1)
	{
	}
}

void start(uint32_t* modulep, void* physbase, void* physfree)
{
    //enable_syscall_extension();
    init_idt();
    set_timer();
    remap_pic();

	struct smap_t {
		uint64_t base, length;
		uint32_t type;
	}__attribute__((packed)) *smap;
	while(modulep[0] != 0x9001) modulep += modulep[1]+2;
	for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
		if (smap->type == 1 /* memory */ && smap->length != 0) {
			printf("Available Physical Memory [%x-%x]\n", smap->base, smap->base + smap->length);
            phys_mem_map.ranges[phys_mem_map.count].start = smap->base;
            phys_mem_map.ranges[phys_mem_map.count].len = smap->length;
            phys_mem_map.count++;
            if (phys_mem_map.max_size < (smap->base + smap->length))
            {
                phys_mem_map.max_size = (smap->base + smap->length);
            }
		}
	}

    memory_init();
    kmalloc_init();
    populateFileSystem();
    //checkFileSystem(root, 0);

    //printf("tarfs in [%p:%p]\n", &_binary_tarfs_start, &_binary_tarfs_end);

    // ######################
    // First process will be the foreground process
    // So make sure what order you call launch_process
    // ######################

    launch_process("bin/idle");

    __asm__ __volatile__ ("sti\n");

    while(1);
	// kernel starts here
}

void enable_syscall_extension()
{
   uint32_t lo, hi, msr;
   uint64_t addr;

   msr = 0xC0000080; // IFER
   lo = 0;
   hi = 0;
   __asm__ __volatile__("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
   lo |= 1;
   __asm__ __volatile__("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));

   msr = 0xC0000082;  // IA32_LSTAR
   addr =  (uint64_t)&syscall_isr;
   lo = addr & 0xFFFFFFFF;
   hi = (addr >> 32) & 0xFFFFFFFF;
   __asm__ __volatile__("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));

   // Setting SS and CS for SYSCALL & SYSRET (SS = CS + 8)
   msr = 0xC0000081;  // IA32_STAR
   lo = 0;
   hi  = (1 << 3) & 0xFFFF;    // For Kernel
   hi |= ((3 << 3) << 16);     // For User
   __asm__ __volatile__("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));

   msr = 0xC0000084;   // IA32_FMASK
   __asm__ __volatile__("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
   printf("Fmask: %x %x\n", hi, lo);
}

void boot(void)
{
	// note: function changes rsp, local stack variables can't be practically used
	//register char *s, *v;
    //int x1 = 10, y = 0;
	__asm__(
		"movq %%rsp, %0;"
		"movq %1, %%rsp;"
		:"=g"(loader_stack)
		:"r"(&stack[INITIAL_STACK_SIZE])
	);

    reload_gdt();
    setup_tss();
    
    _physbase = (void *)&physbase;
    _physfree = (void *)(uint64_t)loader_stack[4];

    kern_virtual_start = 0xffffffff80000000;
    kern_virtual_end = 0xffffffff80000000 + (uint64_t)_physfree;

	start(
		(uint32_t*)((char*)(uint64_t)loader_stack[3] + (uint64_t)&kernmem - (uint64_t)&physbase),
		&physbase,
		(void*)(uint64_t)loader_stack[4]
	);
	//s = "!!!!! start() returned !!!!!";
	//for(v = (char*)0xFFFFFFFF800B8000; *s; ++s, v += 2) *v = *s;
    while(1);
}
