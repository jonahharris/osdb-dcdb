/*	Source File:	flognidx.c	*/

#include <ctype.h>
#include <time.h>
#include <stdio.h>

#include <index.h>
#include <sort.h>

#define	TEST_DATA_SIZE		11
#define	BLOCK_SIZE			256

int main (int argc, char *argv[])
{
  FILE *fp;
  char *data;
  char *cp;
  keyItem *ki;
  Link *lnk;
  Link *found;
  ListHeader *srchList = 0;
  int status;
  dbIndex *idx;
  unsigned int counter = 0UL;
  double t1, t2;

  if (argc != 2) {
    printf ("\n\nUsage: index <file> <file.srt> <file.rev.srt>\n");
    printf ("\twhere <file> is the file to add to the index\n");
    return 1;
  }

  /*
   * Create the index file
   */
  if (!fexists ((char *) "testIndex.idx")) {
    status = createIndexFile ("testIndex.idx",
			      BLOCK_SIZE, TEST_DATA_SIZE, FALSE, TRUE);
    if (_ERROR_ == status) {
      printf ("\n\n***Error: Couldn't create index file %s\n",
	      "testIndex.idx");
      return _ERROR_;
    }
    idx = openIndex ("testIndex.idx", 0, BLOCK_SIZE);
    if (0 == idx) {
      printf ("\n\n***Error: Couldn't open index file, %s\n",
	      idxErrMsg[idxError]);
      return _ERROR_;
    }
  }
  else {
    idx = openIndex ("testIndex.idx", "testIndex.inx", BLOCK_SIZE);
    if (0 == idx) {
      printf ("\n\n***Error: Couldn't open index file, %s\n",
	      idxErrMsg[idxError]);
      return _ERROR_;
    }
    if (idx->hdr->numItems >= 10000000L) {
      printf ("\n\n***Error: 10000000 items entered already\n");
      return _ERROR_;		/* just so the loop will end */
    }
  }
  srchList = initList (UNSORTED, 0);
  if (0 == srchList) {
    printf ("\n\n***Error: couldn't create search list\n");
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
    if (0 == lnk) {
      printf ("\n\n***Error: dynamic memory exhausted\n");
      closeIndexFile (idx, "testIndex.inx");
      return _ERROR_;
    }
    check_pointer (lnk);
    ki = malloc (TEST_DATA_SIZE + sizeof (unsigned int));
    if (0 == ki) {
      printf ("\n\n***Error: fatal memory error allocating a keyItem\n");
      return _ERROR_;
    }
    check_pointer (ki);

    strcpy (ki->key, data);
    check_pointer (ki);

    lnk->data = ki;
    srchList->current = srchList->tail->prev;
    insertLink (srchList, lnk);
  }
  elapsed (&t2);
  fclose (fp);

  printf ("Add time for %ld items: %f\n", idx->hdr->numItems, t2 - t1);
  printf ("Item size %d; %d items per block\n", idx->hdr->itemSize,
	  (idx->hdr->blockSize - sizeof (indexHeader)) / idx->hdr->itemSize);
  printf ("There are %d items in the shell\n", idx->inx->lh->number);

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

  lnk = srchList->head->next;
  while (lnk != srchList->tail) {
    int i;
    found = searchExactIndexItem (idx, lnk);
    if (0 == found) {
      if (idxError == IDX_NOERR) {
	shellLevel lvl;
	nodeLevels (idx->inx, &lvl);
	ki = lnk->data;
	printf ("\n***Error: %s not found\n", ki->key);
/*				printf ("Node levels: 2 %d, 3 %d, 4 %d, 5 %d, 6 %d\n",
						lvl.lvltwo, lvl.lvlthree, lvl.lvlfour, lvl.lvlfive,
						lvl.lvlsix);*/
	printf ("Node levels: ");
	for (i = 0; i < NODE_LEVEL; i++)
	  printf ("%d %d, ", i + 2, lvl.lvl[i]);
	printf ("\n");
	lnk = idx->inx->lh->head->next;
	while (lnk != idx->inx->lh->tail) {
	  ki = lnk->data;
	  printf ("%s\n", ki->key);
	  lnk = lnk->next;
	}
	closeIndexFile (idx, "testIndex.inx");
	return _ERROR_;
      }
      else {
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
      }
    }
    lnk = lnk->next;
  }

  if (srchList != 0) {
    if (srchList->number > 0)
      clearList (srchList);
    delList (srchList);
  }
  status = closeIndexFile (idx, "testIndex.inx");
  if (_ERROR_ == status) {
    printf ("\n\n***Error: closing index, %s\n", idxErrMsg[idxError]);
    return _ERROR_;
  }
  free (data);
  print_block_list ();
  return _OK_;
}
