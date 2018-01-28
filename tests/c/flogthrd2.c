/* Source File: flogthrd1.c */

#include <sort.h>

#ifdef __CINT__
#include "sort.c"
#include "qsort.c"
#endif /* __CINT__ */

#define NUM_THREAD 4
#define MAX_DATA 1000000

typedef struct _thread_data {
  shellHeader *shl;              /* shell to store stuff */
  int threadnum;                 /* number of this thread */
  int number;                    /* number of items in cpp */
  int status;                    /* status of this thread */
  char **cpp;                    /* array of strings to sort */
  Link **lpp;                    /* pre-allocated array of links */
  Link *lnks;                    /* pre-allocated links */
} threadData;

int testCompare (void *p1, void *p2)
{
  return (strcmp ((char *) p1, (char *) p2));
}

//
// Populate a shell with the contents of the file given by file
// and return a pointer to the populated shell.
//
void *populateShell (void *tt)
{
  int i;
  threadData *t;
  int status;

  t = (threadData *)tt;
  for (i = 0; i < t->number; i++) {
    if (t->lpp[i]->data == 0)
      break;
    status = addShellItem (t->shl, t->lpp[i]);
    if (status == _ERROR_) {
      if (t->shl->shlError == SHELL_UNIQUE) {
	set_shlError (t->shl, SHELL_NOERR);
        continue;
      }
      t->status = _ERROR_;
      return (void*)0;
    }
  }
  pthread_exit ((void*)0);

  return (void*)0;
}

//
// OK, so we will open the file, read in all the items, split them up among
// the threadData items and then create the threads and pass on a threadData
// to each one.  The shells will get populated by the populateShell function
// and will be in the threadData items.  Then, I will need to merge those
// back together into single shells.  Hopefully, all of that will be quicker
// than just populating a single shell of that many items.
//
int main (int argc, char *argv[])
{
  threadData *tt[NUM_THREAD];
  void *chk_lnks[NUM_THREAD];
  pthread_t th[NUM_THREAD];
  pthread_attr_t attr;
  char *cp;
  char **cpp;
  double t0, t1, t2;
  int status;
  int i, j;
  int printUsage = FALSE;
  FILE *fp;
  struct stat sbuf;
  int totalcount, itemcount, thnum;
  int count[NUM_THREAD];
  ListHeader *lhs[NUM_THREAD];
  ListHeader *final_lh, *tmp_lh;
  shellHeader *final;
  Link *lnk;

  if (argc != 2)
    printUsage = TRUE;
  if (argv[1] == 0 || argv[1][0] == '\0')
    printUsage = TRUE;
  if (printUsage == TRUE) {
    printf ("\n\nUsage: %s <file>\n", argv[0]);
    printf ("\twhere <file> is the file to sort\n");
    return 1;
  }
  elapsed (&t0);
  //
  // Grab the contents of the file and parse it out by lines.
  //
  fp = fopen (argv[1], "r");
  if (fp == NULL) {
    printf ("\n\n***Error: couldn't open %s\n", argv[1]);
    return _ERROR_;
  }
  stat (argv[1], &sbuf);
  cp = malloc ((size_t)sbuf.st_size+1);
  if (0 == cp) {
    printf ("\n\n***Error: critical memory error allocating file buffer\n");
    fclose (fp);
    return _ERROR_;
  }
  fread (cp, (size_t)sbuf.st_size, 1, fp);
  check_pointer (cp);
  fclose (fp);
  cpp = malloc (MAX_DATA * sizeof (char *));
  if (0 == cpp) {
    printf ("\n\n***Error: critical memory error allocating char array\n");
    free (cp);
    return _ERROR_;
  }
  memset (cpp, 0, MAX_DATA*sizeof(char*));
  check_pointer (cpp);
  totalcount = split_r (cpp, MAX_DATA, cp, '\n');
  itemcount = totalcount/NUM_THREAD;
  printf ("\nProcessing %d items (%d per thread)\n", totalcount, itemcount);

  //
  // Allocate the data slots and populate them as much as possible.
  //
  for (i = 0; i < NUM_THREAD; i++) {
    tt[i] = malloc (sizeof (threadData));
    if (0 == tt[i]) {
      printf ("\n\n***Error: critical memory error in main\n");
      return _ERROR_;
    }
    memset (tt[i], 0, sizeof(threadData));
    tt[i]->threadnum = i;
    tt[i]->number = itemcount;
    tt[i]->shl = initShell (testCompare, FALSE, FALSE);
    if (tt[i]->shl == 0) {
      printf ("\n\n***Error: initShell: %s\n", shlErrorStr[shlError]);
      free (tt[i]);
      free (cpp);
      free (cp);
      return _ERROR_;
    }
    tt[i]->cpp = malloc (itemcount * sizeof (char*));
    if (tt[i]->cpp == 0) {
      printf ("\n\n***Error: critical memory error allocating data array\n");
      delShell (tt[i]->shl, 0);
      free (tt[i]);
      free (cpp);
      free (cp);
      return _ERROR_;
    }
    memset (tt[i]->cpp, 0, itemcount * sizeof (char*));
    tt[i]->lnks = malloc (itemcount*sizeof (Link));
    if (tt[i]->lnks == 0) {
      printf ("\n\n***Error: memory error allocating array of Links\n");
      free (tt[i]->cpp);
      delShell (tt[i]->shl, 0);
      free (tt[i]);
      free (cpp);
      free (cp);
      return _ERROR_;
    }
    memset (tt[i]->lnks, 0, itemcount*sizeof(Link));
    chk_lnks[i] = (void*)tt[i]->lnks;
    tt[i]->lpp = malloc (itemcount * sizeof (Link*));
    if (tt[i]->lpp == 0) {
      printf ("\n\n***Error: memory error allocating pointer to array of Links\n");
      free (tt[i]->lnks);
      free (tt[i]->cpp);
      delShell (tt[i]->shl, 0);
      free (tt[i]);
      free (cpp);
      free (cp);
      return _ERROR_;
    }
    memset (tt[i]->lpp, 0, itemcount*sizeof(Link*));
    for (j = 0; j < itemcount; j++)
      tt[i]->lpp[j] = (Link*)((char *)(tt[i]->lnks)+(j*sizeof(Link)));
  }
  //
  // This looks complex.  That's because it is.  Here's the skinny:
  // Basically, this should split up cpp between each thread's storage
  // area.  When this is finished, all the items pointed to by cpp
  // should be pointed to by tt[?]->cpp[?] and tt[?]->lpp[?].
  //
  for (i = 0, j = 0; cpp[i] != 0 && j < itemcount; j++) {
    for (thnum = 0; thnum < NUM_THREAD; thnum++) {
      tt[thnum]->cpp[j] = cpp[i];
      i++;
      tt[thnum]->lpp[j]->data = tt[thnum]->cpp[j];
      if (cpp[i] == 0)
	break;
    }
  }
  for (i = 0; i < NUM_THREAD; i++)
    check_pointer (tt[i]->lnks);

  elapsed (&t1);
  //
  // Looks like we're OK...so proceed.
  // Create the attr required and create the threads.
  //
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  for (i = 0; i < NUM_THREAD; i++) {
    status = pthread_create (&th[i], &attr, populateShell, tt[i]);
    if (status) {
      printf ("\n\n***Error: pthread_create returned %d\n", status);
      return _ERROR_;
    }
  }
  pthread_attr_destroy (&attr);

  //
  // OK, connect to the threads and wait for them to finish.
  //
  for (i = 0; i < NUM_THREAD; i++) {
    status = pthread_join (th[i], (void**)&count[i]);
    if (status) {
      printf ("\n\n***Error: joining thread %d\n", i);
      return _ERROR_;
    }
  }
 
  //
  // So, I should have all shells populated through separate threads.  I can
  // now merge them to get a single merged shell.
  //
  for (i = 0; i < NUM_THREAD; i++) {
    lhs[i] = shl2List (tt[i]->shl);
  }

  if (NUM_THREAD > 2) {
    final_lh = mergeSorted (lhs[0], lhs[1]);
    if (final_lh == 0) {
      printf ("\n\n***Error: merging sorted lists: %s\n",
	  ListErrorString[ListError]);
      return _ERROR_;
    }
    for (i = 2; i < NUM_THREAD; i++) {
      tmp_lh = final_lh;
      final_lh = mergeSorted (tmp_lh, lhs[i]);
      if (final_lh == 0) {
	printf ("\n\n***Error: merging sorted lists: %s\n",
	    ListErrorString[ListError]);
	return _ERROR_;
      }
    }
  }
  else {
    final_lh = mergeSorted (lhs[0], lhs[1]);
    if (final_lh == 0) {
      printf ("\n\n***Error: merging sorted lists: %s\n",
	  ListErrorString[ListError]);
      return _ERROR_;
    }
  }
  final = list2Shell (final_lh);
  if (final == 0) {
    printf ("\n\n***Error: list2Shell(): %s\n", shlErrorStr[shlError]);
    return _ERROR_;
  }
  elapsed (&t2);
  for (i = 0; i < NUM_THREAD; i++) {
    if (tt[i]->status == _ERROR_) {
      printf ("\n\n***Error: tt[%d]->status == _ERROR_: %s\n",
	  i,shlErrorStr[shlError]);
      status = _ERROR_;
    }
  }
  printf ("\nThere were no errors\n");
  printf ("\n\nAdded %d items in %f seconds\n", final->lh->number, t2-t1);
  printf ("\n\nTotal time: %f seconds (including allocating memory)\n", t2-t0);
  fp = fopen ("inputshl.out", "w");
  if (0 == fp) {
    perror ("\n\n***Error: Could not open inputshl.out");
    goto cleanupAndQuit;
  }
  lnk = final->lh->head->next;
  while (lnk != final->lh->tail) {
    fprintf (fp, "%s\n", (char*)lnk->data);
    lnk = lnk->next;
  }
  fclose (fp);
cleanupAndQuit:
  lnk = removeLink (final->lh);
  while (lnk != 0)
    lnk = removeLink (final->lh);
  delShell (final, 0);
  for (i = 0; i < NUM_THREAD; i++) {
    free (tt[i]->lpp);
    free (tt[i]->lnks);
    if (chk_lnks[i] != tt[i]->lnks) {
      printf ("\n\n***Warning: tt[%d]->lnks != chk_lnks[%d]\n", i, i);
    }
    free (tt[i]->cpp);
    free (tt[i]);
  }
  free (cpp);
  free (cp);
  // That's all, folks.
  print_block_list();
  pthread_exit ((void*)0);

  return 0;
}
