/* Source File: flogreindex1.c */

#ifdef __CINT__
#include "cint_types.h"
#endif

#ifndef __CINT__
#include <stdio.h>
#include <stdlib.h>
#include <interface.h>
#define MAXRECORDS 1000000
#endif

#define NUM_DUP_SSNS 1000

int check_dups (char **dup_ssn, char *ssn)
{
  int i;

  for (i = 0; i < NUM_DUP_SSNS && dup_ssn[i] != 0; i++) {
    if (!strcmp (dup_ssn[i], ssn))
	return TRUE;
  }
  return FALSE;
}

int main (void)
{
  FILE *fp;
  int status, i, numrecs = 0;
  char table[TABLE_NAME_WIDTH+1];
  char *field;
  char line[1024];
  char **spl;
  static char *dup_ssn[NUM_DUP_SSNS];
  int this_dup = 0;
  int header = 0;
  double salary;
  double t1, t2;

  /* First, remove the old db&co. */
  if (fexists ("flogreindex1.db")) {
    fileRemove ("flogreindex1.db");
    fileRemove ("flogreindex1.db.LCK");
    fileRemove ("fr1_ssnidx.idx");
    fileRemove ("fr1_ssnidx.inx");
    fileRemove ("fr1_lnidx.idx");
    fileRemove ("fr1_lnidx.inx");
    fileRemove ("fr1_lfnidx.idx");
    fileRemove ("fr1_lfnidx.inx");
  }
  fp = fopen ("flogreindex1.df", "wb");
  if (0 == fp) {
    printf ("\n\n***Error: could not open flogreindex1.df to write\n");
    return _ERROR_;
  }
  fprintf (fp,
      "//flogreindex1.df\n\n"
      "create table \"flogreindex1.db\"\n"
      "  info \"Table for flogreindex1.c\"\n"
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
      "  idx \"fr1_ssnidx\" 128:case:unique \"SSNumber\";\n"
      "  idx \"fr1_lnidx\" 512:case:dup \"LName\";\n"
      "  midx \"fr1_lfnidx\" 512:case \"LName\", \"FName\";\n"
      "};\n\n"
      );
  fclose (fp);

  status = dbcreate ("flogreindex1.df");
  if (status == _ERROR_) {
    printf ("\n\n***Error: dbcreate(%s): %s\n", "flogreindex1.df", cdberror);
    return _ERROR_;
  }

  field = (char*)dbopen ("flogreindex1.db");
  if (field == 0) {
    printf ("\n\n***Error: dbopen (%s): %s\n", "flogreindex1.db", cdberror);
    return _ERROR_;
  }
  strcpy (table, field);

  t1 = dbtime ();
  fp = fopen ("testAdd.txt", "rb");
  if (0 == fp) {
    printf ("\n\n***Error: could not open testAdd.txt to read\n");
    dbexit ();
    return _ERROR_;
  }

  for (i = 0; i < MAXRECORDS; i++) {
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
      printf ("\n\n***Error: dbsetchar %s AnnualSalary, %s: %s\n",
          table, spl[7], cdberror);
      dbexit();
      return _ERROR_;
    }
    status = dbsetlog (table, "HomeOwner", *spl[8]);
    if (status == _ERROR_) {
      printf ("\n\n***Error: dbsetchar %s HomeOwner, %s: %s\n",
          table, spl[8], cdberror);
      dbexit();
      return _ERROR_;
    }
    status = dbsetdate (table, "DateApplied", spl[9]);
    if (status == _ERROR_) {
      printf ("\n\n***Error: dbsetdate (%s, DateApplied): %s: %s\n",
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
      if (! strcmp (cdberror,
	    "adding record: unique index constraint violated")) {
        printf ("\n\n***Warning: Duplicate SSN at line %d\n", i);
	dup_ssn[this_dup] = malloc (10);
	if (dup_ssn[this_dup] == 0) {
	  printf ("\n\n***Error: memory error allocating dup_ssn[%d]\n",
	      this_dup);
	  dbexit();
	  return _ERROR_;
	}
	strcpy (dup_ssn[this_dup++], dbshow (table, "SSNumber"));
	check_pointer (dup_ssn[this_dup-1]);
	if (this_dup >= NUM_DUP_SSNS) {
	  printf ("\n\n***Error: there are too many duplicates...");
	  printf ("increase NUM_DUP_SSNS, recompile and try again.\n");
	  dbexit();
	  return _ERROR_;
	}
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

  dbclose (table);
  t1 = dbtime ();
  status = dbreindex ("flogreindex1.db");
  if (status == _ERROR_) {
    printf ("\n\n***Error: dbreindex (flogreindex1.db): %s\n", cdberror);
    dbexit();
    return _ERROR_;
  }
  t2 = dbtime ();
  printf ("Reindexed flogreindex1.db in %6.4f seconds\n", t2-t1);

  field = (char*)dbopen ("flogreindex1.db");
  if (field == 0) {
    printf ("\n\n***Error: dbopen (%s): %s\n", "flogreindex1.db", cdberror);
    return _ERROR_;
  }
  strcpy (table, field);

  t1 = dbtime ();
  /*
   * Now, traverse the data and insure consistency.
   */
  dbcurrent (table, "fd1_ssnidx");
  fp = fopen ("testAdd.txt", "rb");
  if (0 == fp) {
    printf ("\n\n***Error: could not open testAdd.txt to read\n");
    dbexit ();
    return _ERROR_;
  }

  for (i = 1; i <= MAXRECORDS; i++) {
    header = 0;
    memset (line, 1020, 0);
    if (fgets (line, 1000, fp) == 0)
      break;
    if (line[0] == '\n')
      continue;
    field = strchr (line, '\n');
    if (field != 0)
      *field = 0;
    spl = split (line, ':');

    status = dbsearchexact (table, spl[1]);
    if (status == 0) {
      printf ("\n\n***Warning: couldn't find %s in the database: line %d\n",
	  spl[1], i);
      continue;
    }
    status = dbretrieve (table);
    if (status == _ERROR_) {
      printf ("\n\n***Error: dbretrieve(%s): %s\n", table, cdberror);
      dbexit();
      return _ERROR_;
    }
    field = dbshow (table, "LName");
    if (strcmp (field, spl[0])) {
      status = check_dups ((char**)dup_ssn, spl[1]);
      if (status)
	continue;
      if (!header) {
        printf ("\n\n***Warning: Record out of sync: LName");
        header = 1;
      }
    }
    field = dbshow (table, "FName");
    if (strcmp (field, spl[2])) {
      status = check_dups ((char**)dup_ssn, spl[1]);
      if (status)
	continue;
      if (!header) {
	printf ("\n\n***Warning: Record out of sync: FName");
	header = 1;
      }
      else {
	printf (" FName");
      }
    }
    field = dbshow (table, "StreetAddress");
    if (strcmp (field, spl[3])) {
      status = check_dups ((char**)dup_ssn, spl[1]);
      if (status)
	continue;
      if (!header) {
	printf ("\n\n***Warning: Record out of sync: StreetAddress");
	header = 1;
      }
      else {
	printf (" StreetAddress");
      }
    }
    field = dbshow (table, "City");
    if (strcmp (field, spl[4])) {
      status = check_dups ((char**)dup_ssn, spl[1]);
      if (status)
	continue;
      if (!header) {
	printf ("\n\n***Warning: Record out of sync: City");
	header = 1;
      }
      else {
	printf (" City");
      }
    }
    field = dbshow (table, "State");
    if (strcmp (field, spl[5])) {
      status = check_dups ((char**)dup_ssn, spl[1]);
      if (status)
	continue;
      if (!header) {
	printf ("\n\n***Warning: Record out of sync: State");
	header = 1;
      }
      else {
	printf (" State");
      }
    }
    field = dbshow (table, "Zip");
    if (strcmp (field, spl[6])) {
      status = check_dups ((char**)dup_ssn, spl[1]);
      if (status)
	continue;
      if (!header) {
	printf ("\n\n***Warning: Record out of sync: Zip");
	header = 1;
      }
      else {
	printf (" Zip");
      }
    }
    field = dbshow (table, "AnnualSalary");
    if (strcmp (field, spl[7])) {
      status = check_dups ((char**)dup_ssn, spl[1]);
      if (status)
	continue;
      if (!header) {
	printf ("\n\n***Warning: Record out of sync: AnnualSalary");
	header = 1;
      }
      else {
	printf (" AnnualSalary");
      }
    }
    field = dbshow (table, "HomeOwner");
    if (strcmp (field, spl[8])) {
      status = check_dups ((char**)dup_ssn, spl[1]);
      if (status)
	continue;
      if (!header) {
	printf ("\n\n***Warning: Record out of sync: HomeOwner");
	header = 1;
      }
      else {
	printf (" HomeOwner");
      }
    }
    field = dbshow (table, "DateApplied");
    if (strcmp (field, spl[9])) {
      status = check_dups ((char**)dup_ssn, spl[1]);
      if (status)
	continue;
      if (!header) {
	printf ("\n\n***Warning: Record out of sync: DateApplied");
	header = 1;
      }
      else {
	printf (" DateApplied");
      }
    }
    field = dbshow (table, "LastUpdated");
    if (strcmp (field, spl[10])) {
      status = check_dups ((char**)dup_ssn, spl[1]);
      if (status)
	continue;
      if (!header) {
	printf ("\n\n***Warning: Record out of sync: LastUpdated");
	header = 1;
      }
      else {
	printf (" LastUpdated");
      }
    }
    if (header) {
      printf (" at line %d\n", i);
    }
  }
  t2 = dbtime();
  printf ("\nVerified the table data in %6.4f seconds\n\n", t2-t1);

  dbexit ();
  print_block_list();
  return 0;
}
