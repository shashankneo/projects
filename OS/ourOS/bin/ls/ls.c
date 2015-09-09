#include<stdio.h>
#include<stdlib.h>
#include<our_string.h>

int main (int argc, char *argv[], char *envp[])
{
    char cwd[256]={0};
    if (argc == 1 )
    {
        getcwd(cwd, sizeof(cwd));
    }
    else
    {
        if ( argv[1][0] == '-' )
        {
            printf("No options supported with this command\n");
            printf("Usage: ls\n");
            printf("       ls FOLDER_PATH\n");
            return -1;
        }
        strncpy(cwd, argv[1], sizeof(cwd));
    }

    void *readp = opendir(cwd);
    struct dirent *dent = NULL;
    while ((dent =readdir(readp)) != NULL)
    {
        printf("%s\n", dent->d_name);
    }
    closedir(readp);
    return 0;
}

