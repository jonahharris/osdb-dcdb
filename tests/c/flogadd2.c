/* Source File: flogadd1.c */

#ifdef __CINT__
#include "cint_types.h"
#endif

#ifndef __CINT__
#include <stdio.h>
#include <stdlib.h>
#include <interface.h>
#define MAXRECORDS 1000000
#endif

/*#define MAX_SPLIT_LENGTH 256*/

int main (int argc, char *argv[])
{
  FILE *fp;
  int status, i;
  char tbl1[TABLE_NAME_WIDTH+1]; 
  char tbl2[TABLE_NAME_WIDTH+1]; 
  char ssn[10];
  char *field;
  double t1, t2;
  int numLoops = 0;
  int numLicenses;
  ListHeader *lh;
  Link *lnk;
  int counter, modulus, mod_inc;

  /*
   * First, check the command-line arguments.
   */
  if (argc == 1) {
    printf ("\n\nUsage: %s numloops\n", argv[0]);
    printf ("  Where numloops is the number of times to iterate\n");
    return 1;
  }
  if (argc == 2) {
    numLoops = atoi (argv[1]);
    if (numLoops > MAXRECORDS || 
	numLoops < 0) {
      printf ("\n\n***Error: invalid value for numLoops: %d\n", numLoops);
      return _ERROR_;
    }
  }
  /*
   * Now, create the list for SSNs.
   */
  lh = initList (UNSORTED, 0);
  if (0 == lh) {
    printf ("\n\n***Error: could not initialize list: %s\n",
	ListErrorString[ListError]);
    return _ERROR_;
  }

  /*
   * Next, remove old .db & co. files.
   */
  if (fexists ("flogadd2_1.db")) {
    fileRemove ("flogadd2_1.db");
    fileRemove ("flogadd2_1.db.LCK");
    fileRemove ("f2_1_ssnidx.inx");
    fileRemove ("f2_1_ssnidx.idx");
    fileRemove ("f2_1_lfmidx.inx");
    fileRemove ("f2_1_lfmidx.idx");
  }
  if (fexists ("flogadd2_2.db")) {
    fileRemove ("flogadd2_2.db");
    fileRemove ("flogadd2_2.db.LCK");
    fileRemove ("f2_2_ssnidx.inx");
    fileRemove ("f2_2_ssnidx.idx");
  }
  fp = fopen ("flogadd2.df", "wb");
  if (0 == fp) {
    printf ("\n\n***Error: could not open flogadd2.df to write\n");
    return _ERROR_;
  }
  fprintf (fp, 
      "// flogadd2.df\n\n"
      "create table \"flogadd2_1.db\"\n"
      "  info \"Table for flogadd2.c\"\n"
      "{\n"
      "  \"LName\" char (65);\n"
      "  \"SSNumber\" char (9);\n"
      "  \"FName\" char (35);\n"
      "  \"NumberLicenses\" number (2:0);\n"
      "} indexed {\n"
      "  idx \"f2_1_ssnidx\" 128:case:unique \"SSNumber\";\n"
      "  midx \"f2_1_lfmidx\" 512:case \"LName\", \"FName\";\n"
      "};\n\n"
      "create table \"flogadd2_2.db\"\n"
      "  info \"Table for flogadd2.c\"\n"
      "{\n"
      "  \"SSNumber\" char (9);\n"
      "  \"License\" char (7);\n"
      "} indexed {\n"
      "  idx \"f2_2_ssnidx\" 128:case:dup \"SSNumber\";\n"
      "};\n\n"
      );
  fclose (fp);

  status = dbcreate ("flogadd2.df");
  if (status == _ERROR_) {
    printf ("\n\n***Error: Could not dbcreate flogadd2.df: %s\n", cdberror);
    return _ERROR_;
  }

  field = (char *)dbopen ("flogadd2_1.db");
  if (0 == field) {
    printf ("\n\n***Error: Could not dbopen flogadd2_1.db: %s\n", cdberror);
    return _ERROR_;
  }
  memset (tbl1, 0, TABLE_NAME_WIDTH+1);
  strcpy (tbl1, field);

  field = (char *)dbopen ("flogadd2_2.db");
  if (0 == field) {
    printf ("\n\n***Error: Could not dbopen flogadd2_2.db: %s\n", cdberror);
    return _ERROR_;
  }
  memset (tbl2, 0, TABLE_NAME_WIDTH+1);
  strcpy (tbl2, field);

  status = fileRemove ("flogadd2.df");
  if (status == _ERROR_) {
    printf ("\n\n***Warning: could not remove flogadd2.df\n");
  }

  /*
   * OK, populate the table with data generated from the dbtest* commands.
   */

  t1 = dbtime ();
  counter = 0;
  modulus = 10;
  mod_inc = 1;
  while (counter < numLoops) {
    int RAND;
    counter++;
    if (counter%1000 == 0) {
      printf (".");
      fflush (stdout);
    }
    field = dbtestupperstring (0, 65);
    status = dbsetchar (tbl1, "LName", field);
    if (status == _ERROR_) {
      printf ("\n\n***Error: dbsetchar (%s, %s, %s): %s\n",
	  tbl1, "LName", field, cdberror);
      dbexit ();
      return _ERROR_;
    }
    field = dbtestnumber (9, 0);
    status = dbsetchar (tbl1, "SSNumber", field);
    if (status == _ERROR_) {
      printf ("\n\n***Error: dbsetchar (%s, %s, %s): %s\n",
	  tbl1, "SSNumber", field, cdberror);
      dbexit ();
      return _ERROR_;
    }
    strcpy (ssn, field);
    field = dbtestupperstring (0, 35);
    status = dbsetchar (tbl1, "FName", field);
    if (status == _ERROR_) {
      printf ("\n\n***Error: dbsetchar (%s, %s, %s): %s\n",
	  tbl1, "FName", field, cdberror);
      dbexit ();
      return _ERROR_;
    }
    /* generate a psuedo random value */
    t2 = dbtime();
    RAND = (long)t2*1000000L;
    RAND %= modulus;
    if (RAND <= 0)
      RAND = 10;
    modulus += mod_inc;
    if (modulus > 40)
      mod_inc = -1;
    if (modulus < 10)
      mod_inc = 1;
    status = dbsetnum (tbl1, "NumberLicenses", (int)RAND);
    if (status == _ERROR_) {
      printf ("\n\n***Error: dbsetnum (%s, %s, %d): %s\n",
 	 tbl1, "NumberLicenses", RAND, cdberror);
      dbexit();
      return _ERROR_;
    }
    /* Add'em */
    status = dbadd (tbl1);
    if (status == _ERROR_) {
      if (!strcmp (cdberror, 
	    "adding record: unique index constraint violated")) {
	counter--;
	continue;
      }
      printf ("\n\n***Error: dbadd(%s): %s\n", tbl1, cdberror);
      dbexit ();
      return _ERROR_;
    }
    /* Now, add RAND items to the secondary table */
    for (i = 0; i < RAND; i++) {
      status = dbsetchar (tbl2, "SSNumber", ssn);
      if (status == _ERROR_) {
        printf ("\n\n***Error: dbsetchar(%s, %s, %s): %s\n",
 	   tbl2, "SSNumber", ssn, cdberror);
        dbexit ();
        return _ERROR_;
      }
      field = dbtestnumstring (7, 0);
      status = dbsetchar (tbl2, "License", field);
      if (status == _ERROR_) {
        printf ("\n\n***Error: dbsetchar(%s, %s, %s): %s\n",
	    tbl2, "License", field, cdberror);
	dbexit();
	return _ERROR_;
      }
      status = dbadd (tbl2);
      if (status == _ERROR_) {
        printf ("\n\n***Error: dbadd(%s): %s\n", tbl2, cdberror);
        dbexit();
        return _ERROR_;
      }
    }
    lnk = malloc (sizeof(Link));
    if (0 == lnk) {
      printf ("\n\n***Error: Critical memory error allocating Link\n");
      dbexit();
      return _ERROR_;
    }
    field = malloc (10);
    if (0 == field) {
      printf ("\n\n***Error: Critical memory error allocating for ssn\n");
      dbexit ();
      return _ERROR_;
    }
    strcpy (field, ssn);
    check_pointer (field);
    lnk->data = field;
    check_pointer (lnk);
    insertLink (lh, lnk);
  }
  t2 = dbtime();
  printf ("\n\nFinished adding...\n%d items in %s, %d items in %s\n",
      dbnumrecs (tbl1), tbl1, dbnumrecs(tbl2), tbl2);
  printf ("%d items added total\n", dbnumrecs(tbl1)+dbnumrecs(tbl2));
  printf ("Done in %6.4f seconds\n\n", t2-t1);

  /*
   * Now, traverse the table and make sure that the data is consistent with
   * what was entered.
   */
  counter = 0;
  dbcurrent (tbl1, "f2_1_ssnidx");
  dbcurrent (tbl2, "f2_2_ssnidx");
  status = dbiseof (tbl1);
  if (status == 1) {
    printf ("\n\n***Error: There are no records in %s on traversal\n", tbl1);
    dbexit ();
    return _ERROR_;
  }
  for (lnk = lh->head->next; lnk != lh->tail; lnk = lnk->next) {
    counter++;
    status = dbsearchexact (tbl1, (char*)lnk->data);
    if (status == 0) {
      printf ("\n\n***Warning: couldn't find \"%s\" in %s: item %d\n",
	  (char*)lnk->data, tbl1, counter);
      continue;
    }
    status = dbretrieve (tbl1);
    if (status == _ERROR_) {
      printf ("\n\n***Error: dbretrieve(%s): %s\n",
	  tbl1, cdberror);
      dbexit();
      return _ERROR_;
    }
    if (status == 0) {
      /* skip deleted */
      continue;
    }
    field = dbshow (tbl1, "NumberLicenses");
    if (0 == field) {
      printf ("\n\n***Error: dbshow (%s, %s): %s\n", tbl1, "NumberLicenses",
	  cdberror);
      dbexit();
      return _ERROR_;
    }
    numLicenses = atoi (field);
    if (numLicenses < 0 || numLicenses > 100) {
      printf ("\n\n***Error: invalid value for numLicenses: %d\n", numLicenses);
      dbexit ();
      return _ERROR_;
    }
    status = dbsearchexact (tbl2, (char*)lnk->data);
    if (status == _ERROR_) {
      printf ("\n\n***Error: dbsearchexact (%s, %s): %s\n",
	  tbl2, (char*)lnk->data, cdberror);
      dbexit ();
      return _ERROR_;
    }
    if (status == 0) {
      printf ("\n\n***Warning: could not find %s in %s\n", (char*)lnk->data,
	  tbl2);
      continue;
    }
    for (i = 0; i < numLicenses;i++) {
      status = dbretrieve(tbl2);
      if (status == _ERROR_) {
	printf ("\n\n***Error: dbretrieve(%s): %s\n", tbl2, cdberror);
	dbexit ();
	return _ERROR_;
      }
      if (status == 0) {
	/* shouldn't be any deleted records, but what the heck. */
	dbnextindex (tbl2);
	continue;
      }
      field = dbshow (tbl2, "SSNumber");
      if (strcmp ((char*)lnk->data, field)) {
	printf ("\n\n***Warning: ssn from %s (%s) != ssn from %s (%s)\n",
	    tbl1, (char*)lnk->data, tbl2, field);
      }
      dbnextindex (tbl2);
    }
  }
  t2 = dbtime();

  /*
   * Clean up and hit it.
   */
  lnk = removeLink (lh);
  while (lnk != 0) {
    free (lnk->data);
    free (lnk);
    lnk = removeLink (lh);
  }
  delList (lh);
  printf ("\n\nVerified the table data in %6.4f seconds\n", t2-t1);
  dbexit ();
  print_block_list();
  return 0;
}
