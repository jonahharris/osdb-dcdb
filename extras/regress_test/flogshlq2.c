/*	Source File:	flogshlq.c	*/

/*
 * This program is written to do a fair comparison between the quick
 * sort and the shell sort as implemented in shell.c.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sort.h>
#include <test.h>


INLINE int testCompare (const void *p1, const void *p2)
{
  return (strcmp ((char *) p1, (char *) p2));
}

#define DATASIZE 40

int main (int argc, char *argv[])
{
  FILE *fp;
  char *data;
  char *ln, *cp;
  double t1, t2;
  shellHeader *shl;
  int numItems = 0;
  ListHeader *lh;
  Link *lnk;
  int isOK = TRUE;

  if (argc != 5) {
    printf ("\nUsage: %s num <file> <file.srt> <file.rev.srt>", argv[0]);
    printf ("\twhere num is the number of items to work with\n");
    printf ("\t  and <file> is the name of the file to read\n");
    return 1;
  }

  numItems = atoi (argv[1]);
  if (numItems == 0 || numItems == -1) {
    printf ("\n***Error: couldn't convert %s to an integer value\n", argv[1]);
    return _ERROR_;
  }
  printf ("\nUsing %d items\n", numItems);
  /*
   * Set up the list
   */
  lh = initList (UNSORTED, 0);
  if (lh == 0) {
    printf ("\n\n***Error: creating list, %s\n", ListErrorString[ListError]);
    return _ERROR_;
  }
  fp = fopen (argv[2], "r");
  if (fp == 0) {
    printf ("\n***Error: couldn't open %s\n", argv[2]);
    return _ERROR_;
  }
  data = malloc (512);
  if (0 == data) {
    printf ("\n***Error: memory exhausted\n");
    delList (lh);
    return _ERROR_;
  }
  check_pointer (data);
  while (!feof (fp)) {
    cp = fgets (data, 80, fp);
    if (0 == cp)
      break;
    cp = strchr (data, '\n');
    if (0 != cp)
      *cp = '\0';
    if (strlen (data) == 0)
      continue;
    ln = malloc (DATASIZE);
    if (0 == ln) {
      printf ("\n***Error: memory exhausted\n");
      /* let the OS unmalloc memory */
      return _ERROR_;
    }
    strncpy (ln, data, DATASIZE);
    ln[DATASIZE - 1] = '\0';
    check_pointer (ln);

    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      printf ("\n\n***Error: dynamic memory exhausted\n");
      return _ERROR_;
    }
    lnk->data = ln;

    lh->current = lh->tail->prev;
    insertLinkHere (lh, lnk);
  }
  fclose (fp);
  elapsed (&t1);
  shl = shlListQsort (lh, DATASIZE, testCompare);
  elapsed (&t2);
  if (shl == 0) {
    printf ("\n\n***Error: populating shell with shlQsort: %s\n",
	    shlErrorStr[shlError]);
    return _ERROR_;
  }
  printf ("\n\nTime to sort %d items: %f sec.\n", shl->lh->number, t2 - t1);

  /* do ordered and reverse ordered compares */
  fp = fopen (argv[3], "r");
  if (0 == fp) {
    printf ("\n\n***Error: couldn't open %s\n", argv[3]);
    return _ERROR_;
  }
  elapsed (&t1);
  lnk = shl->lh->head->next;
  while (!feof (fp) && lnk != 0) {
    cp = fgets (data, 80, fp);
    if (0 == cp)
      break;
    if (data[0] == '\0')
      break;
    cp = strchr (data, '\n');
    if (0 != cp)
      *cp = '\0';
    if (strlen (data) == 0)
      continue;
    if (strcmp (data, (char *) lnk->data)) {
      printf ("Bad compare: \"%s\" - \"%s\"\n", data, (char *) lnk->data);
      isOK = FALSE;
    }
    lnk = lnk->next;
  }

  if (!isOK) {
    printf ("\n***Problem with forward sort\n");
    return _ERROR_;
  }
  fclose (fp);
  fp = fopen (argv[4], "r");
  if (0 == fp) {
    printf ("\n\n***Error: couln't open %s\n", argv[4]);
    return _ERROR_;
  }
  lnk = shl->lh->tail->prev;
  while (!feof (fp) && lnk != 0) {
    cp = fgets (data, 80, fp);
    if (0 == cp)
      break;
    if (data[0] == '\0')
      break;
    cp = strchr (data, '\n');
    if (0 != cp)
      *cp = '\0';
    if (strlen (data) == 0)
      continue;
    if (strcmp (data, (char *) lnk->data)) {
      printf ("Bad compare: \"%s\" - \"%s\"\n", data, (char *) lnk->data);
      isOK = FALSE;
    }
    lnk = lnk->prev;
  }
  elapsed (&t2);

  if (!isOK) {
    printf ("\n***Problem with reverse compare\n");
    return _ERROR_;
  }
  fclose (fp);

  printf ("Compare time: %f\n", t2 - t1);

  /*
   * Force add a new item, just for giggles.
   */
  testString (data, DATASIZE - 1, 0);
  ln = malloc (DATASIZE);
  if (0 == ln) {
    printf ("\n***Error: memory exhausted\n");
    /* let the OS unmalloc memory */
    return _ERROR_;
  }
  strncpy (ln, data, DATASIZE);
  ln[DATASIZE - 1] = '\0';
  check_pointer (ln);

  lnk = malloc (sizeof (Link));
  if (0 == lnk) {
    printf ("\n\n***Error: dynamic memory exhausted\n");
    return _ERROR_;
  }
  lnk->data = ln;
  isOK = addShellItem (shl, lnk);
  if (isOK == _ERROR_) {
    if (shl->shlError == SHELL_LIST) {
      printf ("\n\n***Error: List Error, %s\n", ListErrorString[ListError]);
      free (data);
      delShell (shl, 0);
      return _ERROR_;
    }
    else {
      printf ("\n\n***Error: %s entering \"%s\"\n",
	      shlErrorStr[shl->shlError], data);
      free (data);
      delShell (shl, 0);
      return _ERROR_;
    }
  }


  free (data);
  delShell (shl, 0);
  print_block_list ();
  return _OK_;
}
