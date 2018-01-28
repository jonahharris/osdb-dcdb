/* Source File: flograwadd1.c */

#include <cdb.h>

#define MAXRECORDS 1000000

char *showField (dbTable *table, char *arg)
{
  int i;
  static char rslt[1024];

  i = getFieldNum (table, arg);
  if (i == _ERROR_) {
    printf ("\n\n***Error: getFieldNum (flograwadd1.db, %s): %s\n",
	arg, dbtblerror(table));
    return 0;
  }
  sprintf (rslt, "%s", table->fields[i]);
  return rslt;
}

int main (void)
{
  FILE *fp;
  int status, i, numrecs = 0;
  dbTable *table;
  char line [1024]; /* arbitrarily long */
  char *field;
  char **spl;
  static char *ssn[MAXRECORDS];
  double salary;
  double t1, t2;

  /*
   * First, remove old .db & co. files.
   */
  if (fexists ("flograwadd1.db")) {
    fileRemove ("flograwadd1.db");
    fileRemove ("flograwadd1.db.LCK");
    fileRemove ("f1r_ssnidx.idx");
    fileRemove ("f1r_ssnidx.inx");
    fileRemove ("f1r_lnidx.idx");
    fileRemove ("f1r_lnidx.inx");
  }
  fp = fopen ("flogadd1.df", "wb");
  if (0 == fp) {
    printf ("\n\n***Error: could not open flogadd1.df to write\n");
    return _ERROR_;
  }
  fprintf (fp, 
      "// flograwadd1.df\n\n"
      "create table \"flograwadd1.db\"\n"
      "  info \"Table for flograwadd1.c\"\n"
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
      "  idx \"f1r_ssnidx\" 128:case:unique \"SSNumber\";\n"
      "  idx \"f1r_lnidx\" 512:case:dup \"LName\";\n"
      "};\n\n"
      );
  fclose (fp);

  status = parseDBDef ("flograwadd1.df");
  if (isDBError()) {
    printf ("\n\n***Error: Could not parseDBDef flogadd1.df: %s\n", dberror());
    return _ERROR_;
  }
  status = remove ("flograwadd1.df");
  if (status == _ERROR_) {
    printf ("\n\n***Warning: could not remove flograwadd1.df\n");
  }

  table = openTable ("flograwadd1.db");
  if (isDBError()) {
    printf ("\n\n***Error: Could not open flograwadd1.db: %s\n", dberror());
    return _ERROR_;
  }

  /*
   * OK, populate the table.
   */
  elapsed (&t1);
  fp = fopen ("testAdd.txt", "rb");
  if (0 == fp) {
    printf ("\n\n***Error: could not open testAdd.txt to read\n");
    closeTable(table);
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
    setCharField (table, "LName", spl[0]);
    if (isTableError(table)) {
      printf ("\n\n***Error: setCharField %s LName %s: %s\n",
         "flograwadd1.db", spl[0], dbtblerror(table));
      closeTable (table);
      return _ERROR_;
    }
    setCharField (table, "SSNumber", spl[1]);
    if (isTableError(table)) {
      printf ("\n\n***Error: setCharField %s SSNumber %s: %s\n",
          "flograwadd1.db", spl[1], dbtblerror(table));
      closeTable(table);
      return _ERROR_;
    }
    ssn[i] = malloc (strlen (spl[1]) + 1);
    if (0 == ssn[i]) {
      printf ("\n\n***Error: memory exhausted\n");
      closeTable (table);
      return _ERROR_;
    }
    strcpy (ssn[i], spl[1]);
    check_pointer (ssn[i]);
    setCharField (table, "FName", spl[2]);
    if (isTableError(table)) {
      printf ("\n\n***Error: setCharField %s FName, %s: %s\n",
	  "flograwadd1.db", spl[2], dbtblerror(table));
      closeTable (table);
      return _ERROR_;
    }
    setCharField (table, "StreetAddress", spl[3]);
    if (isTableError(table)) {
      printf ("\n\n***Error: setCharField %s StreetAddress, %s: %s\n",
	  "flograwadd1.db", spl[3], dbtblerror(table));
      closeTable (table);
      return _ERROR_;
    }
    setCharField (table, "City", spl[4]);
    if (isTableError(table)) {
      printf ("\n\n***Error: setCharField %s City, %s: %s\n",
	  "flograwadd1.db", spl[4], dbtblerror(table));
      closeTable (table);
      return _ERROR_;
    }
    setCharField (table, "State", spl[5]);
    if (isTableError(table)) {
      printf ("\n\n***Error: setCharField %s State, %s: %s\n",
	  "flograwadd1.db", spl[5], dbtblerror(table));
      closeTable (table);
      return _ERROR_;
    }
    setCharField (table, "Zip", spl[6]);
    if (isTableError(table)) {
      printf ("\n\n***Error: setCharField %s Zip, %s: %s\n",
	  "flograwadd1.db", spl[6], dbtblerror(table));
      closeTable(table);
      return _ERROR_;
    }
    salary = atof (spl[7]);
    if (salary < 0.00 || salary > 99999.99)
      salary = 95000.00; /* decent salary :o) */
    setNumberField (table, "AnnualSalary", salary);
    if (isTableError(table)) {
      printf ("\n\n***Error: setNumberField %s AnnualSalary, %s: %s\n",
	  "flograwadd1.db", spl[7], dbtblerror(table));
      closeTable(table);
      return _ERROR_;
    }
    setLogicalField (table, "HomeOwner", *spl[8]);
    if (isTableError(table)) {
      printf ("\n\n***Error: setLogicalField %s HomeOwner, %s: %s\n",
	  "flograwadd1.db", spl[8], dbtblerror(table));
      closeTable(table);
      return _ERROR_;
    }
    setDateField (table, "DateApplied", spl[9]);
    if (isTableError(table)) {
      printf ("\n\n***Error: setDateField %s DateApplied, %s: %s\n",
	  "flograwadd1.db", spl[9], dbtblerror(table));
      closeTable(table);
      return _ERROR_;
    }
    setDateField (table, "LastUpdated", spl[10]);
    if (isTableError(table)) {
      printf ("\n\n***Error: setDateField (%s, LastUpdated, %s): %s\n",
	  "flograwadd1.db", spl[10], dbtblerror(table));
      closeTable(table);
      return _ERROR_;
    }

    /*
     * Add it.
     */
    status = addRecord (table);
    if (isTableError(table)) {
      if (table->dbError == DB_UNIQUE) {
	printf ("\n\n***Warning: Duplicate SSN at line %d\n", i);
	(void)dbtblerror(table); /* clear the error */
	continue;
      }
      printf ("\n\n***Error: calling addRecord: %s\n", dbtblerror(table));
      closeTable (table);
      return _ERROR_;
    }
  }
  fclose (fp);
  numrecs = table->hdr->numRecords;
  elapsed (&t2);
  printf ("\n\nFinished adding %d records to %s in %6.4f seconds\n",
      numrecs, "flograwadd1.db", t2-t1);

  fp = fopen ("flogadd1.output", "wb");
  if (0 == fp) {
    printf ("\n\n***Error: Could not open flogadd1.output\n");
    closeTable (table);
    return _ERROR_;
  }
  elapsed (&t1);
  for (i = 0; i < MAXRECORDS && ssn[i] != 0; i++) {
    status = searchExactIndexRecord (table, ssn[i]);
    if (isTableError(table)) {
      printf ("\n\n***Error: searchExactIndexRecord (%s, %s)\n",
	  "flograwadd1.db", ssn[i]);
      closeTable(table);
      return _ERROR_;
    }
    if (status == _NOTFOUND_) {
      printf ("\n\n***Warning: didn't find %s\n", ssn[i]);
    }
    free (ssn[i]);
    /*
     * OK, retrieve the data and make some effort getting the data to the program.
     */
    status = retrieveRecord (table);
    if (isTableError(table)) {
      printf ("\n\n***Error: retrieveRecord (%s)\n",table->fileName); 
      closeTable(table);
      return _ERROR_;
    }
    field = showField (table, "LName");
    if (field == 0) {
      printf ("\n\n***Error: showField (%s, LName): %s\n",
	  table->fileName, dbtblerror(table));
      closeTable (table);
      return _ERROR_;
    }
    fprintf (fp, "%s:", field);
    field = showField (table, "SSNumber");
    if (field == 0) {
      printf ("\n\n***Error: showField (%s, SSNumber): %s\n",
	  table->fileName, dbtblerror(table));
      closeTable(table);
      return _ERROR_;
    }
    fprintf (fp, "%s:", field);
    field = showField (table, "FName");
    if (field == 0) {
      printf ("\n\n***Error: showField (%s, FName): %s\n",
	  table->fileName, dbtblerror(table));
      closeTable(table);
      return _ERROR_;
    }
    fprintf (fp, "%s:", field);
    field = showField (table, "StreetAddress");
    if (field == 0) {
      printf ("\n\n***Error: showField (%s, StreetAddress): %s\n",
	  table->fileName, dbtblerror(table));
      closeTable (table);
      return _ERROR_;
    }
    fprintf (fp, "%s:", field);
    field = showField (table, "City");
    if (field == 0) {
      printf ("\n\n***Error: showField (%s, City): %s\n",
	  table->fileName, dbtblerror(table));
      closeTable(table);
      return _ERROR_;
    }
    fprintf (fp, "%s:", field);
    field = showField (table, "State");
    if (field == 0) {
      printf ("\n\n***Error: showField (%s, State): %s\n",
	  table->fileName, dbtblerror(table));
      closeTable(table);
      return _ERROR_;
    }
    fprintf (fp, "%s:", field);
    field = showField (table, "Zip");
    if (field == 0) {
      printf ("\n\n***Error: showField (%s, LName): %s\n",
	  table->fileName, dbtblerror(table));
      closeTable(table);
      return _ERROR_;
    }
    fprintf (fp, "%s:", field);
    field = showField (table, "AnnualSalary");
    if (field == 0) {
      printf ("\n\n***Error: showField (%s, AnnualSalary): %s\n",
	  table->fileName, dbtblerror(table));
      closeTable(table);
      return _ERROR_;
    }
    fprintf (fp, "%s:", field);
    field = showField (table, "HomeOwner");
    if (field == 0) {
      printf ("\n\n***Error: showField (%s, HomeOwner): %s\n",
	  table->fileName, dbtblerror(table));
      closeTable(table);
      return _ERROR_;
    }
    fprintf (fp, "%s:", field);
    field = showField (table, "DateApplied");
    if (field == 0) {
      printf ("\n\n***Error: showField (%s, DateApplied): %s\n",
	  table->fileName, dbtblerror(table));
      closeTable(table);
      return _ERROR_;
    }
    fprintf (fp, "%s:", field);
    field = showField (table, "LastUpdated");
    if (field == 0) {
      printf ("\n\n***Error: showField (%s, LastUpdated): %s\n",
	  table->fileName, dbtblerror(table));
      closeTable(table);
      return _ERROR_;
    }
    fprintf (fp, "%s\n", field);
  }
  elapsed (&t2);
  fclose (fp);
  printf ("\n\nFinished searching %d records to %s in %6.4f seconds\n",
      numrecs, table->fileName, t2-t1);

  /* Done */
  closeTable(table);
  print_block_list();
  return _OK_;
}
