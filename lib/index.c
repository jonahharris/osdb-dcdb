/*	Source File:	index.c	*/

#include <index.h>

#include <ctype.h>
#include <time.h>
#ifndef	WIN32
#include <unistd.h>
#endif

#ifdef	__cplusplus
externC
#endif
  idxErrorType idxError;

char *idxErrMsg[] = {
  "no error",			/* IDX_NOERR */
  "low-level file system error",	/* IDX_FSYSTEM */
  "fatal memory error",		/* IDX_NOMEMORY */
  "list error",			/* IDX_LIST */
  "shell error",		/* IDX_SHELL */
  "invalid or corrupted index file",	/* IDX_FTYPE */
  "memory data items corrupted",	/* IDX_UNSTABLE */
  "item size is too large",	/* IDX_ITEMS */
  "unique constraint violated",	/* IDX_UNIQUE */
  "sequential access not initiated yet",	/* IDX_SEQUENCE */
  "incorrect isUnique value",	/* IDX_DUP */
  "index pair does not exist",	/* IDX_NEXISTS */
  /* put new ones here */
  "unspecified error",		/* IDX_UNSPECIFIED */
  0
};

#ifdef	DEBUG
#include <stdio.h>
void DebugDumpBlock (idxBlock * blk);
#endif

/*
 * Internal function
 */
#define HASH_STR(x)  (((x)[0]<<15)+((x)[1]<<10)+((x)[2]<<5)+(x)[3])
#define HASH_STR_NC(x)  ((tolower((x)[0])<<15)+(tolower((x)[1])<<10)+(tolower((x)[2])<<5)+tolower((x)[3]))
int caseCompare (const void *p1, const void *p2)
{
  unsigned int v1 = HASH_STR(((keyItem*)p1)->key),
               v2 = HASH_STR(((keyItem*)p2)->key);
  if (v1 < v2)
    return -1;
  else if (v1 > v2)
    return 1;
  else
    return (strcmp (((keyItem *) p1)->key, ((keyItem *) p2)->key));
}

/*
 * Internal function
 */
int noCaseCompare (const void *p1, const void *p2)
{
  unsigned int v1 = HASH_STR_NC(((keyItem*)p1)->key),
               v2 = HASH_STR_NC(((keyItem*)p2)->key);
  if (v1 < v2)
    return -1;
  else if (v1 > v2)
    return 1;
  else
    return (strcasecmp (((keyItem *) p1)->key, ((keyItem *) p2)->key));
}

/*
 * [BeginDoc]
 * [Verbatim] */

int createIndexFile (char *name, int blockSize, int fldsize,
		     int isUnique, int isCase)

 /* [EndDoc]            */
/*
 * [BeginDoc]
 * 
 * createIndexFile() creates the block file for an index.  The ``name''
 * parameter is the name of the block file.  ``blockSize'' is the
 * block size for the block file.  ``fldsize'' is the size of the
 * keys to be stored in the index.  The itemSize for the index will
 * be calculated by adding the size of a 32-bit integer to the
 * fldsize.  ``isUnique'' is a boolean parameter which indicates
 * whether the index is to enforce uniqueness on the data it stores
 * (isUnique is TRUE) or not (isUnique is FALSE).  ``isCase'' will
 * determine whether string data is stored in a case dependent way
 * or not.  if isCase is TRUE, case is important in sorting the
 * data items.  If isCase is FALSE, case is not used to determine
 * the order of data items.
 * 
 * createIndexFile() creates the index file and then closes it before
 * it returns.  The user must call openIndex() after the file has
 * been created before the index can be used.  createIndexFile()
 * returns _OK_ on success, _ERROR_ on error.  If an error occurs,
 * idxErrMsg[idxError] points to a read-only string that describes
 * the error.
 * 
 * =================================================================
 * [EndDoc]
 */
{
  dbIndex *idx;
  indexHeader *hdr;
  int status;

  /*
   * Allocate the necessary structures
   */
  dbg2 (printf ("\n\ncreateIndexFile(%s, ...)\n", name);
    );
  if (!blockSize)
    blockSize = DEFAULT_BLOCK_SIZE;
  if ((fldsize + (int)sizeof (unsigned long)) * MIN_ITEMS_PER_BLOCK > blockSize) {
    idxError = IDX_ITEMS;
    return _ERROR_;
  }
  idx = malloc (sizeof (dbIndex));
  if (0 == idx) {
    idxError = IDX_NOMEMORY;
    return _ERROR_;
  }
  memset (idx, 0, sizeof (dbIndex));
  check_pointer (idx);

  idx->ctrl = malloc (blockSize);
  if (0 == idx->ctrl) {
    free (idx);
    idxError = IDX_NOMEMORY;
    return _ERROR_;
  }
  memset (idx->ctrl, 0, blockSize);
  check_pointer (idx->ctrl);

  hdr = idx->hdr = idx->ctrl;
  hdr->magic = IDX_MAGIC;
  hdr->blockSize = blockSize;
  hdr->lastBlock = 0L;
  hdr->isUnique = isUnique;
  hdr->isCase = isCase;
  hdr->itemSize = fldsize + sizeof (unsigned long);

  idx->fd = createBlockFile (name, blockSize, idx->ctrl);
  if (idx->fd == _ERROR_) {
    free (idx->ctrl);
    free (idx);
    idxError = IDX_FSYSTEM;
    return _ERROR_;
  }

  status = closeBlockFile (idx->fd, blockSize, idx->ctrl);
  if (status == _ERROR_) {
    free (idx->ctrl);
    free (idx);
    idxError = IDX_FSYSTEM;
    return _ERROR_;
  }
  free (idx->ctrl);
  free (idx);
  dbg2 (printf ("\n\ncreateIndexFile() end\n");
    );
  return _OK_;
}

/*
 * [BeginDoc]
 * [Verbatim]

dbIndex *openIndex (char *idxName, char *inxName, int blockSize);
 * [EndDoc] */

/*
 * [BeginDoc]
 * 
 * openIndex() opens an index file that was previously created with
 * createIndexFile().  ``idxName'' is the name of the block file to
 * open.  ``inxName'' is the name of the index into the block file.
 * The index into the block file is an in memory tree structure;
 * inxName should be the name of the file that the in-memory tree
 * was stored in.  (For more information on storing trees, see the
 * documentation for the list module).  If inxName is NULL, it is
 * assumed that the index is newly created and there are currently
 * no key/item pairs stored in the block file.  If the user calls
 * the openIndex() function with a NULL for inxName and there are
 * key/items stored in the block file, index corruption and possibly
 * run-time faults ***WILL*** occur.
 * 
 * On success, openIndex() returns a pointer to an allocated 
 * dbIndex structure.  On error, openIndex() returns NULL.  The
 * dbIndex structure returned by a successful call to openIndex() 
 * will be used by all the remainder of the index functions for
 * managing the in-memory index information.
 * 
 * =================================================================
 * [EndDoc]
 */
dbIndex *openIndex (char *idxName, char *inxName, int blockSize)
{
  dbIndex *idx;
  indexHeader *hdr;
  Link *lnk;
  idxBlock *blk;
  treeStore *ts;
  int i;
  int status;

  dbg2 (printf ("\n\nopenIndex(%s,%s, ...)\n", idxName, inxName);
    );
  if (blockSize == 0)
    blockSize = DEFAULT_BLOCK_SIZE;
  idx = malloc (sizeof (dbIndex));
  if (0 == idx) {
    idxError = IDX_NOMEMORY;
    return 0;
  }
  memset (idx, 0, sizeof (dbIndex));
  check_pointer (idx);

  idx->ctrl = malloc (blockSize);
  if (0 == idx->ctrl) {
    free (idx);
    idxError = IDX_NOMEMORY;
    return 0;
  }
  memset (idx->ctrl, 0, blockSize);
  check_pointer (idx->ctrl);

  hdr = idx->hdr = idx->ctrl;

  /*
   * Open the file and read the data in.
   */
  idx->fd = openBlockFile (idxName, blockSize, idx->ctrl);
  if (_ERROR_ == idx->fd) {
    free (idx->ctrl);
    free (idx);
    idxError = IDX_FSYSTEM;
    return 0;
  }
  if (idx->hdr->magic != IDX_MAGIC) {
    idxError = IDX_FTYPE;
    free (idx->ctrl);
    free (idx);
    return 0;
  }
  if (inxName != 0) {
    idx->inx = malloc (sizeof (shellHeader));
    if (idx->inx == 0) {
      idxError = IDX_NOMEMORY;
      return 0;
    }
    memset (idx->inx, 0, sizeof (shellHeader));
    check_pointer (idx->inx);
    if (idx->hdr->isCase)
      idx->inx->compare = (int (*)(void *, void *)) caseCompare;
    else
      idx->inx->compare = (int (*)(void *, void *)) noCaseCompare;
    ts = malloc (sizeof (treeStore));
    if (0 == ts) {
      idxError = IDX_NOMEMORY;
      free (idx->ctrl);
      free (idx);
      return 0;
    }
    memset (ts, 0, sizeof (treeStore));
    check_pointer (ts);
    status = retrieveShell (idx->inx, ts, inxName, IDX_MAGIC);
    if (_ERROR_ == status) {
      idxError = IDX_SHELL;
      free (idx->ctrl);
      free (idx);
      free (ts);
      return 0;
    }
    free (ts);
  }
  idx->current = 0;

  /*
   * Set up the block queues
   */
  idx->free = initList (QUEUED, NULL);
  if (0 == idx->free) {
    idxError = IDX_LIST;
    free (idx->ctrl);
    free (idx);
    return 0;
  }

  for (i = 0; i < BLOCK_CACHE_SIZE; i++) {
    blk = malloc (sizeof (idxBlock));
    if (0 == blk) {
      idxError = IDX_NOMEMORY;
      free (idx->ctrl);
      free (idx);
      delList (idx->free);
      return 0;
    }
    memset (blk, 0, sizeof (idxBlock));
    check_pointer (blk);

    blk->blk = malloc (blockSize);
    if (0 == blk->blk) {
      idxError = IDX_NOMEMORY;
      free (idx->ctrl);
      free (idx);
      free (blk);
      delList (idx->free);
      return 0;
    }
    memset (blk->blk, 0, blockSize);
    check_pointer (blk->blk);

    blk->bhd = blk->blk;
    blk->isDirty = FALSE;

    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      idxError = IDX_NOMEMORY;
      free (idx->ctrl);
      free (idx);
      free (blk->blk);
      free (blk);
      delList (idx->free);
      return 0;
    }
    memset (lnk, 0, sizeof (Link));
    check_pointer (lnk);

    if (idx->hdr->isCase) {
      if (idx->hdr->isUnique)
	blk->keyItemList = initList (SORTED, caseCompare);
      else
	blk->keyItemList = initList (SLOWSORTED, caseCompare);
    }
    else {
      if (idx->hdr->isUnique)
	blk->keyItemList = initList (SORTED, noCaseCompare);
      else
	blk->keyItemList = initList (SLOWSORTED, noCaseCompare);
    }
    if (blk->keyItemList == 0) {
      idxError = IDX_LIST;
      return 0;
    }

    lnk->data = blk;
    insertLink (idx->free, lnk);
  }
  Assert (idx->free->number == BLOCK_CACHE_SIZE);
  idx->used = initList (QUEUED, NULL);
  if (0 == idx->used) {
    idxError = IDX_LIST;
    return 0;
  }

  dbg2 (printf ("\n\nopenIndex() end\n");
    );
  return idx;
}

/*
 * [BeginDoc]
 * [Verbatim]

int closeIndexFile (dbIndex *idx, char *inxName);
 * [EndDoc] */

/*
 * [BeginDoc]
 * 
 * closeIndexFile() flushes all cached blocks to disk, if necessary,
 * and closes the index file.  If data has been stored to the index,
 * closeIndexFile() will also save the in-memory index into the
 * block file in a file called ``inxName''.  After successful
 * completion of closeIndexFile(), the memory idx points to will be
 * invalid (released back to the system).  Any access of this memory
 * will be (as the ANSI standard says) ``undefined'', which means
 * that the actual outcome is system dependent but it will not be
 * pretty.
 * 
 * closeIndexFile() returns _OK_ on success, _ERROR_ if an error
 * occurred.
 * 
 * =================================================================
 * [EndDoc]
 */
int closeIndexFile (dbIndex * idx, char *inxName)
{
  Link *lnk;
  treeStore *ts;
  idxBlock *blk;
  time_t now;
  int status;

  dbg2 (printf ("\n\ncloseIndexFile(idx, %s)\n", inxName);
    );
  Assert (idx->hdr == idx->ctrl);
  flushIndexBlocks (idx);
  fileFlush (idx->fd);
  Assert (!(idx->inx != 0 && inxName == 0));
  if (idx->inx != 0 && inxName == 0) {
    idxError = IDX_FTYPE;
    return _ERROR_;
  }
  if (idx->inx != 0 && inxName != 0) {
    ts = malloc (sizeof (treeStore));
    if (0 == ts) {
      idxError = IDX_NOMEMORY;
      return _ERROR_;
    }
    memset (ts, 0, sizeof (treeStore));
    check_pointer (ts);
    ts->thisMagic = IDX_MAGIC;
    time (&now);
    strcpy (ts->timeStamp, asctime (localtime (&now)));
    strcpy (ts->description, inxName);
    status = storeShell (idx->inx, ts, idx->hdr->itemSize, inxName);
    Assert (status != _ERROR_);
    if (status == _ERROR_) {
      idxError = IDX_SHELL;
      return _ERROR_;
    }
    free (ts);
    if (!idx->inx->manageAllocs)
      idx->inx->manageAllocs = TRUE;
    delShell (idx->inx, 0);
    idx->inx = 0;
  }
  lnk = removeLink (idx->free);
  while (lnk != 0) {
    blk = lnk->data;
    if (blk->keyItemList == 0 || clearList (blk->keyItemList)) return _ERROR_;
    delList (blk->keyItemList);
    check_pointer (blk->blk);
    free (blk->blk);
    check_pointer (lnk->data);
    free (lnk->data);
    check_pointer (lnk);
    free (lnk);
    lnk = removeLink (idx->free);
  }
  delList (idx->free);
  if (idx->used != 0) {
    lnk = removeLink (idx->used);
    while (lnk != 0) {
      blk = lnk->data;
      if (blk->keyItemList == 0 || clearList (blk->keyItemList)) return _ERROR_;
      delList (blk->keyItemList);
      check_pointer (blk->blk);
      free (blk->blk);
      check_pointer (lnk->data);
      free (lnk->data);
      check_pointer (lnk);
      free (lnk);
      lnk = removeLink (idx->used);
    }
    delList (idx->used);
  }
  dbg2 (printf ("\n\ncloseIndexFile(), at closeBlockFile() statement\n");
    );
  closeBlockFile (idx->fd, idx->hdr->blockSize, idx->ctrl);
  free (idx->ctrl);
  free (idx);
  dbg2 (printf ("\n\ncloseIndexFile() end\n");
    );
  return _OK_;
}

/*
 * [BeginDoc]
 * [Verbatim]

int addIndexItem (dbIndex *idx, keyItem *ki);
 * [EndDoc] */

/*
 * [BeginDoc]
 * 
 * addIndexItem() adds the key/item pair given by ``ki'' to the
 * index given by ``idx'', if possible.  ki should point to an
 * allocated block of size itemSize as stored in the indexHeader
 * for the index.  It is assumed to be populated with valid data
 * by the caller.  addIndexItem() will attempt to add the key/item
 * pair to the ordered index.  If it succeeds, it returns _OK_;
 * _ERROR_ is returned on error.
 * 
 * =================================================================
 * [EndDoc]
 */
int addIndexItem (dbIndex * idx, keyItem * ki)
{
  Link *lnk;
  idxBlock *blk, *newBlock;
  keyItem *treeKey;
  Link *found;
  int status;
  off_t blkNum;

  if (idx->inx == 0) {
    if (idx->hdr->isCase)
      idx->inx = initShell ((int (*)(void *, void *)) caseCompare,
			    idx->hdr->isUnique, TRUE);
    else
      idx->inx = initShell ((int (*)(void *, void *)) noCaseCompare,
			    idx->hdr->isUnique, TRUE);
    if (idx->inx == 0) {
      idxError = IDX_SHELL;
      return _ERROR_;
    }
  }
  if (idx->inx->lh->number == 0) {
    blk = getNewBlock (idx, idx->hdr->lastBlock + 1);
    if (blk == 0) {
      /* idxError set */
      return _ERROR_;
    }
    blk->blockNumber = idx->hdr->lastBlock + 1;
    idx->hdr->lastBlock++;

    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      idxError = IDX_NOMEMORY;
      return _ERROR_;
    }
    memset (lnk, 0, sizeof (Link));
    check_pointer (lnk);

    lnk->data = ki;
    status = insertLink (blk->keyItemList, lnk);
    if (_ERROR_ == status) {
      idxError = IDX_LIST;
      return _ERROR_;
    }

    /*
     * Update the block members
     */
    blk->bhd->numKeyItems = 1;
    blk->bhd->prevBlock = blk->bhd->nextBlock = 0L;
    blk->isDirty = TRUE;
    if (idx->hdr->itemSize != 0)
      blk->numMaxItems = (idx->hdr->blockSize - sizeof (blockHeader)) /
	                       idx->hdr->itemSize;
    else {
      idxError = IDX_UNSTABLE;
      return _ERROR_;
    }
    if (blk->numMaxItems < MIN_ITEMS_PER_BLOCK) {
      idxError = IDX_ITEMS;
      return _ERROR_;
    }
    blk->itemSize = idx->hdr->itemSize;
    blk->currentSize = idx->hdr->itemSize;
    blk->firstItem = ki;

    /*
     * Now, insert the block into the tree
     */
    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      idxError = IDX_NOMEMORY;
      return _ERROR_;
    }
    memset (lnk, 0, sizeof (Link));
    check_pointer (lnk);

    treeKey = malloc (blk->itemSize);
    if (0 == treeKey) {
      idxError = IDX_NOMEMORY;
      return _ERROR_;
    }
    memset (treeKey, 0, blk->itemSize);
    check_pointer (treeKey);
    memmove (treeKey->key, ki->key, idx->hdr->itemSize - sizeof (unsigned long));
    check_pointer (treeKey);
    treeKey->item = blk->blockNumber;
    check_pointer (treeKey);
    lnk->data = treeKey;
    idx->inx->lh->current = idx->inx->lh->head;
    status = insertLinkHere (idx->inx->lh, lnk);
    if (_ERROR_ == status) {
      idxError = IDX_LIST;
      return _ERROR_;
    }
    idx->hdr->numItems++;
    return _OK_;
  }

  /*
   * There are blocks being indexed by the shell already.
   * 
   * First, make sure the item being entered is not less than
   * everything in the tree.  If so, add it to the first block
   * and change the first block index.
   */
  found = idx->inx->lh->head->next;
  check_pointer (ki);
  check_pointer (found->data);
  status = idx->inx->compare (ki, found->data);
  if (status < 0) {
    /*
     * Add this guy to the first block and update
     * blk->firstItem for the first block.
     */
    treeKey = found->data;
    blkNum = treeKey->item;
    blk = getIndexBlock (idx, blkNum);
    if (0 == blk) {
      /* idxError set */
      return _ERROR_;
    }
    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      idxError = IDX_NOMEMORY;
      return _ERROR_;
    }
    memset (lnk, 0, sizeof (Link));
    check_pointer (lnk);
    lnk->data = ki;
    status = insertLink (blk->keyItemList, lnk);
    if (status == _ERROR_) {
      idxError = IDX_LIST;
      return _ERROR_;
    }
    blk->firstItem = ki;
    check_pointer (blk->firstItem);
    memmove (treeKey->key, ki->key, blk->itemSize - sizeof (keyItem));
    idx->hdr->numItems++;
    if (blk->keyItemList->number > (size_t)blk->numMaxItems) {
      /*
       * Split the block.
       */
      newBlock = splitBlock (idx, blk);
      Assert (newBlock->bhd->prevBlock == blk->blockNumber);
      Assert (blk->bhd->nextBlock == newBlock->blockNumber);
      if (newBlock == 0) {
	/* idxError set */
	return _ERROR_;
      }
      treeKey = malloc (blk->itemSize);
      if (0 == treeKey) {
	idxError = IDX_NOMEMORY;
	return _ERROR_;
      }
      memset (treeKey, 0, blk->itemSize);
      check_pointer (treeKey);
      memmove (treeKey->key, newBlock->firstItem->key,
	       blk->itemSize - sizeof (unsigned long));
      check_pointer (treeKey);
      treeKey->item = newBlock->blockNumber;
      lnk = malloc (sizeof (Link));
      if (0 == lnk) {
	idxError = IDX_NOMEMORY;
	return _ERROR_;
      }
      memset (lnk, 0, sizeof (Link));
      check_pointer (lnk);
      lnk->data = treeKey;
      idx->inx->lh->current = found;
      status = insertLinkHere (idx->inx->lh, lnk);
      if (status == _ERROR_) {
	idxError = IDX_SHELL;
	return _ERROR_;
      }
      if (idx->inx->lh->number % SHELL_THRESHOLD == 0) {
	status = restructureShellNodes (idx->inx);
	if (status == _ERROR_) {
	  idxError = IDX_SHELL;
	  return _ERROR_;
	}
      }
    }
    return _OK_;
  }

  /*
   * Find the block to add the item to.
   */
  lnk = malloc (sizeof (Link));
  if (0 == lnk) {
    idxError = IDX_NOMEMORY;
    return _ERROR_;
  }
  memset (lnk, 0, sizeof (Link));
  check_pointer (lnk);

  lnk->data = ki;
  if (idx->hdr->isUnique) {
    if (searchExactIndexItem (idx, lnk)) {
      idxError = IDX_UNIQUE;
      return _ERROR_;
    }
    if (idxError != IDX_NOERR)
      return _ERROR_;
  }
  found = queryShellItem (idx->inx, lnk);
  if (0 == found) {
    idxError = IDX_UNSTABLE;
    return _ERROR_;
  }
  treeKey = found->data;
  blk = getIndexBlock (idx, treeKey->item);
  if (blk == 0) {
    /* idxError set */
    return _ERROR_;
  }
  check_pointer (blk);
  check_pointer (blk->blk);
  check_pointer (blk->keyItemList);
  status = insertLink (blk->keyItemList, lnk);
  if (status == _ERROR_) {
    idxError = IDX_LIST;
    return _ERROR_;
  }
  idx->hdr->numItems++;
  lnk = blk->keyItemList->head->next;
  check_pointer (lnk->data);
  blk->firstItem = lnk->data;
  if (blk->keyItemList->number > (size_t)blk->numMaxItems) {
    /*
     * Split the block.
     */
    newBlock = splitBlock (idx, blk);
    if (newBlock == 0) {
      /* idxError set */
      return _ERROR_;
    }
    treeKey = malloc (blk->itemSize);
    if (0 == treeKey) {
      idxError = IDX_NOMEMORY;
      return _ERROR_;
    }
    memset (treeKey, 0, blk->itemSize);
    check_pointer (treeKey);
    memmove (treeKey->key, newBlock->firstItem->key,
	     blk->itemSize - (sizeof (unsigned long)));
    check_pointer (treeKey);
    treeKey->item = newBlock->blockNumber;
    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      idxError = IDX_NOMEMORY;
      return _ERROR_;
    }
    memset (lnk, 0, sizeof (Link));
    check_pointer (lnk);
    lnk->data = treeKey;
    idx->inx->lh->current = found;
    status = insertLinkHere (idx->inx->lh, lnk);
    /* status = addShellItem (idx->inx, lnk); */
    if (status == _ERROR_) {
      idxError = IDX_LIST;
      return _ERROR_;
    }
    if (idx->inx->lh->number % SHELL_THRESHOLD == 0) {
      status = restructureShellNodes (idx->inx);
      if (status == _ERROR_) {
	idxError = IDX_SHELL;
	return _ERROR_;
      }
    }
  }
  return _OK_;
}

keyItem *deleteIndexItem (dbIndex * idx, keyItem * ki)
{
  Link *lnk, *tmp;
  idxBlock *blk, *prev, *next;
  keyItem *treeKey, *treeKey2;
  Link *found;
  int status;
  int fixPointers = FALSE;

  Assert (ki != 0);
  check_pointer (ki);
  /*
   * Find the block to add the item to.
   */
  lnk = malloc (sizeof (Link));
  if (0 == lnk) {
    idxError = IDX_NOMEMORY;
    return 0;
  }
  memset (lnk, 0, sizeof (Link));
  check_pointer (lnk);
  lnk->data = ki;

  /* start with the first one */
  check_pointer (idx);
  check_pointer (idx->inx);
  check_pointer (idx->inx->lh);
  check_pointer (idx->inx->lh->head);
  check_pointer (idx->inx->lh->head->next);
  found = idx->inx->lh->head->next;
  check_pointer (lnk->data);
  check_pointer (found->data);
  status = idx->inx->compare (lnk->data, found->data);
  if (status == 0)
    goto continueHere;
  found = queryShellItem (idx->inx, lnk);
  if (idx->inx->shlError != SHELL_NOERR) {
    // Set the global variable for error handling and clear the local one.
    set_shlError (0, idx->inx->shlError);
    set_shlError (idx->inx,SHELL_NOERR);
    idxError = IDX_SHELL;
    free (lnk);
    return 0;
  }
  if (0 == found) {
    idxError = IDX_UNSTABLE;
    free (lnk);
    return 0;
  }
  check_pointer (lnk->data);
  check_pointer (found->data);
  status = idx->inx->compare (lnk->data, found->data);
  if (!idx->hdr->isUnique && status == 0 && found->prev != idx->inx->lh->head) {
    /* not a unique index and the shell may have more like this */
    /*
     * search the previous block first
     */
    if (found->prev != idx->inx->lh->head)
      tmp = found->prev;
    else
      goto continueHere;
    check_pointer (found);
    Assert (found != idx->inx->lh->head);
    check_pointer (tmp);
    check_pointer (tmp->data);
    treeKey = tmp->data;
    blk = getIndexBlock (idx, treeKey->item);
    if (0 == blk) {
      idxError = IDX_UNSTABLE;
      free (lnk);
      return 0;
    }
    check_pointer (blk);
    check_pointer (blk->keyItemList->head);
    check_pointer (blk->keyItemList->tail);
    if (blk->keyItemList->head->next == blk->keyItemList->tail)
      goto continueHere;
    tmp = blk->keyItemList->head->next;
    Assert (tmp != blk->keyItemList->tail);
    check_pointer (lnk->data);
    check_pointer (tmp->data);
    check_pointer (tmp->next);
    check_pointer (lnk->data);
    status = idx->inx->compare (tmp->data, lnk->data);
    while (status != 0) {
      tmp = tmp->next;
      if (tmp == blk->keyItemList->tail)
	goto continueHere;
      check_pointer (tmp->data);
      check_pointer (lnk->data);
      status = idx->inx->compare (tmp->data, lnk->data);
    }
    while (status == 0 && tmp != blk->keyItemList->tail) {
      check_pointer (tmp->data);
      check_pointer (tmp->next);
      treeKey = tmp->data;
      if (treeKey->item == ki->item) {
	if (tmp != blk->keyItemList->tail)
	  blk->current = tmp->next;
	check_pointer (blk->current);
	check_pointer (blk->current->next);
	Assert (tmp != blk->keyItemList->tail);
	blk->keyItemList->current = tmp;
	tmp = removeLink (blk->keyItemList);
	if (0 == tmp) {
	  idxError = IDX_LIST;
	  free (lnk);
	  return 0;
	}
	/*blk->keyItemList->current = blk->current; */
	blk->bhd->numKeyItems--;
	if ((size_t)blk->bhd->numKeyItems != blk->keyItemList->number) {
	  idxError = IDX_UNSTABLE;
	  free (lnk);
	  return 0;
	}
	blk->isDirty = TRUE;
	check_pointer (tmp);
	check_pointer (tmp->data);
	if (blk->keyItemList->number == 0) {
	  if (blk->bhd->numKeyItems != 0) {
	    idxError = IDX_UNSTABLE;
	    free (lnk);
	    return 0;
	  }
	  /* the block is empty; remove it */
	  status = putFileBlock (idx->fd, blk->blk, blk->blockNumber,
				 idx->hdr->blockSize);
	  if (_ERROR_ == status) {
	    free (lnk);
	    return 0;
	  }
	  /*
	   * unlink it from the list and restructure the shell nodes.
	   */
	  idx->inx->lh->current = found;
	  found = removeLink (idx->inx->lh);
	  if (found == 0) {
	    idxError = IDX_LIST;
	    free (lnk);
	    return 0;
	  }
	  restructureShellNodes (idx->inx);
	  /*
	   * Now, insert the link into the free list and relink the blocks.
	   */
	  prev = getIndexBlock (idx, blk->bhd->prevBlock);
	  if (prev == 0) {
	    idxError = IDX_UNSPECIFIED;
	    free (lnk);
	    return 0;
	  }
	  next = getIndexBlock (idx, blk->bhd->nextBlock);
	  if (next == 0) {
	    idxError = IDX_UNSPECIFIED;
	    free (lnk);
	    return 0;
	  }
	  prev->bhd->nextBlock = next->blockNumber;
	  next->bhd->prevBlock = prev->blockNumber;
	  status = putFileBlock (idx->fd, prev->blk, prev->blockNumber,
				 idx->hdr->blockSize);
	  if (_ERROR_ == status) {
	    free (lnk);
	    return 0;
	  }
	  status = putFileBlock (idx->fd, next->blk, next->blockNumber,
				 idx->hdr->blockSize);
	  if (_ERROR_ == status) {
	    free (lnk);
	    return 0;
	  }
	  idx->current = next;
	  /*
	   * OK, block is unlinked...add it to the free list.
	   */
	  check_pointer (found);
	  check_pointer (found->data);
	  if (found->data != 0) {
	    free (found->data);
	    found->data = 0;
	  }
	  if (found != 0) {
	    free (found);
	    found = 0;
	  }
	  found = idx->used->tail->prev;
	  while (found != idx->used->head) {
	    if (((idxBlock *) lnk->data)->blockNumber == blk->blockNumber) {
	      idx->used->type = UNSORTED;
	      idx->used->current = found;
	      found = removeLink (idx->used);
	      idx->used->type = QUEUED;
	      break;
	    }
	    found = found->prev;
	  }
	  if (found != idx->used->head)
	    insertLink (idx->free, found);
	  free (lnk);
	  check_pointer (found);
	  check_pointer (found->data);
	}
	else {
	  idx->current = blk;
	  free (lnk);
	}
	treeKey = tmp->data;
	free (tmp);
	idx->hdr->numItems--;
	return treeKey;
      }
      tmp = tmp->next;
    }
  }
  /*
   * When we get here, the following should be true:
   * lnk->data == ki
   * found == a valid lnk in idx->inx->lh
   */
  Assert (lnk->data == ki);
  check_pointer (found);
  check_pointer (found->data);

continueHere:

  while (TRUE) {
    treeKey = found->data;
    blk = getIndexBlock (idx, treeKey->item);
    if (0 == blk) {
      idxError = IDX_UNSTABLE;
      free (lnk);
      return 0;
    }
    check_pointer (blk->keyItemList);
    check_pointer (blk->keyItemList->head->next);
    tmp = blk->keyItemList->head->next;
    if (tmp == blk->keyItemList->tail) {
      found = found->next;
      check_pointer (found);
      check_pointer (found->data);
      if (found == idx->inx->lh->tail) {
	free (lnk);
	idxError = IDX_NEXISTS;
	return 0;
      }
      continue;
    }
    check_pointer (lnk->data);
    check_pointer (tmp->data);
    check_pointer (tmp->next);
    if (!idx->inx->compare (lnk->data, tmp->data)) {
      /* first one matches */
      check_pointer (tmp);
      check_pointer (tmp->data);
      check_pointer (tmp->next);
      treeKey = tmp->data;
      if (treeKey->item == ki->item) {
	/* special case...have to possibly update the first record key */
	if (tmp->next != blk->keyItemList->tail)
	  fixPointers = TRUE;
	else
	  fixPointers = FALSE;
	if (tmp != blk->keyItemList->tail)
	  blk->current = tmp->next;
	check_pointer (blk->current);
	Assert (tmp != blk->keyItemList->tail);
	blk->keyItemList->current = tmp;
	tmp = removeLink (blk->keyItemList);
	if (0 == tmp) {
	  idxError = IDX_LIST;
	  free (lnk);
	  return 0;
	}
	/*blk->keyItemList->current = blk->current; */
	blk->bhd->numKeyItems--;
	if ((size_t)blk->bhd->numKeyItems != blk->keyItemList->number) {
	  /*idxError = IDX_UNSTABLE;
	     free (lnk);
	     return 0; */
	  blk->bhd->numKeyItems = blk->keyItemList->number;
	}
	blk->isDirty = TRUE;
	check_pointer (tmp);
	check_pointer (tmp->data);
	if (blk->keyItemList->number == 0) {
	  /* the block is empty; remove it */
	  fixPointers = FALSE;
	  status = putFileBlock (idx->fd, blk->blk, blk->blockNumber,
				 idx->hdr->blockSize);
	  if (_ERROR_ == status) {
	    free (lnk);
	    return 0;
	  }
	  idx->inx->lh->current = found;
	  found = removeLink (idx->inx->lh);
	  if (found == 0) {
	    idxError = IDX_LIST;
	    free (lnk);
	    return 0;
	  }
	  restructureShellNodes (idx->inx);
	  /*
	   * Now, insert the link into the free list and relink the blocks.
	   */
	  prev = getIndexBlock (idx, blk->bhd->prevBlock);
	  if (prev == 0) {
	    idxError = IDX_UNSPECIFIED;
	    free (lnk);
	    return 0;
	  }
	  next = getIndexBlock (idx, blk->bhd->nextBlock);
	  if (next == 0) {
	    idxError = IDX_UNSPECIFIED;
	    free (lnk);
	    return 0;
	  }
	  prev->bhd->nextBlock = next->blockNumber;
	  next->bhd->prevBlock = prev->blockNumber;
	  status = putFileBlock (idx->fd, prev->blk, prev->blockNumber,
				 idx->hdr->blockSize);
	  if (_ERROR_ == status) {
	    free (lnk);
	    return 0;
	  }
	  status = putFileBlock (idx->fd, next->blk, next->blockNumber,
				 idx->hdr->blockSize);
	  if (_ERROR_ == status) {
	    free (lnk);
	    return 0;
	  }
	  idx->current = next;
	  check_pointer (found);
	  check_pointer (found->data);
	  if (found->data != 0) {
	    free (found->data);
	    found->data = 0;
	  }
	  if (found != 0) {
	    free (found);
	    found = 0;
	  }
	  found = idx->used->tail->prev;
	  while (found != idx->used->head) {
	    if (((idxBlock *) lnk->data)->blockNumber == blk->blockNumber) {
	      idx->used->type = UNSORTED;
	      idx->used->current = found;
	      found = removeLink (idx->used);
	      idx->used->type = QUEUED;
	      break;
	    }
	    found = found->prev;
	  }
	  if (found != idx->used->head)
	    insertLink (idx->free, found);
	  check_pointer (found);
	  free (lnk);
	}
	else {
	  idx->current = blk;
	  free (lnk);
	}
	check_pointer (tmp);
	check_pointer (tmp->data);
	treeKey = tmp->data;
	free (tmp);
	if (fixPointers) {
	  treeKey2 = blk->keyItemList->head->next->data;
	  check_pointer (treeKey2);
	  memmove (((keyItem *) found->data)->key, treeKey2->key,
		   idx->hdr->itemSize - sizeof (unsigned long));
	}
	idx->hdr->numItems--;
	return treeKey;
      }
    }

    /* skip to the first one that matches the input key */
    check_pointer (lnk->data);
    check_pointer (tmp->data);
    while (tmp != blk->keyItemList->tail &&
	   idx->inx->compare (lnk->data, tmp->data)) {
      tmp = tmp->next;
      check_pointer (tmp);
      check_pointer (tmp->data);
      check_pointer (tmp->next);
    }
    while (tmp != blk->keyItemList->tail) {
      check_pointer (lnk->data);
      check_pointer (tmp->data);
      check_pointer (tmp->next);
      if (idx->inx->compare (lnk->data, tmp->data)) {
	idxError = IDX_NEXISTS;
	free (lnk);
	return 0;
      }
      treeKey = tmp->data;
      check_pointer (treeKey);
      if (ki->item == treeKey->item) {
	if (tmp != blk->keyItemList->tail)
	  blk->current = tmp->next;
	check_pointer (blk->current);
	/*check_pointer (blk->current->next); */
	Assert (tmp != blk->keyItemList->tail);
	blk->keyItemList->current = tmp;
	tmp = removeLink (blk->keyItemList);
	if (0 == tmp) {
	  idxError = IDX_LIST;
	  free (lnk);
	  return 0;
	}
	/*blk->keyItemList->current = blk->current; */
	blk->bhd->numKeyItems--;
	if ((size_t)blk->bhd->numKeyItems != blk->keyItemList->number) {
	  /*idxError = IDX_UNSTABLE;
	     free (lnk);
	     return 0; */
	  blk->bhd->numKeyItems = blk->keyItemList->number;
	}
	blk->isDirty = TRUE;
	check_pointer (tmp);
	check_pointer (tmp->data);
	if (blk->keyItemList->number == 0) {
	  /* the block is empty; remove it */
	  status = putFileBlock (idx->fd, blk->blk, blk->blockNumber,
				 idx->hdr->blockSize);
	  if (_ERROR_ == status) {
	    free (lnk);
	    return 0;
	  }
	  /*found->next->prev = found->prev;
	     found->prev->next = found->next; */
	  /*restructureShellNodes (idx->inx); */
	  idx->inx->lh->current = found;
	  found = removeLink (idx->inx->lh);
	  if (found == 0) {
	    idxError = IDX_LIST;
	    return 0;
	  }
	  restructureShellNodes (idx->inx);
	  /*
	   * Now, insert the link into the free list and relink the blocks.
	   */
	  prev = getIndexBlock (idx, blk->bhd->prevBlock);
	  if (prev == 0) {
	    idxError = IDX_UNSPECIFIED;
	    return 0;
	  }
	  next = getIndexBlock (idx, blk->bhd->nextBlock);
	  if (next == 0) {
	    idxError = IDX_UNSPECIFIED;
	    return 0;
	  }
	  prev->bhd->nextBlock = next->blockNumber;
	  next->bhd->prevBlock = prev->blockNumber;
	  status = putFileBlock (idx->fd, prev->blk, prev->blockNumber,
				 idx->hdr->blockSize);
	  if (_ERROR_ == status) {
	    free (lnk);
	    return 0;
	  }
	  status = putFileBlock (idx->fd, next->blk, next->blockNumber,
				 idx->hdr->blockSize);
	  if (_ERROR_ == status) {
	    free (lnk);
	    return 0;
	  }
	  idx->current = next;
	  treeKey = found->data;
	  check_pointer (found);
	  check_pointer (found->data);
	  free (found);
	  free (treeKey);
	  found = idx->used->tail->prev;
	  while (found != idx->used->head) {
	    if (((idxBlock *) lnk->data)->blockNumber == blk->blockNumber) {
	      idx->used->type = UNSORTED;
	      idx->used->current = found;
	      found = removeLink (idx->used);
	      idx->used->type = QUEUED;
	      break;
	    }
	    found = found->prev;
	  }
	  if (found != idx->used->head)
	    insertLink (idx->free, found);
	  free (lnk);
	  check_pointer (found);
	  check_pointer (found->data);
	}
	else {
	  idx->current = blk;
	  free (lnk);
	}
	check_pointer (tmp);
	check_pointer (tmp->data);
	treeKey = tmp->data;
	free (tmp);
	idx->hdr->numItems--;
	return treeKey;
      }
      tmp = tmp->next;
    }
    found = found->next;
    check_pointer (found);
    check_pointer (found->data);
    check_pointer (lnk->data);
    if (found == idx->inx->lh->tail ||
	idx->inx->compare (lnk->data, found->data))
      break;
  }
  free (lnk);
  idxError = IDX_NEXISTS;
  return 0;
}


/*
 * [BeginDoc]
 * [Verbatim]

Link *searchExactIndexItem (dbIndex *idx, Link *lnk);
 * [EndDoc] */
/*
 * [BeginDoc]
 * 
 * searchExactIndexItem() searches the index given by ``idx'' for
 * the key given by ``lnk''->data (see the list module for more
 * information on links).  lnk should be an allocated Link and
 * lnk->data should point to a valid allocated key/item pair.  The
 * key member of the key/item is all that needs to be populated.
 * searchExactIndexItem() returns NULL if an exact match for the
 * key is not found, a valid, read-only pointer to a link if it is.
 * If NULL is returned, the user should verify that idxError is
 * still IDX_NOERR, in which case there was not a critical error.
 * 
 * =================================================================
 * [EndDoc]
 */
Link *searchExactIndexItem (dbIndex * idx, Link * lnk)
{
  Link *found;
  keyItem *ki;
  idxBlock *blk;
  Link *tmp;
  int status;

  check_pointer (lnk);
  check_pointer (lnk->data);
  found = queryShellItem (idx->inx, lnk);
  if (0 == found) {
    idxError = IDX_UNSTABLE;
    return 0;
  }
#ifdef	DEBUG
  if (found == idx->inx->lh->head || found == idx->inx->lh->tail) {
    printf ("queryShellItem returned head or tail of list\n");
  }
#endif
  status = idx->inx->compare (lnk->data, found->data);
  if (!idx->hdr->isUnique	/* not a unique index */
      && status == 0		/* the head of this node = item */
      && found->prev != idx->inx->lh->head) {	/* not at the first node */
    tmp = found->prev;
    ki = tmp->data;
    blk = getIndexBlock (idx, ki->item);
    if (0 == blk) {
      idxError = IDX_UNSTABLE;
      return 0;
    }
    if (blk->keyItemList->head->next == blk->keyItemList->tail)
      goto continueHere;
    tmp = blk->keyItemList->head->next;
    while (tmp != blk->keyItemList->tail) {
      status = idx->inx->compare (tmp->data, lnk->data);
      if (status == 0) {
	idx->isSequential = TRUE;
	idx->current = blk;
	idx->bof = idx->eof = FALSE;
	blk->current = tmp;
	return tmp;
      }
      if (status > 0)
	break;
      tmp = tmp->next;
    }
  }
continueHere:
  check_pointer (found);
  ki = found->data;
  check_pointer (ki);
  blk = getIndexBlock (idx, ki->item);
  if (0 == blk) {
    idxError = IDX_UNSTABLE;
    return 0;
  }
  check_pointer (blk);
  found = blk->keyItemList->head->next;
  ki = found->data;
  while (found != blk->keyItemList->tail) {
    if (!idx->inx->compare (found->data, lnk->data)) {
      idx->isSequential = TRUE;
      idx->current = blk;
      idx->bof = idx->eof = FALSE;
      blk->current = found;
      return found;
    }
    found = found->next;
  }
  return 0;
}

/*
 * [BeginDoc]
 * [Verbatim]

Link *searchIndexItem (dbIndex *idx, Link *lnk);
 * [EndDoc] */

/*
 * [BeginDoc]
 * 
 * searchIndexItem() works similarly to the searchExactIndexItem()
 * with the exception that it will return NULL only if the key being
 * searched for is less than (lexically) than anything else in the
 * index.  If it finds an exact match, it will return information 
 * regarding that.  However, if it does not find an exact match, 
 * it will return information on the key immediately preceding
 * the one searched for.  If searchIndexItem() returns NULL,
 * the user should check the value of idxError and make sure that
 * it is IDX_NOERR, which will indicate that a critical error didn't
 * occur.
 * 
 * =================================================================
 * [EndDoc]
 */
Link *searchIndexItem (dbIndex * idx, Link * lnk)
{
  Link *found;
  keyItem *ki;
  idxBlock *blk;
  Link *tmp;
  int status;

  found = queryShellItem (idx->inx, lnk);
  if (0 == found) {
    idxError = IDX_UNSTABLE;
    return 0;
  }
  status = idx->inx->compare (lnk->data, found->data);
  if (!idx->hdr->isUnique && status == 0 && found->prev != idx->inx->lh->head) {
    tmp = found->prev;
    ki = tmp->data;
    blk = getIndexBlock (idx, ki->item);
    if (0 == blk) {
      idxError = IDX_UNSTABLE;
      return 0;
    }
    if (blk->keyItemList->head->next == blk->keyItemList->tail)
      return 0;
    tmp = blk->keyItemList->head->next;
    while (tmp != blk->keyItemList->tail) {
      status = idx->inx->compare (tmp->data, lnk->data);
      if (status == 0) {
	idx->isSequential = TRUE;
	idx->current = blk;
	idx->bof = idx->eof = FALSE;
	blk->current = tmp;
	return tmp;
      }
      if (status > 0)
	break;
      tmp = tmp->next;
    }
  }
  ki = found->data;
  blk = getIndexBlock (idx, ki->item);
  if (0 == blk) {
    idxError = IDX_UNSTABLE;
    return 0;
  }
  if (blk->keyItemList->head->next == blk->keyItemList->tail)
    return 0;
  found = blk->keyItemList->head->next;
  while (found != blk->keyItemList->tail) {
    status = idx->inx->compare (found->data, lnk->data);
    if (status == 0) {
      idx->isSequential = TRUE;
      idx->current = blk;
      idx->bof = idx->eof = FALSE;
      blk->current = found;
      return found;
    }
    if (status > 0) {
      found = found->prev;
      break;
    }
    found = found->next;
  }
  if (found == blk->keyItemList->head)
    found = blk->keyItemList->head->next;
  if (found == blk->keyItemList->tail)
    found = blk->keyItemList->tail->prev;
  if (found != 0) {
    idx->isSequential = TRUE;
    idx->current = blk;
    blk->current = found;
    idx->bof = idx->eof = FALSE;
  }
  return found;
}

/*
 * [BeginDoc]
 * [Verbatim]

keyItem *firstIndexItem (dbIndex *idx);
 * [EndDoc] */

/*
 * [BeginDoc]
 * 
 * firstIndexItem() returns the key/item pair of the first item in
 * the index given by ``idx'' on success, NULL if unsuccessful.
 * If NULL is returned, the user should check the value of idxError
 * to verify that an error didn't occur, otherwise, a return value
 * of NULL could indicate that there are no records in the index.
 * If a valid pointer is returned, sequential access will be enabled
 * for this index and the current indicators will be reset to the
 * first index.
 * 
 * =================================================================
 * [EndDoc]
 */
keyItem *firstIndexItem (dbIndex * idx)
{
  Link *lnk;
  idxBlock *blk;
  keyItem *ki;

  if (idx->inx->lh->number == 0)
    return 0;
  lnk = idx->inx->lh->head->next;
  ki = lnk->data;
  blk = getIndexBlock (idx, ki->item);
  if (0 == blk) {
    /* idxError set */
    return 0;
  }

  if (blk->keyItemList->number == 0) {
    idxError = IDX_UNSTABLE;
    return 0;
  }
  idx->current = blk;
  lnk = blk->keyItemList->head->next;
  if (lnk == blk->keyItemList->tail) {
    idxError = IDX_UNSTABLE;
    return 0;
  }
  blk->current = lnk;
  idx->isSequential = TRUE;
  idx->bof = TRUE;
  idx->eof = FALSE;
  return ((keyItem *) lnk->data);
}

/*
 * [BeginDoc]
 * [Verbatim]

keyItem *lastIndexItem (dbIndex *idx);
 * [EndDoc] */

/*
 * [BeginDoc]
 * 
 * lastIndexItem() works identically to firstIndexItem() except that
 * the information returned will be for the last item in the index
 * and the currency pointers will be set to the last item.
 * 
 * =================================================================
 * [EndDoc]
 */
keyItem *lastIndexItem (dbIndex * idx)
{
  Link *lnk;
  idxBlock *blk;
  keyItem *ki;

  if (idx->inx->lh->number == 0)
    return 0;
  lnk = idx->inx->lh->tail->prev;
  ki = lnk->data;
  blk = getIndexBlock (idx, ki->item);
  if (0 == blk) {
    /* idxError set */
    return 0;
  }

  if (blk->keyItemList->number == 0) {
    idxError = IDX_UNSTABLE;
    return 0;
  }
  idx->current = blk;
  lnk = blk->keyItemList->tail->prev;
  if (lnk == blk->keyItemList->head) {
    idxError = IDX_UNSTABLE;
    return 0;
  }
  blk->current = lnk;
  idx->isSequential = TRUE;
  idx->eof = TRUE;
  idx->bof = FALSE;
  return ((keyItem *) lnk->data);
}

/*
 * [BeginDoc]
 * [Verbatim]

keyItem *nextIndexItem (dbIndex *idx)
 * [EndDoc] */

/*
 * [BeginDoc]
 * 
 * nextIndexItem() returns information for the next key/item pair
 * for the index given by ``idx''.  If sequential access has not
 * been enabled (with a search or a call to firstIndexItem() or
 * lastIndexItem()).  If we are at the last index item, 
 * nextIndexItem(),  idx->eof will be set to TRUE and NULL will be
 * returned.  If NULL is returned, the value of idxError should be
 * checked.
 * 
 * =================================================================
 * [EndDoc]
 */
keyItem *nextIndexItem (dbIndex * idx)
{
  Link *lnk;
  idxBlock *blk;

  if (idx->isSequential == FALSE) {
    idxError = IDX_SEQUENCE;
    return 0;
  }
  if (idx->inx->lh->number == 0)
    return 0;
  if (idx->current == 0)
    return 0;
  blk = idx->current;
  if (blk->current == 0)
    return 0;
  lnk = blk->current;
  check_pointer (lnk);
  if (lnk->next != 0)
    lnk = lnk->next;
  if (blk->current == blk->keyItemList->head)
    blk->current = blk->current->next;
  if (lnk == blk->keyItemList->tail) {
    if (blk->bhd->nextBlock == 0) {
      /* we're at the end */
      blk->current = blk->keyItemList->tail->prev;
      idx->eof = TRUE;
      return 0;
    }
    blk = getIndexBlock (idx, blk->bhd->nextBlock);
    if (0 == blk) {
      idxError = IDX_UNSTABLE;
      return 0;
    }
    idx->current = blk;
    lnk = blk->keyItemList->head->next;
    if (lnk == blk->keyItemList->tail) {
      idxError = IDX_UNSTABLE;
      return 0;
    }
    blk->current = lnk;
  }
  else
    blk->current = lnk;
  check_pointer (lnk);
  check_pointer (lnk->data);
  return ((keyItem *) lnk->data);
}

/*
 * [BeginDoc]
 * [Verbatim]

keyItem *prevIndexItem (dbIndex *idx);
 * [EndDoc] */

/*
 * [BeginDoc]
 * 
 * prevIndexItem() functions identically to nextIndexItem() except
 * that prevIndexItem() moves backward in the index.  If the
 * index is sequentially at the beginning, idx->bof will be set to
 * TRUE and NULL will be returned.  If the caller get's a NULL value,
 * idxError should be checked to make sure that an error didn't
 * occur.
 * 
 * =================================================================
 * [EndDoc]
 */
keyItem *prevIndexItem (dbIndex * idx)
{
  Link *lnk;
  idxBlock *blk;

  if (idx->isSequential == FALSE) {
    idxError = IDX_SEQUENCE;
    return 0;
  }
  if (idx->inx->lh->number == 0)
    return 0;
  if (idx->current == 0)
    return 0;
  blk = idx->current;
  if (blk->current == 0)
    return 0;
  lnk = blk->current;
  if (lnk->prev != 0)
    lnk = lnk->prev;
  if (lnk == blk->keyItemList->head) {
    if (blk->bhd->prevBlock == 0) {
      /* we're at the beginning */
      blk->current = blk->keyItemList->head->next;
      idx->eof = TRUE;
      return 0;
    }
    blk = getIndexBlock (idx, blk->bhd->prevBlock);
    if (0 == blk) {
      idxError = IDX_UNSTABLE;
      return 0;
    }
    idx->current = blk;
    lnk = blk->keyItemList->tail->prev;
    if (lnk == blk->keyItemList->head) {
      idxError = IDX_UNSTABLE;
      return 0;
    }
    blk->current = lnk;
  }
  else
    blk->current = lnk;
  return ((keyItem *) lnk->data);
}

#ifdef	DEBUG
void DebugDumpBlock (idxBlock * blk)
{
  Link *lnk;
  keyItem *ki;

  dbg2 (DebugCheckList (blk->keyItemList));
  lnk = blk->keyItemList->head->next;
  ki = lnk->data;
  printf ("\nBlock # %d: firstItem %d, %s\n", (int)blk->blockNumber,
	  (int)ki->item, ki->key);
  while (TRUE) {
    printf ("Item %d, %s\n", (int)ki->item, ki->key);
    lnk = lnk->next;
    if (lnk == blk->keyItemList->tail)
      break;
    ki = lnk->data;
  }
}
#endif

/*
 * [BeginDoc]
 * 
 * If you compile index.c with TEST_INDEX defined, a main() function
 * is compiled which can be used to test the index functionality.
 * See the index.c source file for more information.
 * [EndDoc]
 */

#ifdef	TEST_INDEX

#include <stdio.h>

#define	TEST_DATA_SIZE		20

int main (int argc, char *argv[])
{
  FILE *fp;
  char *data;
  char *cp;
  keyItem *ki;
  int status;
  dbIndex *idx;
  unsigned long counter = 0UL;
#ifdef	DEBUG
  int thisLeaf = 0;
  keyItem *debugKI;
  Link *found;
#endif
  time_t t1, t2;
  int isOK = TRUE;

  if (argc != 4) {
    printf ("\n\nUsage: index <file> <file.srt> <file.rev.srt>\n");
    printf ("\twhere <file> is the file to add to the index,\n");
    printf ("\t<file.srt> is the sorted file to compare it with,\n");
    printf ("\t and <file.srt.rev> is the reverse file to compare with.\n\n");
    return 1;
  }

  /*
   * Create the index file
   */
  status = createIndexFile ("testIndex.idx", 0, TEST_DATA_SIZE, FALSE, TRUE);
  if (_ERROR_ == status) {
    printf ("\n\n***Error: Couldn't create index file %s\n", "testIndex.idx");
    return _ERROR_;
  }
  idx = openIndex ("testIndex.idx", 0, 0);
  if (0 == idx) {
    printf ("\n\n***Error: Couldn't open index file, %s\n",
	    idxErrMsg[idxError]);
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

  time (&t1);
  while (TRUE) {
    cp = fgets (data, TEST_DATA_SIZE, fp);
    if (feof (fp))
      break;
    if (cp == 0)
      break;
    if (data[0] == '\0')
      break;

    cp = strchr (data, '\n');
    if (0 != cp)
      *cp = '\0';
    if (strlen (data) == 0)
      continue;
    ki = malloc (TEST_DATA_SIZE + sizeof (unsigned long));
    if (0 == ki) {
      printf ("\n\n***Error: fatal memory error allocating a keyItem\n");
      return _ERROR_;
    }
    memset (ki, 0, TEST_DATA_SIZE + sizeof (unsigned long));
    check_pointer (ki);

    counter++;
    ki->item = counter;
    strcpy (ki->key, data);
    check_pointer (ki);

#ifdef DEBUG
    if (counter % 100 == 0) {
      printf (".");
      fflush (stdout);
    }
#endif
    status = addIndexItem (idx, ki);
    if (status == _ERROR_) {
      if (idxError == IDX_UNIQUE) {
	idxError = IDX_NOERR;
	free (ki);
      }
      else {
	printf ("\n\n***Error: adding item %d, %s\n", counter,
		idxErrMsg[idxError]);
	closeIndexFile (idx, "testIndex.inx");
	return _ERROR_;
      }
    }
    Assert (idx->free->number + idx->used->number == BLOCK_CACHE_SIZE);
  }
  time (&t2);
  fclose (fp);

  printf ("Add time for %d items: %f\n", idx->hdr->numItems,
	  difftime (t2, t1));
  strcpy (idx->hdr->indexName, "testIndex");

  status = closeIndexFile (idx, "testIndex.inx");
  if (_ERROR_ == status) {
    printf ("\n\n***Error: closing index, %s\n", idxErrMsg[idxError]);
    return _ERROR_;
  }

  idx = openIndex ("testIndex.idx", "testIndex.inx", 0);
  if (0 == idx) {
    printf ("\n\n***Error: opening index again, %s\n", idxErrMsg[idxError]);
    return _ERROR_;
  }

  /*
   * Now, open the test files and check the index
   */
  fp = fopen (argv[2], "rb");
  if (0 == fp) {
    printf ("\n\n***Error: couldn't open %s\n", argv[2]);
    return _ERROR_;
  }
  ki = firstIndexItem (idx);
  if (0 == ki) {
    printf ("\n\n***Error: %s\n", idxErrMsg[idxError]);
    return _ERROR_;
  }
  counter = 1L;
  while (TRUE) {
    cp = fgets (data, TEST_DATA_SIZE, fp);
    if (feof (fp))
      break;
    if (cp == 0)
      break;
    if (data[0] == '\0')
      break;

    cp = strchr (data, '\n');
    if (0 != cp)
      *cp = '\0';
    if (strlen (data) == 0)
      continue;
    if (strcmp (data, ki->key)) {
      printf
	("Index discrepancy: counter = %d, data = \"%s\", ki->key = \"%s\"\n",
	 counter, data, ki->key);
      isOK = FALSE;
    }
    ki = nextIndexItem (idx);
    if (0 == ki) {
      if (idxError == IDX_NOERR)
	break;
      printf ("\n\n***Error: %s\n", idxErrMsg[idxError]);
      return _ERROR_;
    }
    counter++;
  }
  if (isOK == FALSE) {
    closeIndexFile (idx, "testIndex.inx");
    return _ERROR_;
  }

  fp = fopen (argv[3], "rb");
  if (0 == fp) {
    printf ("\n\n***Error: couldn't open %s\n", argv[3]);
    return _ERROR_;
  }
  ki = lastIndexItem (idx);
  if (0 == ki) {
    printf ("\n\n***Error: %s\n", idxErrMsg[idxError]);
    return _ERROR_;
  }
  counter = 1L;
  while (TRUE) {
    cp = fgets (data, TEST_DATA_SIZE, fp);
    if (feof (fp))
      break;
    if (cp == 0)
      break;
    if (data[0] == '\0')
      break;

    cp = strchr (data, '\n');
    if (0 != cp)
      *cp = '\0';
    if (strlen (data) == 0)
      continue;
    if (strcmp (data, ki->key)) {
      printf
	("Index discrepancy: counter = %d, data = \"%s\", ki->key = \"%s\"\n",
	 counter, data, ki->key);
      isOK = FALSE;
    }
    ki = prevIndexItem (idx);
    if (0 == ki) {
      if (idxError == IDX_NOERR)
	break;
      printf ("\n\n***Error: %s\n", idxErrMsg[idxError]);
      return _ERROR_;
    }
    counter++;
  }

  if (isOK == FALSE) {
    closeIndexFile (idx, "testIndex.inx");
    return _ERROR_;
  }

  status = closeIndexFile (idx, "testIndex.inx");
  if (_ERROR_ == status) {
    printf ("\n\n***Error: closing index, %s\n", idxErrMsg[idxError]);
    return _ERROR_;
  }
  free (data);
  print_block_list ();
  return _OK_;
}

#endif /* TEST_INDEX */

#ifdef	__cplusplus
endC
#endif
