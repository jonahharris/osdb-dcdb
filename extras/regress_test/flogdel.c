/*	Source File:	flogdel.c	*/

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
  "LastUpdated",		/* time stamp */
  0
};

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
  2, 0, 0, 0,
  0
};

int main (int argc, char **argv)
{
  dbTable *tbl;
  int counter;
  int i, j;
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
  tbl = buildTable ("table.db", dbTableInformation,
		    ftype, names, flens, declens);
  check_pointer (tbl->fields);
  Assert (0 != tbl);
  if (0 == tbl) {
    printf ("\n\n***Error: creating table, %s\n", dbErrMsg[dbError]);
    return _ERROR_;
  }
  printf ("\nCreating SSNumberIndex\n");
  status = createDBIndex (tbl, "SSNumberIndex", "SSNumber", TRUE, FALSE, 256);
  if (_ERROR_ == status) {
    printf ("\n\n***Error: creating SSNumberIndex: ");
    if (dbError == DB_INDEX) {
      if (idxError == IDX_LIST)
	printf ("list error, %s\n", ListErrorString[ListError]);
      else if (idxError == IDX_SHELL) {
	if (shlError == SHELL_LIST)
	  printf ("list error, %s\n", ListErrorString[ListError]);
	else
	  printf ("shell error, %s\n", shlErrorStr[shlError]);
      }
      else
	printf ("index error, %s\n", idxErrMsg[idxError]);
    }
    else
      printf ("db error, %s\n", dbErrMsg[dbError]);
    return _ERROR_;
  }
  check_pointer (tbl);
  check_pointer (tbl->fields);

  printf ("\nCreating LNameIndex\n");
  status = createDBIndex (tbl, "LNameIndex", "LName", TRUE, FALSE, 512);
  if (_ERROR_ == status) {
    printf ("\n\n***Error: creating LNameIndex: ");
    if (dbError == DB_INDEX) {
      if (idxError == IDX_LIST)
	printf ("list error in index, %s\n", ListErrorString[ListError]);
      else if (idxError == IDX_SHELL) {
	if (shlError == SHELL_LIST)
	  printf ("list error in shell, %s\n", ListErrorString[ListError]);
	else
	  printf ("shell error, %s\n", shlErrorStr[shlError]);
      }
      else
	printf ("index error, %s\n", idxErrMsg[idxError]);
    }
    else
      printf ("db error, %s\n", dbErrMsg[dbError]);
    return _ERROR_;
  }
  check_pointer (tbl);
  check_pointer (tbl->fields);
  closeTable (tbl);
  tbl = openTable ("table.db");
  if (tbl == 0) {
    printf ("\n\n***Error: Couldn't reopen the table, %s\n",
	    dbErrMsg[dbError]);
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
    strncpy (tbl->fields[0], nextField, tbl->flens[0]);
    /*tbl->fields[0][tbl->flens[0]] = '\0'; */
    check_pointer (tbl->fields[0]);

    /* SSNumber */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[1], nextField, tbl->flens[1]);
    /*tbl->fields[1][tbl->flens[1]] = '\0'; */
    check_pointer (tbl->fields[1]);

    /* FName */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[2], nextField, tbl->flens[2]);
    /*tbl->fields[2][tbl->flens[2]] = '\0'; */
    check_pointer (tbl->fields[2]);

    /* StreetAddress */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[3], nextField, tbl->flens[3]);
    /*tbl->fields[3][tbl->flens[3]] = '\0'; */
    check_pointer (tbl->fields[3]);
    /* City */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[4], nextField, tbl->flens[4]);
    /*tbl->fields[4][tbl->flens[4]] = '\0'; */
    check_pointer (tbl->fields[4]);
    /* State */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[5], nextField, tbl->flens[5]);
    /*tbl->fields[5][tbl->flens[5]] = '\0'; */
    check_pointer (tbl->fields[5]);
    /* Zip */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[6], nextField, tbl->flens[6]);
    /*tbl->fields[6][tbl->flens[6]] = '\0'; */
    check_pointer (tbl->fields[6]);
    /* AnnualSalary */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[7], nextField, tbl->flens[7]);
    /*tbl->fields[7][tbl->flens[7]] = '\0'; */
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
    /*tbl->fields[8][tbl->flens[8]] = '\0'; */
    check_pointer (tbl->fields[8]);
    /* DateApplied */
    nextField = cp + 1;
    cp = strchr (nextField, ':');
    if (cp == 0)
      continue;
    else
      *cp = '\0';
    strncpy (tbl->fields[9], nextField, tbl->flens[9]);
    /*tbl->fields[9][tbl->flens[9]] = '\0'; */
    check_pointer (tbl->fields[9]);
    /* LastUpdated */
    nextField = cp + 1;
    strncpy (tbl->fields[10], getTimeStamp (), tbl->flens[10]);
    /*tbl->fields[10][tbl->flens[10]] = '\0'; */
    check_pointer (tbl->fields[10]);

    status = addRecord (tbl);
    if (status == _ERROR_) {
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
	printf ("\n\n***Error item %d: %s\n", counter, dbErrMsg[dbError]);
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
  status = storeTableHeader (tbl);
  if (status == _ERROR_) {
    printf ("\n\n***Error: storing table header, %s\n", dbErrMsg[dbError]);
    closeTable (tbl);
    return _ERROR_;
  }
  elapsed (&t2);
  closeTable (tbl);

  printf ("\nAdd time: %f\n", t2 - t1);

  elapsed (&t1);
  tbl = openTable ("table.db");
  if (tbl == 0) {
    printf ("\n\n***Error: Couldn't reopen the table, %s\n",
	    dbErrMsg[dbError]);
    return _ERROR_;
  }

  /*
   * Traverse the table and delete some records.
   */
  for (i = 0, j = 1; i < 500; i++, j += 50) {
    gotoRecord (tbl, j);
    deleteRecord (tbl);
  }
  /* Now, open sorted files and do compares, first SSN */
  status = setCurrentIndex (tbl, "LNameIndex");
  if (_ERROR_ == status) {
    printf ("\n\n***Error: couldn't set current index to SSNumber\n");
    closeTable (tbl);
    return _ERROR_;
  }
  fp = fopen (argv[1], "r");
  if (0 == fp) {
    printf ("\n\n***Error: couldn't open %s\n", argv[1]);
    closeTable (tbl);
    return _ERROR_;
  }
  status = headIndexRecord (tbl);
  if ((int) _ERROR_ == status) {
    if (dbError == DB_INDEX)
      printf ("\n\n***Error: Index error getting prevIndexRecord, %s\n",
	      idxErrMsg[idxError]);
    else
      printf ("\n\n***Error: getting prevIndexRecord, %s\n",
	      dbErrMsg[dbError]);
    closeTable (tbl);
    return _ERROR_;
  }
  tbl->bof = tbl->eof = FALSE;
  while (tbl->eof == FALSE) {
    status = retrieveRecord (tbl);
    if (_ERROR_ == status) {
      printf ("\n\n***Error: I/O error reading record, %s\n",
	      dbErrMsg[dbError]);
      closeTable (tbl);
      return _ERROR_;
    }
    if (0 == status) {
      /* undelete the record */
      undeleteRecord (tbl);
      status = retrieveRecord (tbl);
      if (_ERROR_ == status) {
	printf ("\n\n***Error: I/O error reading record, %s\n",
		dbErrMsg[dbError]);
	closeTable (tbl);
	return _ERROR_;
      }
    }
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
    if (strcmp (tbl->fields[0], line) || strcmp (tbl->fields[1], cp + 1)) {
      printf ("1: file (%s:%s) != table (%s:%s)\n", line, cp + 1,
	      tbl->fields[0], tbl->fields[1]);
    }
    status = nextIndexRecord (tbl);
    if ((int) _ERROR_ == status) {
      if (dbError == DB_INDEX)
	printf ("\n\n***Error: Index error getting nextIndexRecord, %s\n",
		idxErrMsg[idxError]);
      else
	printf ("\n\n***Error: getting nextIndexRecord, %s\n",
		dbErrMsg[dbError]);
      closeTable (tbl);
      return _ERROR_;
    }
  }
  fclose (fp);

  /* then, LName */
  for (i = 0, j = 1; i < 500; i++, j += 50) {
    gotoRecord (tbl, j);
    deleteRecord (tbl);
  }
  status = setCurrentIndex (tbl, "SSNumberIndex");
  if (_ERROR_ == status) {
    printf ("\n\n***Error: couldn't set current index to SSNumber\n");
    closeTable (tbl);
    return _ERROR_;
  }
  fp = fopen (argv[2], "r");
  if (0 == fp) {
    printf ("\n\n***Error: couldn't open %s\n", argv[2]);
    closeTable (tbl);
    return _ERROR_;
  }
  status = headIndexRecord (tbl);
  if ((int) _ERROR_ == status) {
    if (dbError == DB_INDEX)
      printf ("\n\n***Error: Index error getting prevIndexRecord, %s\n",
	      idxErrMsg[idxError]);
    else
      printf ("\n\n***Error: getting prevIndexRecord, %s\n",
	      dbErrMsg[dbError]);
    closeTable (tbl);
    return _ERROR_;
  }
  status = retrieveRecord (tbl);
  if (_ERROR_ == status) {
    printf ("\n\n***Error: I/O error reading record, %s\n",
	    dbErrMsg[dbError]);
    closeTable (tbl);
    return _ERROR_;
  }
  tbl->bof = tbl->eof = FALSE;
  while (tbl->eof == FALSE) {
    status = retrieveRecord (tbl);
    if (_ERROR_ == status) {
      printf ("\n\n***Error: I/O error reading record, %s\n",
	      dbErrMsg[dbError]);
      closeTable (tbl);
      return _ERROR_;
    }
    if (0 == status) {
      /* undelete the record */
      undeleteRecord (tbl);
      status = retrieveRecord (tbl);
      if (_ERROR_ == status) {
	printf ("\n\n***Error: I/O error reading record, %s\n",
		dbErrMsg[dbError]);
	closeTable (tbl);
	return _ERROR_;
      }
    }
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
    if (strcmp (tbl->fields[1], line) || strcmp (tbl->fields[0], cp + 1)) {
      printf ("2: file (%s:%s) != table (%s:%s)\n", line, cp + 1,
	      tbl->fields[1], tbl->fields[0]);
    }
    status = nextIndexRecord (tbl);
    if ((int) _ERROR_ == status) {
      if (dbError == DB_INDEX)
	printf ("\n\n***Error: Index error getting nextIndexRecord, %s\n",
		idxErrMsg[idxError]);
      else
	printf ("\n\n***Error: getting nextIndexRecord, %s\n",
		dbErrMsg[dbError]);
      closeTable (tbl);
      return _ERROR_;
    }
  }
  fclose (fp);
  elapsed (&t2);

  printf ("Total Search time: %f\n", t2 - t1);

  elapsed (&t1);
  /* now, delete and pack */
  printf ("\n\nTesting pack...");
  printf ("%ld records in the table to begin.\n", tbl->hdr->numRecords);
  counter = 0;
  for (i = 1; (unsigned)i < tbl->hdr->numRecords; i++) {
    if (i % 2 == 0) {
      gotoRecord (tbl, i);
      deleteRecord (tbl);
      counter++;
    }
  }
  printf ("%d records deleted.\n", counter);
  tbl = packTable (tbl, FALSE);
  if (0 == tbl) {
    printf ("\n\n***Error: packing table, %s\n", dbErrMsg[dbError]);
    return _ERROR_;
  }
  printf ("%ld records in table after pack.\n\n", tbl->hdr->numRecords);
  elapsed (&t2);

  closeTable (tbl);

  printf ("Total Pack time: %f\n", t2 - t1);

  print_block_list ();
  return 0;
}
