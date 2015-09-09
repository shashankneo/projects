
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
uint64_t memory_usagefull[6]={4*1024,64*1024,4*1024*1024,64*1024*1024,1024*1024*1024,(uint64_t)4*1024*1024*1024};
char memory_usagefull_names[6][20]={"4Kb","64Kb","4Mb","64Mb","1Gb","4Gb"};
void print_all_blocks();
int main(int argc, char* argv[], char* envp[]) {
    uint64_t* heap_top;
    uint64_t* alloted_mem[6];
    uint64_t* free_mem[10];
    int i=0;
    //Current top of heap
    heap_top=sbrk(0);
    printf("Top of heap=%x \n",heap_top);
    //Try to make memory run out of free memory
    while(1)
    {
        alloted_mem[i]=malloc(memory_usagefull[i]);
        
        if(alloted_mem[i]==NULL)
        {
            printf("Malloc couldn't allocate memory \n");
            break;
        }
        
        printf("Malloc successfully alloted %s at address %x \n",&memory_usagefull_names[i],alloted_mem[i]);
        i++;
        if(i==6)
            break;
    }
   //Test for free that it allocates properly from an empty space in the free list
  
   //Let's free [1] of 64KB size
    printf(" Memory of size 64KB to be freed at %x \n",alloted_mem[1]);
    free(alloted_mem[1]);
   //Now allocate another 4 sizes of 16KB size each
    i=0;
    while(i<4)
    {
        free_mem[i]=malloc(16*1024);
        printf(" New memory of 16KB alloted at %x \n",free_mem[i]);
        i++;
    }
    print_all_blocks();
    //Works great till now
      free_mem[4]=malloc(16*1024);
      free_mem[5]=malloc(32*1024);
      free_mem[6]=malloc(100*1024);
      free(alloted_mem[0]);
      free(alloted_mem[2]);
      free(alloted_mem[3]);
      free(alloted_mem[4]);
   //   free(alloted_mem[5]);
  //  free(free_mem[0]);
   // free(free_mem[1]);
  //  free(free_mem[2]);
      free(free_mem[3]);
      free(free_mem[4]);
      free(free_mem[5]);
      free(free_mem[6]);
 //  alloted_mem[0]=malloc(512*1024);
  // alloted_mem[5]=malloc(1024*1024*1000);
   //free_mem[4]=malloc(1024*1024*1024);
    printf("\n");
     print_all_blocks();
}
void print_all_blocks()
{
    struct free_node *ptr  = freep;
    do
    {
        if(ptr->state==FREE)
        {
            printf("Block of size %d at address %x is free \n",ptr->size,TOTAL_SIZE(((unsigned long)ptr)));
        }
        else
        {
             printf("Block of size %d at address %x is used \n",ptr->size,TOTAL_SIZE(((unsigned long)ptr)));
        }
    ptr=ptr->next;
    }while(ptr!=freep);

}