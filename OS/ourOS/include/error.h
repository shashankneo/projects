#ifndef _ERROR_H_
#define _ERROR_H_

#define STD_ERR 2

__thread int errno;

enum
{
    ERR_SUCCESS        = 0,
    ERR_RR_PERM        = 1,
    ERR_NOENT          = 2,
    ERR_SRCH           = 3,
    ERR_INTR           = 4,
    ERR_IO             = 5,
    ERR_2BIG           = 7,
    ERR_NOEXEC         = 8,
    ERR_BADF           = 9,
    ERR_CHILD          = 10,
    ERR_NOMEM          = 12,
    ERR_ACCES          = 13,
    ERR_FAULT          = 14,
    ERR_BLOCK          = 15,
    ERR_BUSY           = 16,
    ERR_EXIST          = 17,
    ERR_NODEV          = 19,
    ERR_NOTDIR         = 20,
    ERR_ISDIR          = 21,
    ERR_INVAL          = 22,
    ERR_NFILE          = 23,
    ERR_MFILE          = 24,
    ERR_TXTBSY         = 26,
    ERR_FBIG           = 27,
    ERR_NOSPC          = 28,
    ERR_SPIPE          = 29,
    ERR_ROFS           = 30,
    ERR_MLINK          = 31,
    ERR_PIPE           = 32,
    ERR_DOM            = 33,
    ERR_RANGE          = 34,
    ERR_NO_CMD         = 35,
    ERR_FILE_READ      = 36,
    ERR_UNSPECIFIED      = 37,
    ERR_CHILDFAIL      = 38,
    ERR_FILELIMIT      = 39,
    ERR_NAMETOOLONG    = 63,
    // Add error codes above this line
    ERR_LAST
};

void print_errno(int err, char *cmd);
void print_error(void *str);

#endif //#ifdef _ERROR_H_