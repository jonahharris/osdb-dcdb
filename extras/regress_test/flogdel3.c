/*	Source File:	flogdel3.c	*/

/*
 * This program is designed to test that after a table is packed,
 * it contains the records that weren't deleted and ONLY the records
 * that weren't deletes.
 */
#include <cdb.h>
#include <stdio.h>

#define	MODULUS		2

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
  "Entered",			/* time stamp */
  0
};

#define	LNAME			0
#define	SSN				1
#define	FNAME			2
#define	SADDR			3
#define	CITY			4
#define	STATE			5
#define	ZIP				6
#define	ANSAL			7
#define	HMOWN			8
#define	APPLIED			9
#define	ENTERED			10

fieldType ftype[] = {
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
  0
};

int flens[] = {
  65, 10, 35, 80, 50, 2, 10,
  10, 1, 8, 26,
  0
};

int declens[] = {
  0, 0, 0, 0, 0, 0, 0,
  3, 0, 0, 0,
  0
};

char *name_idx[] = {
  "LName",
  "FName",
  0
};

int main (void)
{
  dbTable *tbl;
  int counter;
  int i;
  int status;
  FILE *fp;
  char line[512];
  char *cp, *nextField;
  double t1, t2;
  ListHeader *deleted = 0;
  ListHeader *nodeleted = 0;
  char *lname = 0;
  Link *lnk = 0;

  fp = fopen ("flogdel3.df", "wb");
  if (0 == fp) {
    printf ("\n\n***Error: could not open flogdel3.df to write\n");
    return _ERROR_;
  }
  fprintf (fp,
      "//flogdel3.df\n\n"
      "create table \"flogdel3.db\"\n"
      "  info \"Table for flogdel3.c\"\n"
      "{\n"
      "  \"LName\" char (65);\n"
      "  \"SSNumber\" char (9);\n"
      "  \"FName\" char (35);\n"
      "  \"StreetAddress\" char (80);\n"
      "  \"City\" char (50);\n"
      "  \"State\" char (2);\n"
      "  \"Zip\" char (10);\n"
      "  \"AnnualSalary\" number (10:2);\n"
      "  \"HomeOwner\" logical;\n"
      "  \"DateApplied\" date;\n"
      "  \"LastUpdated\" date;\n"
      "} indexed {\n"
      "  idx \"fd3_ssnidx\" 128:case:unique \"SSNumber\";\n"
      "  idx \"fd3_lnidx\" 512:case:dup \"LName\";\n"
      "  midx \"fd3_lfname\" 512:case \"LName\", \"FName\";\n"
      "};\n\n"
      );
  fclose (fp);

  parseDBDef ("flogdel3.df");
  if (isDBError ()) {
    printf ("\n\n***Error after parseDBDef: %s\n", dberror ());
    return 1;
  }
  tbl = openTable ("flogdel3.db");
  if (isDBError ()) {
    printf ("\n\n***Error: opening newly created table, %s\n", dberror ());
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
  counter = 1;
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
    strncpy (tbl->fields[LNAME], nextField, tbl->flens[LNAME]);
    check_pointer (tbl->fields[LNAME]);

    /* SSNumber */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[SSN], nextField, tbl->flens[SSN]);
    check_pointer (tbl->fields[SSN]);

    /* FName */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[FNAME], nextField, tbl->flens[FNAME]);
    check_pointer (tbl->fields[FNAME]);

    /* StreetAddress */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[SADDR], nextField, tbl->flens[SADDR]);
    check_pointer (tbl->fields[SADDR]);
    /* City */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[CITY], nextField, tbl->flens[CITY]);
    check_pointer (tbl->fields[CITY]);
    /* State */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[STATE], nextField, tbl->flens[STATE]);
    check_pointer (tbl->fields[STATE]);
    /* Zip */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[ZIP], nextField, tbl->flens[ZIP]);
    check_pointer (tbl->fields[ZIP]);
    /* AnnualSalary */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[ANSAL], nextField, tbl->flens[ANSAL]);
    tbl->fields[ANSAL][tbl->flens[ANSAL] - tbl->declens[ANSAL] - 1] = '.';
    check_pointer (tbl->fields[ANSAL]);
    /* HomeOwner */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[HMOWN], nextField, tbl->flens[HMOWN]);
    check_pointer (tbl->fields[HMOWN]);
    /* DateApplied */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[APPLIED], nextField, tbl->flens[APPLIED]);
    check_pointer (tbl->fields[APPLIED]);
    /* Entered */
    nextField = cp + 1;
    strncpy (tbl->fields[ENTERED], getTimeStamp (), tbl->flens[ENTERED]);
    check_pointer (tbl->fields[10]);

    status = addRecord (tbl);
    if (isTableError (tbl)) {
      if (tbl->dbError == DB_UNIQUE) {
	printf ("\n\n***Warning: unique constraint violated with %s\n",
		tbl->fields[1]);
	tbl->dbError = DB_NOERROR;
	dbError = DB_NOERROR;
	idxError = IDX_NOERR;
	set_shlError(0, SHELL_NOERR);
	set_ListError(0, LIST_NOERROR);
	goto DoItAgain;
      }
      else {
	printf ("\n\n***Error item %d: %s\n", counter, dberror ());
	closeTable (tbl);
	return _ERROR_;
      }
    }
  DoItAgain:
    counter++;
    if (counter % 1000 == 0) {
      printf (".");
      fflush (stdout);
    }
  }

  printf ("\n\n");
  status = storeTableHeader (tbl);
  if (status == _ERROR_) {
    printf ("\n\n***Error: storing table header, %s\n", dberror ());
    closeTable (tbl);
    return _ERROR_;
  }
  elapsed (&t2);
  closeTable (tbl);

  printf ("\nAdd time: %f\n", t2 - t1);

  tbl = openTable ("flogdel3.db");
  if (tbl == 0) {
    printf ("\n\n***Error: Couldn't reopen the table, %s\n", dberror ());
    return _ERROR_;
  }

  printf ("Deleting records and packing...");
  printf ("%ld records in the table to begin.\n", tbl->hdr->numRecords);
  counter = 1;

  elapsed (&t1);
  deleted = initList (UNSORTED, 0);
  nodeleted = initList (UNSORTED, 0);
  for (i = 1; (unsigned)i <= tbl->hdr->numRecords; i++) {
    if (i % MODULUS == 0) {
      gotoRecord (tbl, i);
      retrieveRecord (tbl);
      lname = malloc (tbl->flens[LNAME] + 1);
      if (lname == 0) {
	printf ("\n\n***Error: dynamic memory exhausted\n");
	return _ERROR_;
      }
      lnk = malloc (sizeof (Link));
      if (lnk == 0) {
	printf ("\n\n***Error: dynamic memory exhausted\n");
	return _ERROR_;
      }
      strcpy (lname, tbl->fields[LNAME]);
      lnk->data = lname;
      deleteRecord (tbl);
      deleted->current = deleted->tail->prev;
      insertLinkHere (deleted, lnk);
    }
    else {
      gotoRecord (tbl, i);
      retrieveRecord (tbl);
      lname = malloc (tbl->flens[LNAME] + 1);
      if (lname == 0) {
	printf ("\n\n***Error: dynamic memory exhausted\n");
	return _ERROR_;
      }
      lnk = malloc (sizeof (Link));
      if (lnk == 0) {
	printf ("\n\n***Error: dynamic memory exhausted\n");
	return _ERROR_;
      }
      strcpy (lname, tbl->fields[LNAME]);
      lnk->data = lname;
      nodeleted->current = nodeleted->tail->prev;
      insertLinkHere (nodeleted, lnk);
    }
  }
  elapsed (&t2);

  printf ("Time to delete %d records, %f\n", deleted->number, t2 - t1);

  elapsed (&t1);
  dbg2 (printf ("\n\npackTable()\n");
    );
  tbl = packTable (tbl, FALSE);
  dbg2 (printf ("\n\npackTable() end\n");
    );
  if (tbl == 0 || isDBError () || isTableError (tbl)) {
    printf ("\n\n***Error: packing table, %s\n", dberror ());
    return _ERROR_;
  }
  status = setCurrentIndex (tbl, "fd3_lnidx");
  if (isTableError (tbl)) {
    printf ("\n\n***Error: setting current index, %s\n", dbtblerror (tbl));
    closeTable (tbl);
    return _ERROR_;
  }

  elapsed (&t2);
  printf ("Time to pack, %f\n", t2 - t1);
  printf ("%ld records in table after pack\n", tbl->hdr->numRecords);

  elapsed (&t1);
  lnk = deleted->head->next;
  while (lnk != deleted->tail) {
    lname = lnk->data;
    if (searchExactIndexRecord (tbl, lname)) {
      if (isTableError (tbl)) {
	printf ("\n\n***Error: searching for deleted items, %s\n",
		dbtblerror (tbl));
	closeTable (tbl);
	return _ERROR_;
      }
      printf ("OOPS: %s deleted but still in table\n", lname);
    }
    lnk = lnk->next;
  }
  lnk = nodeleted->head->next;
  while (lnk != nodeleted->tail) {
    lname = lnk->data;
    if (!searchExactIndexRecord (tbl, lname)) {
      if (isTableError (tbl)) {
	printf ("\n\n***Error: searching for undeleted items, %s\n",
		dbtblerror (tbl));
	closeTable (tbl);
	return _ERROR_;
      }
      printf ("OOPS: %s not deleted and not in table\n", lname);
    }
    lnk = lnk->next;
  }
  elapsed (&t2);

  printf ("Time to search, %f\n", t2 - t1);

  if (deleted != 0)
    if (deleted->number > 0)
      clearList (deleted);
  delList (deleted);
  if (nodeleted != 0)
    if (nodeleted->number > 0)
      clearList (nodeleted);
  delList (nodeleted);
  closeTable (tbl);
  print_block_list ();
  return _OK_;
}
