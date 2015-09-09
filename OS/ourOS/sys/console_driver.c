#include <sys/console_driver.h>
#include <sys/terminal.h>
#include <sys/assembly_utility.h>
#include <sys/sbunix.h>
#include <sys/console.h>

unsigned char scancode_ascii[128] =
{
    0,
    27,
   '1',
   '2',
   '3',
   '4',
   '5',
   '6',
   '7',
   '8',
   '9',
   '0',
   '-',
   '=',
   '\b',
   '\t',
   'q', 
   'w', 
   'e', 
   'r',
   't', 
   'y', 
   'u', 
   'i', 
   'o', 
   'p', 
   '[', 
   ']', 
   '\n',
    0,
   'a', 
   's', 
   'd', 
   'f', 
   'g', 
   'h', 
   'j', 
   'k', 
   'l', 
   ';',
   '\'',
   '`',
    0,
   '\\',
   'z',
   'x',
   'c',
   'v',
   'b',
   'n',
   'm',
   ',',
   '.',
   '/',
    0,
   '*',
    0,
   ' ',
    0,
    0,
    0,
    0,   
    0,   
    0,   
    0,   
    0,   
    0,   
    0,
    0,
    0,
    0,
   '7',
   '8',
   '9',
   '-',
   '4',
   '5',
   '6',
   '+',
   '1',
   '2',
   '3',
   '0',
   '.',
    0,
    0,
    0,
    0,
    0,
    0,
};

unsigned char scancode_ascii_shift_pressed[128] =
{
    0,
    27,
   '!',
   '@',
   '#',
   '$',
   '%',
   '^',
   '&',
   '*',
   '(',
   ')',
   '_',
   '+',
   '\b',
    0,
   'Q',
   'W',
   'E',
   'R',
   'T',
   'Y',
   'U',
   'I',
   'O',
   'P',
   '{',
   '}',
   '\n',
    0,
   'A',
   'S',
   'D',
   'F',
   'G',
   'H',
   'J',
   'K',
   'L',
   ':',
   '"',
   '~',
    0,
   '|',
   'Z',
   'X',
   'C',
   'V',
   'B',
   'N',
   'M',
   '<',
   '>',
   '?',
    0,
   '*',
    0,
   ' ',
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
   '7',
   '8',
   '9',
   '-',
   '4',
   '5',
   '6',
   '+',
   '1',
   '2',
   '3',
   '0',
   '.',
    0,
    0,
    0,
    0,
    0,
    0,
};

void keyboard_handler()
{
    unsigned char scan_code = inb(KEYBOARD_DATA_PORT);
    unsigned char ascii_code = 0;
    static int shift_pressed = 0;
    unsigned key_pressed = 0 ;/* Pressed = 0, Released = Positive value */
    int rc = 0;

    key_pressed = !(scan_code &  KEY_RELEASED);
    if ( !key_pressed )
    {
        /* Eliminate the 7th bit. Its just an indicator of key press/release */
        scan_code ^= KEY_RELEASED;
    }
    
    if ( !shift_pressed )
    {
        ascii_code = scancode_ascii[scan_code];
    }
    else
    {
        ascii_code = scancode_ascii_shift_pressed[scan_code];
    }
    
    /* Check Whether key was pressed or released */
    if ( key_pressed )
    {
        if (scan_code == LEFT_SHIFT_KEY || scan_code == RIGHT_SHIFT_KEY)
        {
            shift_pressed = 1;
        }
        else
        {
            #if __FOR_FUTURE_USE__
                printf("%c", ascii_code);
            #endif
            if ( ascii_code != 0)
            {
                #if 0
                    char string[256];
                    switch(ascii_code)
                    {
                        case '\n':
                            sprintf(string, "Last Glyph pressed:'\\n'");
                            break;
                        case '\b':
                            sprintf(string, "Last Glyph pressed:'\\b'");
                            break;
                        case '\t':
                            sprintf(string, "Last Glyph pressed:'\\t'");
                            break;
                        case 27:
                            sprintf(string, "Last Glyph pressed:'\\e'");
                            break;
                        default:
                            sprintf(string, "Last Glyph pressed:'%c'    ", ascii_code);
                            break;
                    }
                    print_at(string, 45, 21);
                #endif

                // push this character to terminal
                if ( !is_buffer_empty() || !(ascii_code == '\b') )
                {
                    printf("%c", ascii_code);
                    rc = pushchar(ascii_code);
                    if (rc)
                    {
                        printf("Failed to push %c to terminal\n", ascii_code);
                    }
                }
            }
        }
    }
    else
    {
        if (scan_code == LEFT_SHIFT_KEY || scan_code == RIGHT_SHIFT_KEY)
        {
            shift_pressed = 0;
        }
        else
        {
        }
    }
}