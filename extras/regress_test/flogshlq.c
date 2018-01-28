/*	Source File:	flogshlq.c	*/

/*
 * This program is written to do a fair comparison between the quick
 * sort and the shell sort as implemented in shell.c.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sort.h>


INLINE int qtestCompare (const void **p1, const void **p2)
{
  return (strcmp ((char *) *p1, (char *) *p2));
}

INLINE int stestCompare (void *p1, void *p2)
{
  return (strcmp ((char *) p1, (char *) p2));
}

#define DATASIZE 40

int main (int argc, char *argv[])
{
  FILE *fp;
  char *data;
  char *ln, *cp;
  char **qsortArray;
  double t1, t2;
  shellHeader *shl;
  int numItems = 0;
  register int counter;

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
   * Set up the arrays
   */
  qsortArray = malloc (numItems * sizeof (char *));
  if (0 == qsortArray) {
    printf ("\n***Error: memory exhausted\n");
    return _ERROR_;
  }
  memset (qsortArray, 0, numItems * sizeof (char *));
  check_pointer (qsortArray);

  /*
   * File 1 - the unsorted file
   */
  fp = fopen (argv[2], "r");
  if (fp == 0) {
    printf ("\n***Error: couldn't open %s\n", argv[2]);
    return _ERROR_;
  }
  data = malloc (512);
  if (0 == data) {
    printf ("\n***Error: memory exhausted\n");
    free (qsortArray);
    return _ERROR_;
  }
  check_pointer (data);
  counter = 0;
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
    qsortArray[counter] = ln;

    if (++counter >= numItems)
      break;
  }
  fclose (fp);
  elapsed (&t1);
  shl = shlQsort (qsortArray, numItems, sizeof (char *),
		  (int (*)(const void *, const void *)) qtestCompare,
		  stestCompare);
  elapsed (&t2);
  if (shl == 0) {
    printf ("\n\n***Error: populating shell with shlQsort: %s\n",
	    shlErrorStr[shlError]);
    return _ERROR_;
  }
  printf ("\n\nTime to sort %d items: %f sec.\n", shl->lh->number, t2 - t1);

  delShell (shl, 0);
  free (qsortArray);
  return _OK_;
}
