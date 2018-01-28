/*	Source File:	mkdups.c	*/

/*
 * Open a file file assumed to contain words of text on each line 
 * and copy it to a second file, adding duplicate items at specified
 * intervals.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char *argv[])
{
  FILE *infp, *outfp;
  int counter, interval;
  char ln[1024], intLine[1024];
  char *cp;

  if (argc != 4) {
    printf ("\n\nUsage:\n");
    printf ("\tmkdups <infile> <outfile> <modulus>\n");
    printf ("\t  where <infile> is the input file,\n");
    printf ("\t  <outfile> is the output file,\n");
    printf ("\t  and <modulus> is the interval to duplicate at.\n\n");
    return 1;
  }

  infp = fopen (argv[1], "r");
  if (0 == infp) {
    printf ("\n\n***Error: couldn't open input file %s\n", argv[1]);
    return -1;
  }

  outfp = fopen (argv[2], "w");
  if (0 == outfp) {
    printf ("\n\n***Error: couldn't open output file %s\n", argv[2]);
    return -1;
  }

  interval = atoi (argv[3]);
  if (interval == 0) {
    printf ("\n\n***Error: invalid interval, %s\n", argv[3]);
    return -1;
  }

  counter = 0;
  while (!feof (infp)) {
    cp = fgets (ln, 1020, infp);
    if (cp == 0)
      break;
    counter++;
    if (counter == interval) {
      strcpy (intLine, ln);
      fprintf (outfp, "%s", intLine);
      continue;
    }
    if (counter % interval == 0) {
      fprintf (outfp, "%s", intLine);
      fprintf (outfp, "%s", ln);
      continue;
    }
    fprintf (outfp, "%s", ln);
  }
  return 0;
}
