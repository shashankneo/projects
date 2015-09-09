#include <sys/console.h>
#include <sys/kstring.h>
#include <sys/pagetable.h>

#define     MAX_COL     80
#define     MAX_ROW     24

#define     VIDEO_VIRTUAL_ADDRESS ( pa_va_mapping_start + 0xB8000 )
#define     GET_VB_ADDRESS(x, y) ( VIDEO_VIRTUAL_ADDRESS + ( ( (MAX_COL*(y) ) + x) * 2) )

static int X = -1, Y = -1;

// prints the given character at the screen using the position x_pos,y_pos if they're present
int printch(char ch, int *x_pos, int *y_pos)
{
    volatile char * write_buf = NULL;

    if (!x_pos || !y_pos)
    {
        // We should not reach in this condition
        return -1;
    }
     if(*y_pos == STATUS_BAR)
     {
         write_buf = (char *)GET_VB_ADDRESS(*x_pos, STATUS_BAR-1);
        *write_buf++ = ch;
        *write_buf++ = 3; // Color next byte
        (*x_pos)++;
        return 0;
     
     }
 
    //Check for some special characters, and we'rent explicitly writing them all except some
    switch(ch)
    {
        case '\n':
            *x_pos = 0;
            *y_pos = *y_pos + 1;
             if (*y_pos >= MAX_ROW)
            {
                memcpy((void *)VIDEO_VIRTUAL_ADDRESS,
                       (char *)GET_VB_ADDRESS(0, 1), (MAX_ROW - 1) * MAX_COL * 2);
                memset((char *)GET_VB_ADDRESS(0, MAX_ROW - 1),
                        0,
                        MAX_COL * 2);
                *y_pos = MAX_ROW - 1;
            }
            break;
        case '\r':
            *x_pos = 0;
            break;
        case '\b':
            if (*x_pos != 0)
            {
                (*x_pos)--;
            }
            write_buf = (char *)GET_VB_ADDRESS(*x_pos, *y_pos);
            *write_buf++ = ' ';
            *write_buf++ = 3;
            break;
        case '\t':
            printch(' ', x_pos, y_pos);
            printch(' ', x_pos, y_pos);
            printch(' ', x_pos, y_pos);
            printch(' ', x_pos, y_pos);
            break;
        default:
            if (*x_pos >= MAX_COL)
            {
                *y_pos += (*x_pos / MAX_COL);
                *x_pos = (*x_pos % MAX_COL);
            }

            if (*y_pos >= MAX_ROW)
            {
                memcpy((void *)VIDEO_VIRTUAL_ADDRESS,
                       (char *)GET_VB_ADDRESS(0, 1), (MAX_ROW - 1) * MAX_COL * 2);
                memset((char *)GET_VB_ADDRESS(0, MAX_ROW - 1),
                        0,
                        MAX_COL * 2);
                *y_pos = MAX_ROW - 1;
            }
            
            write_buf = (char *)GET_VB_ADDRESS(*x_pos, *y_pos);
            *write_buf++ = ch;
            *write_buf++ = 3; // Color next byte
            (*x_pos)++;
    }

    return 0;
}

int kprintf(char *str)
{
    if(X==-1)
       X=0;
    if(Y==-1)
       Y=0; 

    while(*str)
    {
        printch(*str++, &X, &Y);
    }
  
    return 0;
}

//Main function to output string to console
int print_at(char *str, int x, int y)
{
    //Null the given section for some reason memory is getting corrupted
    char* write_buf = (char *)GET_VB_ADDRESS(x, STATUS_BAR-1);
    memset(write_buf, 0x00, strlen(str)*2);
    // Screen size => 80 x 25
    while(*str)
    {
        printch(*str++, &x, &y);
    }

    return 0;
}