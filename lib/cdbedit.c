/*	Source File:	cdbedit.c	*/  
  
/*
 * [BeginDoc]
 * 
 * \section{DCDB Edit Functions}
 * 
 * This file contains the edit functions for the cdb module.
 * 
 * [EndDoc]
 */ 
  
#include <cdb.h>
#include <string.h>
  
/*
 * [BeginDoc]
 * \subsection{retrieveRecord}
 * \index{retrieveRecord}
 * [Verbatim] */ 
  off_t retrieveRecord (dbTable * tbl)  
/* [EndDoc] */ 
/*
 * [BeginDoc]
 * retrieveRecord() retrieves the current record from the table.
 * The current record is given by record number tbl->crec and is
 * tbl->offset bytes from the beginning of the data file.  This is
 * the function that actually reads data from the table.  The
 * movement functions *do not* read data.  So, if the user calls
 * gotoRecord(), no data is retrieved into the tbl->data item until
 * retrieveRecord() is called.
 * 
 * retrieveRecord() returns _ERROR_ on error, or the number of bytes
 * read on success.  It is an error if the number of bytes read is
 * not the same as the record size, in which case _ERROR_ is
 * returned.  If 0 is returned, that means that the record is marked
 * deleted and must be undeleted before the data can be retrieved.
 * In that case, tbl->data is set to all 0's.
 * 
 * [EndDoc]
 */ 
{
  off_t rd;
  memset (tbl->data, 0, tbl->hdr->sizeRecord);
  fileSeekBegin (tbl->fd, tbl->offset);
  rd = fileRead (tbl->fd, tbl->data, tbl->hdr->sizeRecord);
  Assert (rd == (off_t)tbl->hdr->sizeRecord);
  if (rd != (off_t)tbl->hdr->sizeRecord) {
    tbl->dbError = DB_IO_READWRITE;
    return _ERROR_;
  }
  if (recordDeleted (tbl)) {
    memset (tbl->data, 0, tbl->hdr->sizeRecord);
    return 0;
  }
  record2Field (tbl);
  return rd;
}


/*
 * [BeginDoc]
 * \subsection{updateRecord}
 * \index{updateRecord}
 * [Verbatim] */ 
int updateRecord (dbTable * tbl)  
/* [EndDoc] */ 
/*
 * [BeginDoc]
 * updateRecord() updates the current record in the table ``tbl'' with the
 * data stored in the tbl->fields[] strings.  This is copied into
 * the tbl->data field and written.  Update first stores the original record
 * as a backup to insure no data loss.  Then, it deletes all indexes
 * associated with the original record and replaces them with values from
 * the new record.  If that succeeds, it updates the record in the table
 * and returns _OK_.  If the replacement of indexes fails, updateRecord()
 * restores the table data to the old record and returns _ERROR_.  The
 * calling program should determine whether an error is the result of unique
 * constraints.  If it is, the caller should be careful to reindex the table
 * to get it back to the point of stability that it was before the call to
 * updateRecord().  \emph{See the sample programs for examples on how to do
 * this.}
 *
 * [EndDoc]
 */ 
{
  void *old_rec = 0;
  void *new_rec = 0;
  off_t status;
  off_t written;
  
    /*unsigned long crec; */ 
  char tblName[TABLE_NAME_WIDTH + 1];
  int i;
  int isNoIndex = TRUE;
  keyItem * ki, *rm;
  Link * lnk;
  dbIndex * idx = 0;
  midxField midx;
  char key[1024];
  int sizekey;
  
    /*
     * Here is the game plan:
     * 1. Store the original record as a backup.
     * 2. Delete all indexes associated with the original record.
     * 3. Replace the indexes with new indexes (here is where it is likely to
     *    fail).
     * 4. If that succeeds, replace the data with the new data and chuck the old
     *    data.  We're done.
     * 5. If that doesn't succeed, restore the old record to the tbl->data
     *    item and issue an error message.
     */ 
    for (i = 0; i < tbl->hdr->numFields && isNoIndex == TRUE; i++) {
    switch (tbl->fldAry[i]->indexed) {
    case ITYPE_UNIQUECASE:
    case ITYPE_UNIQUENOCASE:
    case ITYPE_DUPCASE:
    case ITYPE_DUPNOCASE:
      isNoIndex = FALSE;
      break;
    default:
      break;
    }
  }
  if (isNoIndex) {
    
      /*
       * No index to worry about...update in place.
       */ 
      memset (tbl->data, 0, tbl->hdr->sizeRecord);
    field2Record (tbl);
    fileSeekBegin (tbl->fd, tbl->offset);
    written = fileWrite (tbl->fd, tbl->data, tbl->hdr->sizeRecord);
    if (written != (off_t)tbl->hdr->sizeRecord) {
      tbl->dbError = DB_IO_READWRITE;
      return _ERROR_;
    }
    fileFlush (tbl->fd);
    return _OK_;
  }
  
    /*
     * First, store the new and old record.
     */ 
    /*crec = tbl->crec + 1; *//* crec is an offset 0 pointer */ 
    field2Record (tbl);
  new_rec = malloc (tbl->hdr->sizeRecord);
  if (new_rec == 0) {
    tbl->dbError = DB_NOMEMORY;
    return _ERROR_;
  }
  memset (new_rec, 0, tbl->hdr->sizeRecord);
  memmove (new_rec, tbl->data, tbl->hdr->sizeRecord);
  check_pointer (new_rec);
  strcpy (tblName, tbl->fileName);
  
    /*
     * OK, find the blasted indexes and delete them.
     */ 
  status = retrieveRecord (tbl);
  if (_ERROR_ == status) {
    return _ERROR_;
  }
  old_rec = malloc (tbl->hdr->sizeRecord);
  if (old_rec == 0) {
    tbl->dbError = DB_NOMEMORY;
    free (new_rec);
    return _ERROR_;
  }
  memset (old_rec, 0, tbl->hdr->sizeRecord);
  memmove (old_rec, tbl->data, tbl->hdr->sizeRecord);
  check_pointer (old_rec);
  
    /* First, deal with regular indexes */ 
    for (i = 0; i < tbl->hdr->numFields; i++) {
    if (tbl->fldAry[i]->indexed == ITYPE_UNIQUECASE
	 || tbl->fldAry[i]->indexed == ITYPE_UNIQUENOCASE
	 || tbl->fldAry[i]->indexed == ITYPE_DUPCASE
	 || tbl->fldAry[i]->indexed == ITYPE_DUPNOCASE) {
      ki = malloc (tbl->fldAry[i]->fieldLength + 1 + sizeof (unsigned long));
      if (0 == ki) {
	tbl->dbError = DB_NOMEMORY;
	return _ERROR_;
      }
      check_pointer (ki);
      strcpy (ki->key, tbl->fields[i]);
      ki->item = tbl->crec + 1;
      lnk = tbl->idxList->head->next;
      while (lnk != tbl->idxList->tail) {
	idx = lnk->data;
	if (!strcmp (tbl->fldAry[i]->indexName, idx->hdr->indexName))
	  break;
	lnk = lnk->next;
      }
      rm = deleteIndexItem (idx, ki);
      if (rm == 0) {
	
	  /* handle the error */ 
	  /*
	   * The old record is still in the table on disk...just return the
	   * error and let the caller reindex the table.
	   */ 
	  memset (tbl->data, 0, tbl->hdr->sizeRecord);
	memmove (tbl->data, new_rec, tbl->hdr->sizeRecord);
	record2Field (tbl);
	free (ki);
	tbl->dbError = DB_INDEX;
	goto returnError;
      }
      free (rm);
      free (ki);
    }
  }
  
    /* OK, now multi-indexes */ 
    if (tbl->midxList != 0
	&& tbl->midxList->head->next != tbl->midxList->tail) {
    lnk = tbl->midxList->head->next;
    while (lnk != tbl->midxList->tail) {
      idx = lnk->data;
      for (i = 0; tbl->hdr->midxInfo[i][0] != '\0'; i++)
	if (!strncmp
	     (tbl->hdr->midxInfo[i], idx->hdr->indexName,
	      strlen (idx->hdr->indexName)))
	  break;
      str2midxField (&midx, tbl->hdr->midxInfo[i]);
      key[0] = '\0';
      sizekey = 0;
      for (i = 0; i < MAX_IDX && midx.names[i] != 0; i++) {
	status = getFieldNum (tbl, midx.names[i]);
	strcat (key, tbl->fields[status]);
	sizekey += tbl->flens[status];
      }
      ki = malloc (sizekey + 1 + sizeof (unsigned long));
      if (0 == ki) {
	tbl->dbError = DB_NOMEMORY;
	goto returnError;
      }
      memset (ki, 0, sizekey + 1 + sizeof (unsigned long));
      strcpy (ki->key, key);
      ki->item = tbl->crec + 1;
      check_pointer (ki);
      rm = deleteIndexItem (idx, ki);
      if (0 == rm) {
	
	  /* handle the error */ 
	  memset (tbl->data, 0, tbl->hdr->sizeRecord);
	memmove (tbl->data, new_rec, tbl->hdr->sizeRecord);
	record2Field (tbl);
	free (ki);
	tbl->dbError = DB_INDEX;
	goto returnError;
      }
      for (i = 0; midx.names[i] != 0; i++)
	free (midx.names[i]);
      free (rm);
      free (ki);
      lnk = lnk->next;
    }
  }
  
    /*
     * Now, add the new indexes.
     */ 
    memset (tbl->data, 0, tbl->hdr->sizeRecord);
  memmove (tbl->data, new_rec, tbl->hdr->sizeRecord);
  check_pointer (tbl->data);
  record2Field (tbl);
  for (i = 0; i < tbl->hdr->numFields; i++) {
    if (tbl->fldAry[i]->indexed == ITYPE_UNIQUECASE
	 || tbl->fldAry[i]->indexed == ITYPE_UNIQUENOCASE
	 || tbl->fldAry[i]->indexed == ITYPE_DUPCASE
	 || tbl->fldAry[i]->indexed == ITYPE_DUPNOCASE) {
      ki = malloc (tbl->fldAry[i]->fieldLength + 1 + sizeof (unsigned long));
      if (0 == ki) {
	tbl->dbError = DB_NOMEMORY;
	goto returnError;
      }
      check_pointer (ki);
      strcpy (ki->key, tbl->fields[i]);
      ki->item = tbl->crec + 1;
      lnk = tbl->idxList->head->next;
      while (lnk != tbl->idxList->tail) {
	idx = lnk->data;
	if (!strcmp (tbl->fldAry[i]->indexName, idx->hdr->indexName))
	  break;
	lnk = lnk->next;
      }
      status = addIndexItem (idx, ki);
      if (status == _ERROR_) {
	
	  /* handle the error */ 
	  free (ki);
	tbl->dbError = DB_INDEX;
	goto returnError;
      }
    }
  }
  if (tbl->midxList != 0 && tbl->midxList->head->next != tbl->midxList->tail) {
    lnk = tbl->midxList->head->next;
    while (lnk != tbl->midxList->tail) {
      idx = lnk->data;
      for (i = 0; tbl->hdr->midxInfo[i][0] != '\0'; i++)
	if (!strncmp
	     (tbl->hdr->midxInfo[i], idx->hdr->indexName,
	      strlen (idx->hdr->indexName)))
	  break;
      str2midxField (&midx, tbl->hdr->midxInfo[i]);
      key[0] = '\0';
      sizekey = 0;
      for (i = 0; i < MAX_IDX && midx.names[i] != 0; i++) {
	status = getFieldNum (tbl, midx.names[i]);
	strcat (key, tbl->fields[status]);
	sizekey += tbl->flens[status];
      }
      ki = malloc (sizekey + 1 + sizeof (unsigned long));
      if (0 == ki) {
	tbl->dbError = DB_NOMEMORY;
	goto returnError;
      }
      memset (ki, 0, sizekey + 1 + sizeof (unsigned long));
      strcpy (ki->key, key);
      ki->item = tbl->crec + 1;
      check_pointer (ki);
      status = addIndexItem (idx, ki);
      if (_ERROR_ == status) {
	
	  /* handle the error */ 
	  memset (tbl->data, 0, tbl->hdr->sizeRecord);
	memmove (tbl->data, new_rec, tbl->hdr->sizeRecord);
	record2Field (tbl);
	free (ki);
	tbl->dbError = DB_INDEX;
	goto returnError;
      }
      for (i = 0; midx.names[i] != 0; i++)
	free (midx.names[i]);
      lnk = lnk->next;
    }
  }
  memset (tbl->data, 0, tbl->hdr->sizeRecord);
  field2Record (tbl);
  fileSeekBegin (tbl->fd, tbl->offset);
  written = fileWrite (tbl->fd, tbl->data, tbl->hdr->sizeRecord);
  if (written != (off_t)tbl->hdr->sizeRecord) {
    tbl->dbError = DB_IO_READWRITE;
    goto returnError;
  }
  fileFlush (tbl->fd);
  return _OK_;

returnError:
  free (old_rec);
  free (new_rec);
  return _ERROR_;
}

/*
 * [BeginDoc]
 * \subsection{recordDeleted}
 * \index{recordDeleted}
 * [Verbatim] */ 
int recordDeleted (dbTable * tbl)  
/* [EndDoc] */ 
/*
 * [BeginDoc]
 * 
 * recordDeleted() examines the data in tbl->data with the assumption
 * that this data has been recently received into memory from the
 * table.  If the first byte of the record is a "*", the record
 * is marked deleted and TRUE is returned.  Otherwise, FALSE is
 * returned.
 * 
 * [EndDoc]
 */ 
{
  if (*((char *) tbl->data) == '*')
    return TRUE;
  return FALSE;
}


/*
 * [BeginDoc]
 * \subsection{isRecordDeleted}
 * \index{isRecordDeleted}
 * [Verbatim] */ 
int isRecordDeleted (dbTable * tbl)  
/* [EndDoc] */ 
/*
 * [BeginDoc]
 * isRecordDeleted() retrieves the current record into tbl->data and
 * determines whether it has been deleted.  If so, it returns TRUE;
 * FALSE is returned otherwise.
 * 
 * [EndDoc]
 */ 
{
  off_t rd;
  memset (tbl->data, 0, tbl->hdr->sizeRecord);
  fileSeekBegin (tbl->fd, tbl->offset);
  rd = fileRead (tbl->fd, tbl->data, tbl->hdr->sizeRecord);
  Assert (rd == tbl->hdr->sizeRecord);
  if (rd != (off_t)tbl->hdr->sizeRecord) {
    tbl->dbError = DB_IO_READWRITE;
    return _ERROR_;
  }
  if (*((char *) tbl->data) == '*')
    return TRUE;
  return FALSE;
}


/*
 * [BeginDoc]
 * \subsection{deleteRecord}
 * \index{deleteRecord}
 * [Verbatim] */ 
int deleteRecord (dbTable * tbl)  
/* [EndDoc] */ 
/*
 * [BeginDoc]
 * deleteRecord() marks the current record for deletion.  It first
 * retrieves the contents of the record from the table, marks it
 * deleted and saves it.  Once marked as deleted, retrieveRecord()
 * will not retrieve the contents of the record unless it is
 * marked undeleted, even though the actual contents of the record
 * remain unchanged.
 * 
 * deleteRecord() returns _OK_ on success, _ERROR_ on error.
 * 
 * [EndDoc]
 */ 
{
  off_t written;
  if (isRecordDeleted (tbl))
    /* already deleted */ 
    return _OK_;
  *((char *) tbl->data) = '*';
  fileSeekBegin (tbl->fd, tbl->offset);
  written = fileWrite (tbl->fd, tbl->data, tbl->hdr->sizeRecord);
  Assert (written == tbl->hdr->sizeRecord);
  if (written != (off_t)tbl->hdr->sizeRecord) {
    tbl->dbError = DB_IO_READWRITE;
    return _ERROR_;
  }
  
  /*fileFlush (tbl->fd); */ 
   return _OK_;
}


/*
 * [BeginDoc]
 * \subsection{undeleteRecord}
 * \index{undeleteRecord}
 * [Verbatim] */ 
int undeleteRecord (dbTable * tbl)  
/* [EndDoc] */ 
/*
 * [BeginDoc]
 * undeleteRecord() unmarks a record that was previously marked as
 * deleted and then stores the record to disk.  After it is unmarked,
 * the data can be retrieved successfully by retrieveRecord().  This
 * function returns _OK_ on success, _ERROR_ on error.
 * [EndDoc]
 */ 
{
  off_t rd;
  off_t written;
  memset (tbl->data, 0, tbl->hdr->sizeRecord);
  fileSeekBegin (tbl->fd, tbl->offset);
  rd = fileRead (tbl->fd, tbl->data, tbl->hdr->sizeRecord);
  Assert (rd == tbl->hdr->sizeRecord);
  if (rd != (off_t)tbl->hdr->sizeRecord) {
    tbl->dbError = DB_IO_READWRITE;
    return _ERROR_;
  }
  if (*((char *) tbl->data) != '*')
    return _OK_;		/* not deleted; do nothing */
  *((char *) tbl->data) = '\0';
  fileSeekBegin (tbl->fd, tbl->offset);
  written = fileWrite (tbl->fd, tbl->data, tbl->hdr->sizeRecord);
  Assert (written == tbl->hdr->sizeRecord);
  if (written != (off_t)tbl->hdr->sizeRecord) {
    tbl->dbError = DB_IO_READWRITE;
    return _ERROR_;
  }
  
   /*fileFlush (tbl->fd); */ 
   return _OK_;
}


