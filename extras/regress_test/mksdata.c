/*	Source File:	mksdata.c	*/

#include <stdio.h>

#include <test.h>

#ifndef	DEBUG
#define	NUMBER	50000
#endif

#ifdef	DEBUG
#define	NUMBER	100
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
	fprintf (fp, "%s\n",
			 "May:858713945:David:1721 Nowhere NE:Albuquerque:NM:87112:40000.00:Y:19991212:Mon Dec 13 13:01:16 MST 1999");
	while (counter++ < NUMBER-1)	{
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
