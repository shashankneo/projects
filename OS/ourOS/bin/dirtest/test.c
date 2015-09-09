#include <stdio.h>
#include <stdlib.h>
int main(int argc, char* argv[], char* envp[]) 
{
	int ret;
	char * bufcwd;
	void *readp;
	struct dirent *dent = NULL;
	//Null test
	ret=chdir(NULL);
	if(ret<0)
	{
		printf("Error while changing directory \n");
		print_errno(errno, "cd");
	}

	//wrong address
	ret=chdir("/dsdw/dasda");
	if(ret<0)
	{
		printf("Error while changing directory \n");
		print_errno(errno, "cd");
	}

	//give correct directory as address
	ret=chdir("/home/stufs1/shajain");
	if(ret<0)
	{
		printf("Error while changing directory \n");
		print_errno(errno, "cd");
	}

	//current working directory with buff n size as nada
	bufcwd=getcwd(NULL,0);
	if(bufcwd==NULL)
	{
		printf("Error while getting current working directory \n");
		print_errno(errno, "pwd");
	}
	else
	{
		printf("Current working directory is %s \n",bufcwd);
	}

	//give file not present error
	ret=chdir("/home/stufs1/shajain/x");
	if(ret<0)
	{
		printf("Error while changing directory \n");
		print_errno(errno, "cd");
	}

	//current working directory with buff as nada n size as not sufficient
	bufcwd=getcwd(NULL,4);
	if(bufcwd==NULL)
	{
		printf("Error while getting current working directory \n");
		print_errno(errno, "pwd");
	}
	else
	{
		printf("Current working directory is %s \n",bufcwd);
	}

	//Now time for dir open/read/write

	//Open some known directory but very large data
	readp=opendir("/home/stufs1");
	if(readp==NULL)
	{
		printf("Error while getting opening directory \n");
		print_errno(errno, "ls");
	}
	else
	{
		printf("Why the eff \n");
	}
	while ((dent =readdir(readp)) != NULL)
	{
		 printf("%s\n", dent->d_name);
	}
	closedir(readp);

}