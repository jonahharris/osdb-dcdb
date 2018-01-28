/* Source File:	block.c	*/

#include <block.h>
#include <stdlib.h>
#include <string.h>

/*
 * $Id: block.c,v 1.3 2005/07/07 21:21:21 dfmay Exp dfmay $
 * $Log: block.c,v $
 * Revision 1.3  2005/07/07 21:21:21  dfmay
 * I made changes to sort.c to make it thread safe and the changes here were needed
 * to interface with that.
 *
 * Revision 1.2  2005/01/28 17:35:41  dfmay
 * Code cleanup - compiles without warning at high warning level.
 *
 * Revision 1.1  2004/10/27 15:27:38  dfmay
 * Initial revision
 *
 */

externC

/*
 * [BeginDoc]
 * 
 * int createBlockFile (char *name, int blksize, void *control);
 * 
 * Create a block file with name given by name, a blocksize
 * given by blksize, and initialize the control block with the
 * data in control.  The control parameter should be a pointer
 * to a dynamically allocated buffer of size blksize.  If it is
 * smaller, you have serious problemas!  createBlockFile() returns
 * a file handle on success, or _ERROR_ on failure.
 * 
 * =================================================================
 * [EndDoc]
 */
int createBlockFile (char *name, off_t blksize, void *control)
{
  int fd;
  off_t status;

  dbg2 (printf ("\n\ncreateBlockFile(%s, ...)\n", name);
    );
  fd = fileCreate (name);
  if (fd == _ERROR_)
    return _ERROR_;		/* error set by fsystem */
  fileClose (fd);		/* make sure open in binary */
  fd = fileOpen (name);
  if (fd == _ERROR_)
    return _ERROR_;		/* error set by fsystem */

  check_pointer (control);
  status = fileWrite (fd, control, blksize);
  if (blksize != (int) status) {
    set_fioError(FIO_UNSPECIFIED);
    return _ERROR_;
  }
  fileFlush (fd);
  dbg2 (printf ("\n\ncreateBlockFile() end\n"););
  return fd;
}

/*
 * [BeginDoc]
 * 
 * int openBlockFile (char *name, int blksize, void *control);
 * 
 * Open the blockfile given by name with the blocksize given by
 * blksize.  openBlockFile() returns the file descriptor of the
 * opened file if successful, _ERROR_ if an error occurred.  If the
 * function is successful, control will contain the data read from
 * the control block of the block file.  The control variable
 * should point to an allocated block of memory at least blksize
 * in size.  If not, you WILL have problems.
 * 
 * =================================================================
 * 
 * [EndDoc]
 */
int openBlockFile (char *name, off_t blksize, void *control)
{
  int fd;
  off_t status;

  dbg2 (printf ("\n\nopenBlockFile (%s, ...)", name););
  fd = fileOpen (name);
  if (fd == _ERROR_)
    return _ERROR_;		/* error set by fsystem */
  status = fileRead (fd, control, blksize);
  if (blksize != (int) status) {
    set_fioError(FIO_UNSPECIFIED);
    return _ERROR_;
  }
  dbg2 (printf ("\n\nopenBlockFile end\n"););
  return fd;
}

/*
 * [BeginDoc]
 * 
 * int closeBlockFile (int fd, int blksize, void *control);
 * 
 * closeBlockFile() closes the block whose file handle is given by
 * the handle fd.  The control block is saved before the file is
 * closed out.  Any unsaved blocks will not be flushed to disk
 * when the file is closed, so it is important that the user flush
 * all blocks.
 * 
 * closeBlockFile() returns _ERROR_ on error, _OK_ otherwise.
 * 
 * =================================================================
 * 
 * [EndDoc]
 */
int closeBlockFile (int fd, off_t blksize, void *control)
{
  off_t status;

  dbg2 (printf ("\n\ncloseBlockFile()\n"););
  fileSeekBegin (fd, 0L);
  status = fileWrite (fd, control, blksize);
  if (blksize != (off_t) status) {
    set_fioError(FIO_UNSPECIFIED);
    return _ERROR_;
  }
  fileFlush (fd);
  fileClose (fd);
  dbg2 (printf ("\n\ncloseBlockFile() end\n"););
  return _OK_;
}

/*
 * [BeginDoc]
 * 
 * int getFileBlock (int fd, void *blk, off_t blknum, off_t blksize);
 * 
 * getFileBlock() will get the block from the file given by fd and
 * store the data in the buffer given by blk.  blk must be a block
 * of memory dynamically allocated from the heap and must have the
 * capacity for a block of data blksize bytes long.  In order to
 * get block blknum, blknum is multiplied by blksize and that value
 * is used as an offset into the file.
 * 
 * getFileBlock() returns _ERROR_ on error, _OK_ on success.
 * 
 * =================================================================
 * 
 * [EndDoc]
 */
int getFileBlock (int fd, void *blk, off_t blknum, off_t blksize)
{
  off_t offset;
  off_t status;

  check_pointer (blk);
  offset = (long) blknum *blksize;
  status = fileSeekBegin (fd, offset);
  if ((long) status != offset) {
    set_fioError(FIO_UNSPECIFIED);
    return _ERROR_;
  }
  status = fileRead (fd, blk, blksize);
  if (blksize != (off_t) status) {
    set_fioError(FIO_UNSPECIFIED);
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * 
 * int putFileBlock (int fd, void *blk, unsigned long blknum, int blksize);
 * 
 * putFileBlock() saves the block given by blk to the file given by
 * by fd.  The file is assumed to be a block file.  blksize is the
 * size specified when the file was created using the block module.
 * The offset into the file for the save is given by blknum*blksize.
 * 
 * putFileBlock() returns _ERROR_ on error, _OK_ on success.
 * 
 * =================================================================
 * 
 * [EndDoc]
 */
int putFileBlock (int fd, void *blk, off_t blknum, off_t blksize)
{
  off_t offset;
  off_t status;

  check_pointer (blk);
  offset = (off_t) blknum *blksize;
  status = fileSeekBegin (fd, offset);
  if (status != offset) {
    set_fioError(FIO_UNSPECIFIED);
    return _ERROR_;
  }
  status = fileWrite (fd, blk, blksize);
  if (blksize != (int) status) {
    set_fioError(FIO_UNSPECIFIED);
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * 
 * unsigned long appendFileBlock (int fd, void *blk, int blksize);
 * 
 * appendFileBlock() appends the block given by blk to the file fd.
 * The file must be a block file and must have a blocksize of
 * blksize.
 * 
 * If appendFileBlock() fails, it returns (unsigned long)_ERROR_, otherwise
 * it returns the block number of the appended block.
 * 
 * =================================================================
 * 
 * [EndDoc]
 */
off_t appendFileBlock (int fd, void *blk, off_t blksize)
{
  off_t status;
  off_t offset;

  check_pointer (blk);
  offset = fileSeekEnd (fd, 0L);
  status = fileWrite (fd, blk, blksize);
  if (blksize != (int) status) {
    set_fioError(FIO_UNSPECIFIED);
    return _ERROR_;
  }
  Assert (offset % blksize == 0);
  offset /= blksize;
  return offset;
}

/*
 * [BeginDoc]
 * [Verbatim]

void bufferToBlock (void *buf, void *blk, size_t size, 
					size_t offset);

 * [EndDoc]	*/

/*
 * [BeginDoc]
 * bufferToBlock() simply copies the contents of the buffer given by
 * buf to the block given by blk at offset offset in blk.  The block
 * is assumed to be large sufficiently large such that 
 * 
 * size buf + offset < size blk.
 * 
 * If that is not true, you have muchos problemas amigo  :o)
 * 
 * =================================================================
 * 
 * [EndDoc]
 */
void bufferToBlock (void *buf, void *blk, size_t size, size_t offset)
{
  memcpy ((void *) ((char *) blk + offset), buf, size);
}

/*
 * [BeginDoc]
 * [Verbatim]

void blockToBuffer (void *buf, void *blk, size_t size,
					size_t offset);

 * [EndDoc] */
/*
 * [BeginDoc]
 * blockToBuffer() simply copies size bytes from block blk at offset
 * offset to buf.  buf is assumed to point to storage sufficient for
 * size bytes.  If not, hasta la vista baby!
 * 
 * =================================================================
 * 
 * [EndDoc]
 */
void blockToBuffer (void *buf, void *blk, size_t size, size_t offset)
{
  memcpy (buf, (void *) ((char *) blk + offset), size);
}

/*
 * [BeginDoc]
 * [Verbatim]

int blockToList (ListHeader *lh, void *blk, size_t size,
				 size_t num, size_t offset);

 * [EndDoc] */

/*
 * [BeginDoc]
 * blockToList() transfers the data in blk at offset offset to num
 * items of size size.  These items are then placed in the list
 * given by lh.  What the items actually represent are application
 * dependent.
 * 
 * lh must be a list header that has been allocated (using a call
 * to initList(), for example).  The type of lh is changed to
 * UNSORTED to allow the blockToList() function to store the data in
 * the list in physical order (the order in which the data appears
 * in the block).  This function assumes that the data in the block
 * is not variable length.  The Links and data storage for the list
 * are dynamically allocated and must be freed by the user 
 * application.
 * 
 * blockToList() returns _ERROR_ on error, _OK_ if successful.
 * 
 * =================================================================
 * [EndDoc]
 */
int blockToList (ListHeader * lh, void *blk, size_t size,
		 size_t num, size_t offset)
{
  size_t i;
  void *bp;
  Link *lnk;

  Assert (lh != NULL);
  if (lh->type != UNSORTED)
    lh->type = UNSORTED;
  check_pointer (blk);
  for (i = 0; i < num; i++) {
    bp = malloc (size);
    if (0 == bp) {
      set_fioError(FIO_NOMEMORY);
      return _ERROR_;
    }
    memset (bp, 0, size);
    check_pointer (bp);

    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      set_fioError(FIO_NOMEMORY);
      return _ERROR_;
    }
    memset (lnk, 0, sizeof (Link));
    check_pointer (lnk);

    memcpy (bp, (void *) ((char *) blk + offset + i * size), size);
    check_pointer (bp);
    lnk->data = bp;

    lastLink (lh);
    insertLink (lh, lnk);
  }
  dbg2 (DebugCheckList (lh));
  return _OK_;
}

/*
 * [BeginDoc]
 * [Verbatim]

int listToBlock (ListHeader *lh, void *blk, size_t size,
				 size_t num, size_t offset);

 * [EndDoc] */

/*
 * [BeginDoc]
 * listToBlock() places the data in the list given by lh into the
 * block given by blk at offset offset.  The list is assumed to
 * store data in equal length chunks.  What is stored by the list
 * is application dependent.
 * 
 * The list will have num items of size size each after the
 * successful completion of this function.  The block must be a 
 * pointer to dynamically allocated storage that is large enough
 * to contain all the data.  Overrunning the edges of the block
 * will most likely result in **serious** problems (it's one of
 * those things that the ANSI standard likes to call "undefined, but
 * not good").
 * 
 * listToBlock() returns _ERROR_ on error, _OK_ on successful
 * completion.
 * 
 * =================================================================
 * [EndDoc]
 */
int listToBlock (ListHeader * lh, void *blk, size_t size,
		 size_t num, size_t offset)
{
  size_t i;
  Link *lnk;

  dbg2 (DebugCheckList (lh));
  check_pointer (blk);


  lnk = lh->head->next;
  if (lh->number != num) {
    set_fioError(FIO_UNSPECIFIED);
    return _ERROR_;
  }
  for (i = 0; i < num; i++) {
    Assert (lnk != lh->tail);
    check_pointer (blk);
    check_pointer (lnk->data);
    memcpy ((void *) ((char *) blk + offset + i * size), lnk->data, size);
    lnk = lnk->next;
  }
  check_pointer (blk);
  return _OK_;
}

/*
 * [BeginDoc]
 * 
 * To test the block module, compile block.c with TEST_BLOCK defined
 * and incorporate the other modules needed to generate a binary
 * executable (list and fsystem, at least).  A test main() function
 * is compiled which tests the functionality of the block module.
 * 
 * [EndDoc]
 */

#ifdef	TEST_BLOCK

#include <stdio.h>
#include <stdlib.h>

#define	KEY_SIZE		40
#define	TEST_BLOCK_SIZE	512
#define	BLOCK_OFFSET	(strlen(controlBlockTest)+1)

char *controlBlockTest = "Testing Control Block Functionality!!!";

typedef struct __test_stru {
  long item;
  char key[KEY_SIZE];
} testStru;

typedef struct _control_stru {
  long numItems;
  char descript[100];
} controlStru;

int main (int argc, char *argv[])
{
  FILE *fp;
  char *data;
  char *cp;
  int status;
  void *control;
  void *blk;
  long thisItem;
  long nextBlock = 1L;
  long thisBlock;
  testStru *ts;
  ListHeader *lh;
  int threshHold;
  Link *lnk = 0;
  int fd;
  controlStru *cs;
  int remainder;
  int firstTime = 0;

  /*
   * Create and populate the block file.
   */
  if (argc != 2) {
    printf
      ("\n\nUsage: block <file>\n\tWhere file is the file with lines of data\n");
    return 1;
  }
  control = malloc (TEST_BLOCK_SIZE);
  if (0 == control) {
    printf ("\n\n***Error: couldn't allocate a control block\n");
    return _ERROR_;
  }
  memset (control, 0, TEST_BLOCK_SIZE);
  check_pointer (control);
  cs = control;

  blk = malloc (TEST_BLOCK_SIZE);
  if (0 == blk) {
    printf ("\n\n***Error: couldn't allocate a block\n");
    return _ERROR_;
  }
  memset (blk, 0, TEST_BLOCK_SIZE);
  check_pointer (blk);

  strcpy (cs->descript, controlBlockTest);
  fd = createBlockFile ("testfile.blk", TEST_BLOCK_SIZE, control);
  if (_ERROR_ == fd) {
    printf ("\n\n***Error creationg block file, %s\n", fioErrMsg[fioError]);
    return _ERROR_;
  }

  fp = fopen (argv[1], "rb");
  if (0 == fp) {
    printf ("\n\n***Error: Couldn't open %s\n", argv[1]);
    return _ERROR_;
  }
  data = malloc (512);
  if (0 == data) {
    printf ("\n\n***Error: fatal memory error allocating data\n");
    return _ERROR_;
  }
  memset (data, 0, 512);
  check_pointer (data);

  lh = initList (UNSORTED, 0);
  if (0 == lh) {
    printf ("\n\n***Error: List error, %s\n", ListErrorString[ListError]);
    return _ERROR_;
  }

  thisItem = 0L;
  threshHold = ((TEST_BLOCK_SIZE - BLOCK_OFFSET) / sizeof (testStru));
  while (TRUE) {
    cp = fgets (data, KEY_SIZE, fp);
    if (feof (fp))
      break;
    if (cp == 0)
      break;
    if (data[0] == '\0')
      break;

    cp = strchr (data, '\n');
    if (0 != cp)
      *cp = '\0';

    ts = malloc (sizeof (testStru));
    if (0 == ts) {
      printf ("\n\n***Error: couldn't allocate a testStru\n");
      return _ERROR_;
    }
    memset (ts, 0, sizeof (testStru));
    check_pointer (ts);

    thisItem++;
    ts->item = thisItem;
    strcpy (ts->key, data);
    check_pointer (ts);

    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      printf ("\n\n***Error: couldn't allocate a Link\n");
      return _ERROR_;
    }
    memset (lnk, 0, sizeof (Link));
    check_pointer (lnk);

    lnk->data = ts;
    status = insertLink (lh, lnk);
    if (_ERROR_ == status) {
      printf ("\n\n***Error: %s\n", ListErrorString[ListError]);
      return _ERROR_;
    }
    if (lh->number >= threshHold) {
      thisBlock = appendFileBlock (fd, blk, TEST_BLOCK_SIZE);
      if (thisBlock != nextBlock) {
	printf ("\n\n***Error: Blocks out of sync\n");
	return _ERROR_;
      }
      memmove (blk, controlBlockTest, BLOCK_OFFSET);
      status = listToBlock (lh, blk, sizeof (testStru),
			    threshHold, BLOCK_OFFSET);
      if (_ERROR_ == status) {
	printf ("\n\n***Error: %s\n", fioErrMsg[fioError]);
	return _ERROR_;
      }
      status = putFileBlock (fd, blk, nextBlock, TEST_BLOCK_SIZE);
      if (_ERROR_ == status) {
	printf ("\n\n***Error: %s\n", fioErrMsg[fioError]);
	return _ERROR_;
      }
        if (lh == 0 || clearList (lh))
          return _ERROR_;
      nextBlock++;
    }
  }
  if (lh->number > 0) {
    remainder = lh->number;
    thisBlock = appendFileBlock (fd, blk, TEST_BLOCK_SIZE);
    if (thisBlock != nextBlock) {
      printf ("\n\n***Error: Blocks out of sync\n");
      return _ERROR_;
    }
    memmove (blk, controlBlockTest, BLOCK_OFFSET);
    status = listToBlock (lh, blk, sizeof (testStru),
			  remainder, BLOCK_OFFSET);
    if (_ERROR_ == status) {
      printf ("\n\n***Error: %s\n", fioErrMsg[fioError]);
      return _ERROR_;
    }
    status = putFileBlock (fd, blk, nextBlock, TEST_BLOCK_SIZE);
    if (_ERROR_ == status) {
      printf ("\n\n***Error: %s\n", fioErrMsg[fioError]);
      return _ERROR_;
    }
    if (lh == 0 || clearList (lh))
      return _ERROR_;
  }
  else {
    nextBlock--;
    remainder = 0;
  }
  cs->numItems = nextBlock;
  status = closeBlockFile (fd, TEST_BLOCK_SIZE, control);
  if (_ERROR_ == status) {
    printf ("\n\n***Error closing block file, %s\n", fioErrMsg[fioError]);
    return _ERROR_;
  }
  fclose (fp);

  /*
   * Now, reopen the data file and print the contents of each item
   */
  nextBlock = 1L;
  fd = openBlockFile ("testfile.blk", TEST_BLOCK_SIZE, control);
  if (_ERROR_ == fd) {
    printf ("\n\n***Error reopening block file, %s\n", fioErrMsg[fioError]);
    return _ERROR_;
  }
  while (TRUE) {
    if (firstTime != 0)
      printf ("\n");
    firstTime = 1;
    if (lh->number == 0) {
      if (nextBlock > cs->numItems)
	break;
      status = getFileBlock (fd, blk, nextBlock, TEST_BLOCK_SIZE);
      if (status == _ERROR_) {
	printf ("\n\n***Error getting file block, %s\n", fioErrMsg[fioError]);
	return _ERROR_;
      }
      if (nextBlock == cs->numItems && remainder != 0)
	status = blockToList (lh, blk, sizeof (testStru),
			      remainder, BLOCK_OFFSET);
      else
	status = blockToList (lh, blk, sizeof (testStru),
			      threshHold, BLOCK_OFFSET);
      if (_ERROR_ == status) {
	printf ("\n\n***Error, blockToList, %s\n", fioErrMsg[fioError]);
	return _ERROR_;
      }
      lnk = lh->head->next;
      nextBlock++;
    }
    if (lnk == lh->tail) {
      if (lh == 0 || clearList (lh))
        return _ERROR_;
      firstTime = 0;
      continue;
    }
    ts = lnk->data;
    printf ("%s", ts->key);
    lnk = lnk->next;
  }

  status = closeBlockFile (fd, TEST_BLOCK_SIZE, control);
  if (_ERROR_ == status) {
    printf ("\n\n***Error closing block file, %s\n", fioErrMsg[fioError]);
    return _ERROR_;
  }

  free (control);
  free (blk);
  free (data);
  if (lh == 0 || clearList (lh))
    return _ERROR_;
  delList (lh);
  print_block_list ();
  return _OK_;
}

#endif /* TEST_BLOCK */

endC
