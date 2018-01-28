#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main (int ac, char *av[])
{
  char buf[BUFSIZ];
  int todo;
  int maxlen = 50;
  int len;
  long junk;
  int x;
  int y;
  int chaos;

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
    len = ((int) random () % maxlen) + 1;
    if (len < (int) maxlen / 5)
      len = maxlen / 5;
    if (len < 5 && maxlen > 5)
      len = 5;
    for (y = 0;;) {
      chaos = ((int) random () % 2);
      if (chaos == 0) {
	buf[y++] = ((int) random () % 26) + 'a';
	if (y >= len)
	  break;
      }
      if (chaos == 1) {
	buf[y++] = ((int) random () % 26) + 'A';
	if (y >= len)
	  break;
      }
      /*
         if (chaos == 2) {
         buf[y++] = ((int)random() % 10) + '0';
         if (y >= len)
         break;
         }
       */
    }

    buf[len] = '\0';
    (void) printf ("%s", buf);
  }
  printf ("\n");
  exit (0);
}
