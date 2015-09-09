#include <stdlib.h>
#include <stdarg.h>
#include <sys/kstring.h>
#include<sys/defs.h>
#include<sys/sbunix.h>
#include<sys/pagetable.h>
#include<sys/kmalloc.h>

/* 64, 128, 512, 4K */
slab_size_t slab_sizes[] = {{64,MAX_BLOCK_COUNT*8}, {128,2*1024}, {512,3*1024}, 
                            {1024, MAX_BLOCK_COUNT}, {4096,MAX_BLOCK_COUNT}, {-1,-1}};

#define MAX_SLABS sizeof(slab_sizes)/sizeof(slab_size_t)

free_blocks_t slabs[MAX_SLABS];

void kmalloc_init()
{
    uint64_t mem_start = ROUNDUP(kern_virtual_end, PGSIZE, uint64_t);

    for ( int i =0; i < MAX_SLABS; i++ )
    {
        slabs[i].slab_size  = slab_sizes[i].slab_size;
        slabs[i].count      = slab_sizes[i].count;
        if ( i == 0 )
        {
            slabs[i].start_addr = mem_start;
        }
        else
        {
            slabs[i].start_addr = slabs[i-1].start_addr + 
                                  slabs[i-1].slab_size*slabs[i-1].count;
            /* Just in case, round the addr up */
            slabs[i].start_addr = ROUNDUP(slabs[i].start_addr,PGSIZE,uint64_t);
        }
        slabs[i].free_index = 0;
    }
    return;
}

void find_next_free_slot( free_blocks_t *slabs)
{
    uint32_t bits = sizeof(uint64_t) * 8;

    for ( int i = 0; i < slabs->count; i++ )
    {
        if ( !(slabs->free_list[i/bits] & (1 << (i %bits) )) )
        {
            /* Free block found. Set the index and return */
            slabs->free_index = i;
            return;
        }
    }
    /* None of the blocks are free in this slab */
    slabs->free_index = -1;
    return;
}
void *
kmalloc (size_t size)
{
    uint64_t *kptr = NULL;
    int32_t   rc   = 0;

    /* Round up the size to the nearest slab size */
    for ( int i = 0; i < MAX_SLABS; i++ )
    {
        if ( size && (size <= slabs[i].slab_size ) )
        {
            size = slabs[i].slab_size;
            if ( slabs[i].free_index != -1 )
            {
                kptr = (uint64_t *)(slabs[i].start_addr +
                       slabs[i].free_index*slabs[i].slab_size); 

                allocate_slab(&slabs[i], slabs[i].free_index);
                find_next_free_slot(&slabs[i]);
                break;
            }
        }
    }
    if (kptr && !page_lookup(PHYS_TO_VA(phys_kernel_level4), (uint64_t)kptr))
    {
        rc = allocate_memory(phys_kernel_level4,
                            (uint64_t)kptr,
                            size,
                            (PTE_P|PTE_W));
    }
    if ( rc < 0 )
    {
        return NULL;
    }
    else
    {
        memset(kptr, 0, size);
        return kptr;
    }
}
uint32_t find_slab_index ( uint64_t kptr, uint64_t start_addr, uint64_t size )
{
    return (kptr-start_addr)/size;
}

int are_slabs_free(uint64_t *free_list, uint32_t starting_index, uint32_t count)
{
    uint32_t bits = sizeof(uint64_t)*8;
    for ( int i = 0; i < count; i++ )
    {
        if ( free_list[(starting_index+i)/bits] & 
            (1 << ((starting_index+i) % bits)) )
        {
            /* If the bit is set, it means that that slab is not free */
            return 0;
        }
    }
    return 1;
}

void
allocate_slab (free_blocks_t *slab, int index)
{
    uint32_t arr_index = index/(sizeof(uint64_t) * 8);
    uint32_t bit   = index % (sizeof(uint64_t) * 8);
    
    slab->free_list[arr_index] |= (1 << bit);
    return;
}

void free_slab (free_blocks_t *slab, uint64_t addr, uint32_t index)
{
    uint64_t *free_list = slab->free_list;
    uint64_t size       = slab->slab_size;
    uint32_t arr_index = index/(sizeof(uint64_t) * 8);
    uint32_t bit   = index % (sizeof(uint64_t) * 8);
    uint64_t round_down_addr = ROUNDDOWN(addr, PGSIZE, uint64_t);
    uint32_t free_indices = 0;
    uint32_t pages_to_free = 0;
    
    free_list[arr_index] &= ~(1 << bit);
    slab->free_index      = index;

    /* Check if we can free the physical memory associated with this block */
    index = index - (addr- round_down_addr )/size;

    if ( size > PGSIZE )
    {
        free_indices = 1;
        pages_to_free  = size/PGSIZE;
    }
    else
    {
        free_indices  = PGSIZE/size;
        pages_to_free = 1;
    }
    /* From index, if we find 'free_indices' slabs free, then we can free 
     * all physical mappings corresponding to those slabs
     */
    if ( are_slabs_free(free_list, index, free_indices) )
    {
        // Since this book keeping will be in kernel, let's  use kernel's pml4e
        // as we have copied plm4e entry into user page-table
        free_pages(phys_kernel_level4, round_down_addr, pages_to_free);
    }
    
    return;

}
void kfree(const void *ptr)
{
    if (!ptr)
        return;

    uint64_t kptr = (uint64_t)ptr;
    uint32_t index;

    /* Find which slab this block belongs to */
    for ( int i = 0; i < MAX_SLABS-1; i++)
    {
        if ( kptr >= slabs[i].start_addr && kptr < slabs[i+1].start_addr )
        {
            index = find_slab_index(kptr, slabs[i].start_addr, slabs[i].slab_size);
            free_slab(&slabs[i], kptr, index);
            return;
        }
    }

    return;
}