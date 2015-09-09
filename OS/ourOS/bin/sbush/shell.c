#include <stdio.h>
#include <stdlib.h>
#include <error.h>

#include <shell.h>
#include <our_string.h>



#define NUM_PROCESS 100

char    cmd_line[MAX_SHELL_STRING];
char    _envp[MAX_SHELL_STRING];
char    path_env[MAX_SHELL_STRING] = "PATH=" DEFAULT_PATH;
char    *path = path_env + 5;
char    ps1_env[MAX_SHELL_STRING] = "PS1=sbush:";
char    *ps1 = ps1_env + 4;
char    home_env[MAX_SHELL_STRING] = "HOME=";
char    *home = home_env + 5;
char    pwd_env[MAX_SHELL_STRING] = "PWD=";
char    *pwd = pwd_env + 4;

int     ps1_changed = 0;

command_t commands[]= 
{
    {"cd", cd},
    {"set", set},
    {"exit", clean_exit}
};

int main(int argc, char *argv[], char *envp[])
{
    unsigned int    count = 0;
    static char     cmd[NUM_CMDS][CMD_LENGTH],
                    args[NUM_CMDS][NUM_ARGS][ARG_LENGTH];
    int             rc = 0;
    int             i = 0,
                    j = 0;
    int             fd = 0;
    int             bytes = 0;
    bool            is_script = argc >=2 ? true: false;

    arr_char_ptr_t arg_ptr;
    static arr_char_ptr_t arg_ptr_arr[NUM_ARGS];

    if ( is_script )
    {
        /* We are being passed a shell script */
        /* We ignore all arguments passed to shell */
        fd = open_script(argv);
        if ( fd < 0)
        {
            /* We are not able to read the script */
            print_errno(errno, argv[1]);
            return fd;
        }
    }

    extract_env_variables(envp);
   
    while (true)
    {
        if ( is_script )
        {
            bytes = read_line(cmd_line, fd);
            if ( bytes == 0 )
            {
                break;
            }
            if ( bytes < 0 )
            {
                #ifdef DEBUG
                printf("errno: %d\n", errno);
                #endif
                break;
            }
                
            if ( is_comment(cmd_line) )
            {
                continue;
            }
        }
        else
        {
            if (ps1_changed)
            {
                // print user specified PS1
                printf("%s", ps1);
            }
            else
            {
                // print custom PS1
                printf("%s%s$ ", ps1, pwd);
            }

            bytes = read_line(cmd_line, 0);
            if (bytes < 0)
            {
                break;
            }
            if (bytes == 0 )
            {
                continue;
            }
        }

        count = parse_cmd_line(cmd, args);

        #ifdef DEBUG
            for (i=0; i < count; i++)
            {
                printf("cmd %d: %s\n", i+1, cmd[i]);
                for ( j =0; args[i][j][0]; j++)
                {
                    printf("\targs %d: %s\n", j+1, args[i][j]);
                }
            }
        #endif

        for (i=0; i < count; i++)
        {
            for ( j=0; args[i][j][0]; j++)
            {
                arg_ptr[j] = args[i][j];
            }
            arg_ptr[j]=NULL;
            for (j=0; arg_ptr[j]; j++)
            {
                arg_ptr_arr[i][j] = arg_ptr[j];
            }
            arg_ptr_arr[i][j] = arg_ptr[j];
        }
        rc  = execute_pipeline(cmd, arg_ptr_arr, envp, count);
        memset(cmd, 0, count*CMD_LENGTH );
        memset(args, 0, count*NUM_ARGS*ARG_LENGTH );
        memset(arg_ptr_arr, 0, sizeof(arg_ptr_arr));
    }
    close(fd);
    return rc;
}
int
open_script (char *argv[])
{
    char full_filepath[256];
    int i = 0,
        j = 0,
        fd = 0;

    /* argv[1] has the name of the script to be executed */
    fd = open(argv[1], O_RDONLY);
    
    if ( ((fd < 0) && (errno != ERR_NOENT) )|| argv[1][0] == '/')
    {
        #ifdef DEBUG
        printf("Error opening file %s\n", argv[1]);
        #endif
        return fd;
    }
    #ifdef DEBUG
    if ( fd < 0)
    {
        printf("errno: %d\n", errno);
    }
    #endif
    if ( fd < 0 )
    {
        for (i = 0; path[i] ; )
        {
            memset(full_filepath, 0, sizeof(full_filepath));
            
            for (j=0 ; path[i] != ':' && path[i] != '\0'; j++, i++ )
            {
                full_filepath[j] = path[i];
            }
            
            if (path[i] == ':') i++;
            
            full_filepath[j] = '/';
            
            strncat(full_filepath, argv[1], sizeof(full_filepath));

            fd = open(full_filepath, O_RDONLY);
            #ifdef DEBUG
            printf("errno: %d\n", errno);
            #endif
            if ( fd >= 0)
            {
                /* We successfully have opened the file */
                break;
            }
        }
    }

    return fd;
}
void
extract_env_variables (char *envp[])
{
    int index;

    index = extract_env_variable(envp, "PATH=", path);
    if ( index != -1 )
    {
        /* The PATH variable inside envp will, from now on, point to 
         * our path_env[] array
         */
        envp[index] = path_env;
    }
    
    index = extract_env_variable(envp, "PWD=", pwd);
    if ( index != -1 )
    {
        /* The PWD variable inside envp will, from now on, point to 
         * our path_env[] array
         */
        envp[index] = pwd_env;
    }
    
    index = extract_env_variable(envp, "HOME=", home);
    if ( index != -1 )
    {
        /* The HOME variable inside envp will, from now on, point to 
         * our home_env[] array
         */
        envp[index] = home_env;
    }

    index = extract_env_variable(envp, "PS1=", ps1);
    if ( index != -1 )
    {
        /* The PS1 variable inside envp will, from now on, point to 
         * our ps1_env[] array
         */
        envp[index] = ps1_env;
    }    
}

/* Helper function to print envp */
void 
_print_envp( char *envp[])
{
    int i =0;
    while ( envp[i] )
    {
        printf("%s\n", envp[i]);
        i++;
    }
    
}
/* This function searches for PATH variable in the envp array and 
 * populates the global PATH array. Returns the index of the PATH
 * variable found
 */
int  
extract_env_variable(char *envp[], char *search, char *env)
{
    int i =0;
    
    for ( i = 0; envp[i]; i++ )
    {
        if (!strncmp(envp[i], search, strlen(search)))
        {
            strncpy(env, (&envp[i][0] + strlen(search)), MAX_SHELL_STRING);
            return i;
        }
    }
    return -1;
}

int
parse_cmd_line(char (*cmds)[CMD_LENGTH], char (*args)[NUM_ARGS][ARG_LENGTH])
{
    int i = 0,
        j = 0,
        k = 0,
        l = 0;
    
    for ( ; cmd_line[i] ; k++)
    {
        /* Skip Leading spaces */
        while (cmd_line[i] == ' ' )     i++;
        
        /* Extract command. All that is there till the space will be
         * considered as a command
         */
        for(j = 0; cmd_line[i] != ' ' && cmd_line[i] != '\0' && 
                   cmd_line[i] != '|' ; j++, i++)
        {
            cmds[k][j] = cmd_line[i];
        }
        cmds[k][j] = 0;
        
        /* execve(), by convention requires command name as first param
         * in-built commands do not require execve. Hence we skip below 
         * part for built in commands
         */
        l = 0;
        if (is_command(cmds[k]) < 0)
        {
            strncpy( args[k][0], cmds[k], sizeof(args[k][0]) );
            l++;
        }

        for(; cmd_line[i] != '|' && cmd_line[i] != '\0'; l++)
        {
            /* Skip Leading spaces */
            while (cmd_line[i] == ' ' )     i++;
            for ( j=0; cmd_line[i] != '|' && cmd_line[i] != '\0' && 
                       cmd_line[i] != ' '; j++, i++)
            {
                args[k][l][j] = cmd_line[i];
            }
            args[k][l][j] = 0;
        }

        /* If i is pointing to '|', increment i */
        if ( cmd_line[i] == '|' )       i++;
    }
    return k;
}

/* Returns index of command if the passed string is a supported shell command */
int
is_command(char *cmd )
{
    int i = 0;
    
    for ( i = 0; i < COUNT_OF(commands); i++ )
    {
        if ( strcmp(commands[i].cmd_name, cmd) == 0 )
        {
            return i;
        }
    }
    return -1;
}

int set(char *args[])
{
    int i =0;
    int rc = 0;
    char *ptr = NULL;
    
    /* If there are more than one argument to set, error out */
    if ( (args[0] == NULL ) || (args[1] != NULL) )
    {
        print_error("set: Currently, set can set exactly one variable at a time\n");
        print_error("Usage: set VAR=VALUE\n");
        print_error("NOTE: Spacing matters!\n");
        return -1;
    }
    /* Skip leading spaces */
    while(args[0][i]== ' ' )       i++;
    
    if ( strncmp(args[0], "PATH=", 5) == 0)
    {
        strncpy(path, (args[0] + 5), MAX_SHELL_STRING);
    }
    else if ( strncmp(args[0], "PS1=", 4) == 0)
    {
        strncpy(ps1, (args[0] + 4), MAX_SHELL_STRING);
        ps1_changed = 1;
    }
    else if ((ptr = strstr(args[0], "=")) == 0)
    {
        print_error("Usage: set VAR=VALUE\n");
    }
    else
    {
        print_error("The variable you are trying to set is unsupported\n");
    }
    return rc;
}

int cd(char *args[])
{
    char *temp_path = NULL;
    int rc  = 0;

    /* If there are more than one argument to cd, error out */
    if ( args[0] != NULL && args[1] != NULL )
    {
        // All errors are handled at one place in execute
        errno = ERR_2BIG;
        print_errno(errno, "cd");
        return -1;
    }
    
    // If there is no argument then we should go to home directory
    temp_path = (args[0] == NULL ? home : args[0]);

    parse_path(temp_path);

    rc = chdir(temp_path);
    if (rc == 0)
    {
        /* Update PWD */
        strncpy(pwd, temp_path, sizeof(pwd_env) - 4);
    }
    else
    {
        print_error("cd: ");
        /* We should ideally send the failing command to print_erro. But cd is
           is a special case where in we should mention the path for which it
           failed. Hence this tweak.
         */
        print_errno(errno, temp_path);
    }
    return rc;
}

int clean_exit(char *args[])
{
    int rc = 0;
    
    exit(rc);
    
    #ifdef DEBUG
        printf("Unreachable code\n");
    #endif
    
    return 0;
}
/**
Takes an array of string commands which are supposed to be pipelined
**/
int execute_pipeline(char (*cmds)[CMD_LENGTH], arr_char_ptr_t arg_ptr_arr[],
                      char *envp[], int cmd_count)
{
    int prev_fd[2] = {-1, -1}, next_fd[2] = {-1, -1}, status;
    int i = 0, j = 0;
    int pid;
    int wait_process[NUM_PROCESS];
    int num_process=-1;
    int rc = 0;
    int background_proc = 0;
   // printf("cmd_count = %d \n",cmd_count);

    for(i=0;i<cmd_count;i++)
    {
        background_proc = 0;
        j = 0;
        while (arg_ptr_arr[i] &&
               arg_ptr_arr[i][j])
        {
            if (strncmp(arg_ptr_arr[i][j], "&", 2) == 0)
            {
                background_proc = 1;
                // Set next argument as NULL
                arg_ptr_arr[i][j+1] = NULL;
                break;
            }
            j++;
        }

        // For single in-built command we should not create fork
        // but directly run it in current shell
        if (cmd_count == 1 && is_command(cmds[i]) >= 0)
        {
            rc = execute(cmds[i], arg_ptr_arr[i], envp);
            if (rc)
            {
                /* Do we need to do any error handling? */
            }
            break;
        }


        if (i != (cmd_count-1))
        {
            rc = pipe(next_fd);
            if(rc < 0)
            {
                print_errno(errno, cmds[i]);
                return -1;
            }
        }

        pid = fork();

        if(pid==0)
        {
            //If i is equal to 0 then it takes from the stdin
            //Else it takes from previous pipe
            if( i != 0 )
            {
                dup2(prev_fd[0],STD_IN);
               // printf("Child About to dup2 from stdin to fd=%d \n",next_fd[1]);
               // printf("Child About to close prev fd=%d \n",prev_fd[0]);
                close(prev_fd[0]);
               // printf("Child About to close prev fd=%d \n",prev_fd[1]);
                close(prev_fd[1]);
            }
            if( i != cmd_count-1 )
            {
            //    printf("Child About to dup2 from stdout to fd=%d  \n",next_fd[1]);
                dup2(next_fd[1],STD_OUT);
             //   printf("Child About to close next fd=%d \n",next_fd[0]);
                close(next_fd[0]);
             //   printf("Child About to close next fd=%d \n",next_fd[1]);
                close(next_fd[1]);
            }
            rc = execute(cmds[i], arg_ptr_arr[i], envp);
            if (rc)
            {
                /* Do we need any error handling here? */
            }
            exit(1);
        }
        else
        {
            if (!background_proc)
            {
                num_process++;
                wait_process[num_process]=pid;
            }

            if( i != 0 )
            {   
               // printf("Parent about to close prev_fd=%d prev_fd=%d \n",
               // prev_fd[0], prev_fd[1]);
                close(prev_fd[0]);
                close(prev_fd[1]);
            }
            prev_fd[0]=next_fd[0];
            prev_fd[1]=next_fd[1];
        }
    }
    for(i=0; i <= num_process; i++)
    {
        waitpid(wait_process[i],&status,0);
    }
    return 0;
}

int
execute(char *filepath, char *argv[], char *envp[])
{
    char full_filepath[256];
    int i = 0,
        j = 0,
        rc = 0;

    if ((i = is_command(filepath)) != -1)
    {
        // As a foolproof solution we should include all argv into single buffer
        // Also we need to check if we passed extra arguments like for "cd x y"
        return commands[i].cmd(argv);
    }
    /* Do not execute files that don't have relative or absolute paths */
    if ( strstr(filepath, "/"))
    {
        /* execve() doesn't return on SUCCESS */
        rc = execve(filepath, argv, envp);
        if ( rc < 0 )
        {
            print_errno(errno, filepath);
            return rc;
        }
    }
    
    for (i = 0; path[i] ; )
    {
        memset(full_filepath, 0, sizeof(full_filepath));
        
        for (j=0 ; path[i] != ':' && path[i] != '\0'; j++, i++ )
        {
            full_filepath[j] = path[i];
        }
        
        if (path[i] == ':') i++;
        
        full_filepath[j] = '/';
        
        strncat(full_filepath, filepath, sizeof(full_filepath));
        
        /* execve() doesn't return on SUCCESS */
        rc = execve(full_filepath, argv, envp);
        #ifdef DEBUG
            printf("execve failed: %d\n", errno);
        #endif
    }
    if ( errno == ERR_NOENT)
    {
        /* If we are here and couldn't find the executable, then its not a file
         * but a command
         */
        errno = ERR_NO_CMD;
    }
    /* Handle errno */
    print_errno(errno, filepath);

    return -1;
}

char*
truncate_forward_slashes(char *path_str)
{
    int i = strlen(path_str);
    if (!i)
        return path_str;
    while(path_str[--i] == '/')
        ;
    path_str[i+1] = '\0';
    return path_str;
}

char*
truncate_forward_slashes_except_one(char *path_str)
{
    int i = strlen(path_str);
    if (!i)
        return path_str;
    i--;
    while(path_str[i] == '/')
    {
        if (i-1 >= 0 && path_str[i-1] == '/')
            i--;
        else
            break;
    }
    path_str[i+1] = '\0';
    return path_str;
}

char*
truncate_previous_dir(char *path_str)
{
    int i = 0;
    truncate_forward_slashes(path_str);
    i = strlen(path_str);
    if (!i)
        return path_str;
    
    i--;
    while (i >= 0 && path_str[i] != '/')
        i--;
    i++;

    path_str[i] = '\0';
    return path_str;
}

char*
parse_path(char *path_str)
{
    char temp_path[MAX_SHELL_STRING] = {0};
    int i = 0, j = 0, len = 0;
    if (strlen(path_str) == 1 && path_str[0] == '~')
    {
        // It's ~ and we need to go to home directory
        strncpy(path_str, home, MAX_SHELL_STRING);
        return path_str;
    }
    else if (path_str[0] != '/')
    {
        // It's not the absolute path, so lets include pwd
        strncpy(temp_path, pwd, sizeof(temp_path));
        i = strlen(temp_path);
        if (temp_path[i-1] != '/')
        {
            temp_path[i++] = '/';
            temp_path[i] = '\0';
        }
    }

    truncate_forward_slashes_except_one(path_str);
    
    len = strlen(path_str);
    for (j = 0; j < len; )
    {
        // For the first . or later /.
        if ((j == 0 || path_str[j-1] == '/') &&
            path_str[j] == '.')
        {
            j++;
            if (j < len && path_str[j] == '.')
            {
                j++;
                if ((j < len && path_str[j] == '/') ||
                     j == len)
                {
                    truncate_previous_dir(temp_path);
                    i = strlen(temp_path);
                }
                else
                {
                    temp_path[i++] = '.';
                    temp_path[i++] = '.';
                    temp_path[i++] = path_str[j];
                    temp_path[i] = '\0';
                }
            }
            else if (j < len && path_str[j] != '/')
            {
                temp_path[i++] = '.';
                temp_path[i++] = path_str[j];
                temp_path[i] = '\0';
            }
        }
        else
        {
            temp_path[i++] = path_str[j];
            temp_path[i] = '\0';
        }
        j++;
    }
    
    // This situation will be for    ../../../../../../  where we will consume
    // all folders, so we should be at least at the root in such scenarios
    if (i == 0)
        temp_path[i++] = '/';
    
    temp_path[i] = '\0';
    strncpy(path_str, temp_path, MAX_SHELL_STRING);
    return path_str;
}
