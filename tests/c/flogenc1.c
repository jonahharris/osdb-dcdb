/* Source File: flogenc1.c */

#ifdef __CINT__
#include "cint_types.h"
#endif

#ifndef __CINT__
#include <stdio.h>
#include <stdlib.h>
#include <interface.h>
#define MAXRECORDS 1000000
#endif

#define	SIZE 384
#define PASSWD "to be or not to be"

int main (void)
{
  double t1, t2;
  FILE *fr, *fw;
  ListHeader *lh;
  Link *lnk;
  char *ln;
  char line[1024];
  int i;
  char *field;
  char encr[2*SIZE+1];

  fr = fopen ("testAdd.txt", "rb");
  if (0 == fr) {
    printf ("\n\n***Error: could not open testAdd.txt to read\n");
    return _ERROR_;
  }
  fw = fopen ("encrypted.txt", "wb");
  if (0 == fw) {
    printf ("\n\n***Error: could not open encrypted.txt to write\n");
    fclose (fr);
    return _ERROR_;
  }
  lh = initList (UNSORTED, 0);
  if (lh == 0 ) {
    printf ("\n\n***Error: initList(): %s\n", ListErrorString[ListError]);
    return _ERROR_;
  }

  t1 = dbtime ();
  printf ("\nParsing testAdd.txt\n");
  for (i = 0; i < MAXRECORDS && !feof (fr); i++) {
    if (i % 1000 == 0 && i != 0) {
      printf (".");
      fflush (stdout);
    }
    memset (line, 1020, 0);
    if (fgets (line, 1000, fr) == 0)
      break;
    if (line[0] == '\n')
      continue;
    field = strchr (line, '\n');
    if (0 != field)
      *field = 0;

    ln = malloc (strlen (line) + 1);
    if (0 == ln) {
      printf ("\n\n***Error: fatal malloc error allocating a line, %d\n", i+1);
      fclose (fr);
      fclose (fw);
      return _ERROR_;
    }
    strcpy (ln, line);
    check_pointer (ln);
    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      printf ("\n\n***Error: fatal malloc error allocating link, %d\n", i+1);
      fclose (fw);
      fclose (fr);
      return _ERROR_;
    }
    lnk->data = ln;
    check_pointer (lnk);
    insertLink (lh, lnk);

    field = dbencrypt (line, SIZE, PASSWD);
    if (0 == field) {
      printf ("\n\n***Error: dbencrypt (%s, %d, %s): %s\n",
	  line, SIZE, PASSWD, cdberror);
      fclose (fw);
      fclose (fr);
      return _ERROR_;
    }
    strcpy (encr, field);
    fprintf (fw, "%s\n", encr);
  }
  t2 = dbtime ();
  fclose (fw);
  fclose (fr);
  printf ("\nEncrypted %d items in %6.4f seconds\n", i, t2-t1);
  printf ("Parsing encrypted.txt\n");
  fr = fopen ("encrypted.txt", "rb");
  if (0 == fr) {
    printf ("\n\n***Error: could not open encrypted.txt\n");
    return _ERROR_;
  }
  t1 = dbtime ();
  for (i = 0, lnk = lh->head->next;
      i < MAXRECORDS && !feof (fr) && lnk != lh->tail;
      i++, lnk = lnk->next) {
    if (i % 1000 == 0 && i != 0) {
      printf (".");
      fflush (stdout);
    }
    memset (line, 1020, 0);
    if (fgets (line, 1000, fr) == 0)
      break;
    if (line[0] == '\n')
      continue;
    field = strchr (line, '\n');
    if (0 != field)
      *field = 0;

    field = dbdecrypt (line, SIZE, PASSWD);
    if (field == 0) {
      printf ("\n\n***Error: dbdecrypt (%s, %d, %s): %s\n",
	  line, SIZE, PASSWD, cdberror);
      fclose (fr);
      return _ERROR_;
    }
    check_pointer (lnk->data);
    check_pointer (lnk);
    if ((char*) lnk->data != 0) {
      if (strcmp ((char*) lnk->data, field)) {
        printf ("\n***Warning: decrypted string at line %d doesn't match\n", i+1);
      }
    }
    else
      printf ("\n\n***Warning: item in list at line %d is NULL\n", i+1);
  }
  t2 = dbtime ();
  fclose (fr);
  printf ("\nUnencrypted %d items in %6.4f seconds\n", i, t2-t1);
  lnk = removeLink (lh);
  while (lnk != 0) {
    check_pointer (lnk->data);
    free (lnk->data);
    check_pointer (lnk);
    free (lnk);
    lnk = removeLink (lh);
  }
  delList (lh);
  print_block_list();
  return 0;
}
