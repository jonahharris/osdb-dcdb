/* Source File: floghash1.c */

/*
 * Put the hash code through the ringer.
 */

#ifdef __CINT__
#include "cint_types.h"
#endif

#ifndef __CINT__
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sort.h>
#endif

static __inline__ int
testCompare (void *p1, void *p2)
{
  return (strcmp ((char *) p1, (char *) p2));
}

#define MAXDATA		1000000

int
main (int argc, char *argv[])
{
  FILE *fp;
  struct stat statbuf;
  char *cp;
  char *data;
  char **cpp;
  Link *lnk;
  ListHeader *lh;
  int status;
  double t1, t2;
  hashHeader *hsh;
  int i, num;
  char *tmp;
  int counter;
  int min = MAXDATA,
      max = 0;

  if (argc != 2) {
    printf ("\nUsage: %s <file> <srch>\n", argv[0]);
    printf ("\t<file> is the name of the file to process.\n");
    return 1;
  }

  Assert (argv[1] != NULL && argv[1][0] != '\0');

  /*printf ("\nSetting up data structures...");*/
  hsh = initHash (testCompare, 0);
  if (0 == hsh) {
    printf ("\n\n***Error: creating hash: %s\n", shlErrorStr[shlError]);
    return _ERROR_;
  }
  fp = fopen (argv[1], "r");
  if (fp == NULL) {
    printf ("\n***Error: couldn't open %s\n", argv[1]);
    return _ERROR_;
  }
  stat (argv[1], &statbuf);
  cp = malloc ((size_t)statbuf.st_size+1);
  if (0 == cp) {
    printf ("\n\n***Error: critical memory error allocating file buffer\n");
    return _ERROR_;
  }
  fread (cp, (size_t)statbuf.st_size, 1, fp);
  check_pointer (cp);
  cpp = split (cp, '\n');
  fclose (fp);

  lh = initList (UNSORTED, 0);
  if (0 == lh) {
    printf ("\n\n***Error: initList (UNSORTED, 0): %s\n",
	ListErrorString[ListError]);
    free (cp);
    delHash (hsh);
    return _ERROR_;
  }

  elapsed (&t1);
  num = 0;
  for (i = 0; cpp[i] && i < MAXDATA; i++) {
    if (strlen (cpp[i]) == 0)
      continue;
    status = addHashItem (hsh, cpp[i]);
    if (status == _ERROR_) {
      if (shlError == SHELL_UNIQUE) {
	set_shlError(0, SHELL_NOERR);
	continue;
      }
      else {
	printf ("\n\n***Error: addHashItem (hsh, %s): ", cpp[i]);
	delHash (hsh);
	free (cp);
	return _ERROR_;
      }
    }
    if (hsh->numCompares > num)
      num = hsh->numCompares;
    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      printf ("\n\n***Error: allocating a link\n");
      delHash (hsh);
      free (cp);
      return _ERROR_;
    }
    data = malloc (strlen (cpp[i])+1);
    if (0 == data) {
      printf ("\n\n***Error: allocating a string\n");
      free (lnk);
      delHash (hsh);
      free (cp);
      return _ERROR_;
    }
    strcpy (data, cpp[i]);
    check_pointer (data);
    lnk->data = data;
    check_pointer (lnk);
    insertLink (lh, lnk);
  }
  elapsed (&t2);
  free (cp);

  printf ("\nAdd time: %f\n", t2 - t1);
  printf ("Succeeded...%d items stored in buckets\n", hsh->number);
  printf ("Max compares on adds: %d\n", num);

  /*
   * Open test file and search for each item in the hash
   */
  for (i = 0; i < HASH_SIZE; i++) {
    status = restructureShellNodes (hsh->shls[i]);
    if (_ERROR_ == status) {
      printf ("\n\n***Error: restructureShellNodes(): %s\n", shlErrorStr[shlError]);
      return _ERROR_;
    }
    num = hsh->shls[i]->lh->number;
    if (num < min)
      min = num;
    if (num > max)
      max = num;
  }
  printf ("There are %d shells, min num = %d, max num = %d\n",
      HASH_SIZE, min, max);
  num = 0;
  elapsed (&t1);
  lnk = lh->head->next;
  while (lnk != lh->tail) {
    data = findHashItem (hsh, (const char *)lnk->data);
    if (hsh->numCompares > num)
      num = hsh->numCompares;
    if (0 == data)
      printf ("Dind't find %s\n", (char *)lnk->data);
    lnk = lnk->next;
  }
  elapsed (&t2);

  printf ("Search Time: %f\n", t2 - t1);
  printf ("Max number of compares: %d\n", num);

  /*
   * OK, now look for bogus data.
   */
  printf ("Now searching for bogus data\n");
  tmp = malloc (100);
  if (0 == tmp) {
    printf ("\n\n***Error: fatal error allocating tmp buffer (100 bytes)\n");
    lnk = removeLink (lh);
    while (lnk != 0) {
      free (lnk->data);
      free (lnk);
      lnk = removeLink (lh);
    }
    delList (lh);
    delHash (hsh);
  }
  counter = 0;
  num = 0;
  elapsed (&t1);
  lnk = lh->tail->prev;
  while (lnk != lh->head) {
    strncpy (tmp, (char *)lnk->data, 3);
    tmp[3] = '\0';
    strcat (tmp, "ThisIsBogus");
    data = findHashItem (hsh, (const char *)tmp);
    if (hsh->numCompares > num)
      num = hsh->numCompares;
    if (data != 0)
      printf ("Actually found bogus data %s\n", tmp);
    counter ++;
    if (counter % 5000 == 0) {
      printf (".");
      fflush (stdout);
    }
    lnk = lnk->prev;
  }
  printf ("\n");
  elapsed (&t2);

  printf ("Search Time for %d bogus data items: %f\n", counter, t2 - t1);
  printf ("Maximum number of compares: %d\n", num);

  free (tmp);
  lnk = removeLink (lh);
  while (lnk != 0) {
    free (lnk->data);
    free (lnk);
    lnk = removeLink (lh);
  }
  delList (lh);
  delHash (hsh);
  print_block_list ();
  return 0;
}
