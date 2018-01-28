/*	Source File:	mkdata.c	*/

#include <stdio.h>

#include <test.h>

#ifndef	DEBUG
#define	NUMBER	50000
#endif

#ifdef	DEBUG
#define	NUMBER	2000
#endif

/*
 * Makes data for testing using beatupadd.c
 */

int main (void)
{
  char str[100];
  FILE *fp;
  int counter = 0;

  fp = fopen ("testAdd.txt", "w");
  while (counter++ < NUMBER) {
    /* LName */
    testUpperString (str, 0, 64);
    fprintf (fp, "%s:", str);
    /* SSNumber */
    testNumber (str, 9, 0);
    fprintf (fp, "%s:", str);
    /* FName */
    testUpperString (str, 0, 34);
    fprintf (fp, "%s:", str);
    /* StreetAddress */
    testString (str, 75, 0);
    fprintf (fp, "%s:", str);
    /* City */
    testString (str, 45, 0);
    fprintf (fp, "%s:", str);
    /* State */
    fprintf (fp, "%s:", "NM");
    /* Zip */
    testNumber (str, 10, 0);
    fprintf (fp, "%s:", str);
    /* AnnualSalary */
    testNumber (str, 10, 0);
    fprintf (fp, "%s:", str);
    /* HomeOwner */
    fprintf (fp, "%s:", "Y");
    /* DateApplied */
    testNumber (str, 8, 0);
    fprintf (fp, "%s:", str);
    /* LastUpdated */
    testString (str, 26, 0);
    fprintf (fp, "%s\n", str);
  }
  fclose (fp);
  return 0;
}
