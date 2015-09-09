#ifndef _PAGE_TABLE_H_
#define _PAGE_TABLE_H_

#include <sys/defs.h>
#include <sys/process.h>
#define PGSIZE      4096

#define PTE_P       0x1
#define PTE_W       0x2
#define PTE_U       0x4
#define PTE_COW     0x200


#define MAX_PT_ENTRY       512

#define PAGE_TO_PHYS(page_no) (page_no*PGSIZE)
#define PHYS_TO_PAGE(phys_addr) (&phys_page[((phys_addr) / PGSIZE)])
#define PHYS_TO_VA(phys) (uint64_t*)((pa_va_mapping_start) + (uint64_t)(phys))
#define PAGE_DESC_TO_PAGE_NO(page_desc_addr) ((page_desc_addr) - phys_page)
#define PAGE_DESC_TO_PHYS(page_desc_addr) (PAGE_TO_PHYS(PAGE_DESC_TO_PAGE_NO(page_desc_addr)))
#define PAGE_DESC_ADDR_TO_VA(page_desc_addr) PHYS_TO_VA((PAGE_DESC_TO_PHYS(page_desc_addr)))

#define MASK_PAGE_PERM 0xFFFFFFFFFFFFF000

#define FOURTH_LEVEL_TABLE_INDEX(virt_addr)   (((virt_addr) >> 39) & 0x1FF)
#define THIRD_LEVEL_TABLE_INDEX(virt_addr)    (((virt_addr) >> 30) & 0x1FF)
#define SECOND_LEVEL_TABLE_INDEX(virt_addr)   (((virt_addr) >> 21) & 0x1FF)
#define FIRST_LEVEL_TABLE_INDEX(virt_addr)    (((virt_addr) >> 12) & 0x1FF)

#define ROUNDUP(val, by, type)   (type)((uint64_t)(val) + ((by) - ((uint64_t)(val) % (by))) % (by))
#define ROUNDDOWN(val, by, type) (type)((uint64_t)(val) - ((uint64_t)(val) % (by)))

//Type of page faults
#define KERNEL_READ_NO_PTE_FAULT        0
#define KERNEL_READ_PROT_FAULT          1
#define KERNEL_WRITE_NO_PTE_FAULT       2
#define KERNEL_WRITE_PROT_FAULT         3
#define USER_READ_NO_PTE_FAULT          4
#define USER_READ_PROT_FAULT            5
#define USER_WRITE_NO_PTE_FAULT         6
#define USER_WRITE_PROT_FAULT           7

#define         MAX_PHYSICAL_RANGES         8

typedef struct _Pages_t
{
    uint8_t free;
    uint8_t ref;
    struct _Pages_t *next_free;
} Pages;

typedef struct _Physical_Rage_
{
    uint64_t start;
    size_t len;
} Physical_Range;

typedef struct _Physical_Memory_Map_
{
    uint64_t count;
    uint64_t max_size;
    Physical_Range ranges[MAX_PHYSICAL_RANGES];
} Physical_Memory_Map;

extern Pages* phys_page;
extern void *_physbase, *_physfree;
extern uint64_t kern_virtual_start,
                kern_virtual_end,
                pa_va_mapping_start,
                pa_va_mapping_end;

extern uint64_t phys_kernel_level4;

extern Physical_Memory_Map phys_mem_map;

extern uint64_t total_available_pages;
extern uint64_t total_free_pages;

static __inline void setcr3(uint64_t cr3)
{
	__asm __volatile("movq %0,%%cr3" : : "r" (cr3));
}

//static __inline uint64_t getcr3(void)
uint64_t getcr3();

int memory_init();

int32_t map_range_va2pa(uint64_t *level4, uint64_t va_start, uint64_t va_end,uint64_t pa_start, uint16_t perm);
void testKernelMapping(uint64_t* level4,uint64_t va_start,uint64_t va_end,uint64_t pa_start);
uint64_t* page_alloc();
void page_free(Pages *page);
void free_pages (uint64_t phys_page_table_base, uint64_t addr, uint32_t count);
uint64_t* page_lookup(uint64_t *page_table_base, uint64_t addr);
void reclaim_pages ( process_t *proc );
void delete_mapping(uint64_t *va_page_table_base, uint64_t va_addr);

int32_t va_to_phys(uint64_t *va_page_table_base,
                   uint64_t va_addr,
                   uint64_t *phys_addr);
uint64_t get_phys(uint64_t *va_page_table_base,
                   uint64_t va_addr);
                   
int32_t level4create(uint64_t *page_table_base, uint64_t addr, uint64_t pa_addr, uint16_t perm);
int32_t level3create(uint64_t *page_table_base, uint64_t addr, uint64_t pa_addr, uint16_t perm);
int32_t level2create(uint64_t *page_table_base, uint64_t addr, uint64_t pa_addr, uint16_t perm);
int32_t level1create(uint64_t *page_table_base, uint64_t addr, uint64_t pa_addr, uint16_t perm);

int allocate_memory(uint64_t phys_page_table_base,
                    uint64_t va_addr,
                    size_t size,
                    uint64_t perm);

int invalidate_tlb(uint64_t va_addr);

int32_t user_vma_update(uint64_t fault_addr, uint64_t req_perm, uint64_t rsp);
int32_t user_demand_paging_handler(uint64_t fault_addr);
int32_t demand_paging_on_syscall(uint64_t fault_addr, uint64_t perm);

int32_t validate_user_buffer(uint64_t start_addr, size_t len, uint64_t perm);

#endif //_PAGE_TABLE_H_