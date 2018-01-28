/*	Source File:	thrashvm.c	*/

/*
 * This is a fairly simple C utility that you can run in test mode to thrash
 * the virtual machine on a UNIX system.  This should be run as root to bypass
 * any memory throttling.  If there is memory throttling in place that effects
 * root as well, this will not be as meaningful.
 *
 * Warning: this does things that are not usually seen on a production system.
 * DO NOT RUN THIS ON A SYSTEM THAT IS IN PRODUCTION.  This is designed for
 * testing virtual memory ONLY.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define	INIT_ARRAY	1000000L
#define	ARRAY_LEN	100000L
#define	DATA_LEN	100000L

int main (int argc, char *argv[])
{
  char *this_array;		/* current array of memory */
  char *init_array;		/* initial array */
  char **arrayofarrays;		/* 1 bil. should be enough */
  int i, j, k;

  init_array = malloc (INIT_ARRAY);
  if (init_array == 0) {
    printf ("\n\n***Error: Could not allocate initial array\n");
    return -1;
  }
  memset (init_array, 0, INIT_ARRAY);
  arrayofarrays = malloc (ARRAY_LEN * sizeof (char **));
  if (arrayofarrays == 0) {
    printf ("\n\n***Error: could not allocate arrays\n");
    free (init_array);
    return -1;
  }
  memset (&arrayofarrays[0], 0, ARRAY_LEN * sizeof (char **));
  for (i = 0; i < ARRAY_LEN; i++) {
    this_array = malloc (DATA_LEN);
    if (0 == this_array) {
      /* free some memory right away */
      free (init_array);
      printf ("\n\nBailing because memory is exhausted: i = %d\n", i);
      for (j = 0; j < i; j++) {
	if (arrayofarrays[j] != 0) {
	  free (arrayofarrays[j]);
	  arrayofarrays[j] = 0;
	}
	free (arrayofarrays);
      }
      break;
    }
    memset (this_array, 0, DATA_LEN);
    for (j = 0; j < ARRAY_LEN; j++) {
      if (this_array[j] != 0) {
	for (k = 0; k < i; k++) {
	  if (arrayofarrays[k] != 0) {
	    free (arrayofarrays[k]);
	    arrayofarrays[k] = 0;
	  }
	}
	free (this_array);
	free (arrayofarrays);
	return -1;
      }
    }
    arrayofarrays[i] = this_array;
  }
  return 0;
}
