#ifndef __KMALLOC__H__
#define __KMALLOC__H__

#define MAX_BLOCK_COUNT 16384

typedef struct free_blocks
{
    uint16_t slab_size;
    uint64_t start_addr;
    uint64_t count;
    int64_t free_index;
    uint64_t free_list[MAX_BLOCK_COUNT/sizeof(uint64_t)]; /* If a bit is 0, it is free */
} free_blocks_t;

typedef struct slabs
{ 
    uint64_t slab_size;
    uint64_t count;
}slab_size_t;

void *
kmalloc (size_t size);

void kfree(const void *ptr);

void kmalloc_init();

void find_next_free_slot( free_blocks_t *slabs);

uint32_t find_slab_index ( uint64_t kptr, uint64_t start_addr, uint64_t size );

int are_slabs_free(uint64_t *free_list, uint32_t starting_index, uint32_t count);

void free_slab (free_blocks_t *slab, uint64_t addr, uint32_t index);

void allocate_slab (free_blocks_t *slab, int index);

#endif /*  #ifndef __KMALLOC__H__ */
