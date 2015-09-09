#ifndef __MALLOC__H__
#define __MALLOC__H__

/* Align addresses at multiples of 16. Why? Cos Ubuntu 64-bit does it ;)
 * My doubt is we need to align things to multiples of 8, as on a 64-bit 
 * machine, max size of a basic data type is 8
 */
#define TOTAL_SIZE(x) (x_ceil(x + sizeof(free_node), 16) )

#define MINIMUM_MALLOC_SIZE TOTAL_SIZE(1)

typedef struct free_node
{
    unsigned int size;
    int state;
    struct free_node *next;
} free_node;

free_node *freep;

enum state
{
    FREE =0,
    IN_USE = 1
};

size_t x_ceil ( unsigned long num, unsigned long page_size );
void
merge_blocks (free_node *first, free_node* second);
void
add_to_free_list(free_node **head, free_node *ptr, free_node **prev);
void
free ( void *ptr);
free_node *
get_system_memory( size_t size );
free_node *
split_block ( free_node *freep, unsigned long total_size, unsigned long blk_size);
void *
allocate_free_block ( size_t size);
void *
malloc( size_t size );

#endif

