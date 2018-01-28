/*	Source File:	flogrmshl.c	*/

#include <sort.h>
#include <test.h>

#include <stdio.h>

typedef struct _data_struct {
  int count;
  char str[50];
} dataStruct;

INLINE int testCompare (void *p1, void *p2)
{
  if (((dataStruct *) p1)->count == ((dataStruct *) p2)->count)
    return 0;
  if (((dataStruct *) p1)->count > ((dataStruct *) p2)->count)
    return 1;
  return -1;
}

int main (void)
{
  int lowCount = 0, highCount = 100000;
  int status, i, j;
  shellHeader *uniq, *dups;
  Link *lnk, *delLink, *tmplnk;
  dataStruct *data;
  double t1, t2;

  uniq = initShell (testCompare, TRUE, TRUE);
  if (0 == uniq) {
    printf ("\n\n***Error: initializing uniq, %s\n", shlErrorStr[shlError]);
    return _ERROR_;
  }
  dups = initShell (testCompare, FALSE, TRUE);
  if (0 == dups) {
    printf ("\n\n***Error: initializing dups, %s\n", shlErrorStr[shlError]);
    return _ERROR_;
  }
  elapsed (&t1);
  for (i = 0; i < 1000; i++) {
    for (j = 0; j < 5; j++) {
      /*
       * dups
       */
      data = malloc (sizeof (dataStruct));
      if (0 == data) {
	printf ("\n\n***Error: memory exhausted\n");
	delShell (uniq, 0);
	delShell (dups, 0);
	return _ERROR_;
      }
      lnk = malloc (sizeof (Link));
      if (0 == lnk) {
	printf ("\n\n***Error: memory exhausted\n");
	free (data);
	delShell (uniq, 0);
	delShell (dups, 0);
	return _ERROR_;
      }
      lnk->data = data;
      data->count = lowCount + i;
      testString (data->str, 0, 45);
      status = addShellItem (dups, lnk);
      if (status == _ERROR_) {
	printf ("\n\n***Error: adding to dups, %s\n", shlErrorStr[shlError]);
	delShell (uniq, 0);
	delShell (dups, 0);
	return _ERROR_;
      }
      data = malloc (sizeof (dataStruct));
      if (0 == data) {
	printf ("\n\n***Error: memory exhausted\n");
	delShell (uniq, 0);
	delShell (dups, 0);
	return _ERROR_;
      }
      lnk = malloc (sizeof (Link));
      if (0 == lnk) {
	printf ("\n\n***Error: memory exhausted\n");
	free (data);
	delShell (uniq, 0);
	delShell (dups, 0);
	return _ERROR_;
      }
      lnk->data = data;
      data->count = highCount - i;
      testString (data->str, 0, 45);
      status = addShellItem (dups, lnk);
      if (status == _ERROR_) {
	printf ("\n\n***Error: adding to uniq, %s\n", shlErrorStr[shlError]);
	delShell (uniq, 0);
	delShell (dups, 0);
	return _ERROR_;
      }

      /*
       * uniq
       */
      data = malloc (sizeof (dataStruct));
      if (0 == data) {
	printf ("\n\n***Error: memory exhausted\n");
	delShell (uniq, 0);
	delShell (dups, 0);
	return _ERROR_;
      }
      lnk = malloc (sizeof (Link));
      if (0 == lnk) {
	printf ("\n\n***Error: memory exhausted\n");
	free (data);
	delShell (uniq, 0);
	delShell (dups, 0);
	return _ERROR_;
      }
      lnk->data = data;
      data->count = lowCount + j;
      testString (data->str, 0, 45);
      status = addShellItem (uniq, lnk);
      if (status == _ERROR_) {
	printf ("\n\n***Error: adding to uniq, %s\n", shlErrorStr[shlError]);
	delShell (uniq, 0);
	delShell (dups, 0);
	return _ERROR_;
      }
      data = malloc (sizeof (dataStruct));
      if (0 == data) {
	printf ("\n\n***Error: memory exhausted\n");
	delShell (uniq, 0);
	delShell (dups, 0);
	return _ERROR_;
      }
      lnk = malloc (sizeof (Link));
      if (0 == lnk) {
	printf ("\n\n***Error: memory exhausted\n");
	free (data);
	delShell (uniq, 0);
	delShell (dups, 0);
	return _ERROR_;
      }
      lnk->data = data;
      data->count = highCount - j;
      testString (data->str, 0, 45);
      status = addShellItem (uniq, lnk);
      if (status == _ERROR_) {
	printf ("\n\n***Error: adding to uniq, %s\n", shlErrorStr[shlError]);
	delShell (uniq, 0);
	delShell (dups, 0);
	return _ERROR_;
      }
    }
    lowCount += 5;
    highCount -= 5;
  }
  elapsed (&t2);

  printf ("\nStats after populating shells:");
  printf ("\nseconds to populate shells: %f", t2 - t1);
  printf ("\nuniq: %d items, %d nodes", uniq->lh->number, uniq->numNodes);
  printf ("\ndups: %d items, %d nodes\n", dups->lh->number, dups->numNodes);

  /*
   * Now, they are populated; let's do some deletions.
   */
  elapsed (&t1);
  lnk = uniq->lh->head->next;
  j = uniq->lh->number;
  j -= 20;
  for (i = 0; i < j; i++) {
    if (lnk == uniq->lh->tail)
      goto doneUniq;
    tmplnk = lnk->next;
    delLink = removeShellItem (uniq, lnk);
    if (delLink != 0) {
      data = delLink->data;
      free (data);
      free (delLink);
    }
    lnk = tmplnk;
    if (lnk == uniq->lh->tail)
      goto doneUniq;
  }

doneUniq:

  lnk = dups->lh->head->next;
  j = dups->lh->number;
  j -= 20;
  for (i = 0; i < j; i++) {
    if (lnk == dups->lh->tail)
      goto doneDups;
    tmplnk = lnk->next;
    delLink = removeShellItem (dups, lnk);
    if (delLink != 0) {
      data = delLink->data;
      free (data);
      free (delLink);
    }
    lnk = tmplnk;
    if (lnk == dups->lh->tail)
      goto doneDups;
  }
  elapsed (&t2);

doneDups:

  printf ("\n\nStats after deletions:");
  printf ("\nseconds for deletions: %f", t2 - t1);
  printf ("\nuniq: %d items, %d nodes\n", uniq->lh->number, uniq->numNodes);
  printf ("dups: %d items, %d nodes\n\n", dups->lh->number, dups->numNodes);

  delShell (uniq, 0);
  delShell (dups, 0);

  return _OK_;
}
