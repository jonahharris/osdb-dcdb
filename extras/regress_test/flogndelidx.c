/* Source File: flogndelidx.c */

#include <ctype.h>
#include <time.h>
#include <stdio.h>

#include <index.h>
#include <sort.h>

#define	TEST_DATA_SIZE 11
#define	BLOCK_SIZE     256

int main (int argc, char *argv[])
{
  FILE *fp;
  char *data;
  char *cp;
  keyItem *ki, *rm;
  keyItem *tmp;
  int status;
  dbIndex *idx;
  unsigned int counter = 0UL;
  double t1, t2;
  int isOK = TRUE;
  ListHeader *lh;
  Link *lnk;
  Link *found;
/*	keyItem *treeKey;*/
/*	idxBlock *blk;*/
/*	Link *link;*/
/*	Link *found;*/

  if (argc != 4) {
    printf ("\n\nUsage: index <file> <file.srt> <file.rev.srt>\n");
    printf ("\twhere <file> is the file to add to the index,\n");
    printf ("\t<file.srt> is the sorted file to compare it with,\n");
    printf ("\t and <file.srt.rev> is the reverse file to compare with.\n\n");
    return 1;
  }

  /*
   * Create the index file
   */
  status = createIndexFile ("testIndex.idx",
			    BLOCK_SIZE, TEST_DATA_SIZE, FALSE, TRUE);
  if (_ERROR_ == status) {
    printf ("\n\n***Error: Couldn't create index file %s\n", "testIndex.idx");
    return _ERROR_;
  }
  idx = openIndex ("testIndex.idx", 0, BLOCK_SIZE);
  if (0 == idx) {
    printf ("\n\n***Error: Couldn't open index file, %s\n",
	    idxErrMsg[idxError]);
    return _ERROR_;
  }
  fp = fopen (argv[1], "r");
  if (0 == fp) {
    printf ("\n\n***Error: Couldn't open %s\n", argv[1]);
    return _ERROR_;
  }
  data = malloc (512);
  if (0 == data) {
    printf ("\n\n***Error: fatal memory error allocating data\n");
    return _ERROR_;
  }
  /*memset (data, 0, 512); */
  check_pointer (data);
  lh = initList (UNSORTED, 0);

  elapsed (&t1);
  while (TRUE) {
    cp = fgets (data, TEST_DATA_SIZE, fp);
    if (feof (fp))
      break;
    if (cp == 0)
      break;
    if (data[0] == '\0')
      break;

    cp = strchr (data, '\n');
    if (0 != cp)
      *cp = '\0';
    if (strlen (data) == 0)
      continue;
    ki = malloc (TEST_DATA_SIZE + sizeof (unsigned int));
    if (0 == ki) {
      printf ("\n\n***Error: fatal memory error allocating a keyItem\n");
      return _ERROR_;
    }
    /*memset (ki, 0, TEST_DATA_SIZE + sizeof (unsigned int)); */
    check_pointer (ki);

    counter++;
    ki->item = counter;
    strcpy (ki->key, data);
    check_pointer (ki);

    status = addIndexItem (idx, ki);
    if (status == _ERROR_) {
      if (idxError == IDX_UNIQUE) {
	idxError = IDX_NOERR;
	free (ki);
      }
      else {
	printf ("\n\n***Error: adding item %d, %s\n", counter,
		idxErrMsg[idxError]);
	closeIndexFile (idx, "testIndex.inx");
	return _ERROR_;
      }
    }
    lnk = malloc (sizeof (Link));
    cp = malloc (TEST_DATA_SIZE + 5);
    if (lnk == 0 || cp == 0) {
      printf ("\n\n***Error: fatal memory error allocating link\n");
      closeIndexFile (idx, "testIndex.inx");
      return _ERROR_;
    }
    memset (cp, 0, TEST_DATA_SIZE + 5);
    check_pointer (cp);
    strncpy (cp, data, TEST_DATA_SIZE);
    lnk->data = cp;
    lh->current = lh->tail->prev;
    insertLinkHere (lh, lnk);
    /*debugCheckShell (idx->inx); */
  }
  elapsed (&t2);
  fclose (fp);

  printf ("Add time for %ld items: %f\n", idx->hdr->numItems, t2 - t1);
  printf ("Item size %d; %d items per block\n", idx->hdr->itemSize,
	  (idx->hdr->blockSize - sizeof (indexHeader)) / idx->hdr->itemSize);
  printf ("%d items in inx file\n", idx->inx->lh->number);
/*	strcpy (idx->hdr->indexName, "testIndex.idx");*/

  status = closeIndexFile (idx, "testIndex.inx");
  if (_ERROR_ == status) {
    printf ("\n\n***Error: closing index, %s\n", idxErrMsg[idxError]);
    return _ERROR_;
  }

  idx = openIndex ("testIndex.idx", "testIndex.inx", BLOCK_SIZE);
  if (0 == idx) {
    if (idxError == IDX_LIST)
      printf ("\n\n***Error: opening index again, %s\n",
	      ListErrorString[ListError]);
    else if (idxError == IDX_SHELL) {
      if (shlError == SHELL_LIST)
	printf ("\n\n***Error: opening index again, %s\n",
		ListErrorString[ListError]);
      else
	printf ("\n\n***Error: opening index again, %s\n",
		shlErrorStr[shlError]);
    }
    else
      printf ("\n\n***Error: opening index again, %s\n", idxErrMsg[idxError]);
    return _ERROR_;
  }

  /*
   * Now, open the test files and check the index
   */
  fp = fopen (argv[2], "r");
  if (0 == fp) {
    printf ("\n\n***Error: couldn't open %s\n", argv[2]);
    return _ERROR_;
  }
  ki = firstIndexItem (idx);
  if (0 == ki) {
    printf ("\n\n***Error: %s\n", idxErrMsg[idxError]);
    return _ERROR_;
  }
  counter = 1L;
  while (TRUE) {
    cp = fgets (data, TEST_DATA_SIZE, fp);
    if (feof (fp))
      break;
    if (cp == 0)
      break;
    if (data[0] == '\0')
      break;

    cp = strchr (data, '\n');
    if (0 != cp)
      *cp = '\0';
    if (strlen (data) == 0)
      continue;
    if (strcmp (data, ki->key)) {
      printf
	("Index discrepancy: counter = %d, data = \"%s\", ki->key = \"%s\"\n",
	 counter, data, ki->key);
      isOK = FALSE;
    }
    /*printf ("%s\n", (char *)ki->key); */
    ki = nextIndexItem (idx);
    if (0 == ki) {
      if (idxError == IDX_NOERR)
	break;
      printf ("\n\n***Error: %s\n", idxErrMsg[idxError]);
      return _ERROR_;
    }
    counter++;
  }
  if (isOK == FALSE) {
    closeIndexFile (idx, "testIndex.inx");
    return _ERROR_;
  }

  fp = fopen (argv[3], "r");
  if (0 == fp) {
    printf ("\n\n***Error: couldn't open %s\n", argv[3]);
    return _ERROR_;
  }
  ki = lastIndexItem (idx);
  if (0 == ki) {
    printf ("\n\n***Error: %s\n", idxErrMsg[idxError]);
    return _ERROR_;
  }
  counter = 1L;
  while (TRUE) {
    cp = fgets (data, TEST_DATA_SIZE, fp);
    if (feof (fp))
      break;
    if (cp == 0)
      break;
    if (data[0] == '\0')
      break;

    cp = strchr (data, '\n');
    if (0 != cp)
      *cp = '\0';
    if (strlen (data) == 0)
      continue;
    if (strcmp (data, ki->key)) {
      printf
	("Index discrepancy: counter = %d, data = \"%s\", ki->key = \"%s\"\n",
	 counter, data, ki->key);
      isOK = FALSE;
    }
    ki = prevIndexItem (idx);
    if (0 == ki) {
      if (idxError == IDX_NOERR)
	break;
      printf ("\n\n***Error: %s\n", idxErrMsg[idxError]);
      return _ERROR_;
    }
    counter++;
  }

  if (isOK == FALSE) {
    closeIndexFile (idx, "testIndex.inx");
    return _ERROR_;
  }

  found = malloc (sizeof (Link));
  if (0 == found) {
    printf ("\n\n***Error: fatal memory error allocating a link\n");
    return _ERROR_;
  }
  ki = malloc (TEST_DATA_SIZE + sizeof (unsigned int));
  if (0 == ki) {
    printf ("\n\n***Error: fatal memory error allocating a keyItem\n");
    return _ERROR_;
  }
  check_pointer (ki);
  lnk = lh->head->next;
  elapsed (&t1);
  while (lnk != lh->tail) {
    strcpy (ki->key, (char *) lnk->data);
    check_pointer (ki);
    found->data = ki;
    if (!searchExactIndexItem (idx, found)) {
      printf ("***Warning: could not find %s in index\n", (char *) lnk->data);
    }
    lnk = lnk->next;
  }
  elapsed (&t2);
  printf ("\nSearch time: %f\n\n", t2 - t1);

  free (ki);
  free (found);
  clearList (lh);
  delList (lh);

  /* FIXME */
  /*ki = firstIndexItem (idx);
  while (TRUE) {
    printf ("%s\n", (char *)ki->key);
    ki = nextIndexItem (idx);
    if (ki == 0)
      break;
  }
  status = closeIndexFile (idx, "testIndex.inx");
  return 0;*/
  /* FIXME */

  /*
   * Now, delete every 5th index.
   */
  rm = malloc (25);
  if (0 == rm) {
    printf ("\n\n***Error: fatal memory error\n");
    return _ERROR_;
  }
  memset (rm, 0, 25);
  ki = firstIndexItem (idx);
  if (0 == ki) {
    printf ("\n\n***Error: %s\n", idxErrMsg[idxError]);
    return _ERROR_;
  }
  counter = 1L;
  while (TRUE) {
    ki = nextIndexItem (idx);
    if (0 == ki) {
      if (idxError == IDX_NOERR)
	break;
      printf ("\n\n***Error: %s\n", idxErrMsg[idxError]);
      return _ERROR_;
    }
    check_pointer (ki);
    counter++;
    if (counter % 100 == 0) {
      memmove (rm, ki, TEST_DATA_SIZE+5);
      /* FIXME */
      /*printf ("Deleting: key = %s, item = %d, counter = %d\n",
	  rm->key, rm->item, counter);*/
      /* FIXME */
      tmp = deleteIndexItem (idx, rm);
      if (0 == tmp) {
	printf ("\n\n***Error: deleting key = %s, item = %ld, counter = %d: %s\n", 
	    (char *)rm->key, rm->item, counter, idxErrMsg[idxError]);
	return _ERROR_;
      }
      check_pointer (tmp);
      free (tmp);
    }
  }
  printf ("Deleted every 100th index.  Indexes remaining = %ld\n", 
      idx->hdr->numItems);
  printf ("%d items in inx file\n", idx->inx->lh->number);

  status = closeIndexFile (idx, "testIndex.inx");
  if (_ERROR_ == status) {
    printf ("\n\n***Error: closing index, %s\n", idxErrMsg[idxError]);
    return _ERROR_;
  }
  free (data);
  print_block_list ();
  return _OK_;
}
