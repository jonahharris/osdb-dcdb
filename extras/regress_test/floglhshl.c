/* Source File: floglhshl.c */

#include <stdio.h>
#include <sys/stat.h>
#include <sort.h>

#include <time.h>

INLINE int testCompare (void *p1, void *p2)
{
  return (strcmp ((char *) p1, (char *) p2));
}

#define	DATASIZE	40

int main (int argc, char *argv[])
{
  FILE *fp;
  char *data;
  char *ln, *cp;
  Link *lnk = 0, *found = 0;
  int status;
  double t1, t2;
  int isOK = TRUE;
  shellHeader *shl1;
  shellHeader *shl2;
  ListHeader *lh1, *lh2;
  int counter;
  int first = FALSE;

  if (argc != 4) {
    printf ("\nUsage: %s <file> <file.srt> <file.rev.srt>\n", argv[0]);
    printf ("\twhere <file> is the name of the file to read\n");
    printf ("\t  and <file.srt> is a sorted version of <file>\n");
    printf ("\t  and <file.rev.srt> is in reverse sorted order.\n\n");
    return 1;
  }

  fp = fopen (argv[1], "r");
  if (fp == NULL) {
    printf ("\n***Error: couldn't open %s\n", argv[1]);
    return -1;
  }

  data = malloc (512);
  if (0 == data) {
    printf ("\n***Error: fatal memory error\n");
    return -1;
  }
  /* memset (data, 0, 512); */
  check_pointer (data);

  /*
   * Populate the shell
   */
  shl1 = initShell (testCompare, FALSE, TRUE);
  shl2 = initShell (testCompare, FALSE, TRUE);
  elapsed (&t1);
  counter = 0;
  while (!feof (fp)) {
    counter++;
    /* memset (data, 0, 512); */
    cp = fgets (data, 80, fp);
    if (0 == cp) {
      break;
    }
    if (data[0] == '\0')
      break;
    cp = strchr (data, '\n');
    if (0 != cp)
      *cp = '\0';
    /* skip blank lines */
    if (strlen (data) == 0)
      continue;
    ln = malloc (DATASIZE);
    if (0 == ln) {
      printf ("\n***Error: memory error\n");
      return -1;
    }
    /* memset (ln, 0, DATASIZE); */
    strncpy (ln, data, DATASIZE);
    ln[DATASIZE - 1] = '\0';
    check_pointer (ln);

    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      printf ("\n***Error: memory error\n");
      return -1;
    }
    /* memset (lnk, 0, sizeof (Link)); */
    check_pointer (lnk);
    lnk->data = ln;

    if (first)
      status = addShellItem (shl1, lnk);
    else
      status = addShellItem (shl2, lnk);
    if (status == _ERROR_) {
      if (shlError == SHELL_LIST) {
	printf ("\n\n***Error: List Error, item %d, %s\n",
		counter, ListErrorString[ListError]);
	delShell (shl1, 0);
	delShell (shl2, 0);
	return _ERROR_;
      }
      else {
	printf ("\n\n***Error: item %d, %s entering \"%s\"\n",
		counter, shlErrorStr[shlError], ln);
	delShell (shl1, 0);
	delShell (shl2, 0);
	return _ERROR_;
      }
    }
    if (first)
      first = FALSE;
    else
      first = TRUE;
  }
  lh1 = shl2List (shl1);
  lh2 = shl2List (shl2);
  lh1 = mergeSorted (lh1, lh2);
  shl1 = list2Shell (lh1);
  elapsed (&t2);
  fclose (fp);

  printf ("Add time: %f\n", t2 - t1);

  /*
   * Open test file and search for each item in the shell
   */
  fp = fopen (argv[1], "r");
  if (0 == fp) {
    printf ("\n\n***Error: Couldn't open %s\n", argv[1]);
    return _ERROR_;
  }

  lnk = malloc (sizeof (Link));
  if (0 == lnk) {
    printf ("\n\n***Error: fatal memory error\n");
    return _ERROR_;
  }
  check_pointer (lnk);

  elapsed (&t1);
  while (!feof (fp)) {
    cp = fgets (data, 80, fp);
    if (0 == cp) {
      break;
    }
    if (data[0] == '\0')
      break;
    cp = strchr (data, '\n');
    if (0 != cp)
      *cp = '\0';
    if (strlen (data) == 0)
      continue;
    lnk->data = data;
    found = findShellItem (shl1, lnk);
    if (0 == found) {
      printf ("Didn't find \"%s\"\n", (char *) data);
      isOK = FALSE;
    }
    found = queryShellItem (shl1, lnk);
    if (0 == found) {
      printf ("Didn't find (in query) \"%s\"\n", (char *) data);
      isOK = FALSE;
    }
  }
  elapsed (&t2);

  if (!isOK) {
    printf ("\n***Problem with shell data, original filling\n");
    return -1;
  }

  free (lnk);
  fclose (fp);


  printf ("Search Time: %f\n", t2 - t1);

  /*
   * Do the ordered compare
   */
  fp = fopen (argv[2], "r");
  if (0 == fp) {
    printf ("\n\n***Error: Couldn't open %s\n", argv[2]);
    return _ERROR_;
  }

  elapsed (&t1);
  found = shl1->lh->head->next;
  while (!feof (fp) && found != 0) {
    cp = fgets (data, 80, fp);
    if (0 == cp) {
      break;
    }
    if (data[0] == '\0')
      break;
    cp = strchr (data, '\n');
    if (0 != cp)
      *cp = '\0';
    if (strlen (data) == 0)
      continue;
    if (strcmp (data, (char *) found->data)) {
      printf ("Bad compare: \"%s\" <-> \"%s\"\n",
	      (char *) data, (char *) found->data);
      isOK = FALSE;
    }
    found = found->next;
  }

  if (!isOK) {
    printf ("\n***Problem with shell data, original filling\n");
    return -1;
  }

  fclose (fp);

  /*
   * Do the reverse ordered compare
   */
  fp = fopen (argv[3], "r");
  if (0 == fp) {
    printf ("\n\n***Error: Couldn't open %s\n", argv[3]);
    return _ERROR_;
  }

  found = shl1->lh->tail->prev;
  while (!feof (fp) && found != 0) {
    /*memset (data, 0, 512); */
    cp = fgets (data, 80, fp);
    if (0 == cp) {
      break;
    }
    if (data[0] == '\0')
      break;
    cp = strchr (data, '\n');
    if (0 != cp)
      *cp = '\0';
    if (strlen (data) == 0)
      continue;
    if (strcmp (data, (char *) found->data)) {
      printf ("Bad compare: \"%s\" <-> \"%s\"\n",
	      (char *) data, (char *) found->data);
      isOK = FALSE;
    }
    found = found->prev;
  }
  elapsed (&t2);

  if (!isOK) {
    printf ("\n***Problem with shell data, original filling\n");
    return -1;
  }

  fclose (fp);

  /*
   * Now, do a restructureShellNodes() and run through the
   * searches, etc. again.
   */
  elapsed (&t1);
  status = restructureShellNodes (shl1);
  elapsed (&t2);
  if (_ERROR_ == status) {
    printf ("\n\n***Error: restructuring nodes, %s\n", shlErrorStr[shlError]);
    delShell (shl1, 0);
    return _ERROR_;
  }
  printf ("Time to restructure, %f\n", t2 - t1);
  printf ("Number of nodes = %d\n", shl1->numNodes);

  fp = fopen (argv[1], "r");
  if (0 == fp) {
    printf ("\n\n***Error: Couldn't open %s\n", argv[1]);
    return _ERROR_;
  }

  lnk = malloc (sizeof (Link));
  if (0 == lnk) {
    printf ("\n\n***Error: fatal memory error\n");
    return _ERROR_;
  }
  /*memset (lnk, 0, sizeof (Link)); */
  check_pointer (lnk);

  elapsed (&t1);
  while (!feof (fp)) {
    cp = fgets (data, 80, fp);
    if (0 == cp) {
      break;
    }
    if (data[0] == '\0')
      break;
    cp = strchr (data, '\n');
    if (0 != cp)
      *cp = '\0';
    if (strlen (data) == 0)
      continue;
    lnk->data = data;
    found = findShellItem (shl1, lnk);
    if (0 == found) {
      printf ("After restruct: didn't find \"%s\"\n", (char *) data);
      isOK = FALSE;
    }
    found = queryShellItem (shl1, lnk);
    if (0 == found) {
      printf ("After restruct: didn't find (in query) \"%s\"\n",
	      (char *) data);
      isOK = FALSE;
    }
  }
  elapsed (&t2);

  if (!isOK) {
    printf ("\n***Problem with shell data after restructure\n");
    return -1;
  }

  free (lnk);
  fclose (fp);


  printf ("After restruct:search Time: %f\n", t2 - t1);

  /*
   * Do the ordered compare
   */
  fp = fopen (argv[2], "r");
  if (0 == fp) {
    printf ("\n\n***Error: Couldn't open %s\n", argv[2]);
    return _ERROR_;
  }

  elapsed (&t1);
  found = shl1->lh->head->next;
  while (!feof (fp) && found != 0) {
    /*memset (data, 0, 512); */
    cp = fgets (data, 80, fp);
    if (0 == cp) {
      break;
    }
    if (data[0] == '\0')
      break;
    cp = strchr (data, '\n');
    if (0 != cp)
      *cp = '\0';
    if (strlen (data) == 0)
      continue;
    if (strcmp (data, (char *) found->data)) {
      printf ("Bad compare: \"%s\" <-> \"%s\"\n",
	      (char *) data, (char *) found->data);
      isOK = FALSE;
    }
    found = found->next;
  }

  if (!isOK) {
    printf ("\n***Problem with shell data, original filling\n");
    return -1;
  }

  fclose (fp);

  /*
   * Do the reverse ordered compare
   */
  fp = fopen (argv[3], "r");
  if (0 == fp) {
    printf ("\n\n***Error: Couldn't open %s\n", argv[3]);
    return _ERROR_;
  }

  found = shl1->lh->tail->prev;
  while (!feof (fp) && found != 0) {
    /*memset (data, 0, 512); */
    cp = fgets (data, 80, fp);
    if (0 == cp) {
      break;
    }
    if (data[0] == '\0')
      break;
    cp = strchr (data, '\n');
    if (0 != cp)
      *cp = '\0';
    if (strlen (data) == 0)
      continue;
    if (strcmp (data, (char *) found->data)) {
      printf ("Bad compare: \"%s\" <-> \"%s\"\n",
	      (char *) data, (char *) found->data);
      isOK = FALSE;
    }
    found = found->prev;
  }
  elapsed (&t2);

  if (!isOK) {
    printf ("\n***Problem with shell data, original filling\n");
    return -1;
  }

  fclose (fp);

  elapsed (&t1);
  status = saveShell (shl1, "Test shell storage functionality",
		      DATASIZE, "beatupshell.dat");
  if (_ERROR_ == status) {
    if (shlError == SHELL_LIST)
      printf ("\n***Error: storing data, %s\n", ListErrorString[ListError]);
    else
      printf ("\n***Error: storing data, %s\n", shlErrorStr[shlError]);
    return (_ERROR_);
  }
  elapsed (&t2);

  delShell (shl1, NULL);

  elapsed (&t1);
  shl1 = getShell (testCompare, "beatupshell.dat");
  if (shl1 == 0) {
    if (shlError == SHELL_LIST)
      printf ("\n***Error retrieving shell, %s\n",
	      ListErrorString[ListError]);
    else
      printf ("\n***Error retrieving shell, %s\n", shlErrorStr[shlError]);
    return _ERROR_;
  }
  elapsed (&t2);

  printf ("\nTime to re-populate the shell, %f\n", t2 - t1);

  fp = fopen (argv[2], "r");
  if (0 == fp) {
    printf ("\n\n***Error: Couldn't open %s\n", argv[2]);
    return _ERROR_;
  }

  lnk = malloc (sizeof (Link));
  if (0 == lnk) {
    printf ("\n\n***Error: memory error allocating link\n");
    return _ERROR_;
  }
  elapsed (&t1);
  memset (data, 0, 512);
  while (!feof (fp)) {
    cp = fgets (data, 80, fp);
    if (0 == cp) {
      break;
    }
    cp = strchr (data, '\n');
    if (0 != cp)
      *cp = '\0';
    if (data[0] == '\0')
      continue;
    lnk->data = data;
    found = findShellItem (shl1, lnk);
    if (0 == found) {
      printf ("Didn't find \"%s\"\n", (char *) data);
      isOK = FALSE;
    }
    found = queryShellItem (shl1, lnk);
    if (0 == found) {
      printf ("Didn't find (in query) \"%s\"\n", (char *) data);
      isOK = FALSE;
    }
  }
  elapsed (&t2);
  if (!isOK) {
    printf ("\n***Problem with tree data, retieval from data disk\n");
    return -1;
  }
  fclose (fp);
  free (lnk);

  printf ("Search Time: %f\n", t2 - t1);

  /*
   * Do the ordered compare
   */
  fp = fopen (argv[2], "r");
  if (0 == fp) {
    printf ("\n\n***Error: Couldn't open %s\n", argv[2]);
    return _ERROR_;
  }

  elapsed (&t1);
  found = shl1->lh->head->next;
  while (!feof (fp) && found != shl1->lh->tail) {
    /*memset (data, 0, 512); */
    cp = fgets (data, 80, fp);
    if (0 == cp) {
      break;
    }
    if (data[0] == '\0')
      break;
    cp = strchr (data, '\n');
    if (0 != cp)
      *cp = '\0';
    /* skip blank lines */
    if (strlen (data) == 0)
      continue;
    if (strcmp (data, (char *) found->data)) {
      printf ("Bad compare: \"%s\" <-> \"%s\"\n",
	      (char *) data, (char *) found->data);
      isOK = FALSE;
    }
    found = found->next;
  }

  if (!isOK) {
    printf ("\n***Problem with tree data, retrieval from disk\n");
    return -1;
  }

  fclose (fp);

  /*
   * Do the reverse ordered compare
   */
  fp = fopen (argv[3], "r");
  if (0 == fp) {
    printf ("\n\n***Error: Couldn't open %s\n", argv[3]);
    return _ERROR_;
  }

  found = shl1->lh->tail->prev;
  while (!feof (fp) && found != shl1->lh->head) {
    /*memset (data, 0, 512); */
    cp = fgets (data, 80, fp);
    if (0 == cp) {
      break;
    }
    if (data[0] == '\0')
      break;
    cp = strchr (data, '\n');
    if (0 != cp)
      *cp = '\0';
    /* skip blank lines */
    if (strlen (data) == 0)
      continue;
    if (strcmp (data, (char *) found->data)) {
      printf ("Bad compare: \"%s\" <-> \"%s\"\n",
	      (char *) data, (char *) found->data);
      isOK = FALSE;
    }
    found = found->prev;
  }
  elapsed (&t2);

  if (!isOK) {
    printf ("\n***Problem with tree data, retrieval from disk\n");
    return -1;
  }

  fclose (fp);

  free (data);

  delShell (shl1, NULL);
  print_block_list ();

  return 0;
}
