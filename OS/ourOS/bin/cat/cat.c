#include<stdio.h>
#include<stdlib.h>
#define CAT_STRING 100
int main (int argc, char *argv[], char *envp[])
{
    int fd,bytes_read;
    char* buf=malloc(CAT_STRING);
    int file = 0;
    
    //More than 1 file arguments not supported
    if(argc>2)
    {
        printf("Invalid number of arguments for cat. Only 1 file is supported \n");
        return 0;
    }
    if(argc == 2)
    {
        if(argv[1]!=NULL)
        {
            file = 1;
        }
        else
        {
            printf("Enter the argument for cat properly \n");
        }
    }
  
    if(file==1)
    {
        fd = open(argv[1],O_RDONLY);
        if( fd < 0)
        {
            print_errno(errno,"cat");
            return -1;
        }
    }
    else
    {
        fd = STD_IN;
    }
    while((bytes_read=read(fd,buf,CAT_STRING))>0)
    {
       write(STD_OUT, buf, bytes_read);
    }
    if(bytes_read<0)
    {
        printf("Some error have occurred while reading \n");
        print_errno(errno, "cat");
    }
    close(fd);
    return 0;
}

