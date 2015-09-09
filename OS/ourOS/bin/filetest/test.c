#include<stdio.h>
#include<stdlib.h>

int main (int argc, char *argv[], char *envp[])
{
    int fd,bytes_read;
    char* buf=malloc(MAX_STRING);
  //  if(argc<=1 || argv[1]==NULL)
  // {
   //     printf("Please enter the file name properly \n");
  // }
    
    fd = open("x.txt",O_RDONLY);
    if(fd < 0)
    {
        print_errno(errno,"x.txt");
        return -1;
    }
    while((bytes_read=read(fd,buf,20))>0)
    {
       printf("%s",buf);
       lseek(fd,250,SEEK_END);
    }
   // lseek(fd,250,SEEK_SET);
  //  printf("\n \n Offset changed\n");
    while((bytes_read=read(fd,buf,100))>0)
    {
       printf("%s",buf);
       lseek(fd,250,SEEK_END);
    }   
    if(bytes_read<0)
    {
        printf("Some error have occured while reading \n");
        print_errno(errno,"read command");

    }
    close(fd);
    return 0;
}

