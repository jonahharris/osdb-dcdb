/*	Header File:	block.h	*/

#ifndef	__BLOCK_H__
#define	__BLOCK_H__

#include <sort.h>

externC

/*
 * [BeginDoc]
 * 
 * Provides generic block I/O support.  Block I/O is important for
 * reading and writing data files which are best represented by
 * blocks of data rather than streams of data.  The block module
 * provides platform independent methods of creating block files,
 * opening block files, closing block files, reading from and
 * writing to block files, converting blocks (from block files) to
 * and from buffers and lists.
 * 
 * An error condition will be reported using the fsystem module's
 * error reporting strings and constants.  In case of an error, the
 * user can look at fioErrMsg[fioError] for the cause of the error.
 * You need not include fsystem.h because it is included by block.h
 * 
 * A block file is assumed to have a ``control block'', which
 * is the first block in the file (block 0).  What a control block
 * has in it is application dependent, but should include information
 * that is global to all file access.  Each block after the control
 * block is numbered from 1 to the last block in the block file.
 * These are numbered consecutively.  The block size for a file must
 * remain constant with any use of the file.  So, a block file would
 * be laid out as follows:
 * 
 * + ------------------------------------------------------------
 * + 
 * + Control Block
 * + 
 * + ------------------------------------------------------------
 * + 
 * + Block 1
 * + 
 * + ------------------------------------------------------------
 * + 
 * + Block 2
 * + 
 * + ------------------------------------------------------------
 * + .
 * + .
 * + .
 * + ------------------------------------------------------------
 * 
 * =================================================================
 * 
 * [EndDoc]
 */

/*
 * Block functions
 * 
 * @DocInclude ../index/block.c
 */
int createBlockFile (char *name, off_t blksize,
    void *control);
int openBlockFile (char *name, off_t blksize, void *control);
int closeBlockFile (int fd, off_t blksize, void *control);
int getFileBlock (int fd, void *blk, off_t blknum,
    off_t blksize);
int putFileBlock (int fd, void *blk, off_t blknum, 
						off_t blksize);
off_t appendFileBlock (int fd, void *blk, off_t blksize);
void bufferToBlock (void *buf, void *blk, size_t size, 
						 size_t offset);
void blockToBuffer (void *buf, void *blk, size_t size,
						  size_t offset);
int blockToList (ListHeader *lh, void *blk, size_t size,
					   size_t num, size_t offset);
int listToBlock (ListHeader *lh, void *blk, size_t size,
					   size_t num, size_t offset);

endC

#endif	/* __BLOCK_H__ */
