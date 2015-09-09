#ifndef _TARFS_H
#define _TARFS_H

extern char _binary_tarfs_start;
extern char _binary_tarfs_end;

/* Values used in typeflag field.  */
#define REGTYPE	 '0'		/* regular file */
#define AREGTYPE '\0'		/* regular file */
#define LNKTYPE  '1'		/* link */
#define SYMTYPE  '2'		/* reserved */
#define CHRTYPE  '3'		/* character special */
#define BLKTYPE  '4'		/* block special */
#define DIRTYPE  '5'		/* directory */
#define FIFOTYPE '6'		/* FIFO special */
#define CONTTYPE '7'		/* reserved */

struct posix_header_ustar {
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char checksum[8];
	char typeflag[1];
	char linkname[100];
	char magic[6];
	char version[2];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char prefix[155];
	char pad[12];
};

#endif
