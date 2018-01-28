/*	Source File:	flogadd2.c	*/

/*
 * [BeginDoc]
 * 
 * =================================================================
 * [EndDoc]
 */

#include <cdb.h>
#include <index.h>
#include <sort.h>

#include <stdio.h>
#include <test.h>
#include <time.h>

char *dbTableInformation = "primary:Test Table";

char *names[] = {
  "LName",			/* character strings */
  "SSNumber",
  "FName",
  "StreetAddress",
  "City",
  "State",
  "Zip",
  "AnnualSalary",		/* number */
  "HomeOwner",			/* logical */
  "DateApplied",		/* date */
  "LastUpdated",		/* time stamp */
  0
};

int ftype[] = {
  FTYPE_CHAR,
  FTYPE_CHAR,
  FTYPE_CHAR,
  FTYPE_CHAR,
  FTYPE_CHAR,
  FTYPE_CHAR,
  FTYPE_CHAR,
  FTYPE_NUMBER,
  FTYPE_LOGICAL,
  FTYPE_DATE,
  FTYPE_TIME,
  FTYPE_NONE
};

int flens[] = {
  65, 10, 35, 80, 50, 2, 10,
  10, 1, 8, 26, 0
};

int declens[] = {
  0, 0, 0, 0, 0, 0, 0,
  2, 0, 0, 0, 0
};

int main (void)
{
  dbHeader *dbh;
  dbField **flds;
  dbTable *tbl;
  int counter = 1;
  int i;
  int status;
  FILE *fp;
  char line[512];
  char *cp, *nextField;
  char *ln;
  Link *lnk;
  ListHeader *SSNList;
  ListHeader *addList;
  ListHeader *rtnList;
  char *data;
  int foundStatus;
  double t1, t2;

  /*memset (line, 0, 512); */
  SSNList = initList (UNSORTED, 0);
  if (SSNList == 0) {
    printf ("\n\n***Error: creating ssn list, %s\n",
	    ListErrorString[ListError]);
    return _ERROR_;
  }
  addList = initList (UNSORTED, 0);
  if (addList == 0) {
    printf ("\n\n***Error: creating add list, %s\n",
	    ListErrorString[ListError]);
    return _ERROR_;
  }

  /*
   * dbHeader
   */
  dbh = malloc (sizeof (dbHeader));
  Assert (0 != dbh);
  if (0 == dbh)
    return -1;
  memset (dbh, 0, sizeof (dbHeader));

  /*
   * for the header, user must initialize dbh->timeStamp and 
   * dbh->tableInfo
   */
  strcpy (dbh->timeStamp, getTimeStamp ());
  strcpy (dbh->tableInfo, dbTableInformation);

  /*
   * dbField's
   */
  flds = malloc (12 * sizeof (dbField *));
  Assert (0 != flds);
  if (0 == flds)
    return -1;
  memset (flds, 0, 12 * sizeof (dbField *));
  for (i = 0; i < 11; i++) {
    flds[i] = malloc (sizeof (dbField));
    Assert (0 != flds[i]);
    if (0 == flds[i])
      return -1;
    memset (flds[i], 0, sizeof (dbField));
    strcpy (flds[i]->fieldName, names[i]);
    flds[i]->ftype = ftype[i];
    flds[i]->indexed = ITYPE_NOINDEX;
    flds[i]->indexed = ' ';
    flds[i]->fieldLength = flens[i];
    flds[i]->decLength = declens[i];
  }
  flds[11] = NULL;
  tbl = createTable ("table.db", dbh, flds);
  check_pointer (tbl->fields);
  Assert (0 != tbl);
  if (isDBError ()) {
    printf ("\n\n***Error: creating table, %s\n", dberror ());
    return _ERROR_;
  }
  printf ("\nCreating SSNumberIndex\n");
  status = createDBIndex (tbl, "SSNumberIndex", "SSNumber", TRUE, TRUE, 256);
  if (isTableError (tbl)) {
    printf ("\n\n***Error: creating SSNumberIndex: %s\n", dbtblerror (tbl));
    return _ERROR_;
  }
  check_pointer (tbl);
  check_pointer (tbl->fields);

  printf ("\nCreating LNameIndex\n");
  status = createDBIndex (tbl, "LNameIndex", "LName", TRUE, FALSE, 512);
  if (isTableError (tbl)) {
    printf ("\n\n***Error: creating LNameIndex: %s\n", dbtblerror (tbl));
    return _ERROR_;
  }
  check_pointer (tbl);
  check_pointer (tbl->fields);

  closeTable (tbl);
  if (isDBError ())
    printf ("\n\n***Error: closing table, %s\n", dberror ());
  tbl = openTable ("table.db");
  if (isDBError ()) {
    printf ("\n\n***Error: Couldn't reopen the table, %s\n", dberror ());
    return _ERROR_;
  }
  status = setCurrentIndex (tbl, "SSNumberIndex");
  if (isTableError (tbl)) {
    printf ("\n\n***Error: setting current index to SSNumber, %s\n",
	    dbtblerror (tbl));
    closeTable (tbl);
    return _ERROR_;
  }


  counter = 1;
  fp = fopen ("testAdd.txt", "r");
  if (0 == fp) {
    printf ("\n\n***Error: Couldn't open testAdd.txt for reading\n");
    return _ERROR_;
  }
  printf ("\n");
  elapsed (&t1);
  while (TRUE) {
    if (fgets (line, 500, fp) == 0)
      break;
    if (feof (fp))
      break;
    if (line[0] == '\n')
      continue;
    cp = strchr (line, '\n');
    if (0 != cp)
      *cp = '\0';
    if (strlen (line) == 0)
      continue;
    nextField = line;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    for (i = 0; i < tbl->hdr->numFields; i++) {
      memset (tbl->fields[i], 0, tbl->flens[i]);
      check_pointer (tbl->fields[i]);
    }
    /* LName */
    strncpy (tbl->fields[0], nextField, tbl->flens[0]);
    tbl->fields[0][tbl->flens[0]] = '\0';
    check_pointer (tbl->fields[0]);

    /* SSNumber */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[1], nextField, tbl->flens[1]);
    tbl->fields[1][tbl->flens[1]] = '\0';
    check_pointer (tbl->fields[1]);

    ln = malloc (tbl->flens[1]);
    if (0 == ln) {
      printf ("\n\n***Error: fatal memory error\n");
      closeTable (tbl);
      return _ERROR_;
    }
    /*memset (ln, 0, strlen (tbl->fields[1]+1)); */
    check_pointer (ln);
    strcpy (ln, tbl->fields[1]);
    check_pointer (ln);
    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      printf ("\n\n***Error: fatal list error\n");
      closeTable (tbl);
      return _ERROR_;
    }
    /*memset (lnk, 0, sizeof (Link)); */
    check_pointer (lnk);
    lnk->data = ln;
    insertLink (SSNList, lnk);

    /* FName */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[2], nextField, tbl->flens[2]);
    tbl->fields[2][tbl->flens[2]] = '\0';
    check_pointer (tbl->fields[2]);

    /* StreetAddress */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[3], nextField, tbl->flens[3]);
    tbl->fields[3][tbl->flens[3]] = '\0';
    check_pointer (tbl->fields[3]);
    /* City */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[4], nextField, tbl->flens[4]);
    tbl->fields[4][tbl->flens[4]] = '\0';
    check_pointer (tbl->fields[4]);
    /* State */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[5], nextField, tbl->flens[5]);
    tbl->fields[5][tbl->flens[5]] = '\0';
    check_pointer (tbl->fields[5]);
    /* Zip */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[6], nextField, tbl->flens[6]);
    tbl->fields[6][tbl->flens[6]] = '\0';
    check_pointer (tbl->fields[6]);
    /* AnnualSalary */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[7], nextField, tbl->flens[7]);
    tbl->fields[7][tbl->flens[7]] = '\0';
    tbl->fields[7][7] = '.';
    check_pointer (tbl->fields[7]);
    /* HomeOwner */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[8], nextField, tbl->flens[8]);
    tbl->fields[8][tbl->flens[8]] = '\0';
    check_pointer (tbl->fields[8]);
    /* DateApplied */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[9], nextField, tbl->flens[9]);
    tbl->fields[9][tbl->flens[9]] = '\0';
    check_pointer (tbl->fields[9]);
    /* LastUpdated */
    nextField = cp + 1;
    strncpy (tbl->fields[10], nextField, tbl->flens[10]);
    tbl->fields[10][tbl->flens[10]] = '\0';
    check_pointer (tbl->fields[10]);

    field2Record (tbl);
    data = malloc (tbl->hdr->sizeRecord);
    if (0 == data) {
      printf ("\n\n***Error: memory exhausted\n");
      closeTable (tbl);
      return _ERROR_;
    }
    memmove (data, tbl->data, tbl->hdr->sizeRecord);
    check_pointer (data);
    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      printf ("\n\n***Error: memory exhausted\n");
      closeTable (tbl);
      return _ERROR_;
    }
    lnk->data = data;
    addList->current = addList->tail->prev;
    insertLinkHere (addList, lnk);

    counter++;
  }
  rtnList = massAddRecords (tbl, addList);
  if (rtnList == (ListHeader *) _ERROR_) {
    printf ("\n\n***Error: mass adding records, %s\n", dberror ());
    return _ERROR_;
  }
  if (rtnList != 0) {
    if (dbError == DB_NOERROR && tbl->dbError == DB_NOERROR) {
      printf
	("\n\n***Warning: %d records not added because of unique constraints\n",
	 rtnList->number);
      if (rtnList->number > 0)
	clearList (rtnList);
      delList (rtnList);
      rtnList = 0;
    }
    else {			/* critical error */
      printf ("\n\n***Error: mass adding records, %s\n", dbtblerror (tbl));
      closeTable (tbl);
      return _ERROR_;
    }
  }
  elapsed (&t2);
  status = storeTableHeader (tbl);
  if (isTableError (tbl)) {
    printf ("\n\n***Error: storing table header, %s\n", dbtblerror (tbl));
    closeTable (tbl);
    return _ERROR_;
  }
  closeTable (tbl);

  printf ("\nAdd time: %f\n", t2 - t1);

  elapsed (&t1);
  tbl = openTable ("table.db");
  if (isDBError ()) {
    printf ("\n\n***Error: Couldn't reopen the table, %s\n", dberror ());
    return _ERROR_;
  }
  status = setCurrentIndex (tbl, "SSNumberIndex");
  if (isTableError (tbl)) {
    printf ("\n\n***Error: setting current index to SSNumber, %s\n",
	    dbtblerror (tbl));
    closeTable (tbl);
    return _ERROR_;
  }

  lnk = SSNList->head->next;
  while (lnk != SSNList->tail) {
    ln = lnk->data;
    foundStatus = searchIndexRecord (tbl, ln);
    if (isTableError (tbl)) {
      printf ("\n\n***Error: %s\n", dbtblerror (tbl));
      closeTable (tbl);
      return _ERROR_;
    }
    if (foundStatus == 0)
      printf ("***Warning: couldn't find %s\n", ln);
    lnk = lnk->next;
  }
  elapsed (&t2);
  if (addList->number > 0)
    clearList (addList);
  delList (addList);
  if (SSNList->number > 0)
    clearList (SSNList);
  delList (SSNList);

  closeTable (tbl);

  printf ("Search time: %f\n", t2 - t1);

  print_block_list ();
  return 0;
}
