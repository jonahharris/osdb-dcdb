/*	Source File:	idxblk.c	*/

#ifdef	__cplusplus
externC
#endif
#include <index.h>
/*
 * [BeginDoc]
 * 
 * idxBlock *getIndexBlock (dbIndex *idx, off_t blknum);
 * 
 * Get the file block given by blknum from the index file given by
 * idx->fd.  This uses the block I/O module to get a block from file,
 * if necessary.  getIndexBlock() first searches idx->used to
 * see if the block is in use by the system.  If the block is not
 * there, it will try to get a free block and pull the data from
 * disk.  If a free block is not available in idx->free, 
 * getIndexBlock() will get the oldest one from the idx->used
 * list (a QUEUED list), flush the data in the block to disk and
 * use the memory block to store the data.
 * 
 * =================================================================
 * [EndDoc]
 */
  idxBlock * getIndexBlock (dbIndex * idx, off_t blknum)
{
  idxBlock *blk;
  Link *lnk, *found;
  int status;

  /*
   * First, look in the idx->used
   */
  if (!isEmptyList (idx->used)) {
    lnk = idx->used->tail->prev;
    while (lnk != idx->used->head) {
      blk = lnk->data;
      check_pointer (blk);
      check_pointer (blk->blk);
      if (blk->blockNumber == blknum)
	break;
      lnk = lnk->prev;
    }
    if (lnk != idx->used->head) {
      idx->used->type = UNSORTED;
      idx->used->current = lnk;
      lnk = removeLink (idx->used);
      if (0 == lnk) {
	idxError = IDX_UNSTABLE;
	return 0;
      }
      idx->used->type = QUEUED;
      insertLink (idx->used, lnk);
      blk = lnk->data;
      return blk;
    }
  }

  /*
   * Didn't find it in the used blocks; get a free block and 
   * use that if one is available.
   */
  if (!isEmptyList (idx->free)) {
    lnk = removeLink (idx->free);
    blk = lnk->data;
    check_pointer (blk);
    check_pointer (blk->blk);
    found = 0;
  }

  /*
   * A free block wasn't available, so use the first item on the
   * idx->used list, flush the data to disk and read in the
   * the block number needed.
   */
  else {			/* idx->free is empty */
    if (isEmptyList (idx->used)) {
      idxError = IDX_UNSTABLE;
      return 0;
    }
    lnk = removeLink (idx->used);
    if (0 == lnk) {
      idxError = IDX_LIST;
      return 0;
    }
    blk = lnk->data;
    check_pointer (blk);
    check_pointer (blk->blk);
  }

  /*
   * Flush the block to disk
   */
  check_pointer (blk->blk);
  blk->bhd->numKeyItems = blk->keyItemList->number;
  listToBlock (blk->keyItemList, blk->blk, blk->itemSize,
	       blk->bhd->numKeyItems, sizeof (blockHeader));
  check_pointer (blk->blk);
  check_pointer (blk);
  status = putFileBlock (idx->fd, blk->blk, blk->blockNumber,
			 idx->hdr->blockSize);
  /*fileFlush (idx->fd); */
  if (blk->keyItemList == 0 || clearList (blk->keyItemList)) return 0;
  Assert (blk->keyItemList->number == 0);

  /*
   * Get the block from disk.
   */

  check_pointer (blk->blk);
  status = getFileBlock (idx->fd, blk->blk, blknum, idx->hdr->blockSize);
  if (_ERROR_ == status) {
    idxError = IDX_FSYSTEM;
    return 0;
  }

  /*
   * Populate the block data and return the block pointer
   */
  check_pointer (blk);
  check_pointer (blk->blk);
  blk->blockNumber = blknum;
  blk->bhd = (blockHeader *) blk->blk;
  blk->isDirty = TRUE;
  blk->itemSize = idx->hdr->itemSize;
  if (idx->hdr->itemSize != 0)
    blk->numMaxItems = (idx->hdr->blockSize - sizeof (blockHeader)) /
      idx->hdr->itemSize;
  else {
    idxError = IDX_UNSTABLE;
    return 0;
  }
  blk->currentSize = blk->bhd->numKeyItems * idx->hdr->itemSize;
  if (blk->keyItemList == 0) {
    blk->keyItemList = initList (UNSORTED, 0);
    if (0 == blk->keyItemList) {
      idxError = IDX_LIST;
      return 0;
    }
  }
  else
    blk->keyItemList->type = UNSORTED;
  status = blockToList (blk->keyItemList, blk->blk, blk->itemSize,
			blk->bhd->numKeyItems, sizeof (blockHeader));
  check_pointer (blk);
  check_pointer (blk->blk);
  if (status == _ERROR_) {
    idxError = IDX_FSYSTEM;
    return 0;
  }
  blk->keyItemList->type = SLOWSORTED;
  if (idx->hdr->isCase)
    blk->keyItemList->compare = caseCompare;
  else
    blk->keyItemList->compare = noCaseCompare;

  check_pointer (lnk);
  lnk->data = blk;
  insertLink (idx->used, lnk);

  check_pointer (blk);
  check_pointer (blk->blk);
  dbg2 (DebugCheckList (blk->keyItemList));
  return blk;
}

/*
 * [BeginDoc]
 * 
 * idxBlock *getNewBlock (dbIndex *idx, off_t blknum);
 * 
 * getNewBlock() gets a free block from the block management
 * system built into the dbIndex structure.  First, getNewBlock()
 * tries the free block list.  If one is available there, it adds
 * the block to the idx->used list.  If one is not available 
 * from the free list, it will get one from the idx->used list,
 * flush that block to file and make it available.
 * 
 * The block is assumed to be a new block, one that doesn't appear
 * yet in the block file.  Therefore, an entry is made for the
 * block in the block file (the block is appended to the block file).
 * It is important, therefore, that the user insure that blknum is
 * one greater than the last block in the block file.  This will
 * help insure consistency in block file management.
 * 
 * =================================================================
 * [EndDoc]
 */
idxBlock *getNewBlock (dbIndex * idx, off_t blknum)
{
  Link *lnk;
  idxBlock *blk;
  int status;
  off_t newBlock;

  if (!isEmptyList (idx->free)) {
    lnk = removeLink (idx->free);
    blk = lnk->data;
    check_pointer (blk);
    check_pointer (blk->blk);
    if (blk->keyItemList != 0)
      if (!isEmptyList (blk->keyItemList))
	if (clearList (blk->keyItemList)) return 0;
    newBlock = appendFileBlock (idx->fd, blk->blk, idx->hdr->blockSize);
    if (newBlock == (off_t) _ERROR_) {
      idxError = IDX_FSYSTEM;
      return 0;
    }
    if (newBlock != blknum) {
      idxError = IDX_UNSTABLE;
      return 0;
    }
    blk->blockNumber = newBlock;
    insertLink (idx->used, lnk);
    return blk;
  }

  /*
   * A free block wasn't available, so use the first item on the
   * idx->used list, flush the data to disk and read in the
   * the block number needed.
   */
  if (isEmptyList (idx->used)) {
    idxError = IDX_UNSTABLE;
    return 0;
  }
  lnk = removeLink (idx->used);
  if (0 == lnk) {
    idxError = IDX_LIST;
    return 0;
  }
  blk = lnk->data;
  check_pointer (blk);
  check_pointer (blk->blk);

  /*
   * Flush the block to disk
   */
  check_pointer (blk->blk);
  blk->bhd->numKeyItems = blk->keyItemList->number;
  listToBlock (blk->keyItemList, blk->blk, blk->itemSize,
	       blk->bhd->numKeyItems, sizeof (blockHeader));
  check_pointer (blk->blk);
  check_pointer (blk);
  status = putFileBlock (idx->fd, blk->blk, blk->blockNumber,
			 idx->hdr->blockSize);
  if (blk->keyItemList != 0)
    if (!isEmptyList (blk->keyItemList))
      if (clearList (blk->keyItemList)) return 0;

  newBlock = appendFileBlock (idx->fd, blk->blk, idx->hdr->blockSize);
  if (newBlock == (off_t) _ERROR_) {
    idxError = IDX_FSYSTEM;
    return 0;
  }
  if (newBlock != blknum) {
    idxError = IDX_UNSTABLE;
    return 0;
  }
  blk->blockNumber = newBlock;
  insertLink (idx->used, lnk);
  return blk;
}

/*
 * [BeginDoc]
 * 
 * idxBlock *splitBlock (dbIndex *idx, idxBlock *blk);
 * 
 * splitBlock() will split the block given by blk into two blocks.
 * The block blk will contain the ``bottom half'' of the data and
 * the block returned will contain the ``top half''.  Basically,
 * the list maintained by blk will be halved and half of it will
 * go to the block to be returned.  Then, all members will be
 * updated to reflect the new holdings.  However, most of the 
 * information on the block, including what kind of block it is,
 * where it is in the linked list of blocks, etc., is not updated.
 * It is the caller's responsibility to update this information.
 * The il parameter is the indexListItem that manages this index.
 * 
 * splitBlock() returns a valid file block if successful, 0 (NULL)
 * otherwise.
 * 
 * [EndDoc]
 */
idxBlock *splitBlock (dbIndex * idx, idxBlock * blk)
{
  idxBlock *returnBlock, *nxtBlock;
  Link *lnk;
  int num;
  int status;
  off_t nextBlock;

  check_pointer (blk);
  check_pointer (blk->blk);
  nextBlock = blk->bhd->nextBlock;
  returnBlock = getNewBlock (idx, idx->hdr->lastBlock + 1);
  if (0 == returnBlock) {
    return 0;
  }
  check_pointer (returnBlock);
  check_pointer (returnBlock->blk);
  returnBlock->blockNumber = idx->hdr->lastBlock + 1;
  idx->hdr->lastBlock++;

  /*
   * Now, split blk's list into half.
   */
  if (returnBlock->keyItemList == 0) {
    returnBlock->keyItemList = initList (UNSORTED, 0);
    if (returnBlock->keyItemList == 0) {
      idxError = IDX_LIST;
      return 0;
    }
  }
  else {
    /* for now */
    returnBlock->keyItemList->type = UNSORTED;
    returnBlock->keyItemList->compare = 0;
  }
  num = blk->keyItemList->number;
  num /= 2;
  Assert (num > 0);
  if (num <= 0) {
    idxError = IDX_UNSTABLE;
    return 0;
  }
  while (num--) {
    lastLink (blk->keyItemList);
    lnk = removeLink (blk->keyItemList);
    if (lnk == 0) {
      idxError = IDX_UNSTABLE;
      return 0;
    }
    returnBlock->keyItemList->current = returnBlock->keyItemList->head;
    status = insertLinkHere (returnBlock->keyItemList, lnk);
    if (_ERROR_ == status) {
      idxError = IDX_LIST;
      return 0;
    }
  }
  blk->bhd->numKeyItems = blk->keyItemList->number;
  returnBlock->bhd->numKeyItems = returnBlock->keyItemList->number;

  /*
   * Flush the blocks to disk
   */
  check_pointer (blk->blk);
  listToBlock (blk->keyItemList, blk->blk, blk->itemSize,
	       blk->bhd->numKeyItems, sizeof (blockHeader));
  check_pointer (blk->blk);
  check_pointer (blk);
  status = putFileBlock (idx->fd, blk->blk, blk->blockNumber,
			 idx->hdr->blockSize);
  check_pointer (returnBlock->blk);
  listToBlock (returnBlock->keyItemList, returnBlock->blk,
	       returnBlock->itemSize,
	       returnBlock->bhd->numKeyItems, sizeof (blockHeader));
  check_pointer (returnBlock->blk);
  check_pointer (returnBlock);
  status =
    putFileBlock (idx->fd, returnBlock->blk, returnBlock->blockNumber,
		  idx->hdr->blockSize);
  /*
   * reset the keyItemList in returnBlock
   */
  returnBlock->keyItemList->type = SLOWSORTED;
  if (idx->hdr->isCase)
    returnBlock->keyItemList->compare = caseCompare;
  else
    returnBlock->keyItemList->compare = noCaseCompare;

  /*
   * Populate the block information on returnBlock
   */
  if (idx->hdr->itemSize != 0)
    returnBlock->numMaxItems =
      (idx->hdr->blockSize - sizeof (blockHeader)) / idx->hdr->itemSize;
  else {
    idxError = IDX_UNSTABLE;
    return 0;
  }
  returnBlock->isDirty = TRUE;
  returnBlock->itemSize = idx->hdr->itemSize;
  returnBlock->currentSize =
    idx->hdr->itemSize * returnBlock->bhd->numKeyItems;
  returnBlock->firstItem = returnBlock->keyItemList->head->next->data;
  /*
   * Just for giggles
   */
  blk->firstItem = blk->keyItemList->head->next->data;

  /*
   * Set the block pointers in the headers
   */
  returnBlock->bhd->prevBlock = blk->blockNumber;
  returnBlock->bhd->nextBlock = blk->bhd->nextBlock;
  blk->bhd->nextBlock = returnBlock->blockNumber;
  blk->currentSize = blk->bhd->numKeyItems * blk->itemSize;
  nxtBlock = getIndexBlock (idx, nextBlock);
  nxtBlock->bhd->prevBlock = returnBlock->blockNumber;

  check_pointer (returnBlock);
  check_pointer (returnBlock->blk);
  return returnBlock;
}

int flushIndexBlocks (dbIndex * idx)
{
  Link *lnk;
  idxBlock *blk;
  int status;

  if (idx->used->number == 0)
    return _OK_;
  lnk = idx->used->head->next;
  while (lnk != idx->used->tail) {
    blk = lnk->data;
    if (!blk) return _ERROR_;
    if (!blk->isDirty) {
      lnk = lnk->next;
      continue;
    }
    check_pointer (blk->blk);
    check_pointer (blk);
    blk->bhd->numKeyItems = blk->keyItemList->number;
    status = listToBlock (blk->keyItemList, blk->blk, blk->itemSize,
			  blk->keyItemList->number, sizeof (blockHeader));
    check_pointer (blk->blk);
    check_pointer (blk);
    if (status == _ERROR_)
      return _ERROR_;
    status = putFileBlock (idx->fd, blk->blk, blk->blockNumber,
			   idx->hdr->blockSize);
    if (status == _ERROR_)
      return _ERROR_;
    blk->isDirty = FALSE;
  }
  return _OK_;
}

#ifdef	__cplusplus
endC
#endif
