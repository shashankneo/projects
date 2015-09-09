#include <sys/pagetable.h>
#include <sys/kstring.h>
#include <sys/sbunix.h>
#include <sys/process_mem_layout.h>
#include <sys/basic_util.h>

Pages *free_phys_pages = NULL;
uint64_t nPages = 0;
Pages* phys_page = NULL;
uint64_t phys_kernel_level4 = 0;
void *_physbase = NULL, *_physfree = NULL;
uint64_t kern_virtual_start = 0, kern_virtual_end = 0;
uint64_t pa_va_mapping_start = 0xFFFFFFFF00000000;
uint64_t pa_va_mapping_end   = 0xFFFFFFFF40000000;

uint64_t total_available_pages = 0;
uint64_t total_free_pages = 0;

Physical_Memory_Map phys_mem_map = {0};

uint64_t getcr3()
{
	uint64_t cr3;
	__asm __volatile("movq %%cr3,%0" : "=r" (cr3));
	return cr3;
}

int is_free_during_init(uint64_t page_index)
{
    size_t i;
    size_t phys_addr;
    int is_free = 0;

    phys_addr = (uint64_t)PAGE_TO_PHYS(page_index);
    // First check whether it falls under available physical range or not
    for (i = 0; i < phys_mem_map.count; i++)
    {
        if (phys_mem_map.ranges[i].start <= phys_addr &&
            phys_addr <= (phys_mem_map.ranges[i].start + phys_mem_map.ranges[i].len))
        {
            is_free = 1;
            break;
        }
    }

    // check whether it is indeed free
    // i.e. not used by kernel
    if (is_free == 1)
    {
        if ((uint64_t)_physbase <= phys_addr &&
            phys_addr <= (kern_virtual_end - kern_virtual_start))
        {
            is_free = 0;
        }
    }

    return is_free;
}

void phys_pages_init()
{
    uint64_t i = 0;
    nPages = phys_mem_map.max_size/PGSIZE;
    phys_page = ROUNDUP(kern_virtual_end, PGSIZE, Pages *);
    memset(phys_page, nPages*sizeof(Pages), 0x0);
    kern_virtual_end += nPages*sizeof(Pages);

    for(i = 0; i < nPages; i++)
    {
        if(is_free_during_init(i))
        {
            total_free_pages++;
            phys_page[i].free=1;
            phys_page[i].ref=0;
            phys_page[i].next_free = NULL;
        }
        else
        {
            phys_page[i].free = 0;
            phys_page[i].ref = 0;
            phys_page[i].next_free=NULL;
            continue;
        }
        
        if (free_phys_pages == NULL)
        {
              free_phys_pages = &phys_page[i];  
        }
        else
        {
              phys_page[i].next_free = free_phys_pages;
              free_phys_pages = &phys_page[i];  
        }
    }
}

uint64_t num_of_free_pages ()
{
    Pages *tmp = free_phys_pages;
    uint64_t count = 0;
    while ( tmp != NULL )
    {
        tmp = tmp->next_free;
        count++;
    }
    return count;
}
void reclaim_pages ( process_t *proc )
{
    if (!proc)
        return;

    //uint64_t * pa_lookup = NULL;
    uint64_t *l4_table_base = NULL;
    uint64_t *l3_table_base = NULL;
    uint64_t *l2_table_base = NULL;
    uint64_t *l1_table_base = NULL;
    uint64_t kernel_start   = FOURTH_LEVEL_TABLE_INDEX(kern_virtual_start);

    // Free pagetable pages 
    if ( proc->page_table_base != NULL )
    {
        l4_table_base = PHYS_TO_VA(proc->page_table_base);
        /* For the fourth level page table, we need to free only the user level
         * entries. Above 'kernel_start', shared kernel level page tables exist
         * which are not supposed to be freed */
        for ( int level4 = 0; level4 < kernel_start ; level4++ )
        {
            if ( l4_table_base[level4] & PTE_P )
            {
                l3_table_base = PHYS_TO_VA((l4_table_base[level4] & MASK_PAGE_PERM));
                for ( int level3 = 0; level3 < MAX_PT_ENTRY ; level3++)
                {
                    if ( l3_table_base[level3] & PTE_P )
                    {
                        l2_table_base = PHYS_TO_VA((l3_table_base[level3] & MASK_PAGE_PERM));
                        for ( int level2 = 0; level2 < MAX_PT_ENTRY ; level2++)
                        {
                            if ( l2_table_base[level2] & PTE_P )
                            {
                                l1_table_base = PHYS_TO_VA((l2_table_base[level2] & MASK_PAGE_PERM));
                                for ( int level1 = 0; level1 < MAX_PT_ENTRY ; level1++)
                                {
                                    if ( l1_table_base[level1] & PTE_P )
                                    {
                                        page_free(PHYS_TO_PAGE((l1_table_base[level1] & MASK_PAGE_PERM)));
                                    }
                                }
                                page_free(PHYS_TO_PAGE((l2_table_base[level2] & MASK_PAGE_PERM)));
                            }
                        }
                        page_free(PHYS_TO_PAGE((l3_table_base[level3] & MASK_PAGE_PERM)));
                    }
                }
                page_free(PHYS_TO_PAGE((l4_table_base[level4] & MASK_PAGE_PERM )));
            }
        }
        page_free(PHYS_TO_PAGE((uint64_t)proc->page_table_base));
    }
    //printf("After freeing page table pages: %x\n", num_of_free_pages());
    return;
}

uint64_t* page_alloc()
{
    if(free_phys_pages==NULL)
    {
        printf("Ran out of physical pages\n");
        return NULL;
    }
    
    Pages* free_page = free_phys_pages;

    free_phys_pages = free_phys_pages->next_free;

    free_page->next_free = NULL;
    free_page->free = 0; 
    free_page->ref++;
    
    memset(PAGE_DESC_ADDR_TO_VA(free_page), 0, PGSIZE);

    total_free_pages--;

    return (uint64_t*)PAGE_DESC_TO_PHYS(free_page);
}

void delete_mapping(uint64_t *va_page_table_base, uint64_t va_addr)
{
    uint64_t *p_pte = page_lookup(va_page_table_base, va_addr);
    if (p_pte)
    {
        Pages *page = PHYS_TO_PAGE(*p_pte);
        if (page)
        {
            page_free(page);
        }
        else
        {
            #ifdef LOG
                printf("Ouch!\n");
            #endif
        }
        *p_pte = 0;
        invalidate_tlb(va_addr);
    }
}

void free_pages (uint64_t phys_page_table_base, uint64_t addr, uint32_t count )
{
    //int32_t rc = 0;
    //uint64_t phys_addr;
    while ( count-- )
    {
        delete_mapping(PHYS_TO_VA(phys_page_table_base), addr);
        /*
            rc = va_to_phys(PHYS_TO_VA(phys_page_table_base), 
                            addr,
                            &phys_addr);
            if (rc < 0)
            {
                #ifdef LOG
                    printf("Failed to free %p", addr);
                #endif
                return;
            }
            invalidate_tlb(addr);
            page_free(PHYS_TO_PAGE(phys_addr));
        */
        addr += PGSIZE;
    }
}

void page_free(Pages *page)
{
    page->ref -= 1; 
    if (page->ref == 0)
    {
        if (free_phys_pages == NULL )
        {
            free_phys_pages = page;
        }
        else
        {
            page->next_free = free_phys_pages;
            free_phys_pages = page;
        }
        total_free_pages++;
    }
}

int memory_init()
{
    int i;

    for (i = 0; i < phys_mem_map.count; i++)
    {
        total_available_pages += ROUNDUP(phys_mem_map.ranges[i].len, PGSIZE, uint64_t)/PGSIZE;
    }

    phys_pages_init();
    //printf("Number of free pages: %d\n", num_of_free_pages());
    phys_kernel_level4 = (uint64_t)page_alloc(); 
    if ( phys_kernel_level4 == 0 )
    {
        return -ERR_NOMEM;
    }
    
    map_range_va2pa(PHYS_TO_VA(phys_kernel_level4),
                    kern_virtual_start + (uint64_t)_physbase,
                    kern_virtual_end,
                    (uint64_t)_physbase,
                    PTE_P | PTE_W);

    //Map pages for PA to VA 1:1 mapping
    map_range_va2pa(PHYS_TO_VA(phys_kernel_level4),
                    pa_va_mapping_start,
                    pa_va_mapping_end,
                    0,
                    PTE_P | PTE_W);
    
    // testKernelMapping(PHYS_TO_VA(phys_kernel_level4),
                       // pa_va_mapping_start, 
                       // pa_va_mapping_end,
                       // 0);
    //Map a COW page
    uint64_t phys_cow_page=(uint64_t)page_alloc();
    if ( phys_cow_page == 0 )
    {
        return -ERR_NOMEM;
    }
    map_range_va2pa(PHYS_TO_VA(phys_kernel_level4),
                    KERNEL_COW_START,
                    KERNEL_COW_START + PGSIZE,
                    phys_cow_page,
                    PTE_P | PTE_W);
    
    setcr3(phys_kernel_level4);
    
    return 0;
}

int32_t map_range_va2pa(uint64_t* level4, 
                        uint64_t va_start,
                        uint64_t va_end,
                        uint64_t pa_start,
                        uint16_t perm)
{
    int32_t    ret          = 0;
    va_start                = ROUNDDOWN(va_start, PGSIZE, uint64_t);
    va_end                  = ROUNDUP(va_end, PGSIZE, uint64_t);
    pa_start                = ROUNDDOWN(pa_start, PGSIZE, uint64_t);
   
    for(; va_start<va_end;va_start+=PGSIZE,pa_start+=PGSIZE)
    {
        ret = level4create(level4, va_start, pa_start, perm);
        if( ret < 0)
        {
            #ifdef LOG
                printf("Unable to map page at VA %p to PA %p\n", va_start, pa_start); 
            #endif
            return ret;
        }
    }
    return ret;
}
void testKernelMapping(uint64_t* level4,uint64_t va_start,uint64_t va_end,uint64_t pa_start)
{
//uint64_t va_start=kern_virtual_start + (uint64_t)_physbase;
//uint64_t va_end=kern_virtual_end;
//uint64_t pa_start=(uint64_t)_physbase;
uint64_t* pa_lookup;
for(; va_start<va_end;va_start+=PGSIZE,pa_start+=PGSIZE)
    {
        pa_lookup = page_lookup(level4, va_start);
        if( ((*pa_lookup)& MASK_PAGE_PERM) != pa_start)
        {
            #ifdef LOG
                printf("Wrong page for VA=%p Phys=%p got phys=%p\n",
                        va_start,pa_start,*pa_lookup );
            #endif
        }
       else
        {
            #ifdef LOG
                printf("Right page for VA=%p Phys=%p got phys=%p\n",
                        va_start,pa_start,*pa_lookup );
            #endif
        }
    }

}

int32_t va_to_phys(uint64_t *va_page_table_base,
                   uint64_t va_addr,
                   uint64_t *phys_addr)
{
    int32_t rc = -1;
    uint64_t *p_pte;

    if (!phys_addr || !va_page_table_base)
        return -1;

    if (pa_va_mapping_start <= va_addr &&
        va_addr <= pa_va_mapping_end)
    {
        *phys_addr = (va_addr - pa_va_mapping_start);
        rc = 0;
    }
    else
    {
        // We cannot use subtraction logic since va_addr doesn't fall in 1:1 mapping 
        // ranges. So, let's try and use page_lookup
        p_pte = page_lookup(va_page_table_base, va_addr);
        if (p_pte && (*p_pte & PTE_P))
        {
            *phys_addr = (*p_pte & MASK_PAGE_PERM);
            rc = 0;
        }
    }
    return rc;
}

uint64_t get_phys(uint64_t *va_page_table_base,
                   uint64_t va_addr)
{
    uint64_t *p_pte;

    if (!va_page_table_base)
        return 0;

    if (pa_va_mapping_start <= va_addr &&
        va_addr <= pa_va_mapping_end)
    {
        return (va_addr - pa_va_mapping_start);
    }
    else
    {
        // We cannot use subtraction logic since va_addr doesn't fall in 1:1 mapping 
        // ranges. So, let's try and use page_lookup
        p_pte = page_lookup(va_page_table_base, va_addr);
        if (p_pte && (*p_pte & PTE_P))
        {
            return  (*p_pte & MASK_PAGE_PERM);
        }
    }
    return 0;
}

uint64_t* page_lookup(uint64_t *page_table_base, uint64_t addr)
{
    uint64_t indexL4, indexL3, indexL2, indexL1;
    uint64_t *base_pointer;
    
    indexL4 = FOURTH_LEVEL_TABLE_INDEX(addr);
    if(((uint64_t)(base_pointer = (uint64_t *)page_table_base[indexL4]) & PTE_P) == 0)
        return NULL;

    base_pointer = (uint64_t*)PHYS_TO_VA((uint64_t)base_pointer & MASK_PAGE_PERM);
    indexL3 = THIRD_LEVEL_TABLE_INDEX(addr);
    if(((uint64_t)(base_pointer = (uint64_t *)base_pointer[indexL3]) & PTE_P) == 0)
        return NULL;

    base_pointer = (uint64_t*)PHYS_TO_VA((uint64_t)base_pointer & MASK_PAGE_PERM);
    indexL2 = SECOND_LEVEL_TABLE_INDEX(addr);
    if(((uint64_t)(base_pointer = (uint64_t *)base_pointer[indexL2]) & PTE_P) == 0)
        return NULL;
        
    base_pointer = (uint64_t*)PHYS_TO_VA((uint64_t)base_pointer & MASK_PAGE_PERM);
    indexL1 = FIRST_LEVEL_TABLE_INDEX(addr);
    if((base_pointer[indexL1] & PTE_P) == 0)
        return NULL;
        
    return &base_pointer[indexL1];
}

int32_t level4create(uint64_t *page_table_base,uint64_t addr, uint64_t pa_addr, uint16_t perm)
{
    uint64_t ret = 0;
    uint64_t indexL4 = FOURTH_LEVEL_TABLE_INDEX(addr);
    uint8_t bCreated = 0;
    //Entry not present
    if((page_table_base[indexL4] & PTE_P)==0)
    {
            //Creates an page entry for 3rd level page table
            page_table_base[indexL4] = (uint64_t)page_alloc();
            bCreated = 1;
            if(page_table_base[indexL4] == 0)
            {
                #ifdef LOG
                    printf("Error while allotting page table for 3rd level page table \n");
                #endif
                return -ERR_NOMEM;
            }
     }
    
    //Retrieve page_table_entry from level3 walk
    ret = level3create(PHYS_TO_VA(page_table_base[indexL4] & MASK_PAGE_PERM), addr, pa_addr, perm);
    if (ret < 0)
    {
        if (bCreated)
        {
            page_free(PHYS_TO_PAGE(page_table_base[indexL4] & MASK_PAGE_PERM));
            page_table_base[indexL4] = 0;
        }
        return -1;
    }

    //Allocate permission bit for this particular entry
    page_table_base[indexL4] = page_table_base[indexL4] | (perm & 0xFFF);
    return ret;
}

int32_t level3create(uint64_t *page_table_base, uint64_t addr, uint64_t pa_addr, uint16_t perm)
{
    uint64_t ret = 0;
    uint64_t indexL3=THIRD_LEVEL_TABLE_INDEX(addr);
    uint8_t bCreated = 0;
    //Entry not present
    if((page_table_base[indexL3] & PTE_P)==0)
    {
            //Creates an page entry for 3rd level page table
            page_table_base[indexL3] = (uint64_t)page_alloc();
            bCreated = 1;
            if(page_table_base[indexL3] == 0)
            {
                #ifdef LOG 
                    printf("Error while allocating page table for 3rd level page table \n");
                #endif
                return -ERR_NOMEM;
            }
    }
    
    //Retrieve page_table_entry from level3 walk
    ret = level2create(PHYS_TO_VA(page_table_base[indexL3] & MASK_PAGE_PERM), addr, pa_addr, perm);
    if (ret < 0)
    {
        if (bCreated)
        {
            page_free(PHYS_TO_PAGE(page_table_base[indexL3] & MASK_PAGE_PERM));
            page_table_base[indexL3] = 0;
        }
        return -1;
    }

    //Allocate permission bit for this particular entry
    page_table_base[indexL3] = page_table_base[indexL3] | (perm & 0xFFF);
    return ret;
}

int32_t level2create(uint64_t *page_table_base, uint64_t addr, uint64_t pa_addr, uint16_t perm)
{
    uint64_t ret = 0;
    uint64_t indexL2=SECOND_LEVEL_TABLE_INDEX(addr);
    uint8_t bCreated = 0;
    //Entry not present
    if((page_table_base[indexL2] & PTE_P)==0)
    {
            //Creates an page entry for 1st level page table
            page_table_base[indexL2] = (uint64_t)page_alloc();
            bCreated = 1;
            if(page_table_base[indexL2] == 0)
            {
                #ifdef LOG 
                    printf("Error while allocating page table for 2nd level page table \n");
                #endif
                return -ERR_NOMEM;
            }
    }
    //Retrieve page_table_entry from level1 walk
    ret = level1create(PHYS_TO_VA(page_table_base[indexL2] & MASK_PAGE_PERM), addr, pa_addr, perm);
    if (ret < 0)
    {
        if (bCreated)
        {
            page_free(PHYS_TO_PAGE(page_table_base[indexL2] & MASK_PAGE_PERM));
            page_table_base[indexL2] = 0;
        }
        return -1;
    }

    //Allocate permission bit for this particular entry
    page_table_base[indexL2] = page_table_base[indexL2] | (perm & 0xFFF);
    return ret;
}

int32_t level1create(uint64_t *page_table_base, uint64_t addr, uint64_t pa_addr, uint16_t perm)
{
    uint64_t indexL1 = FIRST_LEVEL_TABLE_INDEX((uint64_t)addr);
    page_table_base[indexL1] = ((uint64_t)pa_addr & MASK_PAGE_PERM) | (perm & 0xFFF);
    return 0;
}

int allocate_memory(uint64_t phys_page_table_base,
                    uint64_t va_addr,
                    size_t size,
                    uint64_t perm)
{
    int ret_code = 0;
    uint64_t* page_pa = NULL;
    uint64_t increment = 0;
    
    va_addr = ROUNDDOWN(va_addr, PGSIZE, uint64_t);
    size = ROUNDUP(size, PGSIZE, uint64_t);
    perm = perm & 0xFFF;

    for (increment = 0; increment < size; increment += PGSIZE)
    {
        page_pa = page_alloc();
        if (page_pa == NULL)
        {
            #ifdef LOG
                printf("Ran out of memory\n");
            #endif
            
            return -ERR_NOMEM;
        }

        ret_code = map_range_va2pa(PHYS_TO_VA(phys_page_table_base),
                                   va_addr + increment,
                                   va_addr + increment + PGSIZE,
                                   (uint64_t)page_pa,
                                   perm);

        if (ret_code < 0)
        {
            #ifdef LOG
                printf("Failed to map new page\n");
            #endif
            page_free(PHYS_TO_PAGE((uint64_t)page_pa));
            return ret_code;
        }
        invalidate_tlb(va_addr + increment);
    }
    return ret_code;
}

int invalidate_tlb(uint64_t va_addr)
{
    __asm__ __volatile__("invlpg (%0)\n"
                         :
                         :"r"(va_addr)
                         :"cc", "memory");
    return 0;
}

int32_t user_vma_update(uint64_t fault_addr, uint64_t req_perm, uint64_t rsp)
{
    int32_t ret_code = 0;
    process_t *faulting_proc = curr_running_proc;
    vma_t     *vma           = NULL;
    //printf(" faulting addr= %x \n",fault_addr);
    //We don't need to check for heap as it does not grow automatically

    if (positive_diff(fault_addr, rsp) <= PGSIZE &&
        fault_addr < USER_STACK_TOP)
    {
        vma = find_vma_by_flag(faulting_proc, VMA_STACK);
        vma_t *heap_vma = find_vma_by_flag(faulting_proc, VMA_HEAP);
        if (!vma ||
            !heap_vma ||
            (vma->perm & req_perm) != req_perm)
        {
            #ifdef LOG
                printf("DemandPaging: Stack perm issue - perm:0x%x\n", req_perm);
            #endif
            return -ERR_FAULT;
        }

        uint64_t rdown_fault_addr = ROUNDDOWN(fault_addr, PGSIZE, uint64_t);

        if ((heap_vma->vma_start + heap_vma->memsz) >=
            rdown_fault_addr - PGSIZE)
        {
            return -ERR_NOMEM;
        }

        ret_code = increase_vma_size(vma, vma->vma_start - rdown_fault_addr);
        if(ret_code < 0)
        {
            #ifdef LOG
                printf("DemandPaging: Stack increment issue - perm:0x%x\n", req_perm);
            #endif
            return ret_code;
        }
    }
    else // All other cases
    {
        vma = find_vma_by_vaddr( faulting_proc, fault_addr);
        //Check appropriate permissions for vma
        if(!vma ||
           (vma->perm & req_perm) != req_perm)
        {
            #ifdef LOG
                printf("DemandPaging: perm issue - perm:0x%x\n", req_perm);
            #endif
            return -ERR_FAULT;
        }
    }

    if( vma == NULL)
    {
        #ifdef LOG
            printf("Address out of VMA: %p\n", fault_addr);
        #endif
        return -ERR_FAULT;
    }

    return ret_code;
}

int32_t user_demand_paging_handler(uint64_t fault_addr)
{
    int32_t ret_code = 0;
    vma_t *vma = NULL;
    uint64_t* faulting_pte = NULL;
    uint64_t perm = PTE_P;

    uint64_t vma_read_start;
    uint64_t vma_read_end;
    uint64_t file_read_start;

    vma = find_vma_by_vaddr(curr_running_proc, fault_addr);
    if (!vma)
    {
        #ifdef LOG
            printf("Failed to get vma for addr:%p. Theoretically we should NEVER REACH HERE\n", fault_addr);
        #endif
        return -ERR_FAULT;
    }
    
    perm = PTE_U | PTE_P | PTE_W;

    ret_code = allocate_memory((uint64_t)curr_running_proc->page_table_base,
                               fault_addr,
                               PGSIZE,
                               perm);
    if (ret_code < 0)
    {
        #ifdef LOG
            printf("Failed to map new page\n");
        #endif
        return -ERR_NOMEM;
    }

    // why "fault_addr < (vma->vma_start + vma->filesz)"
    //  -- when we hot a fault in BSS then read_end may go beyond read_start
    if (vma->file_ptr != 0 &&
        fault_addr < (vma->vma_start + vma->filesz))
    {
        vma_read_start = max(vma->vma_start, 
                             ROUNDDOWN(fault_addr, PGSIZE, uint64_t));
        
        // filesz will always be smaller than memsz, so just check that only
        // why ROUNDUP(fault_addr + 1, ...)?
        //    Consider when fault_addr is already page aligned and 
        //    greater than vma->vma_start
        //    In such cases vma_read_start = vma_read_end
        vma_read_end   = min((vma->vma_start + vma->filesz),
                              ROUNDUP(fault_addr + 1, PGSIZE, uint64_t));

        file_read_start = vma->file_ptr + (vma_read_start - vma->vma_start);

        // we don't need to zero memsz-filesz space since we are already zeroing 
        // whole page while allocating it
        memcpy((void *)vma_read_start,
                (void *)file_read_start,
                vma_read_end - vma_read_start);
    }

    // We have put WRITE permission for copying contents to that page
    // Now remove that permission if it's really not there
    if ((vma->perm & VMA_PERM_WRITE) == 0)
    {
        faulting_pte = page_lookup(PHYS_TO_VA(curr_running_proc->page_table_base)
                       , fault_addr);
        *faulting_pte = (((uint64_t)*faulting_pte & MASK_PAGE_PERM) |
                        (( PTE_U | PTE_P) & 0xFFF));
        invalidate_tlb(fault_addr);
    }

    return ret_code;
}

int32_t demand_paging_on_syscall(uint64_t fault_addr, uint64_t perm)
{
    int32_t ret_code = 0;
    uint64_t req_vma_perm = VMA_PERM_READ;
    if (perm & PTE_W)
    {
        req_vma_perm |= VMA_PERM_WRITE;
    }

    ret_code = user_vma_update( fault_addr,
                                req_vma_perm,
                                curr_running_proc->registers.rsp);
    if (ret_code < 0)
    {
        #ifdef LOG
            printf("demand_paging_on_syscall Failed for %p\n", fault_addr);
        #endif

        if (ret_code == -ERR_NOMEM)
        {
            printf("Killing process [%d]:%s, since no page for demand paging\n", 
                    curr_running_proc->pid, curr_running_proc->name);
            kill_process(curr_running_proc);
        }

        return ret_code;
    }

    //printf("demand paging %p\n", fault_addr);
    ret_code = user_demand_paging_handler(fault_addr);
    if (ret_code == -ERR_NOMEM)
    {
        printf("Killing process [%d]:%s, since no page for demand paging\n",
                curr_running_proc->pid, curr_running_proc->name);
        kill_process(curr_running_proc);
    }
    return ret_code;
}

int32_t validate_user_buffer(uint64_t start_addr, size_t len, uint64_t perm)
{
    uint64_t *pa_lookup;
    uint64_t end_addr = 0;
    int32_t rc = 0;

    start_addr = ROUNDDOWN(start_addr, PGSIZE, uint64_t);
    end_addr = ROUNDUP((start_addr + len), PGSIZE, uint64_t);

    while (start_addr < end_addr)
    {
        pa_lookup = page_lookup(PHYS_TO_VA(curr_running_proc->page_table_base),
                                start_addr);
        if (!pa_lookup ||
            !(*pa_lookup & PTE_P) ||
            ((perm & PTE_W) && (*pa_lookup & PTE_COW)))
        {
            // This entry is not at all present,
            // lets try to do demand page if possible
            rc = demand_paging_on_syscall(start_addr, perm);
            if (rc < 0)
            {
                return rc;
            }
        }
        else if ((*pa_lookup & perm) != perm)
        {
            // Entry is not present with sufficient permissions
            return -1;
        }

        start_addr += PGSIZE;
    }

    return rc;
}


int verify_issue_page_perm(uint64_t *page_table_base, uint64_t addr, uint64_t perm)
{
    uint64_t indexL4, indexL3, indexL2, indexL1;
    uint64_t *base_pointer;
    
    indexL4 = FOURTH_LEVEL_TABLE_INDEX(addr);
    if(((uint64_t)(base_pointer = (uint64_t *)page_table_base[indexL4]) & PTE_P) == 0 ||
        ((uint64_t)base_pointer & perm ) != perm)
        return 4;

    base_pointer = (uint64_t*)PHYS_TO_VA((uint64_t)base_pointer & MASK_PAGE_PERM);
    indexL3 = THIRD_LEVEL_TABLE_INDEX(addr);
    if(((uint64_t)(base_pointer = (uint64_t *)base_pointer[indexL3]) & PTE_P) == 0 ||
       ((uint64_t)base_pointer & perm ) != perm)
        return 3;

    base_pointer = (uint64_t*)PHYS_TO_VA((uint64_t)base_pointer & MASK_PAGE_PERM);
    indexL2 = SECOND_LEVEL_TABLE_INDEX(addr);
    if(((uint64_t)(base_pointer = (uint64_t *)base_pointer[indexL2]) & PTE_P) == 0 ||
       ((uint64_t)base_pointer & perm ) != perm)
        return 2;
        
    base_pointer = (uint64_t*)PHYS_TO_VA((uint64_t)base_pointer & MASK_PAGE_PERM);
    indexL1 = FIRST_LEVEL_TABLE_INDEX(addr);
    if((base_pointer[indexL1] & PTE_P) == 0 ||
       (base_pointer[indexL1] & perm ) != perm)
        return 1;
        
    return 0;
}
