/*	Source File:	cdbindex.c	*/

#include <cdb.h>
#include <stdlib.h>

externC

/*
 * [BeginDoc]
 * 
 * 
 * \section{DCDB Index Functions}
 * 
 * This file provides the index support to the cdb table 
 * functions.  It includes the functionality to create, open,
 * close, query and search on indexes.
 * 
 * [EndDoc]
 */


/*
 * [BeginDoc]
 * \subsection{createDBIndex}
 * \index{createDBIndex}
 * [Verbatim] */

  int createDBIndex (dbTable * tbl, char *idxName, char *fldName,
		     int isCase, int isUnique, int blkSize)

/* [EndDoc] */
/*
 * [BeginDoc]
 * createDBIndex() creates an index file for the table ``tbl''.
 * ``idxName'' is the name of the index.  There will be two files
 * created for each index, one with the extension ``.inx'' and
 * one with the extension ``.idx''.  The ``.inx'' file is the memory
 * index and the ``.idx'' file is the block file for the index.
 * The names of these files will be ``idxName'' concatenated with
 * ``.inx'' or ``.idx'', respectively.
 * ``fldname'' is
 * the name of the field to create the index for.  If fldname is
 * NULL or does not contain a valid field (one that matches at least
 * partially one in the field list), an error is indicated.  ``isCase''
 * is TRUE if the index should use case sensitive sorts, FALSE
 * otherwise.  ``isUnique'' is TRUE if the index should not allow
 * duplicates, FALSE if duplicates are allowed.
 * ``blkSize'' will
 * be the block size of the block file the index is stored in.  If
 * ``blkSize'' is 0, INDEX_BLOCK_SIZE (defined in cdb.h) will be
 * used.  The
 * block size for an index file should be a power of two such that
 * 128 <= block size <= 4096.  A good rule of thumb (where
 * performance is concerned) is to use a block size such that there
 * are between 20 and 35 items stored per block.  More items than
 * this per block and performance tends to degrade.  However, it
 * is recommended that the block size be chosen based on performance
 * tests for the items that are going to be indexed.
 * 
 * If successful, createDBIndex() creates the block file for the
 * index, sets the current index to the newly created index 
 * and returns _OK_.  If an error occurs, isTableError(tbl)
 * for that table will be TRUE and dbtblerror() will return a
 * description of the error.
 * 
 * [EndDoc]
 */
  {
    int i;
    char tmpStr[INDEX_NAME_WIDTH + 1];
    dbIndex *idx;
    Link *lnk;
    int status;

      dbg2 (printf ("\n\ncreateIndex(tbl,%s,%s,...)\n", idxName, fldName);
      );
    if (tbl->hdr->numRecords > 0) {
      tbl->dbError = DB_CREATEINDEX;
      return _ERROR_;
    } for (i = 0; i < tbl->hdr->numFields; i++)
      if (!strncmp (fldName, tbl->fieldNames[i], strlen (fldName)))
	break;
    if (i == tbl->hdr->numFields) {
      tbl->dbError = DB_FIELD;
      return _ERROR_;
    }
    memset (tbl->fldAry[i]->indexName, 0, INDEX_NAME_WIDTH + 1);
    strncpy (tbl->fldAry[i]->indexName, idxName, INDEX_NAME_WIDTH - 5);
    /*tbl->fldAry[i]->indexName[INDEX_NAME_WIDTH-5] = '\0'; */
    if (isCase) {
      if (isUnique)
	tbl->fldAry[i]->indexed = ITYPE_UNIQUECASE;
      else
	tbl->fldAry[i]->indexed = ITYPE_DUPCASE;
    }
    else {
      if (isUnique)
	tbl->fldAry[i]->indexed = ITYPE_UNIQUENOCASE;
      else
	tbl->fldAry[i]->indexed = ITYPE_DUPNOCASE;
    }
    if (tbl->fldAry[i]->indexed <= ITYPE_NOINDEX ||
	tbl->fldAry[i]->indexed >= ITYPE_LAST) {
      tbl->dbError = DB_UNSTABLE;
      return _ERROR_;
    }
    if (blkSize == 0)
      tbl->fldAry[i]->indexBlkSize = INDEX_BLOCK_SIZE;
    else
      tbl->fldAry[i]->indexBlkSize = blkSize;

    /*
     * Now, create the index file, etc.
     */
    strcpy (tmpStr, tbl->fldAry[i]->indexName);
    strcat (tmpStr, ".idx");
#ifndef	QUICKINDEX
    status = createIndexFile (tmpStr, tbl->fldAry[i]->indexBlkSize,
			      tbl->fldAry[i]->fieldLength + 1,
			      isUnique, isCase);
    if (status != _OK_) {
      tbl->dbError = DB_INDEX;
      return _ERROR_;
    }
/*	strcpy (tmpStr2, tbl->fldAry[i]->indexName);
	strcat (tmpStr2, ".inx");*/
    idx = openIndex (tmpStr, 0, tbl->fldAry[i]->indexBlkSize);
    if (0 == idx) {
      tbl->dbError = DB_INDEX;
      return _ERROR_;
    }
/*	strncpy (idx->hdr->indexName, tbl->fldAry[i]->indexName, INDEX_NAME_WIDTH-5);*/
#endif
#ifdef	QUICKINDEX
    idx = createIndex (tmpStr, tbl->fldAry[i]->fieldLength + 1,
		       isUnique, isCase);
    if (0 == idx) {
      tbl->dbError = DB_INDEX;
      return _ERROR_;
    }
#endif
    strcpy (idx->hdr->indexName, tbl->fldAry[i]->indexName);

    /*
     * Make an entry in the tbl->idxList list.
     */
    if (tbl->idxList == 0) {
      tbl->idxList = initList (UNSORTED, 0);
      if (0 == tbl->idxList) {
	tbl->dbError = DB_LISTERROR;
	return _ERROR_;
      }
    }
    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      tbl->dbError = DB_NOMEMORY;
      return _ERROR_;
    }
    memset (lnk, 0, sizeof (Link));
    check_pointer (lnk);

    lnk->data = idx;
    tbl->idxList->current = tbl->idxList->tail->prev;
    insertLinkHere (tbl->idxList, lnk);
    tbl->current = idx;

    /*
     * Make sure the dbField item in the table is updated.
     */
    storeFieldHeader (i, tbl);

    dbg2 (printf ("\n\ncreateIndex() end\n");
      );
    return _OK_;
  }

/*
 * [BeginDoc]
 * \subsection{createMultiIndex}
 * \index{createMultiIndex}
 * [Verbatim] */

  int createMultiIndex (dbTable * tbl, char *midxName, char **names,
			int isCase, int blkSize)

/* [EndDoc] */
/*
 * [BeginDoc]
 * 
 * createMultiIndex() creates a multifield index for the table given by
 * ``tbl''.  ``midxName'' is the name of the index (the naming of multi indexes
 * is the same as it is for field indexes in createDBIndexes()).  ``names'' is
 * a NULL terminated array of character strings that indicates which fields are
 * represented in the multiple index.  ``isCase'' will determine whether case
 * is important in this index.  If ``isCase'' is TRUE, case is important.
 * ``blkSize'' is the block size for the index block file.  The rules for
 * selecting the optimal block size for multiple indexes are the same as those
 * for a field oriented index file, except that you have to accumulate the
 * total size of all the fields represented by the multiple index when you
 * calculate the block size.  Again, it is advised that the block size be
 * based on testing if performance is an issue.
 * 
 * The current index for the table is unchanged after a call to
 * createMultiIndex().
 * 
 * If createMultiIndex() is successful, _OK_ is returned.  If it is not,
 * isTableError(tbl) will be true for this table and dbtblerror() will
 * contain a description of the error.
 * 
 * [EndDoc]
 */
  {
    char tmpStr[INDEX_NAME_WIDTH + 1];
    /*char midxScratch[INDEX_NAME_WIDTH+1]; */
    dbIndex *idx;
    Link *lnk;
    int status;
    int i;
    int numidx;
    /*int indlen = 0; */
    int keylen = 0;
    midxField midx;

    dbg2 (printf ("\n\ncreateMultiIndex (tbl,%s,...)\n", midxName);
      );
    if (tbl->hdr->numRecords > 0) {
      tbl->dbError = DB_CREATEINDEX;
      return _ERROR_;
    }
    if (blkSize == 0)
      blkSize = INDEX_BLOCK_SIZE;
    strncpy (tmpStr, midxName, INDEX_NAME_WIDTH);
    tmpStr[INDEX_NAME_WIDTH] = '\0';
    strcat (tmpStr, ".idx");
    /* determine which number of midx we're dealing with */
    for (numidx = 0; numidx < MAX_MIDX; numidx++)
      if (tbl->hdr->midxInfo[numidx][0] == '\0')
	break;
    if (numidx == MAX_MIDX) {
      tbl->dbError = DB_CREATEINDEX;
      return _ERROR_;
    }
    midx.isUnique = FALSE;
    strcpy (midx.indexName, midxName);
    midx.blkSize = blkSize;
    midx.isCase = isCase;
    for (i = 0; i < MAX_IDX && names[i] != 0; i++) {
      midx.names[i] = malloc (strlen (names[i]) + 1);
      if (0 == midx.names[i]) {
	tbl->dbError = DB_NOMEMORY;
	return _ERROR_;
      }
      memset (midx.names[i], 0, strlen (names[i] + 1));
      check_pointer (midx.names[i]);
      strcpy (midx.names[i], names[i]);
      midx.names[i + 1] = 0;
      status = getFieldNum (tbl, names[i]);
      if (status == _ERROR_) {
	tbl->dbError = DB_FIELD;
	return _ERROR_;
      }
      keylen += tbl->flens[status];
    }
    /*dumpMidxField (&midx); */
    status = midxField2str (&midx, tbl->hdr->midxInfo[numidx]);
    if (status == _ERROR_) {
      tbl->dbError = DB_CREATEINDEX;
      memset (tbl->hdr->midxInfo[numidx], 0, TABLE_INFO_WIDTH * 2 + 1);
      return _ERROR_;
    }
    status = createIndexFile (tmpStr, blkSize, keylen + 1, FALSE, isCase);
    if (status == _ERROR_) {
      tbl->dbError = DB_INDEX;
      memset (tbl->hdr->midxInfo[numidx], 0, TABLE_INFO_WIDTH * 2 + 1);
      return _ERROR_;
    }
    idx = openIndex (tmpStr, 0, blkSize);
    if (idx == 0) {
      tbl->dbError = DB_INDEX;
      memset (tbl->hdr->midxInfo[numidx], 0, TABLE_INFO_WIDTH * 2 + 1);
      return _ERROR_;
    }
    for (i = 0; i < MAX_IDX && names[i] != 0; i++)
      free (midx.names[i]);
    memset (idx->hdr->indexName, 0, INDEX_NAME_WIDTH + 1);
    strncpy (idx->hdr->indexName, midxName, INDEX_NAME_WIDTH);

    /*
     * Make an entry in tbl->midxList
     */
    if (tbl->midxList == 0) {
      tbl->midxList = initList (UNSORTED, 0);
      if (0 == tbl->midxList) {
	tbl->dbError = DB_LISTERROR;
	memset (tbl->hdr->midxInfo[numidx], 0, TABLE_INFO_WIDTH * 2 + 1);
	return _ERROR_;
      }
      lnk = malloc (sizeof (Link));
      if (lnk == 0) {
	tbl->dbError = DB_NOMEMORY;
	memset (tbl->hdr->midxInfo[numidx], 0, TABLE_INFO_WIDTH * 2 + 1);
	return _ERROR_;
      }
      memset (lnk, 0, sizeof (Link));
      check_pointer (lnk);
      lnk->data = idx;
      tbl->midxList->current = tbl->midxList->tail->prev;
      insertLinkHere (tbl->midxList, lnk);
    }
    storeTableHeader (tbl);
    dbg2 (printf ("\n\ncreateMultiIndex() end\n");
      );
    return _OK_;
  }


/*
 * [BeginDoc]
 * \subsection{openDBIndexes}
 * \index{openDBIndexes}
 * [Verbatim] */

  int openDBIndexes (dbTable * tbl)

/* [EndDoc] */
/*
 * openDBIndexes() opens the indexes that have been created for
 * the table given by ``tbl''.  If it is successful, tbl->idxList
 * will be the header of a list of open indexes and _OK_ is returned.
 * If an error occurs, isTableError(tbl) is TRUE for this table and
 * dbtblerror(tbl) returns a description of the error.
 * 
 * If a table is being created, the user should create the table
 * with a call to buildTable(), and then should create all the
 * needed indexes with calls to createDBIndex().  Then, the user
 * should close the table (which closes all open indexes) and reopen
 * everything.  This allows all the variables to be initialized with
 * reasonable startup values.  The current index after a call to
 * openDBIndexes() will be the one on the last indexed field.
 * 
 * [EndDoc]
 */
  {
    int i;
    char tstr1[INDEX_NAME_WIDTH + 1];
    char tstr2[INDEX_NAME_WIDTH + 1];
    dbIndex *idx;
    Link *lnk;
    int isUnique = FALSE, isCase = FALSE;

    /*
     * traverse the dbField array looking for indexed fields
     */
    dbg2 (printf ("\n\nopenDBIndexes()\n");
      );
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
#ifdef	QUICKINDEX
	idx = openIndex (tstr1, tbl->fldAry[i]->fieldLength + 1,
			 isUnique, isCase);
#endif
#ifndef	QUICKINDEX
	if (fexists ((char *) tstr2))
	  idx = openIndex (tstr1, tstr2, tbl->fldAry[i]->indexBlkSize);
	else
	  idx = openIndex (tstr1, 0, tbl->fldAry[i]->indexBlkSize);
#endif
	if (0 == idx) {
	  tbl->dbError = DB_INDEX;
	  return _ERROR_;
	}
	strcpy (idx->hdr->indexName, tbl->fldAry[i]->indexName);
	if (tbl->idxList == 0) {
	  tbl->idxList = initList (UNSORTED, 0);
	  if (0 == tbl->idxList) {
	    tbl->dbError = DB_LISTERROR;
	    return _ERROR_;
	  }
	}
	lnk = malloc (sizeof (Link));
	if (0 == lnk) {
	  tbl->dbError = DB_NOMEMORY;
	  return _ERROR_;
	}
	memset (lnk, 0, sizeof (Link));
	check_pointer (lnk);
	lnk->data = idx;
	tbl->idxList->current = tbl->idxList->tail->prev;
	insertLinkHere (tbl->idxList, lnk);
      }
    }
    if (tbl->idxList != 0 && tbl->idxList->number > 0)
      tbl->current = tbl->idxList->tail->prev->data;
    dbg2 (printf ("\n\nopenDBIndexes() end\n");
      );
    return _OK_;
  }

/*
 * [BeginDoc]
 * \subsection{openMultiIndexes}
 * \index{openMultiIndexes}
 * [Verbatim] */

  int openMultiIndexes (dbTable * tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * openMultiIndexes() opens the multiple field indexes associated
 * with the table ``tbl''.  As the indexes are opened, the index
 * descriptors are placed in tbl->midxList (a linked list).  If
 * openMultiIndexes() is successful it returns _OK_.  If an error
 * occurs, isTableError(tbl) will be TRUE for this table and
 * dbtblerror(tbl) will return a description of the error.  The
 * current index for the table is unchanged by a call to
 * openMultiIndexes().
 * 
 * [EndDoc]
 */
  {
    int i, j;
    char tstr1[INDEX_NAME_WIDTH + 1];
    char tstr2[INDEX_NAME_WIDTH + 1];
    dbIndex *idx;
    Link *lnk;
    int blkSize;
    midxField midx;

    dbg2 (printf ("\n\nopenMultiIndexes()\n");
      );
    for (i = 0; i < MAX_MIDX; i++) {
      if (tbl->hdr->midxInfo[i][0] == '\0')
	break;
      memset (&midx, 0, sizeof (midxField));
      str2midxField (&midx, tbl->hdr->midxInfo[i]);
      /*dumpMidxField (&midx); */
      strcpy (tstr1, midx.indexName);
      /*tstr1[strlen (midx.indexName] = '\0'; */
      strcpy (tstr2, midx.indexName);
      /*tstr2[(int)((char *)cp-(char *)tbl->hdr->midxInfo[i])] = '\0'; */
      strcat (tstr1, ".idx");
      strcat (tstr2, ".inx");
      blkSize = midx.blkSize;
      if (fexists ((char *) tstr2))
	idx = openIndex (tstr1, tstr2, blkSize);
      else
	idx = openIndex (tstr1, 0, blkSize);
      if (0 == idx) {
	tbl->dbError = DB_INDEX;
	return _ERROR_;
      }
      memset (idx->hdr->indexName, 0, INDEX_NAME_WIDTH + 1);
      strcpy (idx->hdr->indexName, midx.indexName);

      for (j = 0; j < MAX_IDX && midx.names[j] != 0; j++)
	free (midx.names[j]);

      /*
       * Link it into tbl->midxList
       */
      if (tbl->midxList == 0) {
	tbl->midxList = initList (UNSORTED, 0);
	if (0 == tbl->midxList) {
	  tbl->dbError = DB_LISTERROR;
	  return _ERROR_;
	}
      }
      lnk = malloc (sizeof (Link));
      if (0 == lnk) {
	tbl->dbError = DB_NOMEMORY;
	return _ERROR_;
      }
      memset (lnk, 0, sizeof (Link));
      check_pointer (lnk);
      lnk->data = idx;
      tbl->midxList->current = tbl->midxList->tail->prev;
      insertLinkHere (tbl->midxList, lnk);
    }
    dbg2 (printf ("\n\nopenMultiIndexes() end\n");
      );
    return _OK_;
  }

/*
 * [BeginDoc]
 * \subsection{closeDBIndexes}
 * \index{closeDBIndexes}
 * [Verbatim] */

  int closeDBIndexes (dbTable * tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * closeDBIndexes() closes the open indexes associated with the 
 * table ``tbl''.  It is not an error if there are no open indexes
 * for tbl; _OK_ is simply returned in that instance.  If 
 * closeDBIndexes() is successful, all the indexes for a table will
 * be flushed to disk and closed, and _OK_ will be returned.  If
 * an error occurs, isTableError(tbl) is TRUE for this table and
 * dbtblerror(tbl) will return a description of the error.
 * 
 * [EndDoc]
 */
  {
    int status;
    Link *lnk;
    dbIndex *idx;
    char tmpStr[INDEX_NAME_WIDTH + 1];

    dbg2 (printf ("\n\ncloseDBIndexes()\n");
      );
    if (tbl->idxList == 0)
      return _OK_;
    if (tbl->idxList->number == 0) {
      delList (tbl->idxList);
      tbl->idxList = 0;
      return _OK_;
    }
    lnk = removeLink (tbl->idxList);
    while (lnk != 0) {
      idx = lnk->data;
      /*
       * Get the index name from the idx->hdr->indexName field.
       */
      strcpy (tmpStr, idx->hdr->indexName);
      strcat (tmpStr, ".inx");
#ifndef	QUICKINDEX
      status = closeIndexFile (idx, tmpStr);
#endif
#ifdef	QUICKINDEX
      status = closeIndex (idx);
#endif
      if (status == _ERROR_) {
	tbl->dbError = DB_INDEX;
	return _ERROR_;
      }
      free (lnk);
      lnk = removeLink (tbl->idxList);
    }
    delList (tbl->idxList);
    tbl->idxList = 0;
    tbl->current = 0;
    dbg2 (printf ("\n\ncloseDBIndexes() end\n");
      );
    return _OK_;
  }

  int closeMultiIndexes (dbTable * tbl) {
    int status;
    Link *lnk;
    dbIndex *idx;
    char tmpStr[INDEX_NAME_WIDTH + 1];

    dbg2 (printf ("\n\ncloseMultiIndexes()\n");
      );
    if (tbl->midxList == 0)
      return _OK_;
    if (tbl->midxList->number == 0) {
      delList (tbl->midxList);
      tbl->midxList = 0;
      return _OK_;
    }
    lnk = removeLink (tbl->midxList);
    while (lnk != 0) {
      idx = lnk->data;
      strcpy (tmpStr, idx->hdr->indexName);
      strcat (tmpStr, ".inx");
      status = closeIndexFile (idx, tmpStr);
      if (status == _ERROR_) {
	tbl->dbError = DB_INDEX;
	return _ERROR_;
      }
      free (lnk);
      lnk = removeLink (tbl->midxList);
    }
    delList (tbl->midxList);
    tbl->midxList = 0;
    dbg2 (printf ("\n\ncloseMultiIndexes() end\n");
      );
    return _OK_;
  }

/*
 * [BeginDoc]
 * \subsection{addDBIndexes}
 * \index{addDBIndexes}
 * [Verbatim] */

  int addDBIndexes (dbTable * tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * addDBIndexes() will add the items in tbl->fields[] to the open
 * indexes for each field being indexed.  It is not an error to
 * call addDBIndexes() for a table that has no indexed fields; _OK_
 * is simply returned in that instance.  A check is made before any
 * data is added to insure that no unique constraints are violated.
 * This way, the user can
 * insure that no index data is altered when a call to addRecord()
 * fails due to a unique index violation.  addDBIndexes() returns
 * _OK_ on success.  If an error occurs, isTableError(tbl) will be
 * TRUE for this table and dbtblerror(tbl) will return a description
 * of the error.
 * 
 * [EndDoc]
 */
  {
    keyItem *ki;
    Link *lnk;
    Link *link;
    Link *found;
    dbIndex *idx = 0;
    int status;
    int i;

    if (tbl->idxList == 0 && tbl->midxList != 0) {
      status = addMultiIndexes (tbl);
      return status;
    }
    if (tbl->idxList == 0)
      return _OK_;
    /*
     * First, check to make sure no unique constraints are
     * violated.
     */
    lnk = tbl->idxList->head->next;
    while (lnk != tbl->idxList->tail) {
      idx = lnk->data;
/*		if (idx->hdr->isUnique && idx->inx->lh->number > 0)	{*/
      if (idx->hdr->isUnique && idx->hdr->numItems > 0) {
	for (i = 0; i < tbl->hdr->numFields; i++)
	  if (!strncmp (tbl->fldAry[i]->indexName,
			idx->hdr->indexName, strlen (idx->hdr->indexName)))
	    break;
	if (i == tbl->hdr->numFields) {
	  tbl->dbError = DB_UNSTABLE;
	  return _ERROR_;
	}
	ki = malloc (tbl->fldAry[i]->fieldLength + 1 + sizeof (unsigned long));
	if (0 == ki) {
	  tbl->dbError = DB_NOMEMORY;
	  return _ERROR_;
	}
	memset (ki, 0, tbl->fldAry[i]->fieldLength + 1 + sizeof (unsigned long));
	check_pointer (ki);
	strcpy (ki->key, tbl->fields[i]);
	check_pointer (ki);
	found = idx->inx->lh->head->next;
	status = idx->inx->compare (ki, found->data);
	if (status < 0) {
	  lnk = lnk->next;
	  free (ki);
/*				free (link);*/
	  continue;
	}
	link = malloc (sizeof (Link));
	if (0 == link) {
	  tbl->dbError = DB_NOMEMORY;
	  return _ERROR_;
	}
	memset (link, 0, sizeof (Link));
	check_pointer (link);
	link->data = ki;
	if (searchExactIndexItem (idx, link)) {
	  tbl->dbError = DB_UNIQUE;
	  free (ki);
	  memset (link, 0, sizeof (Link));
	  free (link);
	  check_pointer (link);
	  return _ERROR_;
	}
	free (ki);
	free (link);
      }
      lnk = lnk->next;
    }

    /*
     * No problema.  So, update the index.
     */
    for (i = 0; i < tbl->hdr->numFields; i++) {
      if (strlen (tbl->fldAry[i]->indexName) != 0) {
	ki = malloc (tbl->fldAry[i]->fieldLength + 1 + sizeof (unsigned long));
	if (0 == ki) {
	  tbl->dbError = DB_NOMEMORY;
	  return _ERROR_;
	}
	memset (ki, 0, tbl->fldAry[i]->fieldLength + 1 + sizeof (unsigned long));
	check_pointer (ki);

	ki->item = tbl->hdr->numRecords + 1;
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
	  tbl->dbError = DB_UNSTABLE;
	  free (ki);
	  return _ERROR_;
	}
	status = addIndexItem (idx, ki);
	if (_ERROR_ == status) {
	  if (idxError == IDX_UNIQUE)
	    tbl->dbError = DB_UNIQUE;
	  else if (idxError == IDX_LIST)
	    tbl->dbError = DB_LISTERROR;
	  else
	    tbl->dbError = DB_INDEX;
	  return _ERROR_;
	}
      }
    }
    /*
     * Good, now update any multi-indexes, if they are open
     */
    status = addMultiIndexes (tbl);
    return status;
  }

/*
 * [BeginDoc]
 * \subsection{addMultiIndexes}
 * \index{addMultiIndexes}
 * [Verbatim] */

  int addMultiIndexes (dbTable * tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * 
 * addMultiIndex() adds index information to the multiple field
 * indexes associated with table ``tbl''.  If it is successful,
 * addMultiIndex() returns _OK_.  Otherwise, isTableError(tbl)
 * is true for this table and dbtblerror(tbl) will return a
 * description of the error.
 * 
 * [EndDoc]
 */
  {
    int status;
    Link *lnk;
    dbIndex *idx;
    int numidx, i;
    int sizekey;
    char key[1024];
/*	char midxInfoCopy[TABLE_INFO_WIDTH*2+1];*/
    midxField midx;
    /*char *cp; */
    /*char *thisField; */
    keyItem *ki;

    /*printf ("Starting addMultiIndexes\n\n"); */
    if (tbl->midxList == 0)
      return _OK_;
    lnk = tbl->midxList->head->next;
    while (lnk != tbl->midxList->tail) {
      idx = lnk->data;
      for (numidx = 0; tbl->hdr->midxInfo[numidx][0] != '\0'; numidx++)
	if (!strncmp (tbl->hdr->midxInfo[numidx], idx->hdr->indexName,
		      strlen (idx->hdr->indexName)))
	  break;
      if (numidx >= MAX_MIDX) {
	tbl->dbError = DB_UNSTABLE;
	return _ERROR_;
      }
      memset (key, 0, 1024);
      memset (&midx, 0, sizeof (midxField));
      str2midxField (&midx, tbl->hdr->midxInfo[numidx]);
      sizekey = 0;
      for (i = 0; i < MAX_IDX && midx.names[i] != 0; i++) {
	status = getFieldNum (tbl, midx.names[i]);
	strcat (key, tbl->fields[status]);
	sizekey += tbl->flens[status];
      }
      /*
       * key should now have the index key information
       */
      ki = malloc (sizekey + 1 + sizeof (unsigned long));
      if (0 == ki) {
	tbl->dbError = DB_NOMEMORY;
	return _ERROR_;
      }
      memset (ki, 0, sizekey + 1 + sizeof (unsigned long));
      check_pointer (ki);
      strcpy (ki->key, key);
      ki->item = tbl->hdr->numRecords + 1;
      status = addIndexItem (idx, ki);
      if (_ERROR_ == status) {
	if (idxError == IDX_LIST)
	  tbl->dbError = DB_LISTERROR;
	else
	  tbl->dbError = DB_INDEX;
	return _ERROR_;
      }
      for (i = 0; midx.names[i] != 0; i++)
	free (midx.names[i]);
      lnk = lnk->next;
    }
    return _OK_;
  }


/*
 * [BeginDoc]
 * \subsection{setCurrentIndex}
 * \index{setCurrentIndex}
 * [Verbatim] */

  int setCurrentIndex (dbTable * tbl, char *idxName)

/* [EndDoc] */
/*
 * [BeginDoc]
 * setCurrentIndex() sets the current index pointer for the table
 * ``tbl'' to the first index with the name that matches
 * ``idxName'' up to strlen(idxName).  
 * If setCurrentIndex() returns _OK_, tbl->current
 * will be set to the current dbIndex in idxList.
 * If an error occurs, tbl->current will be unchanged,
 * isTableError(tbl) will be TRUE for this table and
 * dbtblerror(tbl) will return a description of the error.
 * 
 * [EndDoc]
 */
  {
    Link *lnk;
    dbIndex *idx;

    if (tbl == NULL) {
      dbError = DB_PARAM;
      return _ERROR_;
    }
    if (idxName == 0 || strlen (idxName) == 0) {
      tbl->dbError = DB_PARAM;
      return _ERROR_;
    }
    if (tbl->idxList == 0 && tbl->midxList == 0)
      return _OK_;
    if (tbl->idxList != 0) {
      lnk = tbl->idxList->head->next;
      while (lnk != tbl->idxList->tail) {
	idx = lnk->data;
	if (!strncmp (idx->hdr->indexName, idxName, strlen (idxName))) {
	  tbl->current = idx;
	  return _OK_;
	}
	lnk = lnk->next;
      }
    }
    if (tbl->midxList != 0) {
      lnk = tbl->midxList->head->next;
      while (lnk != tbl->midxList->tail) {
	idx = lnk->data;
	if (!strncmp (idx->hdr->indexName, idxName, strlen (idxName))) {
	  tbl->current = idx;
	  return _OK_;
	}
	lnk = lnk->next;
      }
    }
    tbl->dbError = DB_PARAM;
    return _ERROR_;
  }

endC
