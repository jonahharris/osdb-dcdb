/* Source File: rough_sort.c */

/*
 * This program reads in a list of strings and roughly sorts them by the first
 * three characters and spits out the quasi-sorted data.  This is useful to
 * take a relatively random data set and make it not-so-random.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sort.h>

#ifdef __TINYC__
#include "sort.c"
#include "qsort.c"
#define print_block_list() /* */
#endif

#define HASH_INC_MAX ((256<<16)+(256<<8)+256)
#define HASH(x)  (((x)<<16)+((x)<<8)+(x))
#define HASH_INC ((HASH('z')) - (HASH('/')))
#define MAX_DATA 10000000 // 10 Million lines.
#define MAX_LEN 4096

typedef struct _hash_link {
  char *data;
  struct _hash_link *next;
} hashLink;

int main (int argc, char *argv[])
{
  register unsigned int i;
  register int counter;
  static hashLink *hash_ary[HASH_INC];
  hashLink **lnks;
  hashLink *lnk;
  hashLink *lnkp;
  unsigned int hash_val;
  unsigned int sort_min = 0, sort_max = 0;
  FILE *fp;
  char *cp;
  char **cpp;
  double t1, t2;
  struct stat statbuf;
  int status;
  int numitems, len;

  /*
   * Read the whole file into a memory buffer and split it by lines.
   */
  if (argc != 4) {
    printf ("\n\n***Usage: %s num len file\n", argv[0]);
    printf ("\twhere num is the number of items to sort,\n");
    printf ("\tlen is the max length of the items, and\n");
    printf ("\tfile is a file to roughly sort.\n");
    return _ERROR_;
  }
  numitems = atoi (argv[1]);
  if (numitems < 0 || numitems > MAX_DATA) {
    printf ("\n\n***Error: invalid \"num\" value: %d\n", numitems);
    return _ERROR_;
  }
  len = atoi (argv[2]);
  if (len < 0 || len > MAX_LEN) {
    printf ("\n\n***Error: invalid \"len\" value: %d\n", len);
    return _ERROR_;
  }
  fp = fopen (argv[3], "r");
  if (fp == NULL) {
    printf ("\n\n***Error: Could not open %s: ", argv[3]);
    perror (" ");
    return _ERROR_;
  }
  stat (argv[3], &statbuf);
  cp = malloc ((size_t) statbuf.st_size+1);
  if (0 == cp) {
    fprintf (stderr, "\n\n***Error: could not allocate file buffer for %s\n", argv[3]);
    return _ERROR_;
  }
  fread (cp, (size_t)statbuf.st_size, 1, fp);
  fclose (fp);
  cpp = malloc ((numitems+1)*sizeof(char*));
  if (0 == cpp) {
    printf ("\n\n***Error: couldn't allocate cpp\n");
    free (cp);
    return _ERROR_;
  }
  memset (cpp, 0, (numitems+1)*sizeof(char*));
  check_pointer (cpp);
  status =  split_r (cpp, numitems, cp, '\n');
  check_pointer (cpp);
  if (status > numitems) {
    printf ("\n\n***Warning: the number of lines in %s is greater than %d\n",
	argv[3], numitems);
    printf ("\tWe will only process %d items\n", numitems);
  }
  lnks = malloc (numitems*sizeof(hashLink*));
  if (0 == lnks) {
    printf ("\n\n***Error: couldn't allocate lnks\n");
    free (cp);
    free (cpp);
    return _ERROR_;
  }
  memset (lnks, 0, numitems*sizeof(hashLink*));
  check_pointer (lnks);
  lnkp = malloc (numitems*sizeof(hashLink));
  if (0 == lnkp) {
    printf ("\n\n***Error: could not allocate array of links\n");
    free (cp);
    free (cpp);
    free (lnks);
    return _ERROR_;
  }
  memset (lnkp, 0, numitems*sizeof(hashLink));
  check_pointer (lnkp);
  for (i = 0; i < (unsigned) numitems; i++)
    lnks[i] = (hashLink*)(((char*)lnkp)+i*sizeof(hashLink));
  elapsed (&t1);
  counter = 0;
  for (i = 0; i < (unsigned)numitems && cpp[i]; i++) {
    hash_val = (int)(((unsigned char**)cpp)[i][0]<<16)+
       (int)(((unsigned char**)cpp)[i][1]<<8)+
       (int)(((unsigned char**)cpp)[i][2]);
    hash_val -= HASH('/');
    if (hash_val > HASH_INC) {
      printf ("\n\n***Error: invalid value \"%s\" produced hash of %d\n",
	  cpp[i], hash_val);
      free (cp);
      free (cpp);
      free (lnks);
      free (lnkp);
      return _ERROR_;
    }
    // initialize sort_min & sort_max if necessary.
    if (sort_min == 0)
      sort_min = hash_val;
    if (sort_max == 0)
      sort_max = hash_val;
    if (sort_min > hash_val)
      sort_min = hash_val;
    if (sort_max < hash_val)
      sort_max = hash_val;
    lnk = lnks[i];
    lnk->data = cpp[i];
    if (hash_ary[hash_val] == 0) {
      lnk->next = 0;
      hash_ary[hash_val] = lnk;
    }
    else {
      lnk->next = hash_ary[hash_val];
      hash_ary[hash_val] = lnk;
    }
    counter++;
  }
  elapsed (&t2);
  // Now, print them out in order.
  for (i = sort_min; i <= sort_max; i++) {
    if (hash_ary[i] != 0) {
      while (hash_ary[i] != 0) {
	lnk = hash_ary[i];
	hash_ary[i] = hash_ary[i]->next;
	printf ("%s\n", lnk->data);
      }
    }
  }
  free (cp);
  free (cpp);
  free (lnks);
  free (lnkp);
  fprintf (stderr, "Roughly sorted %d items in %f seconds\n", counter, t2-t1);
  print_block_list();
  return _OK_;
}
