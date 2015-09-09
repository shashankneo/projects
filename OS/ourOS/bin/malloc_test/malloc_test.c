#include <stdio.h>
#include <stdlib.h>
#include <our_string.h>

//~14 TB difference between heap and stack

#define GB (size_t)1024*1024*1024*1024
#define MAX_HEAP 0xFFFFFFEE000

void test1()
{
    int* arr = malloc(GB);
    size_t count =0;
    int page =0;
    while(count<GB)
    {
        *(arr+count) = page;
        printf(" For page %d we have its copy %d \n", page,  *(arr+count));
        count = count + 4096;
        page = page + 1;
    }
}

void rec(int i)
{
    char str[4096*4096] = {0};
    strncpy(str, "In rec", 10);
    printf("%s %d\n", str, i);
    rec(i++);
}

// check
void test2()
{
    int* arr = malloc(GB);
    arr[0] = 10;
    rec(1);
}

int main(int argc, char* argv[], char* envp[])
{
    test1();
}