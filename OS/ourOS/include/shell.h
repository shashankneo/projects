#ifndef _SHELL_H_
#define _SHELL_H_

#define MAX_SHELL_STRING      512
#define CMD_LENGTH            256
#define ARG_LENGTH            256
#define STD_IN  0
#define STD_OUT 1
#define DEFAULT_PATH "/opt/gridengine/bin/lx26-amd64:/usr/kerberos/bin:/usr/java/latest/bin:/usr/local/bin:/bin:/usr/bin:/opt/Bio/ncbi/bin:/opt/Bio/mpiblast/bin/:/opt/Bio/EMBOSS/bin:/opt/Bio/clustalw/bin:/opt/Bio/t_coffee/bin:/opt/Bio/phylip/exe:/opt/Bio/mrbayes:/opt/Bio/fasta:/opt/Bio/glimmer/bin:/opt/Bio/glimmer/scripts:/opt/Bio/gmap/bin:/opt/Bio/gromacs/bin/:/opt/eclipse:/opt/ganglia/bin:/opt/ganglia/sbin:/opt/maven/bin:/opt/openmpi/bin/:/opt/rocks/bin:/opt/rocks/sbin:/opt/condor/bin:/opt/condor/sbin:/usr/X11R6/bin"

#define NUM_CMDS        100
#define NUM_ARGS        100

typedef char* arr_char_ptr_t[NUM_ARGS];

typedef enum BOOL
{
    false=0,
    true
}bool;

typedef struct Command 
{
    char *cmd_name;
    int  (*cmd)(char *args[]);
} command_t;

#define COUNT_OF(x) sizeof((x))/sizeof(x[0])
#define MAX_PATH_LENGTH 100
int set(char *args[]);
int cd(char *args[]);
int clean_exit(char *args[]);

void
set_path( char *p );

void
set_ps1( char *ps );

char *
get_path();

char *
get_ps1();

/* Returns index of command if the passed string is a supported shell command */
int
is_command(char *cmd );

int 
open_script(char *argv[]);

bool
is_comment ( char *line);

void
extract_ps1(char *envp[]);

int
parse_cmd_line(char (*cmds)[CMD_LENGTH], char (*args)[NUM_ARGS][ARG_LENGTH]);

int
execute(char *filepath, char *argv[], char *envp[]);

int execute_pipeline(char (*cmds)[CMD_LENGTH], arr_char_ptr_t arg_ptr_arr[],
                      char *envp[], int cmd_count);
void 
_print_envp( char *envp[]);

void
extract_env_variables (char *envp[]);

int 
extract_env_variable(char *envp[], char *search, char *env);

char*
truncate_forward_slashes(char *path_str);

char*
truncate_forward_slashes_except_one(char *path_str);

char*
truncate_previous_dir(char *path_str);

char*
parse_path(char *path_str);

#endif // #ifndef _SHELL_H_