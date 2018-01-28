/* Source File: flognum2-gen.c */

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
	int scale;
	int numsize;
	int numiterations;
	int i;
	char *ops="+-/x";
	int op;
	char *cp;
	char num1[1024]; // arbitrarily large
	char num2[1024]; // arbitrarily large
	FILE *fp;

	if (argc != 2) {
		fprintf (stderr, "\n\nUsage: %s num\n"
				"  where num is number of items to generate.\n", argv[0]);
		return _ERROR_;
	}
	numiterations = atoi (argv[1]);
	if (numiterations < 0) {
		fprintf (stderr, "\n\nBad argument %s\n", argv[1]);
		return _ERROR_;
	}
	// First, figure out scale.
	cp = dbtestnumber (2, 0);
	scale = atoi (cp);
	scale %= 20;
	// Now, figure out the numsize.
	cp = dbtestnumber (2, 0);
	numsize = atoi (cp);
	numsize += scale;
	// Create and open the file.
	fp = fopen ("testAdd.txt", "w");
	if (0 == fp) {
		printf ("\n\n***Error: could not open testAdd.txt\n");
		return _ERROR_;
	}
	for (i = 0; i < numiterations; i++) {
	  // OK, snarf the numbers for this iteration.
  	cp = dbtestnumber (numsize, 0);
  	memset (num1, 0, 1024);
  	strncpy (num1, cp, 1000);
		num1[numsize-scale-1] = '.';
  	cp = dbtestnumber (0, numsize);
  	memset (num2, 0, 1024);
  	strncpy (num2, cp, 1000);
		if (strlen (num2) > (unsigned) scale)
		  num2[strlen(num2)-scale-1] = '.';
		// Get an operator for this iteration.
		cp = dbtestnumber (4, 0);
		op = atoi (cp);
		op %= 4;
		// Shorten the second number to something more reasonable
		// for multiplication
		if (ops[op] == 'x') {
			num2[4] = '\0';
		}
		// Print the suckers.
		fprintf (fp, "%d:%s:%c:%s\n",
				scale, num1, ops[op], num2);
	}
	fclose (fp);
	return 0;
}
