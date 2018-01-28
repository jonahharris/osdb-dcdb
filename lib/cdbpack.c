/*	Source File:	cdbpack.c	*/

#include <cdb.h>
#include <test.h>
#include <stdio.h>

/*
 * [BeginDoc]
 * 
 * \section{DCDB Pack and Reindex Routines}
 * 
 * These are the routines that allow the user to pack a DCDB table
 * and reindex a DCDB table.
 * 
 * [EndDoc]
 */

static void *tblBuffer[TABLE_BUFFER_SIZE];

/* internal function */
int str2midxField (midxField * midx, char *str)
{
  char *cp, *nextField;
  char tempMidxStr[TABLE_INFO_WIDTH * 2];
  int j;

  /*
   * midxInfo string looks like this:
   * 
   * indexName:blkSize:case|nocase:field1,field2,...
   */
  strcpy (tempMidxStr, str);
  cp = strchr (tempMidxStr, ':');
  if (cp == 0) {
    dbError = DB_PARAM;
    return _ERROR_;
  }
  *cp = '\0';
  strcpy (midx->indexName, tempMidxStr);
  nextField = cp + 1;
  cp = strchr (nextField, ':');
  if (*cp == 0) {
    dbError = DB_PARAM;
    return _ERROR_;
  }
  *cp = '\0';
  midx->blkSize = atoi (nextField);
  nextField = cp + 1;
  cp = strchr (nextField, ':');
  if (cp == 0) {
    dbError = DB_PARAM;
    return _ERROR_;
  }
  *cp = '\0';
  if (!strcmp (nextField, "case"))
    midx->isCase = TRUE;
  else
    midx->isCase = FALSE;
  midx->isUnique = FALSE;
  nextField = cp + 1;
  cp = strchr (nextField, ',');
  j = 0;
  while (cp != 0) {
    *cp = '\0';
    if (j >= MAX_IDX - 1)
      break;
    midx->names[j] = malloc (strlen (nextField) + 1);
    if (midx->names[j] == 0) {
      dbError = DB_NOMEMORY;
      return _ERROR_;
    }
    strcpy (midx->names[j], nextField);
    midx->names[j + 1] = 0;
    j++;
    nextField = cp + 1;
    cp = strchr (nextField, ',');
  }
  if (j < MAX_IDX - 1) {
    midx->names[j] = malloc (strlen (nextField) + 1);
    if (midx->names[j] == 0) {
      dbError = DB_NOMEMORY;
      return _ERROR_;
    }
    strcpy (midx->names[j], nextField);
    midx->names[j + 1] = 0;
  }
  return _OK_;
}

/* internal function */
int midxField2str (midxField * midx, char *str)
{
  char tempInfoStr[TABLE_INFO_WIDTH * 2];
  char blkSize[10];
  int i;

  strcpy (tempInfoStr, midx->indexName);
  strcat (tempInfoStr, ":");
  if (midx->blkSize == 0)
    sprintf (blkSize, "%d", DEFAULT_BLOCK_SIZE);
  else
    sprintf (blkSize, "%d", midx->blkSize);
  if (strlen (tempInfoStr) + strlen (blkSize) >= TABLE_INFO_WIDTH * 2) {
    dbError = DB_PARAM;
    return _ERROR_;
  }
  strcat (tempInfoStr, blkSize);
  strcat (tempInfoStr, ":");
  if (midx->isCase) {
    if (strlen (tempInfoStr) + strlen ("case") >= TABLE_INFO_WIDTH * 2) {
      dbError = DB_PARAM;
      return _ERROR_;
    }
    strcat (tempInfoStr, "case");
  }
  else {
    if (strlen (tempInfoStr) + strlen ("nocase") >= TABLE_INFO_WIDTH * 2) {
      dbError = DB_PARAM;
      return _ERROR_;
    }
    strcat (tempInfoStr, "nocase");
  }
  strcat (tempInfoStr, ":");
  for (i = 0; i < MAX_IDX && midx->names[i] != 0; i++) {
    if (strlen (tempInfoStr) + strlen (midx->names[i]) >=
	TABLE_INFO_WIDTH * 2) {
      dbError = DB_PARAM;
      return _ERROR_;
    }
    strcat (tempInfoStr, midx->names[i]);
    if (midx->names[i + 1] != 0)
      strcat (tempInfoStr, ",");
  }

  strcpy (str, tempInfoStr);
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{packTable}
 * \index{packTable}
 * [Verbatim] */

dbTable *packTable (dbTable * tbl, int save)

/* [EndDoc] */
/*
 * [BeginDoc]
 * 
 * The packTable() function packs the records in the table ``tbl''.  Any
 * records flagged as deleted are not copied to the new table when it is
 * packed.  If ``save'' is FALSE, the old table is deleted and replaced
 * by the new, packed table.  If ``save'' is TRUE, a copy of the old table
 * without indexes will be saved in the file ``.cdbpack.packTable.tmp.db''.
 * This can then be stored by the application in whatever form is required.
 * The file name ``.cdbpack.packTable.tmp.db'' is used for each pack in 
 * which the file is saved, so if something is not done with the backup
 * copy of the table right away, it will be deleted and replaced.
 * 
 * Upon success, packTable() returns a pointer to an open, newly packed
 * table.  If an error occurs, isDBError() will be TRUE and dberror() will
 * return a string that describes the error.
 * 
 * It should be noted that packing a table can take considerable time,
 * depending on how many records are in the table.  Also, the system
 * tends to become bogged down reading from and writing to the same disk
 * (disk contention).  Therefore, packing of tables should be done
 * sparingly for performance sake.
 * 
 * [EndDoc]
 */
{
  dbTable *ret;
  dbHeader *dbh;
  dbField **flds;
  char tblName[TABLE_NAME_WIDTH + 1];
  char idxname[MAX_IDX][INDEX_NAME_WIDTH + 1];
  char tmpname[TABLE_NAME_WIDTH + 1];
  Link *lnk;
  dbIndex *idx;
  int i;
  int status = _OK_;
  int isUnique = 0, isCase = 0;
  midxField midx[MAX_MIDX + 1];
  int counter;
  int j;

  if (tbl->idxList != 0) {
    if (tbl->idxList->number >= MAX_IDX) {
      dbError = DB_TOOMANYIDX;
      return 0;
    }
  }
  /* first, delete indexes and generate temporary names */
  idxname[0][0] = '\0';
  if (tbl->idxList != 0) {
    if (tbl->idxList->number > 0) {
      for (i = 0, lnk = tbl->idxList->head->next;
	   lnk != tbl->idxList->tail && i < MAX_IDX; i++, lnk = lnk->next) {
	idx = lnk->data;
	strcpy (idxname[i], idx->hdr->indexName);
	idxname[i + 1][0] = '\0';
      }
    }
  }
  midx[0].indexName[0] = '\0';
  if (tbl->midxList != 0) {
    if (tbl->midxList->number > 0) {
      for (i = 0, lnk = tbl->midxList->head->next;
	   lnk != tbl->midxList->tail && i < MAX_MIDX; i++, lnk = lnk->next) {
	idx = lnk->data;
	strcpy (midx[i].indexName, idx->hdr->indexName);
	midx[i].isCase = idx->hdr->isCase;
	midx[i].isUnique = FALSE;
	midx[i].blkSize = idx->hdr->blockSize;
	midx[i + 1].indexName[0] = '\0';
      }
    }
  }
  if (tbl->idxList != 0) {
    status = closeDBIndexes (tbl);
    if (status == _ERROR_) {
      dbError = tbl->dbError;
      return 0;
    }
    tbl->idxList = 0;
  }
  if (tbl->midxList != 0) {
    status = closeMultiIndexes (tbl);
    if (status == _ERROR_) {
      dbError = tbl->dbError;
      return 0;
    }
    tbl->midxList = 0;
  }
  for (i = 0; i < MAX_IDX; i++) {
    if (idxname[i][0] == '\0')
      break;
    strcpy (tmpname, idxname[i]);
    strcat (tmpname, ".idx");
    status = fileRemove (tmpname);
    Assert (status != _ERROR_);
    if (status == _ERROR_) {
      dbError = fioError;
      return 0;
    }
    strcpy (tmpname, idxname[i]);
    strcat (tmpname, ".inx");
    status = fileRemove (tmpname);
    if (status == _ERROR_) {
      dbError = fioError;
      return 0;
    }
  }
  for (i = 0; i < MAX_MIDX; i++) {
    if (midx[i].indexName[0] == '\0')
      break;
    strcpy (tmpname, midx[i].indexName);
    strcat (tmpname, ".idx");
    status = fileRemove (tmpname);
    if (status == _ERROR_) {
      dbError = fioError;
      return 0;
    }
    strcpy (tmpname, midx[i].indexName);
    strcat (tmpname, ".inx");
    status = fileRemove (tmpname);
    if (status == _ERROR_) {
      dbError = fioError;
      return 0;
    }
  }
  if (tbl->idxList != 0) {
    delList (tbl->idxList);
    tbl->idxList = 0;
  }
  if (tbl->midxList != 0) {
    delList (tbl->midxList);
    tbl->midxList = 0;
  }
  while (TRUE) {
    memset (tmpname, 0, TABLE_NAME_WIDTH + 1);
#ifndef __MINGW32__
    status = testString (tmpname, 10, 0);
    if (status == _ERROR_) {
      dbError = DB_UNSPECIFIED;
      return 0;
    }
#endif /* __MINGW32__ */
#ifdef __MINGW32__
    status++;
    snprintf (tmpname, TABLE_NAME_WIDTH, "%s%d", "__MGW_tmp__", status);
#endif /* __MINGW32__ */
    strcpy (tblName, tmpname);
    strcat (tblName, ".db");
    if (!fexists ((char *) tblName))
      break;
  }

  /*
   * Allocate a table and copy fields over.
   */
  dbh = malloc (sizeof (dbHeader));
  if (0 == dbh) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (dbh, 0, sizeof (dbHeader));
  memmove (dbh, tbl->hdr, sizeof (dbHeader));
  dbh->numRecords = 0;
  flds = malloc ((dbh->numFields + 1) * sizeof (dbField *));
  if (flds == 0) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (flds, 0, (dbh->numFields + 1) * sizeof (dbField *));
  for (i = 0; i < dbh->numFields; i++) {
    flds[i] = malloc (sizeof (dbField));
    if (0 == flds[i]) {
      for (status = 0; status < i; status++)
	free (flds[status]);
      free (flds);
      return 0;
    }
    memset (flds[i], 0, sizeof (dbField));
    memmove (flds[i], tbl->fldAry[i], sizeof (dbField));
    check_pointer (flds[i]);
  }
  flds[dbh->numFields] = 0;
  /*
   * Create the new table and start copying records.
   */
  ret = createTable (tblName, dbh, flds);
  if (ret == 0) {
    return 0;
  }
  ret->idxList = 0;
  for (i = 0; i < ret->hdr->numFields; i++) {
    if (ret->fldAry[i]->indexed <= ITYPE_NOINDEX ||
	ret->fldAry[i]->indexed >= ITYPE_LAST)
      continue;
    strcpy (tmpname, ret->fldAry[i]->indexName);
    strcat (tmpname, ".idx");
    switch (ret->fldAry[i]->indexed) {
    case ITYPE_UNIQUECASE:
      isUnique = TRUE;
      isCase = TRUE;
      break;
    case ITYPE_DUPCASE:
      isUnique = FALSE;
      isCase = TRUE;
      break;
    case ITYPE_UNIQUENOCASE:
      isUnique = TRUE;
      isCase = FALSE;
      break;
    case ITYPE_DUPNOCASE:
      isUnique = FALSE;
      isCase = FALSE;
      break;
    case ITYPE_NOINDEX:	/* just to squelch warnings */
    case ITYPE_LAST:
      break;
    }
    status = createIndexFile (tmpname, ret->fldAry[i]->indexBlkSize,
			      ret->fldAry[i]->fieldLength + 1,
			      isUnique, isCase);
    if (status != _OK_) {
      dbError = DB_INDEX;
      return 0;
    }
    idx = openIndex (tmpname, 0, ret->fldAry[i]->indexBlkSize);
    if (0 == idx) {
      dbError = DB_INDEX;
      return 0;
    }
    if (ret->idxList == 0) {
      ret->idxList = initList (UNSORTED, 0);
      if (0 == ret->idxList) {
	dbError = DB_LISTERROR;
	return 0;
      }
    }
    idx->hdr->magic = IDX_MAGIC;
    strcpy (idx->hdr->indexName, ret->fldAry[i]->indexName);
    closeIndexFile (idx, 0);
    idx = openIndex (tmpname, 0, ret->fldAry[i]->indexBlkSize);
    if (0 == idx) {
      dbError = DB_INDEX;
      return 0;
    }
    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      dbError = DB_NOMEMORY;
      return 0;
    }
    lnk->data = idx;
    ret->idxList->current = ret->idxList->tail->prev;
    insertLinkHere (ret->idxList, lnk);
    ret->current = idx;
    storeFieldHeader (i, ret);
  }
  /* Create multi indexes */
  ret->midxList = 0;
  for (i = 0; i < MAX_MIDX; i++) {
    if (tbl->hdr->midxInfo[i][0] == '\0')
      break;
    for (counter = 0; counter < MAX_MIDX; counter++) {
      if (midx[counter].indexName[0] == '\0')
	break;
      if (!strncmp (tbl->hdr->midxInfo[i], midx[counter].indexName,
		    strlen (midx[counter].indexName)))
	break;
    }
    if (counter >= MAX_MIDX)
      break;
    status = str2midxField (&midx[counter], tbl->hdr->midxInfo[i]);
    if (_ERROR_ == status) {
      return 0;
    }
    memset (ret->hdr->midxInfo[i], 0, TABLE_INFO_WIDTH * 2);
    status = createMultiIndex (ret, midx[counter].indexName,
			       midx[counter].names,
			       midx[counter].isCase, midx[counter].blkSize);
    if (_ERROR_ == status) {
      dbError = DB_INDEX;
      return 0;
    }
    for (j = 0; midx[counter].names[j] != 0; j++)
      free (midx[counter].names[j]);
  }
  storeTableHeader (ret);
  closeTable (ret);
  ret = openTable (tblName);
  if (0 == ret)
    return 0;
  tbl->current = 0;

  memset (tblBuffer, 0, TABLE_BUFFER_SIZE * sizeof (void *));
  for (i = 0; i < TABLE_BUFFER_SIZE; i++) {
    tblBuffer[i] = malloc (tbl->hdr->sizeRecord);
    if (tblBuffer[i] == 0) {
      dbError = DB_NOMEMORY;
      closeTable (ret);
      closeTable (tbl);
      return 0;
    }
  }
  headRecord (tbl);
  counter = tbl->hdr->numRecords;
  while (counter > TABLE_BUFFER_SIZE) {
    /*
     * Copy to table buffer
     */
    for (i = 0, j = 0; i < TABLE_BUFFER_SIZE && counter >= 0;
	 i++, counter--, nextRecord (tbl)) {
      if (isRecordDeleted (tbl)) {
	i--;
	continue;
      }
      memmove (tblBuffer[i], tbl->data, tbl->hdr->sizeRecord);
      j++;
    }
    /*
     * now, copy to return table
     */
    for (i = 0; j > 0; i++, j--) {
      memmove (ret->data, tblBuffer[i], tbl->hdr->sizeRecord);
      record2Field (ret);
      status = addRecord (ret);
      if (_ERROR_ == status || isTableError (ret)) {
	/* Just skip any unique index violations */
	if (ret->dbError == DB_UNIQUE) {
	  ret->dbError = DB_NOERROR;
	}
	else {
	  dbError = ret->dbError;
	  closeTable (tbl);
	  closeTable (ret);
	  return 0;
	}
      }
    }
    /*
     * Set up to do it again.
     */
  }
  /*for (i = 0; i < TABLE_BUFFER_SIZE; i++)
     memset (tblBuffer[i], 0, tbl->hdr->sizeRecord); */
  for (i = 0, j = 0; i < TABLE_BUFFER_SIZE && counter > 0 && !tbl->eof;
       i++, counter--, nextRecord (tbl)) {
    if (isRecordDeleted (tbl)) {
      i--;
      continue;
    }
    memmove (tblBuffer[i], tbl->data, tbl->hdr->sizeRecord);
    j++;
  }
  for (i = 0; j > 0; i++, j--) {
    memmove (ret->data, tblBuffer[i], tbl->hdr->sizeRecord);
    record2Field (ret);
    status = addRecord (ret);
    if (_ERROR_ == status || isTableError (ret)) {
      /* Skip unique index violations */
      if (ret->dbError == DB_UNIQUE) {
	ret->dbError = DB_NOERROR;
      }
      else {
	dbError = ret->dbError;
	closeTable (tbl);
	closeTable (ret);
	return 0;
      }
    }
  }
  strcpy (tmpname, tbl->fileName);
  tbl->idxList = 0;
  tbl->midxList = 0;
  closeTable (tbl);
  if (!save) {
    status = fileRemove (tmpname);
    if (status == _ERROR_) {
      dbError = fioError;
      return 0;
    }
  }
  else {
    if (fexists ((char *) ".cdbpack.packTable.tmp.db")) {
      status = fileRemove (".cdbpack.packTable.tmp.db");
      if (status == _ERROR_) {
	dbError = fioError;
	return 0;
      }
    }
    status = fileRename (tmpname, ".cdbpack.packTable.tmp.db");
    if (status == _ERROR_) {
      dbError = fioError;
      return 0;
    }
  }
  closeTable (ret);
  if (isDBError ())
    return 0;
  status = fileRename (tblName, tmpname);
  if (status == _ERROR_) {
    dbError = fioError;
    return 0;
  }
  ret = openTable (tmpname);
  if (ret == 0 || isDBError () || isTableError (ret)) {
    if (ret != 0)
      closeTable (ret);
    return 0;
  }
  for (i = 0; i < TABLE_BUFFER_SIZE; i++)
    free (tblBuffer[i]);
  return ret;
}

/*
 * [BeginDoc]
 * \subsection{reindexTable}
 * \index{reindexTable}
 * [Verbatim] */

int reindexTable (const char *tblname)

/* [EndDoc] */
/*
 * [BeginDoc]
 * reindexTable() reindexes the closed table given by ``tblname''.  The
 * table is opened without index support, all indexes associated with the
 * table are deleted and then are rebuilt.  Upon successful completion,
 * the table and associated (newly built) indexes are closed and _OK_
 * is returned.  If an error occurs, isDBError() is TRUE and dberror()
 * will return a string that describes the error.
 * 
 * Reindexing is a time-consuming process that should be used sparingly
 * for performance sake.  Also, it is possible that an error in the
 * process of reindexing a table could leave the table without indexes.
 * However, this is no worse than having the data table with corrupted
 * indexes (which is, conceivably, why you would use reindexTable()).
 * 
 * [EndDoc]
 */
{
  dbTable *tbl;
  dbIndex *idx = 0;
  int status;
  char tstr1[INDEX_NAME_WIDTH + 1];
  char tstr2[INDEX_NAME_WIDTH + 1];
  midxField midx[MAX_MIDX + 1];
  midxField smidx;
  int i, j, counter;
  Link *lnk;
  keyItem *ki;
  int isUnique = FALSE, isCase = TRUE;
  int fldLength;
  int numidx, sizekey;
  char key[1024];

  tbl = openTableNoIndexes (tblname);
  if (0 == tbl) {
    /* tbl->dbError is set */
    return _ERROR_;
  }
  tbl->idxList = 0;
  tbl->midxList = 0;

  for (i = 0; i < tbl->hdr->numFields; i++) {
    if (strlen (tbl->fldAry[i]->indexName) != 0) {
      strcpy (tstr1, tbl->fldAry[i]->indexName);
      strcat (tstr1, ".idx");
      strcpy (tstr2, tbl->fldAry[i]->indexName);
      strcat (tstr2, ".inx");
      switch (tbl->fldAry[i]->indexed) {
      case ITYPE_UNIQUECASE:
	isUnique = TRUE;
	isCase = TRUE;
	break;
      case ITYPE_UNIQUENOCASE:
	isUnique = TRUE;
	isCase = FALSE;
	break;
      case ITYPE_DUPCASE:
	isUnique = FALSE;
	isCase = TRUE;
	break;
      case ITYPE_DUPNOCASE:
	isUnique = FALSE;
	isCase = FALSE;
	break;
      default:
	/* should do some error handling here, but... */
	break;
      }

      if (fexists (tstr1)) {
	status = fileRemove (tstr1);
	if (status == _ERROR_) {
	  dbError = fioError;
	  return _ERROR_;
	}
      }
      if (fexists (tstr2)) {
	status = fileRemove (tstr2);
	if (status == _ERROR_) {
	  dbError = fioError;
	  return _ERROR_;
	}
      }
      status = createIndexFile (tstr1, tbl->fldAry[i]->indexBlkSize,
				tbl->fldAry[i]->fieldLength + 1,
				isUnique, isCase);
      if (status == _ERROR_) {
	dbError = DB_INDEX;
	return _ERROR_;
      }
      idx = openIndex (tstr1, 0, tbl->fldAry[i]->indexBlkSize);
      if (0 == idx) {
	dbError = DB_INDEX;
	return _ERROR_;
      }
      idx->hdr->magic = IDX_MAGIC;
      strcpy (idx->hdr->indexName, tbl->fldAry[i]->indexName);
      status = closeIndexFile (idx, 0);
      if (status == _ERROR_) {
	dbError = DB_INDEX;
	goto errorExit;
      }
      idx = openIndex (tstr1, 0, tbl->fldAry[i]->indexBlkSize);
      if (0 == idx) {
	dbError = DB_INDEX;
	return _ERROR_;
      }
      if (tbl->idxList == 0) {
	tbl->idxList = initList (UNSORTED, 0);
	if (tbl->idxList == 0) {
	  dbError = DB_LISTERROR;
	  closeIndexFile (idx, 0);
	  goto errorExit;
	}
      }
      lnk = malloc (sizeof (Link));
      if (0 == lnk) {
	dbError = DB_NOMEMORY;
	closeIndexFile (idx, 0);
	goto errorExit;
      }
      lnk->data = idx;
      tbl->idxList->current = tbl->idxList->tail->prev;
      insertLinkHere (tbl->idxList, lnk);
    }
  }
  if (tbl->idxList != 0)
    if (tbl->idxList->number > 0)
      tbl->current = tbl->idxList->tail->prev->data;

  for (i = 0; i < MAX_MIDX; i++) {
    if (tbl->hdr->midxInfo[i][0] == '\0')
      break;
    str2midxField (&midx[i], tbl->hdr->midxInfo[i]);
  }
  for (i = 0; i < MAX_MIDX; i++) {
    if (tbl->hdr->midxInfo[i][0] == '\0')
      break;
    for (counter = 0; counter < MAX_MIDX; counter++) {
      if (midx[counter].indexName[0] == '\0')
	break;
      if (!strncmp (tbl->hdr->midxInfo[i], midx[counter].indexName,
		    strlen (midx[counter].indexName)))
	break;
    }
    strcpy (tstr1, midx[counter].indexName);
    strcat (tstr1, ".idx");
    strcpy (tstr2, midx[counter].indexName);
    strcat (tstr2, ".inx");
    if (fexists (tstr1)) {
      status = fileRemove (tstr1);
      if (status == _ERROR_) {
	dbError = fioError;
	return _ERROR_;
      }
    }
    if (fexists (tstr2)) {
      status = fileRemove (tstr2);
      if (status == _ERROR_) {
	dbError = fioError;
	return _ERROR_;
      }
    }
    fldLength = 0;
    for (j = 0; j < MAX_MIDX; j++) {
      if (midx[counter].names[j] == 0)
	break;
      status = getFieldNum (tbl, midx[counter].names[j]);
      if (status == _ERROR_) {
	dbError = DB_INDEX;
	goto errorExit;
      }
      fldLength += tbl->flens[status];
    }
    status = createIndexFile (tstr1, midx[counter].blkSize,
			      fldLength + 1, midx[counter].isCase, FALSE);
    if (status == _ERROR_) {
      dbError = DB_INDEX;
      return _ERROR_;
    }
    idx = openIndex (tstr1, 0, tbl->fldAry[i]->indexBlkSize);
    if (0 == idx) {
      dbError = DB_INDEX;
      return _ERROR_;
    }
    strcpy (idx->hdr->indexName, midx[counter].indexName);

    if (tbl->midxList == 0) {
      tbl->midxList = initList (UNSORTED, 0);
      if (tbl->midxList == 0) {
	dbError = DB_LISTERROR;
	closeIndexFile (idx, 0);
	goto errorExit;
      }
    }
    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      dbError = DB_NOMEMORY;
      closeIndexFile (idx, 0);
      goto errorExit;
    }
    lnk->data = idx;
    tbl->midxList->current = tbl->midxList->tail->prev;
    insertLinkHere (tbl->midxList, lnk);

    for (j = 0; midx[counter].names[j] != 0; j++)
      free (midx[counter].names[j]);
  }
  storeTableHeader (tbl);

  /*
   * Now, traverse the table and add index data for each item.
   */
  headRecord (tbl);
  tbl->bof = tbl->eof = FALSE;
  while (tbl->eof == FALSE) {
    retrieveRecord (tbl);
    record2Field (tbl);
    if (tbl->idxList != 0) {
      /*
       * We are not checking to make sure that uniqueness is maintained
       * in a unique index.  The user needs to insure that there will be
       * no problems with this regard.
       */
      for (i = 0; i < tbl->hdr->numFields; i++) {
	if (strlen (tbl->fldAry[i]->indexName) != 0) {
	  ki = malloc (tbl->fldAry[i]->fieldLength + 1 + sizeof (unsigned long));
	  if (0 == ki) {
	    dbError = DB_NOMEMORY;
	    return _ERROR_;
	  }
	  check_pointer (ki);

	  ki->item = tbl->crec + 1;
	  strcpy (ki->key, tbl->fields[i]);
	  check_pointer (ki);

	  /*
	   * Now, find the index to add the item to.
	   */
	  lnk = tbl->idxList->head->next;
	  while (lnk != tbl->idxList->tail) {
	    idx = lnk->data;
	    if (!strcmp (tbl->fldAry[i]->indexName, idx->hdr->indexName))
	      break;
	    lnk = lnk->next;
	  }
	  if (lnk == tbl->idxList->tail) {
	    dbError = DB_UNSTABLE;
	    return _ERROR_;
	  }
	  status = addIndexItem (idx, ki);
	  if (_ERROR_ == status) {
	    if (idxError == IDX_UNIQUE)
	      dbError = DB_UNIQUE;
	    else if (idxError == IDX_LIST)
	      dbError = DB_LISTERROR;
	    else
	      dbError = DB_INDEX;
	    return _ERROR_;
	  }
	}
      }
    }
    if (tbl->midxList != 0) {
      lnk = tbl->midxList->head->next;
      while (lnk != tbl->midxList->tail) {
	idx = lnk->data;
	for (numidx = 0; tbl->hdr->midxInfo[numidx][0] != '\0'; numidx++)
	  if (!strncmp (tbl->hdr->midxInfo[numidx], idx->hdr->indexName,
			strlen (idx->hdr->indexName)))
	    break;
	if (numidx >= MAX_MIDX) {
	  dbError = DB_UNSTABLE;
	  return _ERROR_;
	}
	memset (key, 0, 1024);
	memset (&smidx, 0, sizeof (midxField));
	str2midxField (&smidx, tbl->hdr->midxInfo[numidx]);
	sizekey = 0;
	for (i = 0; i < MAX_IDX && smidx.names[i] != 0; i++) {
	  status = getFieldNum (tbl, smidx.names[i]);
	  strcat (key, tbl->fields[status]);
	  sizekey += tbl->flens[status];
	}
	/*
	 * key should now have the index key information
	 */
	ki = malloc (sizekey + 1 + sizeof (unsigned long));
	if (0 == ki) {
	  dbError = DB_NOMEMORY;
	  return _ERROR_;
	}
	memset (ki, 0, sizekey + 1 + sizeof (unsigned long));
	strcpy (ki->key, key);
	ki->item = tbl->crec + 1;
	status = addIndexItem (idx, ki);
	if (_ERROR_ == status) {
	  if (idxError == IDX_LIST)
	    dbError = DB_LISTERROR;
	  else
	    dbError = DB_INDEX;
	  return _ERROR_;
	}
	for (i = 0; smidx.names[i] != 0; i++)
	  free (smidx.names[i]);
	lnk = lnk->next;
      }
    }
    nextRecord (tbl);
  }
  /*
   * We're done; get the heck outa here!
   */
  closeTable (tbl);
  return _OK_;

errorExit:
  closeTable (tbl);
  return _ERROR_;
}
