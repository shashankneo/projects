#define FILEPATH_LEN 256
struct _process_t_;
typedef struct _tree_node_t
{
    char name[NAME_MAX];
    struct posix_header_ustar* fileInfo;
    struct _tree_node_t *child;
    struct _tree_node_t *sibling;
} Tree;
typedef struct _pipe_descriptor
{
    struct _process_t_* proc;
    uint16_t fd_num;
    struct _pipe_descriptor* next;
}PipeDesc;
typedef struct _pipe
{
    int read_index; //points to where it should start reading from
    int write_index; //points to where it should start writing from
    int size;
    int total_bytes_written; // Records how many unread bytes are there in pipe
    PipeDesc* readDesc;
    PipeDesc* writeDesc;
    struct _process_t_* waiting_proc;
}pipes;

typedef struct _file_descriptor
{
    int64_t offset;
    uint8_t  type;
    Tree* file;
    uint64_t* buffer;
    pipes*  vpipe;
} fileDesc;

enum { STDIN=0,STDOUT,STDERR,DESC_FILE,PIPE_READ,PIPE_WRITE};
#define PIPE_SIZE PGSIZE
void populateFileSystem();
void checkFileSystem(Tree* node,int len);
Tree* findNodeInFileSystembyPath(Tree* startNode, char* pathname);
int getDataFromFile(fileDesc* currDesc,char* buf,size_t count);
int wakeProcDueToPipe(pipes* vpipe);
void blockProcDueToPipe(struct _process_t_* proc, uint64_t flags, pipes* vpipe);
int writeToPipe(pipes* vpipe,uint64_t* writeBuf, uint64_t* procBuf,size_t count);
int readFromPipe(pipes* vpipe,uint64_t* readBuf, uint64_t* procBuf,size_t count);
void updatePipeStruc(pipes* vpipes, uint64_t read_index, uint64_t write_index,
                    uint64_t total_bytes_written);

uint8_t addPipeWriteDesc(pipes* vpipe, struct _process_t_* proc, uint16_t fd_num);
uint8_t addPipeReadDesc(pipes* vpipe, struct _process_t_* proc, uint16_t fd_num);
fileDesc *fd_to_file ( int fd);

int64_t
get_children(Tree *dir_node, struct dirent *dirp, unsigned int count, 
             fileDesc *file);


extern Tree* root;

uint8_t checkOpenWriteDesc(pipes* vpipe);
uint8_t checkOpenReadDesc(pipes* vpipe);
int removePipeWriteDesc(pipes* vpipe, struct _process_t_* proc, uint16_t fd_num);
int removePipeReadDesc(pipes* vpipe, struct _process_t_* proc, uint16_t fd_num);
extern Tree* root;

