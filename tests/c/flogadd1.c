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

int main (void)
{
  FILE *fp;
  int status, i, numrecs = 0;
  char table[TABLE_NAME_WIDTH+1]; 
  char line [1024]; /* arbitrarily long */
  char *field;
  char **spl;
  static char *ssn[MAXRECORDS];
  double salary;
  double t1, t2;

  /*
   * First, remove old .db & co. files.
   */
  if (fexists ("flogadd1.db")) {
    fileRemove ("flogadd1.db");
    fileRemove ("flogadd1.db.LCK");
    fileRemove ("f1_ssnidx.idx");
    fileRemove ("f1_ssnidx.inx");
    fileRemove ("f1_lnidx.idx");
    fileRemove ("f1_lnidx.inx");
  }
  fp = fopen ("flogadd1.df", "wb");
  if (0 == fp) {
    printf ("\n\n***Error: could not open flogadd1.df to write\n");
    return _ERROR_;
  }
  fprintf (fp, 
      "// flogadd1.df\n\n"
      "create table \"flogadd1.db\"\n"
      "  info \"Table for flogadd1.c\"\n"
      "{\n"
      "  \"LName\" char (65);\n"
      "  \"SSNumber\" char (9);\n"
      "  \"FName\" char (35);\n"
      "  \"StreetAddress\" char (75);\n"
      "  \"City\" char (50);\n"
      "  \"State\" char (2);\n"
      "  \"Zip\" char (10);\n"
      "  \"AnnualSalary\" number (10:2);\n"
      "  \"HomeOwner\" logical;\n"
      "  \"DateApplied\" date;\n"
      "  \"LastUpdated\" date;\n"
      "} indexed {\n"
      "  idx \"f1_ssnidx\" 128:case:unique \"SSNumber\";\n"
      "  idx \"f1_lnidx\" 512:case:dup \"LName\";\n"
      "};\n\n"
      );
  fclose (fp);

  status = dbcreate ("flogadd1.df");
  if (status == _ERROR_) {
    printf ("\n\n***Error: Could not dbcreate flogadd1.df: %s\n", cdberror);
    return _ERROR_;
  }
  status = remove ("flogadd1.df");
  if (status == _ERROR_) {
    printf ("\n\n***Warning: could not remove flogadd1.df\n");
  }

  field = (char *)dbopen ("flogadd1.db");
  if (0 == field) {
    printf ("\n\n***Error: Could not open flogadd1.db: %s\n", cdberror);
    return _ERROR_;
  }
  memset (table, 0, TABLE_NAME_WIDTH+1);
  strcpy (table, field);

  /*
   * OK, populate the table.
   */
  t1 = dbtime ();
  fp = fopen ("testAdd.txt", "rb");
  if (0 == fp) {
    printf ("\n\n***Error: could not open testAdd.txt to read\n");
    dbexit ();
    return _ERROR_;
  }

  for (i = 0; i < MAXRECORDS && ! feof (fp); i++) {
    if (i % 1000 == 0 && i != 0) {
      printf (".");
      fflush (stdout);
    }
    memset (line, 1020, 0);
    if (fgets (line, 1000, fp) == 0)
      break;
    if (line[0] == '\n')
      continue;
    field = strchr (line, '\n');
    if (field != 0)
      *field = 0;
    spl = split (line, ':');
    status = dbsetchar (table, "LName", spl[0]);
    if (status == _ERROR_) {
      printf ("\n\n***Error: dbsetchar %s LName %s: %s\n",
         table, spl[0], cdberror);
      dbexit ();
      return _ERROR_;
    }
    status = dbsetchar (table, "SSNumber", spl[1]);
    if (status == _ERROR_) {
      printf ("\n\n***Error: dbsetchar %s SSNumber %s: %s\n",
          table, spl[1], cdberror);
    dbexit ();
      return _ERROR_;
    }
    ssn[i] = malloc (strlen (spl[1]) + 1);
    if (0 == ssn[i]) {
      printf ("\n\n***Error: memory exhausted\n");
      dbexit();
      return _ERROR_;
    }
    strcpy (ssn[i], spl[1]);
    check_pointer (ssn[i]);
    ssn[i+1]=0;
    status = dbsetchar (table, "FName", spl[2]);
    if (status == _ERROR_) {
      printf ("\n\n***Error: dbsetchar %s FName, %s: %s\n",
	  table, spl[2], cdberror);
      dbexit();
      return _ERROR_;
    }
    status = dbsetchar (table, "StreetAddress", spl[3]);
    if (status == _ERROR_) {
      printf ("\n\n***Error: dbsetchar %s StreetAddress, %s: %s\n",
	  table, spl[3], cdberror);
      dbexit();
      return _ERROR_;
    }
    status = dbsetchar (table, "City", spl[4]);
    if (status == _ERROR_) {
      printf ("\n\n***Error: dbsetchar %s City, %s: %s\n",
	  table, spl[4], cdberror);
      dbexit();
      return _ERROR_;
    }
    status = dbsetchar (table, "State", spl[5]);
    if (status == _ERROR_) {
      printf ("\n\n***Error: dbsetchar %s State, %s: %s\n",
	  table, spl[5], cdberror);
      dbexit();
      return _ERROR_;
    }
    status = dbsetchar (table, "Zip", spl[6]);
    if (status == _ERROR_) {
      printf ("\n\n***Error: dbsetchar %s Zip, %s: %s\n",
	  table, spl[6], cdberror);
      dbexit();
      return _ERROR_;
    }
    salary = atof (spl[7]);
    if (salary < 0.00 || salary > 99999.99)
      salary = 95000.00; /* decent salary :o) */
    status = dbsetnum (table, "AnnualSalary", salary);
    if (status == _ERROR_) {
      printf ("\n\n***Error: dbsetnum %s AnnualSalary, %s: %s\n",
	  table, spl[7], cdberror);
      dbexit();
      return _ERROR_;
    }
    status = dbsetlog (table, "HomeOwner", *spl[8]);
    if (status == _ERROR_) {
      printf ("\n\n***Error: dbsetlog %s HomeOwner, %s: %s\n",
	  table, spl[8], cdberror);
      dbexit();
      return _ERROR_;
    }
    status = dbsetdate (table, "DateApplied", spl[9]);
    if (status == _ERROR_) {
      printf ("\n\n***Error: dbsetdate %s DateApplied, %s: %s\n",
	  table, spl[9], cdberror);
      dbexit();
      return _ERROR_;
    }
    status = dbsetdate (table, "LastUpdated", spl[10]);
    if (status == _ERROR_) {
      printf ("\n\n***Error: dbsetdate (%s, LastUpdated, %s): %s\n",
	  table, spl[10], cdberror);
      dbexit();
      return _ERROR_;
    }

    /*
     * Add it.
     */
    status = dbadd (table);
    if (status != _OK_) {
      if (! strcmp (cdberror, "adding record: unique index constraint violated") ) {
	printf ("\n\n***Warning: Duplicate SSN at line %d\n", i);
	continue;
      }
      printf ("\n\n***Error: calling dbadd: %s\n", cdberror);
      dbexit ();
      return _ERROR_;
    }
  }
  fclose (fp);
  numrecs = dbnumrecs (table);
  t2 = dbtime ();
  printf ("\n\nFinished adding %d records to %s in %6.4f seconds\n",
      numrecs, table, t2-t1);

  fp = fopen ("flogadd1.output", "wb");
  if (0 == fp) {
    printf ("\n\n***Error: Could not open flogadd1.output\n");
    dbexit ();
    return _ERROR_;
  }
  t1 = dbtime ();
  for (i = 0; i < MAXRECORDS && ssn[i] != 0; i++) {
    status = dbsearchexact (table, ssn[i]);
    if (status == _ERROR_) {
      printf ("\n\n***Error: dbsearch (%s, %s)\n", table, ssn[i]);
      dbexit ();
      return _ERROR_;
    }
    free (ssn[i]);
    if (status == _NOTFOUND_) {
      printf ("\n\n***Warning: didn't find %s\n", ssn[i]);
      continue;
    }
    /*
     * OK, retrieve the data and make some effort getting the data to the program.
     */
    status = dbretrieve (table);
    if (status == _ERROR_) {
      printf ("\n\n***Error: dbretrieve (%s)\n",table); 
      dbexit ();
      return _ERROR_;
    }
    /* printf ("table = %s\n", table); */
    field = dbshow (table, "LName");
    /*printf ("table = %s\n", table);*/
    if (field == 0) {
      printf ("\n\n***Error: dbshow (%s, LName): %s\n",
	  table, cdberror);
      dbexit();
      return _ERROR_;
    }
    fprintf (fp, "%s:", field);
    field = dbshow (table, "SSNumber");
    if (field == 0) {
      printf ("\n\n***Error: dbshow (%s, SSNumber): %s\n",
	  table, cdberror);
      dbexit();
      return _ERROR_;
    }
    fprintf (fp, "%s:", field);
    field = dbshow (table, "FName");
    if (field == 0) {
      printf ("\n\n***Error: dbshow (%s, FName): %s\n",
	  table, cdberror);
      dbexit();
      return _ERROR_;
    }
    fprintf (fp, "%s:", field);
    field = dbshow (table, "StreetAddress");
    if (field == 0) {
      printf ("\n\n***Error: dbshow (%s, StreetAddress): %s\n",
	  table, cdberror);
      dbexit();
      return _ERROR_;
    }
    fprintf (fp, "%s:", field);
    field = dbshow (table, "City");
    if (field == 0) {
      printf ("\n\n***Error: dbshow (%s, City): %s\n",
	  table, cdberror);
      dbexit();
      return _ERROR_;
    }
    fprintf (fp, "%s:", field);
    field = dbshow (table, "State");
    if (field == 0) {
      printf ("\n\n***Error: dbshow (%s, State): %s\n",
	  table, cdberror);
      dbexit();
      return _ERROR_;
    }
    fprintf (fp, "%s:", field);
    field = dbshow (table, "Zip");
    if (field == 0) {
      printf ("\n\n***Error: dbshow (%s, LName): %s\n",
	  table, cdberror);
      dbexit();
      return _ERROR_;
    }
    fprintf (fp, "%s:", field);
    field = dbshow (table, "AnnualSalary");
    if (field == 0) {
      printf ("\n\n***Error: dbshow (%s, AnnualSalary): %s\n",
	  table, cdberror);
      dbexit();
      return _ERROR_;
    }
    fprintf (fp, "%s:", field);
    field = dbshow (table, "HomeOwner");
    if (field == 0) {
      printf ("\n\n***Error: dbshow (%s, HomeOwner): %s\n",
	  table, cdberror);
      dbexit();
      return _ERROR_;
    }
    fprintf (fp, "%s:", field);
    field = dbshow (table, "DateApplied");
    if (field == 0) {
      printf ("\n\n***Error: dbshow (%s, DateApplied): %s\n",
	  table, cdberror);
      dbexit();
      return _ERROR_;
    }
    fprintf (fp, "%s:", field);
    field = dbshow (table, "LastUpdated");
    if (field == 0) {
      printf ("\n\n***Error: dbshow (%s, LastUpdated): %s\n",
	  table, cdberror);
      dbexit();
      return _ERROR_;
    }
    fprintf (fp, "%s\n", field);
  }
  t2 = dbtime ();
  fclose (fp);
  printf ("\n\nFinished searching %d records to %s in %6.4f seconds\n",
      numrecs, table, t2-t1);

  /* Done */
  dbexit ();
  print_block_list();
  return _OK_;
}
