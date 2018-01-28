/* Source File: beatuptree.c */

/*
 * Put the shell code through the ringer.
 */

#include <sort.h>

INLINE int
testCompare (void *p1, void *p2)
{
  return (strcmp ((char *) p1, (char *) p2));
}

#define	DATASIZE	10
#define MAXDATA		1000000

int
main (int argc, char *argv[])
{
  FILE *fp;
  struct stat statbuf;
  char *cp;
  char *newcp;
  char **cpp;
  char **newcpp;
  Link *lnk;
  Link *found;
  int status;
  double t1, t2;
  int isOK = TRUE;
  shellHeader *shl;
  int counter;
  shellLevel lvl;
  int i;
  static int compares[MAXDATA];
  int maxCompares;
  double avgCompares;

  if (argc != 4) {
    printf ("\nUsage: %s <num> <file> <srt> <revsrt>\n", argv[0]);
    printf ("\t<file> is the name of the file to read\n");
    printf ("\t<srt> is the name of the sorted file\n");
    printf ("\tand <revsrt> is the name of the reverse sorted file\n");
    return 1;
  }

  Assert (argv[1] != NULL && argv[1][0] != '\0');
  Assert (argv[2] != NULL && argv[2][0] != '\0');
  Assert (argv[3] != NULL && argv[3][0] != '\0');

  /*printf ("\nSetting up data structures...");*/
  lnk = malloc (MAXDATA*sizeof (Link));
  if (0 == lnk) {
    printf ("\n\n***Error: critical memory error allocating links\n");
    return _ERROR_;
  }
  memset (lnk, 0, MAXDATA*sizeof(Link));
  check_pointer (lnk);
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

  for (i = 0; cpp[i] && i < MAXDATA; i++)
    lnk[i].data = cpp[i];

  /*printf ("\nInitializing shell...");*/
  shl = initShell (testCompare, FALSE, FALSE);
  if (shl == 0) {
    printf ("\n\n***Error: initilizing shell, %s\n", shlErrorStr[shlError]);
    return _ERROR_;
  }

  /*
   * Populate the shell
   */
  /*printf ("\nPopulating shell...");*/
  elapsed (&t1);
  for (counter = 0; cpp[counter] && counter < MAXDATA; counter++) {
    /* skip blank lines */
    status = addShellItem (shl, &lnk[counter]);
    compares[counter] = shl->numCompares;
    if (status == _ERROR_) {
      if (shlError == SHELL_LIST) {
	printf ("\n\n***Error: List Error, item %d, %s\n",
		counter, ListErrorString[ListError]);
	delShell (shl, 0);
	return _ERROR_;
      }
      else {
	printf ("\n\n***Error: item %d, %s entering \"%s\"\n",
		counter, shlErrorStr[shlError], cpp[counter]);
	delShell (shl, 0);
	return _ERROR_;
      }
    }
    /*
    if (shl->numCompares > 10000) {
      restructureShellNodes (shl);
      shl->numCompares = 1;
    }
    */
    debugCheckShell (shl);
    if (shlError == SHELL_UNSPECIFIED)
      return _ERROR_;
  }
  elapsed (&t2);

  nodeLevels (shl, &lvl);

  printf ("\nAdd time: %f\n", t2 - t1);
  printf ("Succeeded...%d lines stored\n", shl->lh->number);
  printf ("Number of nodes = %d\n", shl->numNodes);
  printf ("Node levels: ");
  for (i = 0; i < NODE_LEVEL; i++)
    printf ("%d %d, ", i, lvl.lvl[i]);
  printf ("\n");

  maxCompares = 0;
  avgCompares = 0.0;
  for (i = 0; i < counter; i++) {
    if (compares[i] > maxCompares)
      maxCompares = compares[i];
    avgCompares += compares[i];
  }
  avgCompares /= counter;
  printf ("\n\nCompares during adds: max = %d, avg = %f\n",
	  maxCompares, avgCompares);


  /*
   * Open test file and search for each item in the shell
   */
  status = restructureShellNodes (shl);
  if (_ERROR_ == status) {
    printf ("\n\n***Error: restructureShellNodes(): %s\n", shlErrorStr[shlError]);
    return _ERROR_;
  }
  elapsed (&t1);
  for (counter = 0; cpp[counter] && counter < MAXDATA; counter++) {
    found = findShellItem (shl, &lnk[counter]);
    if (0 == found) {
      printf ("Didn't find \"%s\"\n", cpp[counter]);
      isOK = FALSE;
    }
    compares[counter] = shl->numCompares;
  }
  elapsed (&t2);

  if (!isOK) {
    printf ("\n***Problem with shell data, original filling\n");
    return -1;
  }

  printf ("Search Time: %f\n", t2 - t1);

  maxCompares = 0;
  avgCompares = 0.0;
  for (i = 0; i < counter; i++) {
    if (compares[i] > maxCompares)
      maxCompares = compares[i];
    avgCompares += compares[i];
  }
  avgCompares /= counter;
  printf ("\nCompares during find: max = %d, avg = %f\n",
	  maxCompares, avgCompares);

  /*
   * Traverse backwards and forwards and make sure everything is OK.
   */
  fp = fopen (argv[2], "r");
  if (fp == NULL) {
    printf ("\n***Error: couldn't open %s\n", argv[2]);
    return _ERROR_;
  }
  stat (argv[2], &statbuf);
  newcp = malloc ((size_t)statbuf.st_size+1);
  if (0 == newcp) {
    printf ("\n\n***Error: critical memory error allocating file buffer\n");
    return _ERROR_;
  }
  memset (newcp, 0, (size_t)statbuf.st_size);
  fread (newcp, (size_t)statbuf.st_size, 1, fp);
  check_pointer (newcp);
  newcpp = split (newcp, '\n');
  fclose (fp);
  for (counter = 0, found = shl->lh->head->next; 
      found != shl->lh->tail;
      counter++, found=found->next) {
    if (strcmp (cpp[counter], (char *)found->data)) {
      printf ("Bad forward compare: \"%s\" <-> \"%s\"\n",
	  newcpp[counter], (char *)found->data);
      isOK = FALSE;
    }
  }
  if (!isOK) {
    printf ("\n\nProblem with forward compare...bailing now.\n");
    return _ERROR_;
  }

  fp = fopen (argv[3], "r");
  if (fp == NULL) {
    printf ("\n***Error: couldn't open %s\n", argv[2]);
    return _ERROR_;
  }
  /* Here we ASSUME that the size of the files is the same...should be. */
  free (newcp);
  stat (argv[3], &statbuf);
  newcp = malloc ((size_t)statbuf.st_size+1);
  memset (newcp, 0, (size_t)statbuf.st_size);
  if (0 == newcp) {
    printf ("\n\n***Error: critical memory error allocating file buffer\n");
    return _ERROR_;
  }
  fread (newcp, (size_t)statbuf.st_size, 1, fp);
  check_pointer (newcp);
  newcpp = split (newcp, '\n');
  fclose (fp);
  for (counter = 0, found = shl->lh->tail->prev; 
      newcpp[counter] && counter < MAXDATA && found != 0; 
      counter++, found=found->prev) {
    if (strcmp (cpp[counter], (char *)found->data)) {
      printf ("Bad reverse compare: \"%s\" <-> \"%s\"\n",
	  newcpp[counter], (char *)found->data);
      isOK = FALSE;
    }
  }
  if (!isOK) {
    printf ("\n\nProblem with reverse compare...bailing now.\n");
    return _ERROR_;
  }
  print_block_list();
  return _OK_;

  delShell (shl, 0);
  printf ("\n\nDone.\n");
  free (newcp);
  free (cp);
  free (lnk);
}
