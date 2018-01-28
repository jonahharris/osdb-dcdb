/* Source File: mkdata.c */

#ifdef __CINT__
#include "cint_types.h"
#endif

#ifndef __CINT__
#include <stdio.h>
#include <interface.h>
#define MAXRECORDS 1000000
#endif

#define DEFAULT_NUMBER 10000

int main (int argc, char *argv[])
{
  char *str;
  FILE *fp;
  int counter=0;
  int count = DEFAULT_NUMBER;
  int smalldate;

  if (argc > 1) {
    count = atoi (argv[1]);
    if (count < 0 || count > MAXRECORDS) {
      count = 10000;
    }
  }

  fp = fopen ("testAdd.txt", "w+b");
  while (counter++ < count) {
    /* LName */
    str = dbtestupperstring (0, 64);
    if (str == 0) {
      printf ("\n\n***Error: LName: %s\n", cdberror);
      return _ERROR_;
    }
    fprintf(fp, "%s:", str);
    /* SSNumber */
    str = dbtestnumber (9, 0);
    if (str == 0) {
      printf ("\n\n***Error: SSNumber: %s\n", cdberror);
      return _ERROR_;
    }
    fprintf(fp, "%s:", str);
    /* FName */
    str = dbtestupperstring (0, 34);
    if (str == 0) {
      printf ("\n\n***Error: FName: %s\n", cdberror);
      return _ERROR_;
    }
    fprintf(fp, "%s:", str);
    /* StreetAddress */
    str = dbteststring (75, 0);
    if (str == 0) {
      printf ("\n\n***Error: StreetAddress: %s\n", cdberror);
      return _ERROR_;
    }
    fprintf(fp, "%s:", str);
    /* City */
    str = dbteststring (45, 0);
    if (str == 0) {
      printf ("\n\n***Error: City: %s\n", cdberror);
      return _ERROR_;
    }
    fprintf(fp, "%s:", str);
    /* State */
    fprintf(fp, "%s:", "NM");
    /* Zip */
    str = dbtestnumber (10, 0);
    if (str == 0) {
      printf ("\n\n***Error: Zip: %s\n", cdberror);
      return _ERROR_;
    }
    fprintf(fp, "%s:", str);
    /* AnnualSalary */
    str = dbtestnumber (5, 0);
    if (str == 0) {
      printf ("\n\n***Error: FName: %s\n", cdberror);
      return _ERROR_;
    }
    fprintf(fp, "%s.00:", str);
    /* HomeOwner */
    fprintf (fp, "Y:");
    /* DateApplied */
    str = dbtestnumber (6, 0);
    if (str == 0) {
      printf ("\n\n***Error: FName: %s\n", cdberror);
      return _ERROR_;
    }
    smalldate = atoi (str);
    if (smalldate < 0 || smalldate > 991231)
      smalldate = 651010; /* arbitrary birthdate */
    smalldate += 19000000;
    fprintf(fp, "%d:", smalldate);
    /* LastUpdated */
    str = dbtestnumber (4, 0);
    if (str == 0) {
      printf ("\n\n***Error: FName: %s\n", cdberror);
      return _ERROR_;
    }
    smalldate = atoi (str);
    if (smalldate < 0 || smalldate > 1231)
      smalldate = 1010;
    smalldate += 20020000;
    fprintf(fp, "%d\n", smalldate);
  }
  fclose (fp);
  return 0;
}
