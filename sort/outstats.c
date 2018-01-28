/* Source File: outstats.c */

/*
 * Put the shell code through the ringer.
 */

#include <sort.h>

INLINE int testCompare (void *p1, void *p2)
{
        return (strcmp ((char *)p1, (char *)p2));
}

INLINE void printDump (void *data)
{
	printf ("%s\n", (char *)data);
}

#define	DATASIZE	25
#define MAXDATA		1000000

char data[512];
int main (int argc, char *argv[])
{
  FILE *fp;
  static char *pdata[MAXDATA];
  char **cpp;
  char *data;
  char *cp, *qsortarray, *sysqsarray;
  int i, number;
  struct stat statbuf;
  int status;
  double t1, t2;
  double sh_t, mq_t, sq_t;

  if (argc != 3)  {
    printf ("\nUsage: %s <file> <num>\n", argv[0]);
    printf ("\twhere <file> is the name of the file to read\n");
    printf ("\tand <num> is the number of items to sort\n\n");
    return 1;
  }

  number = atoi (argv[2]);
  if (number < 0 || number > MAXDATA) {
    printf ("\n\n***Error: invalid number of items\n");
    return _ERROR_;
  }

  Assert (argv[1] != NULL && argv[1][0] != '\0');
  fp = fopen (argv[1], "r");
  if (fp == NULL) {
    printf ("\n***Error: couldn't open %s\n", argv[1]);
    return -1;
  }

  stat (argv[1], &statbuf);
  data = malloc ((size_t)statbuf.st_size+1);
  if (0 == data) {
    printf ("\n\n***Error: fatal memory error allocating data\n");
    return _ERROR_;
  }
  fread (data, (size_t)statbuf.st_size, 1, fp);
  fclose (fp);

  cpp = split (data, '\n');
  for (i = 0; cpp[i] && i < MAXDATA; i++)
    pdata[i] = cpp[i];

  elapsed (&t1);
  status = shsort ((void **)pdata, i, testCompare);
  elapsed (&t2);
  if (status == _ERROR_) {
    printf ("\n\n***Error: calling shsort: %s\n", shlErrorStr[shlError]);
    return _ERROR_;
  }
  sh_t = t2-t1;
  /*
   * Now, use qsort to do the sorting.
   */
  qsortarray = malloc (number * DATASIZE);
  if (0 == qsortarray) {
    free (data);
    printf ("\n\n***Error: memory exhausted allocating qsort array\n");
    return _ERROR_;
  }
  memset (qsortarray, 0, number*DATASIZE);
  check_pointer (qsortarray);

  elapsed(&t1);
  for (i = 0; i < number; i++) {
    cp = qsortarray+i*DATASIZE;
    strncpy (cp, cpp[i], DATASIZE);
    cp[DATASIZE-1] = '\0';
  }
  bqsort ((void*)qsortarray, number, DATASIZE,
      (int(*)(const void*, const void*))testCompare);
  elapsed(&t2);
  mq_t = t2-t1;

  sysqsarray = malloc (number * DATASIZE);
  if (0 == sysqsarray) {
    free (data);
    free (qsortarray);
    printf ("\n\n***Error: memory exhausted allocating qsort array\n");
    return _ERROR_;
  }
  memset (sysqsarray, 0, number*DATASIZE);
  check_pointer (sysqsarray);

  elapsed (&t1);
  for (i = 0; i < number; i++) {
    cp = sysqsarray+i*DATASIZE;
    strncpy (cp, cpp[i], DATASIZE);
    cp[DATASIZE-1] = '\0';
  }
  qsort ((void*)sysqsarray, number, DATASIZE,
      (int(*)(const void*, const void*))testCompare);
  elapsed (&t2);
  sq_t = t2-t1;

  free (qsortarray);
  free (sysqsarray);
  free (data);

  printf ("%f:%f:%f\n", sh_t, mq_t, sq_t);
  return _OK_;
}
