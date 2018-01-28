/* Source File: flognum1.c */

#ifdef __CINT__
#include "cint_types.h"
#endif

#ifndef __CINT__
#include <stdio.h>
#include <stdlib.h>
#include <interface.h>
#endif

int main (int argc, char *argv[])
{
  int status;
  int i, counter;
  int numIterations;
  char *cp;
  long value, value1;
  long sub;
  char val[1024];
  char line[1024];

  // check the command-line
  if (argc != 2) {
    printf ("\n\nUsage: %s numIterations\n", argv[0]);
    printf ("\twhere numIterations is the number of times to iterate for each test\n");
    return 1;
  }
  numIterations = atoi (argv[1]);
  if (numIterations <= 0) {
    printf ("\n\n***Error: improper value for numIterations: %d\n", 
	numIterations);
    return _ERROR_;
  }
  // ajust numIterations if it is too low or high
  if (numIterations < 2)
    numIterations = 2;
  if (numIterations > 100)
    numIterations = 100;

  memset (val, 0, 200);

  /*
   * OK, let the user know what the heck is going on and lock and load.
   */
  printf ("\n***Exercise dbnumadd...\n");
  for (i = 1; i <= numIterations; i++) {
    printf ("Iteration %d\n", i);
    cp = dbtestnumber (9, 0);
    strcpy (val, cp);
    strcat (val, ".00");
    value = atol (val);
    counter = 0;
    while (value > 0) {
      counter ++;
      if (counter % 10000 == 0) {
	printf (".");
	fflush (stdout);
      }
      value1 = value;
      cp = dbtestnumber (4, 0);
      sub = atol (cp);
      strcpy (line, "-");
      strcat (line, cp);
      cp = bcnumadd (val, line, 2);
      if (cp == 0) {
	printf ("\n\n***Error: bcnumadd (%s, %s, 2): %s\n",
	    val,line,cdberror);
	bcnumuninit();
	return _ERROR_;
      }
      strcpy (line, cp);
      value = atol (line);
      value1 -= sub;
      sprintf (val, "%ld", value);
      sprintf (line, "%ld", value1);
      status = bcnumcompare (val, line, 2);
      if (status != 0) {
	printf ("%s and %s are not equal and they should be\n",val, line);
      }
    }
    printf ("\ncounter = %d\n", counter);
  }
  // Maybe include the other tests later...or, maybe not.

  bcnumuninit ();

  print_block_list ();
  return 0;
}
