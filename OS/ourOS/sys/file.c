#include <sys/sbunix.h>
#include <sys/kstring.h>
#include <sys/pagetable.h>
#include <sys/process_mem_layout.h>
#include <sys/tarfs.h>
#include <sys/syscall.h>
#include <sys/kmalloc.h>
Tree* root = NULL;
char testPath[FILEPATH_LEN]="/";

/*
Let the path be /in/abc/zyf
And we /abc/zyf then this would set abc in name
and returns the offset by which the current full path is
to be moved forward
*/
int findNextNameinPath(char* path,char* name)
{
    int i = 0;
    while(*(path+i)!='\0' && *(path+i)!='/')
    {
        *(name+i) = *(path+i);
        i++;
    }
    *(name+i)='\0';
    return i;
}

Tree* createNode(char* name)
{
    Tree* newNode;
    newNode =(Tree*)kmalloc(sizeof(Tree));
    if(newNode == NULL)
        return NULL;
    strncpy(newNode->name, name, strlen(name));
    newNode->child = NULL;
    newNode->sibling = NULL;
    return newNode;
}
//Find the next child Node by name
Tree* findChildNodebyName(Tree* currNode, char* name)
{
    Tree *nextNode;
    if(currNode->child == NULL)
    {
        return NULL;
    }
    nextNode = currNode->child;
    while(nextNode)
    {
        if(strcmp(nextNode->name, name)==0)
        {
            //Match found, return current node as next in path
            return nextNode;
        }
        nextNode = nextNode-> sibling;    
    }
    if(nextNode==NULL)
    {
       //current path not found
       return NULL;
    }
    return nextNode;
}

void populateFileSystem()
{
    char name[FILEPATH_LEN];
    //This tells how much we have to move relative to the start of path
    int move_index;
    
   
    Tree* currentNode, *childNode, *nextNode, *newNode;
       
    //Initialize the read only file system root node
    root = createNode("/");
    if( root == NULL)
    {
        #ifdef LOG
            printf(" Cannot allocate memory for tarfs files \n");
        #endif
        return;
    }
    #if 1
    root->fileInfo = kmalloc(sizeof(struct posix_header_ustar));
    if ( root->fileInfo == NULL )
    {
        #ifdef LOG
            printf(" Cannot allocate memory for tarfs files \n");
        #endif
        return;
    }
    root->fileInfo->typeflag[0] = '5'; /* Root is a directory */
    strncpy(root->fileInfo->mode, "755", sizeof(root->fileInfo->mode));
    #endif
    
    struct posix_header_ustar* startfs= &((struct posix_header_ustar*)&_binary_tarfs_start)[0];
    uint64_t size=0;
    //Below loops through all files present in tarfs
    while ( startfs < (struct posix_header_ustar*)&_binary_tarfs_end)
    {
        //printf("File Name: %s\n", startfs->name);
        move_index = 0;
        currentNode = root;
        if ( startfs->name[0] == 0 )
               break;
            
        while(1)
        {
           
            //move_index contains the index of char next to '/'
            move_index = move_index + findNextNameinPath(startfs->name+move_index , name);
            //To go to next character of '/'
            move_index = move_index + 1;
            if(*name=='\0')
            {
                //Path ended here
                currentNode->fileInfo = startfs;
                //Now move to map next file entry
                break;
            }
            childNode = findChildNodebyName(currentNode,name);
            //If no childnode present below the parent
            //Then create one now
            if( childNode == NULL)
            {
                newNode = createNode(name);
                if( newNode == NULL)
                {
                    #ifdef LOG
                        printf(" Cannot allocate memory for tarfs files \n");
                    #endif
                    return;
                }
                if(currentNode->child ==NULL)
                {
                    currentNode->child = newNode;
                }
                else
                {
                    nextNode = currentNode->child;
                    while(nextNode->sibling)
                    {
                        nextNode = nextNode->sibling;    
                    }
                    nextNode->sibling = newNode;
                }
                currentNode = newNode;
            }
            else
            {
                currentNode = childNode;
            }
        }
        atoo(&size, startfs->size);
        startfs =  (struct posix_header_ustar *)((uint64_t)startfs + ROUNDUP(size, 512, uint64_t) + sizeof(struct posix_header_ustar));
    }
    //root->fileInfo->typeflag[0] = '5'; /* Root is a directory */
    //strncpy(root->fileInfo->mode, "755", sizeof(root->fileInfo->mode));
    return;
}

void checkFileSystem(Tree* node,int len)
{
    if(node==NULL)
       return;
    int par_len = len;
    if(strcmp(testPath,"/")!=0)
    {
        memcpy(testPath+len,"/",1);
        len = len + 1;
    }
    memcpy(testPath+len,node->name,strlen(node->name));
    len = len + strlen(node->name);
    testPath[len]='\0';
    if(node->child==NULL)
    {
        #ifdef LOG
            printf("%s \n",testPath);
        #endif
    }
    else
    {
        checkFileSystem(node->child,len);
    }
    testPath[par_len]='\0';
    checkFileSystem(node->sibling, par_len);
}
Tree* findNodeInFileSystembyPath(Tree* startNode, char* pathname)
{
    char name[FILEPATH_LEN];
    int move_index = 0;
    Tree* currentNode,*childNode;
    currentNode = startNode;
    while(1)
    {
        //To go to next character of '/' for next iteration
        if(pathname[move_index]=='/')
        {
            move_index = move_index + 1;
        }
        move_index = move_index + findNextNameinPath( pathname+move_index , name);

        //Entry found
        if(*name=='\0')
        {
           return currentNode;
        }
        childNode = findChildNodebyName(currentNode,name);
        //If no childnode present below the parent
        //Then create one now
        if( childNode == NULL)
        {
           return NULL;
        }
        currentNode = childNode;
    }
}
int getDataFromFile(fileDesc* currDesc,char* buf,size_t count)
{
    if(!currDesc || !buf)
    {
        return -1;
    }
    uint64_t file_size=0;
    uint64_t offset = currDesc->offset;
    struct posix_header_ustar* fileInfo = currDesc->file->fileInfo;
    atoo(&file_size,fileInfo->size);
    uint64_t file_start = (uint64_t)(currDesc->buffer);
    
    //Offset is at the end, simply return 0
    if(offset>=file_size)
        return 0;
    //Check for read limits
    if( offset + count > file_size)
    {
        count = file_size - offset;
    }
    
    memcpy(buf,(uint64_t*)(file_start + offset),count);
    currDesc->offset = currDesc->offset + count;
    return count;
}

int writeToPipe(pipes* vpipe,uint64_t* writeBuf, uint64_t* procBuf,size_t count)
{
    if(!vpipe || !procBuf || !writeBuf)
    {
        return -1;
    }
  
    int read_index = vpipe-> read_index;
    int write_index = vpipe-> write_index;
    int size = vpipe-> size;
   
    //These two variables are to determine number of bytes
    //written during current run
    int bytes_to_write,bytes_written = 0;
    //This tells total number of bytes written but not read
    int total_bytes_written = vpipe->total_bytes_written;
    while(1)
    {
      
        //If there is space in buffer to write
        if(total_bytes_written!=size)
        {
            //Calculate how many bytes can we possibly write in this run
            if(write_index >= read_index)
            {
                bytes_to_write = size - write_index;
            }
            else
            {
                bytes_to_write = read_index - write_index;
            }
            
            //Check number of bytes to be written(in total for this
             //write pipe call)   are not more than count
            if( bytes_to_write + bytes_written > count)
            {
                bytes_to_write = count-bytes_written;
            }
            
            //Write bytes to writeBuf at write_index
            //from procBuf+bytes_written(number of bytes written
            //in current run
            memcpy((uint8_t*)procBuf+write_index,(uint8_t*) writeBuf + bytes_written, bytes_to_write);
             
             //Update the variable for total bytes written
             //by how many bytes we wrote in present run
            bytes_written = bytes_written + bytes_to_write;
            //How many bytes written but not yet read
            total_bytes_written = total_bytes_written + bytes_to_write;
                  
            //Update the write_index by how much bytes we have 
            //written in present run
            write_index = (write_index + bytes_to_write)%size;
            
            //Check whether total bytes written matches the count
            
            if( bytes_written == count)
            {   //Job done for now
                break;
            }
        }
        else 
        {
            //Pipe is full.If more bytes to write then.
            //Block
            //If no read descriptors are open then return
            if(checkOpenReadDesc(vpipe)==0)
            {
              //  printf("No read descriptor open \n");
                return bytes_written;
            }
            //Update pipe structure so that is seen by read
            //descriptor
            updatePipeStruc(vpipe,read_index,write_index,total_bytes_written); 
            blockProcDueToPipe(curr_running_proc, WAITING_FOR_PIPEWRITE, vpipe);
            wait_in_kernel();
            //Update its variable which were updated in write
            read_index = vpipe-> read_index;
            write_index = vpipe-> write_index;
            total_bytes_written = vpipe->total_bytes_written;
        }
        //Keep on polling to release any blocked read
        wakeProcDueToPipe(vpipe);
    }
    updatePipeStruc(vpipe,read_index,write_index,total_bytes_written); 
    wakeProcDueToPipe(vpipe);
    return bytes_written;
}

int readFromPipe(pipes* vpipe,uint64_t* readBuf, uint64_t* procBuf,size_t count)
{
    if(!vpipe || !procBuf || !readBuf)
    {
        return -1;
    }
  
    int read_index = vpipe-> read_index;
    int write_index = vpipe-> write_index;
    int size = vpipe-> size;
    int bytes_to_read,bytes_read = 0;
    //This tells total number of bytes written but not read
    int total_bytes_written = vpipe->total_bytes_written;
    while(1)
    {
       
        //Some bytes must exist to be read
        if(total_bytes_written)
        {
             //Calculate how many bytes can we possibly read in this run
            if(write_index > read_index)
            {
                bytes_to_read = write_index - read_index;
            }
            else
            {
                bytes_to_read = size - read_index;
            }
            
            //Check number of bytes to be written are not more than count
            if( bytes_to_read + bytes_read > count)
            {
                bytes_to_read  = count - bytes_read;
            }
            //Read bytes from readBuf to procBuf 
            memcpy((uint8_t*)readBuf+bytes_read,(uint8_t*) procBuf + read_index, bytes_to_read);
               

            bytes_read = bytes_read + bytes_to_read;
            //Update the read_index by how much bytes we have 
            //read in present run
           // printf("read_index=%d \n",read_index);
            read_index = (read_index + bytes_to_read)%size;
            
            //Reduce the total number of bytes written but not read
            total_bytes_written = total_bytes_written - bytes_to_read;   
           
            //First condition tells everything has been read till count
            if( bytes_read == count || total_bytes_written==0)
            {   //Job done for now
                break;
            }
        }
        else
        {
            //No bytes left to read
            //If no open write descriptors left
            if(checkOpenWriteDesc(vpipe)==0)
            {
               
               // printf("No write descriptor open \n");
                return bytes_read;
            }
            //Block
            //Update pipe structure so that is seen by write
            //descriptor 
            updatePipeStruc(vpipe,read_index,write_index,total_bytes_written); 
            blockProcDueToPipe(curr_running_proc, WAITING_FOR_PIPEWRITE, vpipe);
            wait_in_kernel();
       //   printf("Read awake %s %d\n",readDesc->proc->name,readDesc->proc->pid);
            //Update its variable which were updated in write
            read_index = vpipe-> read_index;
            write_index = vpipe-> write_index;
            total_bytes_written = vpipe->total_bytes_written;
        }
        
        //Keep on polling to release any blocked write
        wakeProcDueToPipe(vpipe);
    }
    updatePipeStruc(vpipe,read_index,write_index,total_bytes_written); 
    wakeProcDueToPipe(vpipe);
    return bytes_read;
}
void updatePipeStruc(pipes* vpipes, uint64_t read_index, uint64_t write_index,
                    uint64_t total_bytes_written)
{
    vpipes->read_index = read_index;
    vpipes->write_index = write_index; 
    vpipes->total_bytes_written = total_bytes_written; 
}
void blockProcDueToPipe(process_t* proc, uint64_t flags, pipes* vpipe)
{
    proc->flags = proc->flags | flags;
    vpipe->waiting_proc = proc;
}
int wakeProcDueToPipe(pipes* vpipe)
{
    int rc = 0;
    if(!vpipe)
    {
        return -1;
    }
    process_t* proc = vpipe->waiting_proc;
    if (proc)
    {
        if (proc->state == PROCESS_WAITING &&
            (proc->flags & WAITING_FOR_PIPEREAD))
        {
            proc->state = PROCESS_RUNNABLE;
            proc->flags &= ~WAITING_FOR_PIPEREAD;
            rc = switch_process_queue(proc, PROCESS_RUNNABLE_QUEUE);
        }
        else  if (proc->state == PROCESS_WAITING &&
            (proc->flags & WAITING_FOR_PIPEWRITE))
        {
            proc->state = PROCESS_RUNNABLE;
            proc->flags &= ~WAITING_FOR_PIPEWRITE;
            rc = switch_process_queue(proc, PROCESS_RUNNABLE_QUEUE);
        }
        vpipe->waiting_proc = NULL;
    }
    return rc;
}

fileDesc *
fd_to_file ( int fd)
{
    if ( fd > (MAX_FILE_DESCRIPTORS-1) || fd < 0)
    {
        return NULL;
    }
    return curr_running_proc->listFileDesc[fd];
}
int64_t
get_children(Tree *dir_node, struct dirent *dirp, unsigned int count, 
             fileDesc *file)
{
    /* offset represents the number of element in the siblings list */
    Tree *files_list = dir_node->child;
    
    for ( int i =0; i < file->offset; i++ )
    {
        if ( files_list )
        {
            files_list = files_list->sibling;
        }
    }
    if ( files_list == NULL )
    {
        return 0;
    }
    dirp->d_ino = (long)files_list; /* Address is unique. Can use it as i-node */
    dirp->d_off = file->offset+1;
    file->offset++;
    dirp->d_reclen = sizeof(struct dirent);
    dirp->d_type   = files_list->fileInfo->typeflag[0] - '0';
    strncpy(dirp->d_name, files_list->name, NAME_MAX);
    
    return dirp->d_reclen;
}
uint8_t checkOpenReadDesc(pipes* vpipe)
{
  if(vpipe->readDesc==NULL)
     return 0;
  else
     return 1;
}
uint8_t checkOpenWriteDesc(pipes* vpipe)
{
  if(vpipe->writeDesc==NULL)
     return 0;
  else
     return 1;
}

//Remove the read descriptor from pipe strucure
//In other words remove Reverse mapping
int removePipeReadDesc(pipes* vpipe, process_t* proc, uint16_t fd_num)
{
  if(!vpipe || !proc)
  {
    return -1;
  }
  PipeDesc* tempdesc = vpipe->readDesc;
  PipeDesc* freedesc;
  //If there is only one read desc to this pipe
  if(tempdesc)
  {
    if(tempdesc->proc == proc && tempdesc->fd_num == fd_num)
    {
        vpipe->readDesc = tempdesc->next;
        kfree(tempdesc);
        return 1;
    }
  }
  //If there is a list
  while(tempdesc)
  {
    if(tempdesc->next && tempdesc->next->proc == proc
    && tempdesc->next->fd_num == fd_num)
    {
        freedesc = tempdesc->next;
        tempdesc->next = tempdesc->next->next;
        kfree(freedesc);
        return 1;
    }
    tempdesc= tempdesc->next;
  }
  //Not found
  return 0;
}
//Remove the write descriptor from pipe strucure
//In other words remove Reverse mapping
int removePipeWriteDesc(pipes* vpipe, process_t* proc, uint16_t fd_num)
{
  if(!vpipe || !proc)
  {
    return -1;
  }
  PipeDesc* tempdesc = vpipe->writeDesc;
  PipeDesc* freedesc;

  //If there is only one read desc to this pipe
  if(tempdesc)
  {
    if(tempdesc->proc == proc && tempdesc->fd_num == fd_num)
    {
      
        vpipe->writeDesc = tempdesc->next;
        kfree(tempdesc);
        return 1;
    }
  }
  //If there is a list of more than one write desc
  while(tempdesc)
  {
    if(tempdesc->next && tempdesc->next->proc == proc
    && tempdesc->next->fd_num == fd_num)
    {
        freedesc = tempdesc->next;
        tempdesc->next = tempdesc->next->next;
        kfree(freedesc);
        return 1;
    }
    tempdesc= tempdesc->next;
  }
  //Not found
  return 0;

}
uint8_t addPipeReadDesc(pipes* vpipe, process_t* proc, uint16_t fd_num)
{
    PipeDesc* tempdesc = (PipeDesc*)kmalloc(sizeof(PipeDesc));
    if(!tempdesc)
    {
        return -1;
    }
    tempdesc->next = vpipe->readDesc ;
    tempdesc->proc = proc;
    tempdesc->fd_num = fd_num;
    vpipe->readDesc = tempdesc;
    return 1;
}
uint8_t addPipeWriteDesc(pipes* vpipe, process_t* proc, uint16_t fd_num)
{
    PipeDesc* tempdesc = (PipeDesc*)kmalloc(sizeof(PipeDesc));
    if(!tempdesc)
    {
        return -1;
    }
    tempdesc->next = vpipe->writeDesc ;
    tempdesc->proc = proc;
    tempdesc->fd_num = fd_num;
    vpipe->writeDesc = tempdesc;
    return 1;
}