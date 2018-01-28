/*	Source File:	flogdel2.c	*/

#include <cdb.h>
#include <index.h>
#include <sort.h>
#include <test.h>

#include <stdio.h>
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

int main (int argc, char **argv)
{
  dbTable *tbl;
  int counter;
  int backCounter;
  int i;
  int status;
  FILE *fp;
  char line[512];
  char *cp, *nextField;
  double t1, t2;

  /*
   * Check command line arguments.
   */
  if (argc != 3) {
    printf ("\n\n***Error: usage, %s file1 file2\n", argv[0]);
    printf ("Where file1 = SSN:LName sorted by SSN\n");
    printf ("and file2 = LName:SSN sorted by LName.\n\n");
    return _ERROR_;
  }
  if (!fexists ((char *) "table.db")) {
    tbl = buildTable ("table.db", dbTableInformation,
		      ftype, names, flens, declens);
    check_pointer (tbl->fields);
    Assert (0 != tbl);
    if (isDBError ()) {
      printf ("\n\n***Error: creating table, %s\n", dberror ());
      return _ERROR_;
    }
    status = createDBIndex (tbl, "SSNumberIndex", "SSNumber",
			    TRUE, FALSE, 256);
    if (isTableError (tbl)) {
      printf ("\n\n***Error: creating SSNumber index, %s\n",
	      dbtblerror (tbl));
      return _ERROR_;
    }
    check_pointer (tbl);
    check_pointer (tbl->fields);

    status = createDBIndex (tbl, "LNameIndex", "LName", TRUE, FALSE, 512);
    if (isTableError (tbl)) {
      printf ("\n\n***Error: creating LName index, %s\n", dbtblerror (tbl));
      return _ERROR_;
    }

    status = createMultiIndex (tbl, "NamesIndex", name_idx, TRUE, 512);
    if (isTableError (tbl)) {
      printf ("\n\n***Error: creating NamesIndex: %s", dbtblerror (tbl));
      return _ERROR_;
    }
    check_pointer (tbl);
    check_pointer (tbl->fields);
    closeTable (tbl);
  }
  tbl = openTable ("table.db");
  if (isDBError ()) {
    printf ("\n\n***Error: Couldn't reopen the table, %s\n", dberror ());
    return _ERROR_;
  }
  if (tbl->hdr->numRecords >= 100000) {
    printf ("\n\n***Error: 500000 records in table already\n");
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
  for (i = 0; i < tbl->hdr->numFields; i++) {
    memset (tbl->fields[i], 0, tbl->flens[i]);
    check_pointer (tbl->fields[i]);
  }
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
    /*tbl->fields[LNAME][tbl->flens[LNAME]] = '\0'; */
    check_pointer (tbl->fields[LNAME]);

    /* SSNumber */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[SSN], nextField, tbl->flens[SSN]);
    /*tbl->fields[SSN][tbl->flens[SSN]] = '\0'; */
    check_pointer (tbl->fields[SSN]);

    /* FName */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[FNAME], nextField, tbl->flens[FNAME]);
    /*tbl->fields[2][tbl->flens[2]] = '\0'; */
    check_pointer (tbl->fields[FNAME]);

    /* StreetAddress */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[SADDR], nextField, tbl->flens[SADDR]);
    /*tbl->fields[3][tbl->flens[3]] = '\0'; */
    check_pointer (tbl->fields[SADDR]);
    /* City */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[CITY], nextField, tbl->flens[CITY]);
    /*tbl->fields[4][tbl->flens[4]] = '\0'; */
    check_pointer (tbl->fields[CITY]);
    /* State */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[STATE], nextField, tbl->flens[STATE]);
    /*tbl->fields[5][tbl->flens[5]] = '\0'; */
    check_pointer (tbl->fields[STATE]);
    /* Zip */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[ZIP], nextField, tbl->flens[ZIP]);
    /*tbl->fields[6][tbl->flens[6]] = '\0'; */
    check_pointer (tbl->fields[ZIP]);
    /* AnnualSalary */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[ANSAL], nextField, tbl->flens[ANSAL]);
    /*tbl->fields[7][tbl->flens[7]] = '\0'; */
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
    /*tbl->fields[8][tbl->flens[8]] = '\0'; */
    check_pointer (tbl->fields[HMOWN]);
    /* DateApplied */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[APPLIED], nextField, tbl->flens[APPLIED]);
    /*tbl->fields[9][tbl->flens[9]] = '\0'; */
    check_pointer (tbl->fields[APPLIED]);
    /* Entered */
    nextField = cp + 1;
    strncpy (tbl->fields[ENTERED], getTimeStamp (), tbl->flens[ENTERED]);
    /*tbl->fields[10][tbl->flens[10]] = '\0'; */
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
	printf ("\n\n***Error item %d: %s\n", counter, dbtblerror (tbl));
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
  if (isTableError (tbl)) {
    printf ("\n\n***Error: storing table header, %s\n", dbtblerror (tbl));
    closeTable (tbl);
    return _ERROR_;
  }
  elapsed (&t2);
  closeTable (tbl);

  printf ("\nAdd time: %f\n", t2 - t1);

  elapsed (&t1);
  tbl = openTable ("table.db");
  if (isDBError ()) {
    printf ("\n\n***Error: Couldn't reopen the table, %s\n", dberror ());
    return _ERROR_;
  }

  /* Now, open sorted files and do compares, first LName */
  status = setCurrentIndex (tbl, "LNameIndex");
  if (isTableError (tbl)) {
    printf ("\n\n***Error: couldn't set current index to LName, %s\n",
	    dbtblerror (tbl));
    closeTable (tbl);
    return _ERROR_;
  }
  fp = fopen (argv[1], "r");
  if (0 == fp) {
    printf ("\n\n***Error: couldn't open %s\n", argv[1]);
    closeTable (tbl);
    return _ERROR_;
  }
  while (TRUE) {
    memset (line, 0, 500);
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
    cp = strchr (line, ':');
    if (cp == 0)
      continue;
    *cp = '\0';
    status = searchExactIndexRecord (tbl, line);
    if (isTableError (tbl)) {
      printf ("\n\n***Error: searching index item, LName, %s\n",
	      dbtblerror (tbl));
      closeTable (tbl);
      return _ERROR_;
    }
    if (status != _FOUND_) {
      printf ("***Error: couldn't find %s in LName index\n", line);
      closeTable (tbl);
      return _ERROR_;
    }
  }
  fclose (fp);

  status = setCurrentIndex (tbl, "SSNumberIndex");
  if (isTableError (tbl)) {
    printf ("\n\n***Error: couldn't set current index to SSNumber, %s\n",
	    dbtblerror (tbl));
    closeTable (tbl);
    return _ERROR_;
  }
  fp = fopen (argv[2], "r");
  if (0 == fp) {
    printf ("\n\n***Error: couldn't open %s\n", argv[2]);
    closeTable (tbl);
    return _ERROR_;
  }
  while (TRUE) {
    memset (line, 0, 500);
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
    cp = strchr (line, ':');
    if (cp == 0)
      continue;
    *cp = '\0';
    status = searchExactIndexRecord (tbl, line);
    if (isTableError (tbl)) {
      printf ("\n\n***Error: searching table, SSNumber index, %s\n",
	      dbtblerror (tbl));
      closeTable (tbl);
      return _ERROR_;
    }
    if (status != _FOUND_) {
      printf ("***Warning: couldn't find %s in SSN index\n", line);
      closeTable (tbl);
      return _ERROR_;
    }
  }
  fclose (fp);
  elapsed (&t2);

  printf ("Total Search time: %f\n", t2 - t1);

  elapsed (&t1);
  /* now, delete and pack */
  printf ("\n\nDeleting records and packing...");
  printf ("%ld records in the table to begin.\n", tbl->hdr->numRecords);
  counter = 0;
  while (TRUE) {
    for (i = 1; i < 100; i++) {
      if ((unsigned)counter >= tbl->hdr->numRecords || counter >= 500)
	break;
      gotoRecord (tbl, counter);
      deleteRecord (tbl);
      counter++;
    }
    if ((unsigned)counter >= tbl->hdr->numRecords || counter >= 500)
      break;
    for (i = 0; i < 10; i++, counter++)
      if ((unsigned)counter >= tbl->hdr->numRecords || counter >= 500)
	break;
    if ((unsigned)counter >= tbl->hdr->numRecords || counter >= 500)
      break;
  }
  counter = tbl->hdr->numRecords - 1;
  backCounter = counter - 500;
  while (TRUE) {
    for (i = 1; i < 100; i++) {
      if ((unsigned)counter >= tbl->hdr->numRecords || counter <= backCounter)
	break;
      gotoRecord (tbl, counter);
      deleteRecord (tbl);
      counter--;
    }
    if ((unsigned)counter >= tbl->hdr->numRecords || counter <= backCounter)
      break;
    for (i = 0; i < 10; i++, counter--)
      if ((unsigned)counter >= tbl->hdr->numRecords || counter <= backCounter)
	break;
    if ((unsigned)counter >= tbl->hdr->numRecords || counter <= backCounter)
      break;
  }
  tbl = packTable (tbl, FALSE);
  if (isDBError ()) {
    printf ("\n\n***Error: packing table, %s\n", dberror ());
    return _ERROR_;
  }
  printf ("%ld records in table after pack.\n\n", tbl->hdr->numRecords);
  elapsed (&t2);

  closeTable (tbl);

  printf ("Total Delete and Pack time: %f\n", t2 - t1);

  print_block_list ();
  return 0;
}
