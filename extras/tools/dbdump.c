/* Source File: dbdump.c */

#include <stdio.h>
#include <stdlib.h>
#include <cdb.h>

int numFields;

int main (int argc, char *argv[])
{
  dbTable *tbl;
  int i;

  if (argc != 2) {
    printf (
        "Usage: dbdump <file>\n"
        "  Where <file> is the database (.db) file to dump.\n"
        "\nUse dfgen first to create a .df file for the table and then\n"
        "use this program to generate the contents.  Then you can archive\n"
        "both of those for later if you need to rebuild the table.\n\n");
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
  numFields = tbl->hdr->numFields;
  /*
   * OK, print the data in recreatable form.
   */
  printf ("#\n#load libcdbtcl*.so\n");
  printf ("# and execute this under tcl8.x.\n\n");
  printf ("set db [dbopen %s]\n\n", tbl->fileName);

  headRecord (tbl);
  while (!tbl->eof) {
    retrieveRecord (tbl);
    for (i = 0; i < numFields; i++) {
      switch (tbl->fldAry[i]->ftype) {
        case FTYPE_CHAR:
          printf ("dbsetchar $db \"%s\" \"%s\"\n", tbl->fldAry[i]->fieldName,
              tbl->fields[i]);
          break;
        case FTYPE_NUMBER:
          printf ("dbsetnum $db \"%s\" \"%s\"\n", tbl->fldAry[i]->fieldName,
              tbl->fields[i]);
          break;
        case FTYPE_LOGICAL:
          printf ("dbsetlog $db \"%s\" \"%s\"\n", tbl->fldAry[i]->fieldName,
              tbl->fields[i]);
          break;
        case FTYPE_DATE:
          printf ("dbsetdate $db \"%s\" \"%s\"\n", tbl->fldAry[i]->fieldName,
              tbl->fields[i]);
          break;
        case FTYPE_TIME:
          printf ("dbsettime $db \"%s\" \"%s\"\n", tbl->fldAry[i]->fieldName,
              tbl->fields[i]);
          break;
        case FTYPE_NONE:
        case FTYPE_LAST:
          break;
      }
    }
    printf ("\ndbadd $db\n\n");
    nextRecord (tbl);
  }
  printf ("dbclose $db\n\n");
      

  closeTable (tbl);
  return 0;
}
