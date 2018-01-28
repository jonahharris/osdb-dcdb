/* Source File: flogdelidx2.c */

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
  ListHeader *lh;

  if (argc != 2) {
    printf ("\n\nUsage: index <file>\n");
    printf ("\twhere <file> is the file to add to the index,\n");
    return 1;
  }

  /*
   * Create the index file
   */
  status = createIndexFile ("testIndex.idx",
			    BLOCK_SIZE, TEST_DATA_SIZE, TRUE, TRUE);
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
   * Now, delete'em all.
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
    counter++;
    /*printf ("Deleting key = %s, item = %d, counter = %d\n",
	(char*)ki->key, ki->item, counter);*/
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
    ki = firstIndexItem (idx);
    if (0 == ki) {
      if (idxError == IDX_NOERR)
	break;
      printf ("\n\n***Error: %s\n", idxErrMsg[idxError]);
      return _ERROR_;
    }
    check_pointer (ki);
  }
  free(rm);
  printf ("Deleted every index item.  Indexes remaining = %ld\n(should be 0)\n", 
      idx->hdr->numItems);

  status = closeIndexFile (idx, "testIndex.inx");
  if (_ERROR_ == status) {
    printf ("\n\n***Error: closing index, %s\n", idxErrMsg[idxError]);
    return _ERROR_;
  }
  free (data);
  print_block_list ();
  return _OK_;
}
