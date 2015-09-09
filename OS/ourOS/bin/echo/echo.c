#include <stdio.h>
#include <stdlib.h>
#include <error.h>

#include <our_string.h>

#define MAX_SHELL_STRING 512

char string_global[1024];
char temp_global[1024];
char env_value[256];

/* This function searches for given ENV variable in the envp array and 
 * populates the env array. Returns the index of the ENV
 * variable found
 */
int  
extract_env_variable(char *envp[], char *search, char *env)
{
    int i =0;
    
    for ( i = 0; envp[i]; i++ )
    {
        //printf("%s: envp[%d] = %s. Search=%s\n", __func__, i, envp[i], search);
        if (!strncmp(envp[i], search, strlen(search)))
        {
            strncpy(env, (&envp[i][0] + strlen(search)), MAX_SHELL_STRING);
            return i;
        }
    }
    return -1;
}

int
get_var(char *src, char *dst )
{
    int count =0;
    while ( *src && *src != ' ' )
    {
        count++;
        *dst++ = *src++;
    }
    *dst = 0;
    if ( count == 0 )
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

int 
process_dollar(char *string, char *envp[])
{
    char var[256] = {0};
    char *tmp = temp_global;
    int rc = 0;
    for ( int i =0; *string ; i++ )
    {
        var[0] = 0;
        if ( *string == '$' )
        {
            rc = get_var(string+1, var);
            if ( rc < 0 )
            {
                /* $ is standing on its own. Isnt followed by any variable */
                *tmp++ = *string++;
            }
            else
            {
                strncat(var, "=", 1); /* Ugly hack */
                
                /* Search for the given variable in the env variable list */
                rc = extract_env_variable(envp, var, env_value);
                var[strlen(var)-1] = 0;
                if ( rc < 0 )
                {
                    printf("Undefined variable:%s\n", var);
                    return rc;
                }
                //printf("env_value = %s\n", env_value);
                strncat(tmp, env_value, strlen(env_value));
                tmp += strlen(env_value);
                string += strlen(var)+ 1; // +1 to accomodate the $
            }
        }
        else
        {
            *tmp++ = *string++;
        }
    }
    //printf(temp_global);
    strncpy(string_global, temp_global, 1024);
    return 0;
}
int process_special_chars(char *argv[], char *envp[], char *string)
{
    int rc = 0;
    for ( int i=1; argv[i]; i++ )
    {
        strncat(string, argv[i], strlen(argv[i])+1);
        if ( argv[i+1] != NULL )
        {
            strncat(string, " ", 1);
        }
    }
    rc = process_dollar(string, envp);
    
    return rc;
}
int main(int argc, char *argv[], char *envp[])
{
    int rc = 0;
    #if 0
        for ( int i =1; i < argc; i++ )
        {
            printf("%s", argv[i]);
            if ( i+1 != argc )
            {
                printf(" ");
            }
        }
        printf("\n");
    #else
        rc = process_special_chars(argv, envp, string_global);
        
        if ( rc == 0 )
        {
            printf("%s\n", string_global);
        }
    #endif
    
    return 0;
}