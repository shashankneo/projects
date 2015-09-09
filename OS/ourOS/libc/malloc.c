#include<stdio.h>
#include<stdlib.h>
#include<malloc.h>
#include<error.h>

size_t x_ceil ( unsigned long num, unsigned long page_size )
{
    return num + (page_size-(num % page_size)) % page_size;
}

void
merge_blocks (free_node *first, free_node* second)
{
    first->size += second->size;
    first->next = second->next;
}

free_node *
find_prev_node (free_node *ptr)
{
    free_node *curr = ptr;
    while ( ptr->next != curr)
    {
        ptr = ptr->next;
    }
    return ptr;
}
void
add_to_free_list(free_node **head, free_node *ptr, free_node **prev)
{
    free_node *traverse = *head;
    
    /* Handle the two corner cases */
    if ( *head == NULL )
    {
        ptr->next = ptr;
        *head = ptr;
        if ( prev != NULL && *prev != NULL)
        {
            *prev = *head;
        }
        return;
    }
    if ( *head > ptr)
    {
        ptr->next = *head;
        while (traverse->next != *head)
        {
            traverse = traverse->next;
        }
        /* Point last node to new node */
        traverse->next = ptr;
        /* Store previous pointer. We need it to merge */
        if ( prev != NULL && *prev != NULL)
        {
            *prev = traverse;
        }
        *head = ptr;
        return;
    }
    
    while (traverse->next != *head)
    {
        if ( traverse < ptr )
        {
            traverse = traverse->next;
        }
        else
        {
            break;
        }
    }
    /* We've found out the location where the new node gets inserted */
    ptr->next = traverse->next;
    traverse->next = ptr;
    if (prev != NULL && *prev != NULL)
    {
        *prev = traverse;
    }
    
    return;
}

void
free ( void *ptr)
{
    free_node *free_ptr = (free_node *) ptr;
    free_node *prev = NULL;

    if (ptr == NULL)
        return;

    /* Go back so that we can access the free_node structure */
    free_ptr = (free_node *)((unsigned long )free_ptr - TOTAL_SIZE(0));
    
    /* Check for double free */
    if (free_ptr->state == FREE)
    {
        print_error("Double Free or Corruption detected\n");
        exit(-1);
    }
    else
    {
        /* Mark node as free */
        free_ptr->state = FREE;
    }

    /* Find the prev block of the currently freed block. We need to merge it 
     * with current block if its found to be free
     */
    prev = find_prev_node(free_ptr);
    /* If the block on the right of the current block is free, merge it with 
     * current one
     */
    if ( (free_ptr->next->state == FREE) )
    {
        /* Merge the blocks only if they are adjacent */
        if ( (unsigned long)free_ptr + free_ptr->size == (unsigned long)free_ptr->next)
        {
            merge_blocks(free_ptr, free_ptr->next);
        }
    }
    /* If the block on the left of the current block is free, merge it with 
     * current one
     */
    if ( (prev->state == FREE) )
    {
        /* Merge the blocks only if they are adjacent */
        if ( (unsigned long)prev + prev->size == (unsigned long)free_ptr)
        {
            merge_blocks(prev, free_ptr);
        }
    }
    return;
}

free_node *
get_system_memory( size_t size )
{
    int rc = 0;
    size_t total_size = 0;
    free_node *free_block  = NULL;

    /* Get memory in terms of page size */
    total_size = x_ceil( TOTAL_SIZE(size), 4096 );
    free_block = sbrk(0);

    if ( free_block == (void *) -1 )
    {
        return NULL;
    }

    rc = brk((char*)free_block + total_size);
    if ( rc == -1 )
    {
        return NULL;
    }
    free_block->size = total_size;
    free_block->state = FREE;
    free_block->next = free_block;
    
    return free_block;
}

free_node *
split_block ( free_node *freep, unsigned long total_size, unsigned long blk_size)
{
    free_node *free_block =(struct free_node *)((char*)freep + total_size);
    free_block->next = freep->next;
    free_block->size = blk_size;
    free_block->state = FREE;
    
    return free_block;
}

void *
allocate_free_block ( size_t size)
{
    struct free_node *ptr  = freep;

    struct free_node *free_block = NULL;
    struct free_node *alloc_block = NULL;
    size_t           remaining = 0;

    do
    {
        if (ptr &&
            ((ptr->state == FREE) && (ptr->size  >= TOTAL_SIZE(size))))
        {
            remaining = ptr->size - TOTAL_SIZE(size);
            if (remaining > MINIMUM_MALLOC_SIZE)
            {
                /* If there is remaining free space, create a new free block
                 * out of it in the doubly linked list
                 */
                free_block = split_block(ptr, TOTAL_SIZE(size), remaining);
                ptr->next = free_block;
                ptr->size = TOTAL_SIZE(size);
            }
            /* remove the allocated block from the free list */
            alloc_block = ptr;
            break;
        }
        ptr = ptr->next;
    } while (ptr != freep);

    /* None of the free blocks that we have can satisfy the given request.
     * Ask for more memory from system
     */
    if ( alloc_block == NULL)
    {
        alloc_block = get_system_memory(size);
        if ( alloc_block == NULL)
        {
            return NULL;
        }
        add_to_free_list(&freep, alloc_block, NULL);

        remaining = alloc_block->size - TOTAL_SIZE(size);
        if (remaining > MINIMUM_MALLOC_SIZE)
        {
            /* If there is remaining free space, create a new free block
             * out of it in the linked list
             */
            free_block = split_block(alloc_block, TOTAL_SIZE(size), remaining);

            alloc_block->next = free_block;
            alloc_block->size = TOTAL_SIZE(size);
        }
    }
    alloc_block->state = IN_USE;

    return (void*)TOTAL_SIZE(((unsigned long)alloc_block)) ;
}
void *
malloc( size_t size )
{
    void *ptr = NULL;
    if ( size == 0 )
    {
        return NULL;
    }
    if ( freep == NULL )
    {
        /* This code executes only when we (malloc()) don't have 
         * any memory at all
         */
        freep = get_system_memory(size);
        if (freep == NULL)
        {
            errno = ERR_NOMEM;
            return NULL;
        }
    }

    ptr = allocate_free_block(size );
    if (ptr == NULL)
        errno = ERR_NOMEM;
    return ptr;
}
