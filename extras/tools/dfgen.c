/* Source File: dfgen.c */

#include <stdio.h>
#include <stdlib.h>
#include <cdb.h>

int numFields;

int main (int argc, char *argv[])
{
  dbTable *tbl;
  int isIndexed = FALSE;
  char midxTmp[TABLE_INFO_WIDTH*2+1];
  char *midxnm, *midxbs, *midxcase, *midxfld;
  char *cp;
  int i;

  if (argc != 2) {
    printf (
        "Usage: dfgen <file>\n"
        "  Where <file> is the database (.db) file to create a .df from.\n\n"
        "Use this program first to create the .df file, then use dbdump to\n"
        "dump the data.  Then, you can archive the two files to recreate the\n"
        "table data if necessary.\n\n");
    return 1;
  }
  tbl = openTable (argv[1]);
  if (0 == tbl) {
    printf ("\n\nCouldn't open %s: %s\n", argv[1], dberror());
    return 1;
  }
  /*
   * OK, build the table definition information.
   */
  printf ("// %s\n\n", tbl->fileName);
  printf (
      "//\n"
      "// Save this to a .df file and do the following:\n"
      "//\n"
      "// tclsh8.x  # Whatever your version of tcl - must be 8.x\n"
      "// load ~/lib/libcdbtcl*.so # Whatever your libcdbtcl.so is called\n"
      "// dbcreate file.df # Replace file with whatever you called your .df file\n"
      "//\n");
  printf ("\n");
  printf ("create table \"%s\"\n", tbl->fileName);
  printf ("  info \"%s\"\n", tbl->hdr->tableInfo);
  printf ("{\n");
  numFields = tbl->hdr->numFields;
  for (i = 0; i < numFields; i++) {
    if (tbl->fldAry[i]->indexed != ITYPE_NOINDEX)
      isIndexed = TRUE;
    printf ("  \"%s\" ", tbl->fldAry[i]->fieldName);
    switch (tbl->fldAry[i]->ftype) {
      case FTYPE_CHAR:
        printf ("char (%d);", tbl->fldAry[i]->fieldLength);
        break;
      case FTYPE_NUMBER:
        printf ("number (%d:%d);", tbl->fldAry[i]->fieldLength,
            tbl->fldAry[i]->decLength);
        break;
      case FTYPE_LOGICAL:
        printf ("logical;");
        break;
      case FTYPE_DATE:
        printf ("date;");
        break;
      case FTYPE_TIME:
        printf ("time;");
        break;
      case FTYPE_NONE:
      case FTYPE_LAST:
        break;
    }
    printf ("\n");
  }
  if (isIndexed == 0 && tbl->hdr->midxInfo[0][0] != '\0')
    isIndexed = TRUE;
  if (isIndexed) {
    printf ("} indexed {\n");
    /* First, handle the regular indexes. */
    for (i = 0; i < numFields; i++) {
      if (tbl->fldAry[i]->indexed == ITYPE_NOINDEX)
        continue;
      printf ("  idx \"%s\" %d:", tbl->fldAry[i]->indexName,
          tbl->fldAry[i]->indexBlkSize);
      switch (tbl->fldAry[i]->indexed) {
        case ITYPE_UNIQUECASE:
          printf ("case:unique");
          break;
        case ITYPE_UNIQUENOCASE:
          printf ("nocase:unique");
          break;
        case ITYPE_DUPCASE:
          printf ("case:dup");
          break;
        case ITYPE_DUPNOCASE:
          printf ("nocase:dup");
          break;
        case ITYPE_NOINDEX:
        case ITYPE_LAST:
          break;
      }
      printf (" \"%s\";\n", tbl->fldAry[i]->fieldName);
    }
    /* OK, now multi-indexes */
    for (i = 0; tbl->hdr->midxInfo[i][0] != '\0'; i++) {
      strcpy (midxTmp, tbl->hdr->midxInfo[i]);
      midxnm = midxTmp;
      cp = strchr (midxTmp, ':');
      if (cp != 0)
        *cp = '\0';
      midxbs = cp+1;
      cp = strchr (midxbs, ':');
      if (cp != 0)
        *cp = '\0';
      midxcase = cp+1;
      cp = strchr (midxcase, ':');
      if (cp != 0)
        *cp = '\0';
      midxfld = cp+1;
      printf ("  midx \"%s\" %s:%s ", midxnm, midxbs, midxcase);
      cp = strchr (midxfld, ',');
      while (cp != 0) {
        *cp = '\0';
        printf ("\"%s\", ", midxfld);
        midxfld = cp+1;
        cp = strchr (midxfld, ',');
      }
      printf ("\"%s\";\n", midxfld);
    }
    printf ("};\n\n");
  }

  /* We're done...close the door on the way out. */
  closeTable (tbl);
  return 0;
}
