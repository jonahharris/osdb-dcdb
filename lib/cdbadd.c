/* Source File: cdbadd.c */

/*
 * [BeginDoc]
 * 
 * \section{DCDB Add Functions}
 * 
 * This file contains the add functions for the cdb module.
 * 
 * [EndDoc]
 */

#include <cdb.h>

/*
 * [BeginDoc]
 * \subsection{addRecord}
 * \index{addRecord}
 * [Verbatim] */
int addRecord (dbTable * tbl)
/* [EndDoc] */
/*
 * [BeginDoc]
 * addRecord() adds the data in tbl->fields[] to the end of the
 * table given by tbl->fd.  The data is copied from the fields[]
 * array to tbl->data using field2Record().
 * 
 * All indexes are updated before the record is added.  If a unique
 * violation would result from adding the current record, no data
 * is added to the indexes or tables and an _ERROR_ is flagged.
 * 
 * Because of the checking that is done by addRecord() for unique
 * constraint violations, it is faster to add records for a table
 * in which there are no unique constraints.
 * 
 * On successful completion, addRecord() returns _OK_.  If an error
 * occurs, isTableError(tbl) will be TRUE for this table and
 * dbtblerror(tbl) will return a string that identifies the problem.
 * 
 * [EndDoc]
 */
{
  off_t where;
  off_t here;
  long thismuch;
  int i;

  /*
   * First, update the indexes (if any).  This will insure
   * that a record isn't added if a unique constraint is
   * violated.
   */
  i = addDBIndexes (tbl);
  if (i != _OK_) {
    return _ERROR_;
  }

  /*
   * Add the record to the data file.
   */
  where = sizeof (dbHeader) + tbl->hdr->sizeFields +
    (tbl->hdr->numRecords * tbl->hdr->sizeRecord);
  here = fileSeekBegin (tbl->fd, where);
  Assert (here == where);
  if (here != where) {
    tbl->dbError = DB_UNSPECIFIED;
    return _ERROR_;
  }
  field2Record (tbl);
  thismuch = fileWrite (tbl->fd, tbl->data, tbl->hdr->sizeRecord);
  Assert (thismuch == tbl->hdr->sizeRecord);
  if (thismuch != (long)tbl->hdr->sizeRecord) {
    tbl->dbError = DB_IO_READWRITE;
    return _ERROR_;
  }
  tbl->offset = here;		/* offset = beginning of record */
  tbl->hdr->numRecords++;
  tbl->crec = tbl->hdr->numRecords;
  if (tbl->hdr->numRecords % TABLE_FLUSH) {
    i = storeTableHeader (tbl);
    if (_OK_ != i)
      return _ERROR_;
    fileFlush (tbl->fd);	/* flush file */
  }
  return _OK_;
}


/*
 * [BeginDoc]
 * \subsection{massAddRecords}
 * \index{massAddRecords}
 * [Verbatim] */
ListHeader *massAddRecords (dbTable * tbl, ListHeader * lh)
/* [EndDoc] */
/*
 * [BeginDoc]
 * 
 * massAddRecords() adds the records stored in the list ``lh'' to the
 * table ``tbl''.  Each item in ``lh'' should be tbl->hdr->sizeRecord
 * in size and should be allocated dynamically on the heap.  The
 * easiest way to populate these items is to:
 * 
 * \begin{enumerate}
 * 
 * \item use the setXXXXXField() functions to populate the field items
 *    for this record.
 * 
 * \item use field2Record() to translate from the field values to the
 *   tbl->data member.
 * 
 * \item allocate a block of memory of tbl->hdr->sizeRecord size.
 * 
 * \item use memmov() to copy tbl->data to the allocated block.
 * 
 * \item add the allocated block to the list.
 * 
 * \end{enumerate}
 * 
 * massAddRecords() will attempt to add all the records to the table.
 * If it is unsuccessful because of a unique constraint violation, the
 * records that could not be added will be placed in a list and returned
 * to the caller.  It is the caller's responsibility to delete the list
 * that is returned and to clean up the items in ``lh''.  On successful
 * return, all the items in ``lh'' were successfully added to the table,
 * the items in the return list were not added because of unique constraints.
 * If an error occurs, isTableError(tbl) will be TRUE for this table and
 * dbtblerror(tbl) will return a string describing the error.
 * 
 * [EndDoc]
 */
{
  ListHeader *rtnList;
  Link *lnk;
  Link *rmLink;
  off_t where;
  off_t here;
  long thismuch;
  int i;
  rtnList = 0;
  lnk = lh->head->next;
  while (lnk != lh->tail) {
    memmove (tbl->data, lnk->data, tbl->hdr->sizeRecord);
    record2Field (tbl);

    /*status = addRecord (tbl); */
    i = addDBIndexes (tbl);
    if (i != _OK_) {
      if (tbl->dbError == DB_UNIQUE)
	goto uniqueError;
      if (rtnList != 0) {
	if (rtnList->number > 0)
	  if (clearList (rtnList)) return 0;
	delList (rtnList);
      }
      return 0;
    }
    where = sizeof (dbHeader) + tbl->hdr->sizeFields +
      (tbl->hdr->numRecords * tbl->hdr->sizeRecord);
    here = fileSeekBegin (tbl->fd, where);
    Assert (here == where);
    if (here != where) {
      tbl->dbError = DB_UNSPECIFIED;
      if (rtnList != 0) {
	if (rtnList->number > 0)
	  if (clearList (rtnList)) return 0;
	delList (rtnList);
	if (lh != 0 && lh->number > 0)
	  if (clearList (lh)) return 0;
	delList (lh);
      }
      return 0;
    }

    /*field2Record (tbl); */
    thismuch = fileWrite (tbl->fd, tbl->data, tbl->hdr->sizeRecord);
    Assert (thismuch == tbl->hdr->sizeRecord);
    if (thismuch != (long)tbl->hdr->sizeRecord) {
      tbl->dbError = DB_IO_READWRITE;
      if (rtnList != 0) {
	if (rtnList->number > 0)
	  if (clearList (rtnList)) return 0;
	delList (rtnList);
	if (lh != 0 && lh->number > 0)
	  if (clearList (lh)) return 0;
	delList (lh);
      }
      return 0;
    }
    tbl->offset = here;
    tbl->hdr->numRecords++;
    tbl->crec = tbl->hdr->numRecords;
  uniqueError:if (!(tbl->dbError == DB_NOERROR && dbError == DB_NOERROR)) {
      if (tbl->dbError == DB_UNIQUE) {
	lh->current = lnk;
	lnk = lnk->next;
	rmLink = removeLink (lh);
	if (rtnList == 0) {
	  rtnList = initList (UNSORTED, 0);
	  if (rtnList == 0) {
	    tbl->dbError = DB_LISTERROR;
	    return (ListHeader *) _ERROR_;
	  }
	}
	rtnList->current = rtnList->tail->prev;
	insertLinkHere (rtnList, rmLink);
	dbError = DB_NOERROR;
	tbl->dbError = DB_NOERROR;
	idxError = IDX_NOERR;
	set_shlError(0, SHELL_NOERR);
	set_ListError(0, LIST_NOERROR);
	continue;
      }

      else {			/* critical error */
	if (rtnList != 0) {

	  /*
	   * Things are honked up here, now, so delete the
	   * list and return.
	   */
	  if (rtnList->number > 0)
	    if (clearList (rtnList)) return 0;
	  delList (rtnList);
	}

	/* error should be set */
	return (ListHeader *) _ERROR_;
      }
    }
    lnk = lnk->next;
  }
  storeTableHeader (tbl);
  fileFlush (tbl->fd);		/* flush file only at the end */
  return rtnList;
}
