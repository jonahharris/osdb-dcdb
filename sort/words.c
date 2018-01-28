#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/*
 * #define if you want the lengths to be constant.
 */
/*#define CONSTANT_LEN*/

int main (int ac, char *av[])
{
  char buf[BUFSIZ];
  int todo;
  int maxlen = 50;
  int len;
  long junk;
  int x;
  int y;

  (void) time (&junk);
  (void) srandom ((int) junk);

  if (ac < 3) {
    (void) fprintf (stderr, "usage: words count [len]\n");
    exit (1);
  }

  /* how many inserts to perform */
  todo = atoi (av[1]);

  if (ac > 2) {
    maxlen = atoi (av[2]);
    if (maxlen < 1) {
      (void) fprintf (stderr, "max len less than 1 is not acceptable!\n");
      exit (1);
    }
  }

  for (x = 0; x < todo; x++) {
    if (x != 0)
      printf ("\n");
    memset (buf, 0, BUFSIZ);
#ifndef CONSTANT_LEN
    len = ((int) random () % maxlen) + 1;
    if (len < (int) maxlen / 5)
      len = maxlen / 5;
    if (len < 5 && maxlen > 5)
      len = 5;
#else
    len = maxlen;
#endif
    for (y = 0; y < len; y++) {
      buf[y] = ((int) random () % 26) + 'a';
    }

    buf[len] = '\0';
    (void) printf ("%s", buf);
  }
  printf ("\n");
  exit (0);
}
