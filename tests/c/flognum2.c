/* Source File: flognum2.c */

/*
 * Make a program that takes three arguments and prints a value on the console.
 * For example, it could work as follows:
 * ./flognum 2 198332.29 - 32.29
 * 198300.00
 *
 * Here, '2' is the scale, '198332.29' and '32.29' are numbers, and '-' is
 * the operation.
 *
 * Then, the flognum2-gen.c program generates data sets with numbers and
 * operators (this will be called by flognum2.sh).  Then, flognum2.sh parses
 * the data sets and calls flognum2 with them and calls bc and compares the
 * output.
 */

/*
 * XXX - I am just ignoring the scenerio where the output is so big that it
 * can't fit in the result string.  I don't believe this is a real problem,
 * but I need to give it some thought.  Do I really want to do that, or do
 * I want to arbitrarily increase the size of the output string?
 */
#ifdef __CINT
#include "cint_types.h"
#endif

#ifndef __CINT__
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <interface.h>
#endif

int check_number (char *num)
{
  char *cp;
  cp = num;
  while (*cp != '\0') {
    if (!isdigit (*cp) && *cp != '.')
      return FALSE;
    cp++;
  }
  return TRUE;
}

void usage (char *prog)
{
  fprintf (stderr,
	   "\n\nUsage: %s scale num [+-x/^] num\n"
	   "where scale is the decimal scale (0-20)\n"
	   "and num is a decimal real number\n"
	   "The operation indicated will be applied\n"
	   "to the numbers in the expected (algebraic) way\n", prog);
}

int main (int argc, char *argv[])
{
  char val[1024];		// arbitrarily large
  char *cp;
  int scale;

  if (argc != 5) {
    usage (argv[0]);
    return 1;
  }
  if (check_number (argv[2]) == FALSE || check_number (argv[4]) == FALSE) {
    fprintf (stderr, "\n\nBad number:");
    usage (argv[0]);
    return 2;
  }
  if (check_number (argv[1]) == FALSE) {
    fprintf (stderr, "\n\nBad scale:");
    usage (argv[0]);
    return 3;
  }
  scale = atoi (argv[1]);
  if (scale < 0 || scale > 20) {
    fprintf (stderr, "\n\nBad scale:");
    usage (argv[0]);
    return 3;
  }
  switch (argv[3][0]) {
  case '+':
    cp = bcnumadd (argv[2], argv[4], scale);
    if (cp == 0) {
			if (!strcmp (cdberror, "bcnum error: output string is too small"))
				// just bail
				return 4;
      fprintf (stderr, "\n\n***Error: bcnumadd (%s, %s, %d): %s\n",
	       argv[2], argv[4], scale, cdberror);
      bcnumuninit ();
      return _ERROR_;
    }
    memset (val, 0, 1024);
    strncpy (val, cp, 1000);
    printf ("%s\n", val);
    break;
  case '-':
    cp = bcnumsub (argv[2], argv[4], scale);
    if (cp == 0) {
			if (!strcmp (cdberror, "bcnum error: output string is too small"))
				// just bail
				return 4;
      fprintf (stderr, "\n\n***Error: bcnumsub (%s, %s, %d): %s\n",
	       argv[2], argv[4], scale, cdberror);
      bcnumuninit ();
      return _ERROR_;
    }
    memset (val, 0, 1024);
    strncpy (val, cp, 1000);
    printf ("%s\n", val);
    break;
  case 'x':
    cp = bcnummultiply (argv[2], argv[4], scale);
    if (cp == 0) {
			if (!strcmp (cdberror, "bcnum error: output string is too small"))
				// just bail
				return 4;
      fprintf (stderr, "\n\n***Error: bcnummultiply (%s, %s, %d): %s\n",
	       argv[2], argv[4], scale, cdberror);
      bcnumuninit ();
      return _ERROR_;
    }
    memset (val, 0, 1024);
    strncpy (val, cp, 1000);
    printf ("%s\n", val);
    break;
  case '/':
    cp = bcnumdivide (argv[2], argv[4], scale);
    if (cp == 0) {
			if (!strcmp (cdberror, "bcnum error: output string is too small"))
				// just bail
				return 4;
      fprintf (stderr, "\n\n***Error: bcnumdivide (%s, %s, %d): %s\n",
	       argv[2], argv[4], scale, cdberror);
      bcnumuninit ();
      return _ERROR_;
    }
    memset (val, 0, 1024);
    strncpy (val, cp, 1000);
    printf ("%s\n", val);
    break;
  default:
    fprintf (stderr, "\n\nInvalid operator: %s", argv[3]);
    usage (argv[0]);
    return _ERROR_;
    break;
  }
  //
  // Note: the following is needed to clean up data from the bcnum* functions.
  //
  bcnumuninit ();
  return 0;
}
