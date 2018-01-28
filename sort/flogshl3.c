/* Source File: beatuptree.c */

/*
 * Put the shell code through the ringer.
 */

#include <sort.h>

#ifndef __CINT__
INLINE int
testCompare (void *p1, void *p2)
{
  return (strcmp ((char *) p1, (char *) p2));
}
/* Use for case-insensitive compares */
INLINE int noCaseCompare (void *p1, void *p2)
{
        return (strcasecmp ((char *)p1, (char *)p2));
}
#endif

#ifdef __CINT__
#include "sort.c"
#include "qsort.c"

int testCompare (void *p1, void *p2)
{
  return (strcmp ((char *) p1, (char *) p2));
}
/* Use for case-insensitive compares */
int noCaseCompare (void *p1, void *p2)
{
        return (strcasecmp ((char *)p1, (char *)p2));
}
#endif


#define	DATASIZE	10
#define MAXDATA		1000000
#define HASH_INCREMENT 256
/*#define HI (HASH_INCREMENT*4+HASH_INCREMENT*2+HASH_INCREMENT)*/
#define HI (HASH_INCREMENT*8+HASH_INCREMENT)
/*#define HI HASH_INCREMENT*/

int main (int argc, char *argv[])
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
  int isOK = TRUE;
  shellHeader *shl;
  int counter;
  shellLevel lvl;
  int i,number;
  static int compares[MAXDATA];
  int maxCompares;
  double avgCompares;
  static Link *hash[HI];
  int sort_min = 0, sort_max = 0;
  register int this_val = 0;
#ifndef __CINT__
  double t1, t2, sum = 0;
#endif

  if (argc != 5) {
    printf ("\nUsage: %s <num> <file> <srt> <rev>\n", argv[0]);
    printf ("\twhere <num> is the number of items to sort\n");
    printf ("\t<file> is the name of the file to read\n");
    printf ("\t<srt> is <file> in sorted order\n");
    printf ("\tand <rev> is <file> in reverse sorted order\n");
    return 1;
  }

  number = atoi (argv[1]);
  if (number < 0 || number > MAXDATA) {
    printf ("\n\n***Error: invalid value for number: %s\n", argv[1]);
    return _ERROR_;
  }

  lnk = malloc (MAXDATA*sizeof (Link));
  if (0 == lnk) {
    printf ("\n\n***Error: critical memory error allocating links\n");
    return _ERROR_;
  }
  memset (lnk, 0, MAXDATA*sizeof(Link));
  check_pointer (lnk);

  fp = fopen (argv[2], "r");
  if (fp == NULL) {
    printf ("\n***Error: couldn't open %s\n", argv[2]);
    return _ERROR_;
  }
  stat ((char*)argv[2], &statbuf);
  cp = malloc ((size_t)statbuf.st_size+1);
  if (0 == cp) {
    printf ("\n\n***Error: critical memory error allocating file buffer\n");
    return _ERROR_;
  }
  fread (cp, (size_t)statbuf.st_size, 1, fp);
  check_pointer (cp);
  cpp = split (cp, '\n');
  fclose (fp);

#ifndef __CINT__
  elapsed(&t1);
#endif
  for (i = 0; cpp[i] && i < MAXDATA; i++) {
    lnk[i].data = cpp[i];
    /*
    if (hash[(int)(cpp[i][0]<<2)+(cpp[i][1]<<1)+cpp[i][2]] == 0) {
      hash[(int)(cpp[i][0]<<2)+(cpp[i][1]<<1)+cpp[i][2]] = &lnk[i];
      lnk[i].next = 0;
    }
    else {
      lnk[i].next = hash[(int)(cpp[i][0]<<2)+(cpp[i][1]<<1)+cpp[i][2]];
      hash[(int)(cpp[i][0]<<2)+(cpp[i][1]<<1)+cpp[i][2]] = &lnk[i];
    }
    */
    this_val = (cpp[i][0]*8)+cpp[i][1];
    if (sort_min == 0) 
      sort_min = this_val;
    else
      if ( this_val < sort_min)
	sort_min = this_val;
    if (sort_max == 0)
      sort_max = this_val;
    else
      if ( this_val > sort_max)
	sort_max = this_val;
    if (hash[this_val] == 0) {
      hash[this_val] = &lnk[i];
      lnk[i].next = 0;
    }
    else {
      lnk[i].next = hash[this_val];
      hash[this_val] = &lnk[i];
    }
    /*
    if (hash[(int)cpp[i][0]] == 0) {
      hash[(int)cpp[i][0]] = &lnk[i];
      lnk[i].next = 0;
    }
    else {
      lnk[i].next = hash[(int)cpp[i][0]];
      hash[(int)cpp[i][0]] = &lnk[i];
    }
    */
  }
#ifndef __CINT__
  elapsed(&t2);
  sum = t2-t1;
#endif

  /*printf ("\nInitializing shell...");*/
  shl = initShell (noCaseCompare, FALSE, FALSE);
  if (shl == 0) {
    printf ("\n\n***Error: initilizing shell, %s\n", shlErrorStr[shlError]);
    return _ERROR_;
  }

  /*
   * Populate the shell
   */
  /*printf ("\nPopulating shell...");*/
#ifndef __CINT__
  elapsed (&t1);
#endif
/*  for (i = (int)'0'-1, counter = 0; i < (int)'z'+1; i++) {*/
/*  for (i = (int)('a'*2+'a'), counter = 0; i <= (int)('z'*2+'z'); i++) {*/
/*  for (i = (int)('a'*8+'a'), counter = 0; i <= (int)('z'*8+'z'); i++) {*/
  for (i = sort_min, counter = 0; i <= sort_max; i++) {
    if (hash[i] != 0) {
      while (hash[i] != 0) {
        found = hash[i];
        hash[i] = hash[i]->next;
	status = addShellItem (shl, found);
	compares[counter] = shl->numCompares;
	counter++;
	if (status == _ERROR_) {
	  if (shlError == SHELL_LIST) {
	    printf ("\n\n***Error: list error, item %d, %s\n",
		i, ListErrorString[ListError]);
	  }
	  else {
	    printf ("\n\n***Error: item %d, %s entering \"%s\"\n",
		i, shlErrorStr[shlError], (char *)found->data);
	  }
	  delShell(shl, 0);
	  return _ERROR_;
	}
      }
      hash[i] = 0;
    }
  }
#ifndef __CINT__
  elapsed (&t2);

  nodeLevels (shl, &lvl);
  printf ("\nAdd time: %f\n", sum+(t2-t1));
  printf ("Succeeded...%d lines stored\n", shl->lh->number);
  printf ("Number of nodes = %d\n", shl->numNodes);
  printf ("Node levels: ");
  for (i = 0; i < NODE_LEVEL; i++)
    printf ("%d %d, ", i, lvl.lvl[i]);
  printf ("\n");
#endif

  maxCompares = 0;
  avgCompares = 0.0;
  for (i = 0; i < number; i++) {
    if (compares[i] > maxCompares)
      maxCompares = compares[i];
    avgCompares += compares[i];
  }
  avgCompares /= number;
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
#ifndef __CINT__
  elapsed (&t1);
#endif
  for (counter = 0; cpp[counter] && counter < MAXDATA; counter++) {
    found = findShellItem (shl, &lnk[counter]);
    if (0 == found) {
      /*printf ("Didn't find \"%s\"\n", cpp[counter]);*/
      isOK = FALSE;
    }
    compares[counter] = shl->numCompares;
  }
#ifndef __CINT__
  elapsed (&t2);
#endif

  if (!isOK) {
    printf ("\n***Problem with shell data, original filling\n");
    return -1;
  }

#ifndef __CINT__
  printf ("Search Time: %f\n", t2 - t1);
#endif

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
  fp = fopen (argv[3], "r");
  if (fp == NULL) {
    printf ("\n***Error: couldn't open %s\n", argv[3]);
    return _ERROR_;
  }
  stat (argv[3], &statbuf);
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
/*    if (strcasecmp (cpp[counter], (char *)found->data)) {*/
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

  fp = fopen (argv[4], "r");
  if (fp == NULL) {
    printf ("\n***Error: couldn't open %s\n", argv[4]);
    return _ERROR_;
  }
  /* Here we ASSUME that the size of the files is the same...should be. */
  free (newcp);
  stat (argv[4], &statbuf);
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
/*    if (strcasecmp (cpp[counter], (char *)found->data)) {*/
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
