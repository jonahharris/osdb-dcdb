/*	Source File:	cdbmove.c	*/  
  
/*
 * [BeginDoc]
 * 
 * \section{DCDB Table Traversal Functions}
 * 
 * This file contains the record traversing functions for the DCDB
 * module.  All the traversal functions set the current record
 * pointers for the table only; no data is read into the record
 * buffer until a call to retrieveRecord() is made.  This is done
 * for efficiency; the user does not always want to read data into
 * the data buffers when moving through the table.  However, it does
 * place the burden of insuring that retrieveRecord() is called after
 * each movement where the user wants to work with the data in the
 * record.
 * 
 * [EndDoc]
 */ 
  
#include <cdb.h>
  
/*
 * [BeginDoc]
 * \subsection{gotoRecord}
 * \index{gotoRecord}
 * [Verbatim] */ 
  off_t gotoRecord (dbTable * tbl, off_t recno)  
/* [EndDoc] */ 
/*
 * [BeginDoc]
 * gotoRecord() sets the file pointer to the beginning of the record
 * number given by ``recno'' in table ``tbl''.  If recno is too small 
 * or big, the current
 * record number becomes 0 or number of records, respectively.  No
 * data is read from the file by a call to gotoRecord(); the file
 * pointer is just moved to point to the record number specified.
 * This behavior is consistent in all the cdb move functions; no
 * data is retrieved until retrieveRecord() is called.
 * 
 * On success, gotoRecord() returns the offset into the table
 * where the current pointer is located.  If an error occurs,
 * isTableError(tbl) is TRUE for this table and dbtblerror(tbl)
 * returns a string that describes the error.
 * 
 * [EndDoc]
 */ 
{
  off_t loc;
  if (0 == tbl->hdr->numRecords)
    return tbl->offset;
  if (1 == tbl->hdr->numRecords) {
    tbl->crec = 0;
    tbl->offset = sizeof (dbHeader) + tbl->hdr->sizeFields;
    return tbl->offset;
  }
  if (recno > (off_t)tbl->hdr->numRecords) {
    tbl->eof = TRUE;
    recno = tbl->hdr->numRecords;
  }
  if (recno <= 0) {
    tbl->bof = TRUE;
    recno = 1;
  }
  tbl->crec = recno - 1;
  tbl->offset =
    sizeof (dbHeader) + tbl->hdr->sizeFields +
    tbl->hdr->sizeRecord * tbl->crec;
  loc = fileSeekBegin (tbl->fd, tbl->offset);
  Assert (loc == tbl->offset);
  if (loc != tbl->offset) {
    tbl->dbError = DB_UNSPECIFIED;
    return _ERROR_;
  }
  return tbl->offset;
}


/*
 * [BeginDoc]
 * \subsection{nextRecord}
 * \index{nextRecord}
 * [Verbatim] */ 
  off_t nextRecord (dbTable * tbl)  
/* [EndDoc] */ 
/*
 * [BeginDoc]
 * nextRecord() moves the file pointer to the next record in the
 * table given by ``tbl''.  Nothing happens if the table has no records.
 * If the table pointer is at the end of the table, nextRecord()
 * returns the offset to the last record and sets tbl->eof to TRUE.
 * 
 * nextRecord() returns the offset
 * into the table of the current record from the beginning of the 
 * file on success.  If an error occurs, isTableError(tbl) is TRUE
 * for tbl and dbtblerror(tbl) will return a string describing the
 * error.
 * 
 * [EndDoc]
 */ 
{
  off_t loc;
  if (0 == tbl->hdr->numRecords) {
    tbl->eof = TRUE;
    return tbl->offset;
  }
  if (1 == tbl->hdr->numRecords) {
    tbl->crec = 0;
    tbl->offset = sizeof (dbHeader) + tbl->hdr->sizeFields;
    tbl->eof = TRUE;
    return tbl->offset;
  }
  if (tbl->crec >= (off_t)tbl->hdr->numRecords - 1) {
    tbl->crec = tbl->hdr->numRecords - 1;
    tbl->eof = TRUE;
    tbl->offset =
      sizeof (dbHeader) + tbl->hdr->sizeFields +
      tbl->hdr->sizeRecord * tbl->crec;
  }
  /* BUGBUG - this doesn't make sense with tbl->crec of type unsigned long */
  /*
  else if (tbl->crec < 0) {
    tbl->crec = 0;
    tbl->offset = sizeof (dbHeader) + tbl->hdr->sizeFields;
    tbl->bof = tbl->eof = FALSE;
  }
  */
  /* BUGBUG */
  
  else {
    tbl->crec++;
    tbl->offset += tbl->hdr->sizeRecord;
    tbl->bof = tbl->eof = FALSE;
  }
  loc = fileSeekBegin (tbl->fd, tbl->offset);
  Assert (loc == tbl->offset);
  if (loc != tbl->offset) {
    tbl->dbError = DB_UNSPECIFIED;
    return _ERROR_;
  }
  return tbl->offset;
}


/*
 * [BeginDoc]
 * \subsection{nextIndexRecord}
 * \index{nextIndexRecord}
 * [Verbatim] */ 
  off_t nextIndexRecord (dbTable * tbl)  
/* [EndDoc] */ 
/*
 * [BeginDoc]
 * nextIndexRecord() takes the current record pointer for the table
 * ``tbl'' to the next record as given by the index tbl->current.
 * if tbl->current is not set, nextIndexRecord() decays to a call to
 * nextRecord(tbl).  If nextIndexRecord() results in the record
 * pointer going off of the end of the index, the last record by the
 * index is returned and tbl->eof is set to TRUE.  If an error occurs
 * in a call to nextIndexRecord(), isTableError(tbl) is TRUE for this
 * table and dbtblerror() returns a string that describes the error.
 * 
 * [EndDoc]
 */ 
{
  keyItem * ki;
  off_t loc;
  if (tbl->current == 0)
    return nextRecord (tbl);
  if (0 == tbl->hdr->numRecords) {
    tbl->eof = TRUE;
    return tbl->offset;
  }
  if (1 == tbl->hdr->numRecords) {
    tbl->crec = 0;
    tbl->offset = sizeof (dbHeader) + tbl->hdr->sizeFields;
    tbl->eof = TRUE;
    return tbl->offset;
  }
  if (tbl->current->isSequential == FALSE) {
    ki = firstIndexItem (tbl->current);
    if (idxError != IDX_NOERR)
      return _ERROR_;
    return (gotoRecord (tbl, ki->item));
  }
  if (tbl->eof == TRUE)
    return _OK_;
  ki = nextIndexItem (tbl->current);
  if (0 == ki) {
    if (idxError == IDX_NOERR) {
      tbl->eof = TRUE;
      tbl->crec = tbl->hdr->numRecords - 1;
      tbl->offset = sizeof (dbHeader) + tbl->hdr->sizeFields 
	+(tbl->hdr->numRecords - 1) * tbl->hdr->sizeRecord;
      loc = fileSeekBegin (tbl->fd, tbl->offset);
      Assert (loc == tbl->offset);
      if (loc != tbl->offset) {
	tbl->dbError = DB_UNSPECIFIED;
	return _ERROR_;
      }
      return _OK_;
    }
    
    else {
      tbl->dbError = DB_INDEX;
      return _ERROR_;
    }
  }
  if (ki->item > tbl->hdr->numRecords) {
    tbl->dbError = DB_INDEX;
    return _ERROR_;
  }
  gotoRecord (tbl, ki->item);
  if (tbl->current->eof == TRUE) {
    tbl->eof = TRUE;
    return _OK_;
  }
  return _OK_;
}


/*
 * [BeginDoc]
 * \subsection{prevRecord}
 * \index{prevRecord}
 * [Verbatim] */ 
  off_t prevRecord (dbTable * tbl)  
/* [EndDoc] */ 
/*
 * [BeginDoc]
 * prevRecord() is the same as nextRecord() except it moves back one
 * record in the table.  If the table pointer backs up to before the
 * first record, the offset of the first record is returned and 
 * tbl->bof is set to TRUE.  On error, isTableError(tbl) is TRUE
 * for tbl and dbtblerror(tbl) returns a string that describes the
 * error.
 * 
 * [EndDoc]
 */ 
{
  off_t loc;
  if (0 == tbl->hdr->numRecords) {
    tbl->bof = TRUE;
    return tbl->offset;
  }
  if (1 == tbl->hdr->numRecords) {
    tbl->crec = 0;
    tbl->offset = sizeof (dbHeader) + tbl->hdr->sizeFields;
    tbl->eof = TRUE;
    return tbl->offset;
  }
  if (tbl->crec <= 0) {
    tbl->crec = 0;
    tbl->offset = sizeof (dbHeader) + tbl->hdr->sizeFields;
    tbl->bof = TRUE;
  }
  
  else if (tbl->crec > (off_t)tbl->hdr->numRecords - 1) {
    tbl->crec = tbl->hdr->numRecords - 1;
    tbl->eof = FALSE;
    tbl->offset =
      sizeof (dbHeader) + tbl->hdr->sizeFields +
      tbl->hdr->sizeRecord * tbl->crec;
  }
  
  else {
    tbl->crec--;
    tbl->offset -= tbl->hdr->sizeRecord;
    tbl->bof = tbl->eof = FALSE;
  }
  loc = fileSeekBegin (tbl->fd, tbl->offset);
  Assert (loc == tbl->offset);
  if (loc != tbl->offset) {
    tbl->dbError = DB_UNSPECIFIED;
    return _ERROR_;
  }
  return tbl->offset;
}


/*
 * [BeginDoc]
 * \subsection{prevIndexRecord}
 * \index{prevIndexRecord}
 * [Verbatim] */ 
  off_t prevIndexRecord (dbTable * tbl)  
/* [EndDoc] */ 
/*
 * [BeginDoc]
 * prevIndexRecord() works the same as nextIndexRecord() except the
 * offset of the previous record by the current index is returned.
 * If the current record pointer moves past the first record, the
 * offset of the first record is returned and tbl->bof is set to
 * TRUE.  On error, isTableError(tbl) is TRUE
 * for tbl and dbtblerror(tbl) returns a string that describes the
 * error.
 * 
 * [EndDoc]
 */ 
{
  keyItem * ki;
  off_t loc;
  if (tbl->current == 0) {
    tbl->bof = TRUE;
    return prevRecord (tbl);
  }
  if (0 == tbl->hdr->numRecords) {
    tbl->bof = TRUE;
    return tbl->offset;
  }
  if (1 == tbl->hdr->numRecords) {
    tbl->crec = 0;
    tbl->offset = sizeof (dbHeader) + tbl->hdr->sizeFields;
    tbl->bof = TRUE;
    return tbl->offset;
  }
  if (tbl->current->isSequential == FALSE) {
    ki = lastIndexItem (tbl->current);
    if (idxError != IDX_NOERR)
      return _ERROR_;
    return (gotoRecord (tbl, ki->item));
  }
  if (tbl->bof == TRUE)
    return _OK_;
  ki = prevIndexItem (tbl->current);
  if (0 == ki) {
    if (idxError == IDX_NOERR) {
      tbl->bof = TRUE;
      tbl->crec = 0;
      tbl->offset = sizeof (dbHeader) + tbl->hdr->sizeFields;
      loc = fileSeekBegin (tbl->fd, tbl->offset);
      Assert (loc == tbl->offset);
      if (loc != tbl->offset) {
	tbl->dbError = DB_UNSPECIFIED;
	return _ERROR_;
      }
      return _OK_;
    }
    
    else {
      tbl->dbError = DB_INDEX;
      return _ERROR_;
    }
  }
  if (ki->item > tbl->hdr->numRecords) {
    tbl->dbError = DB_UNSTABLE;
    return _ERROR_;
  }
  gotoRecord (tbl, ki->item);
  if (tbl->current->bof == TRUE) {
    tbl->bof = TRUE;
    return _OK_;
  }
  return _OK_;
}


/*
 * [BeginDoc]
 * \subsection{headRecord}
 * \index{headRecord}
 * [Verbatim] */ 
  off_t headRecord (dbTable * tbl)  
/* [EndDoc] */ 
/*
 * [BeginDoc]
 * headRecord() moves the file pointer to the first record in the
 * table.  No data is read from the file and the same diagnostics
 * are returned as the other movement functions.  tbl->bof and
 * tbl->eof are set to FALSE if headRecord() is successful.
 * On error, isTableError(tbl) is TRUE
 * for tbl and dbtblerror(tbl) returns a string that describes the
 * error.
 * 
 * [EndDoc]
 */ 
{
  off_t loc;
  if (0 == tbl->hdr->numRecords)
    return tbl->offset;
  if (1 == tbl->hdr->numRecords) {
    tbl->crec = 0;
    tbl->offset = sizeof (dbHeader) + tbl->hdr->sizeFields;
    tbl->bof = tbl->eof = FALSE;
    return tbl->offset;
  }
  tbl->crec = 0;
  tbl->offset = sizeof (dbHeader) + tbl->hdr->sizeFields;
  loc = fileSeekBegin (tbl->fd, tbl->offset);
  Assert (loc == tbl->offset);
  if (loc != tbl->offset) {
    tbl->dbError = DB_UNSPECIFIED;
    return _ERROR_;
  }
  tbl->bof = tbl->eof = FALSE;
  return tbl->offset;
}


/*
 * [BeginDoc]
 * \subsection{headIndexRecord}
 * \index{headIndexRecord}
 * [Verbatim] */ 
  off_t headIndexRecord (dbTable * tbl)  
/* [EndDoc] */ 
/*
 * [BeginDoc]
 * headIndexRecord() moves the record pointers to the head of the
 * table by the current index.  tbl->eof and tbl->bof are set to
 * FALSE and an offset of the first record (by the index) is
 * returned.  If there is no current index, headIndexRecord() decays
 * to a call to headRecord().  On error, isTableError(tbl) is TRUE
 * for tbl and dbtblerror(tbl) returns a string that describes the
 * error.
 * 
 * [EndDoc]
 */ 
{
  keyItem * ki;
  if (tbl->current == 0)
    return headRecord (tbl);
  if (0 == tbl->hdr->numRecords)
    return tbl->offset;
  if (1 == tbl->hdr->numRecords) {
    tbl->crec = 0;
    tbl->offset = sizeof (dbHeader) + tbl->hdr->sizeFields;
    tbl->eof = tbl->bof = FALSE;
    return tbl->offset;
  }
  ki = firstIndexItem (tbl->current);
  if (idxError != IDX_NOERR) {
    tbl->dbError = DB_INDEX;
    return _ERROR_;
  }
  tbl->bof = tbl->eof = FALSE;
  return gotoRecord (tbl, ki->item);
}


/*
 * [BeginDoc]
 * \subsection{tailRecord}
 * \index{tailRecord}
 * [Verbatim] */ 
  off_t tailRecord (dbTable * tbl)  
/* [EndDoc] */ 
/*
 * [BeginDoc]
 * tailRecord() is the same as headRecord() except it moves the
 * file pointer to the last record in the file.  tbl->eof and
 * tbl->bof are both set to FALSE on a successful call to
 * tailRecord().  On error, isTableError(tbl) is TRUE
 * for tbl and dbtblerror(tbl) returns a string that describes the
 * error.
 * 
 * [EndDoc]
 */ 
{
  off_t loc;
  if (0 == tbl->hdr->numRecords)
    return tbl->offset;
  if (1 == tbl->hdr->numRecords) {
    tbl->crec = 0;
    tbl->offset = sizeof (dbHeader) + tbl->hdr->sizeFields;
    tbl->eof = tbl->bof = FALSE;
    return tbl->offset;
  }
  tbl->crec = tbl->hdr->numRecords - 1;
  tbl->offset = sizeof (dbHeader) + tbl->hdr->sizeFields 
    +(tbl->hdr->numRecords - 1) * tbl->hdr->sizeRecord;
  loc = fileSeekBegin (tbl->fd, tbl->offset);
  Assert (loc == tbl->offset);
  if (loc != tbl->offset) {
    tbl->dbError = DB_UNSPECIFIED;
    return _ERROR_;
  }
  tbl->bof = tbl->eof = FALSE;
  return tbl->offset;
}


/*
 * [BeginDoc]
 * \subsection{tailIndexRecord}
 * \index{tailIndexRecord}
 * [Verbatim] */ 
  off_t tailIndexRecord (dbTable * tbl)  
/* [EndDoc] */ 
/*
 * [BeginDoc]
 * tailIndexRecord() works the same as headIndexRecord(), except the
 * current record pointer is set to the last record by the current
 * index.  On error, isTableError(tbl) is TRUE
 * for tbl and dbtblerror(tbl) returns a string that describes the
 * error.
 * 
 * [EndDoc]
 */ 
{
  keyItem * ki;
  if (tbl->current == 0)
    return tailRecord (tbl);
  if (0 == tbl->hdr->numRecords)
    return tbl->offset;
  if (1 == tbl->hdr->numRecords) {
    tbl->crec = 0;
    tbl->offset = sizeof (dbHeader) + tbl->hdr->sizeFields;
    tbl->bof = tbl->eof = FALSE;
    return tbl->offset;
  }
  ki = lastIndexItem (tbl->current);
  if (idxError != IDX_NOERR)
    return _ERROR_;
  tbl->bof = tbl->eof = FALSE;
  return gotoRecord (tbl, ki->item);
}


/*
 * [BeginDoc]
 * \subsection{searchIndexRecord}
 * \index{searchIndexRecord}
 * [Verbatim] */ 
  off_t searchIndexRecord (dbTable * tbl, char *fieldValue) 
/* [EndDoc] */ 
/*
 * [BeginDoc]
 * searchIndexRecord() searches for the item given by ``fieldValue''
 * in the current index for the table given by ``tbl''.  If there
 * is no current index or the table has no records, _NOTFOUND_ is returned.
 * On success, 
 * the tbl->eof and tbl->bof are set to FALSE and offset of the
 * record ``found'' is returned.  The currency pointers for the 
 * table are set to the ``found'' record.  searchIndexRecord()
 * uses a best fit algorithm.  If an exact match is not found,
 * searchIndexRecord() returns the next best fit for a match.  For
 * example, if the item being searched for is less than or equal
 * to anything else in the index, the first record by the current
 * index is returned.  Otherwise, the returned item indicates where
 * the searched for item would be inserted on an add.
 * 
 * On error, isTableError(tbl) is TRUE
 * for tbl and dbtblerror(tbl) returns a string that describes the
 * error.
 * 
 * [EndDoc]
 */ 
{
  keyItem * ki;
  Link * found;
  Link * lnk;
  if (tbl->current == 0)
    return _NOTFOUND_;
  if (0 == tbl->hdr->numRecords)
    return _NOTFOUND_;
  ki = malloc (tbl->current->hdr->itemSize);
  if (0 == ki) {
    tbl->dbError = DB_NOMEMORY;
    return (off_t) _ERROR_;
  }
  memset (ki, 0, tbl->current->hdr->itemSize);
  check_pointer (ki);
  lnk = malloc (sizeof (Link));
  if (lnk == 0) {
    tbl->dbError = DB_NOMEMORY;
    return (off_t) _ERROR_;
  }
  check_pointer (lnk);
  lnk->data = ki;
  memmove (ki->key, (void *) fieldValue,
	    tbl->current->hdr->itemSize - sizeof (unsigned long) - 1);
  
    /*ki->key[tbl->current->hdr->itemSize-sizeof(unsigned long)-1] = '\0'; */ 
    check_pointer (ki);
  found = searchIndexItem (tbl->current, lnk);
  free (lnk);
  free (ki);
  if (found == 0 || found->data == 0)
    return _NOTFOUND_;
  tbl->bof = tbl->eof = FALSE;
  ki = found->data;
  if (gotoRecord (tbl, ki->item) == _ERROR_)
    return _ERROR_;
  return _FOUND_;
}


/*
 * [BeginDoc]
 * \subsection{searchExactIndexRecord}
 * \index{searchExactIndexRecord}
 * [Verbatim] */ 
  off_t searchExactIndexRecord (dbTable * tbl, char *fieldValue) 
/* [EndDoc] */ 
/*
 * [BeginDoc]
 * searchExactIndexRecord() returns _NOTFOUND_ unless an exact match is made
 * in the search.  Otherwise, it works exactly like searchIndexRecord().
 * On error, isTableError(tbl) is TRUE
 * for tbl and dbtblerror(tbl) returns a string that describes the
 * error.  See page \pageref{ErrorHandling} for information on properly handling
 * errors.  My recommendation is that you do something as follows:
 * [Verbatim]

setCurrentIndex (tbl, "lastname");
if (isTableError(tbl) {...handle the error...}
status = searchExactIndexRecord (tbl, "May");
if (isTableError(tbl) {...handle the error...}
if (status == _NOTFOUND_) {...didn't find it...}
else {...found it...}
 
 * [EndDoc]
 */ 
{
  keyItem * ki;
  Link * lnk;
  Link * found;
  
/*	printf ("searching %s\n", fieldValue);*/ 
    if (tbl->current == 0)
    return _NOTFOUND_;
  if (0 == tbl->hdr->numRecords)
    return _NOTFOUND_;
  ki = malloc (tbl->current->hdr->itemSize);
  if (0 == ki) {
    tbl->dbError = DB_NOMEMORY;
    return (off_t) _ERROR_;
  }
  check_pointer (ki);
  lnk = malloc (sizeof (Link));
  if (lnk == 0) {
    tbl->dbError = DB_NOMEMORY;
    return (off_t) _ERROR_;
  }
  check_pointer (lnk);
  lnk->data = ki;
  memmove (ki->key, (void *) fieldValue,
	    tbl->current->hdr->itemSize - sizeof (unsigned long) - 1);
  ki->key[tbl->current->hdr->itemSize - sizeof (unsigned long) - 1] = '\0';
  check_pointer (ki);
  check_pointer (lnk);
  found = searchExactIndexItem (tbl->current, lnk);
  free (lnk);
  free (ki);
  tbl->bof = tbl->eof = FALSE;
  if (found == 0)
    return 0;
  ki = found->data;
  if (gotoRecord (tbl, ki->item) == _ERROR_)
    return _ERROR_;
  return _FOUND_;
}


