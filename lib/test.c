/*	Source File:	test.c	*/

#include <test.h>

#include <time.h>
#include <stdlib.h>
#include <string.h>

#include <sort.h>

#ifdef	__BORLANDC__
#define	RANDOM	rand
#define	SRANDOM	srand
#endif

#ifndef	__BORLANDC__
#define	RANDOM	random
#define	SRANDOM	srandom
#endif

/*
 * [BeginDoc]
 * 
 * int testString (char *str, int len, int maxLen);
 * 
 * testString() generates a string of data in the space provided
 * by ``str''.  The data is of length ``len''.  If ``len'' is 0,
 * it is assumed that the user wants a variable length string will
 * be such that 5<=``len''<=``maxLen''.  It is an error if ``len''
 * and ``maxLen'' are both 0.  ``str'' must point to enough space
 * to hold ``len''+1 bytes; if it doesn't, dire consequences will
 * result.  The string produced by testString() is lower-case only.
 * 
 * testString() returns _OK_ on success, _ERROR_ if ``len'' and
 * ``maxLen'' are both 0 or str == 0.
 * 
 * =================================================================
 * [EndDoc]
 */

int testString (char *str, int len, int maxLen)
{
  time_t seed;
  int i;
  static int firstTime = FALSE;

  if (firstTime == FALSE) {
    time (&seed);
    SRANDOM (seed);
    firstTime = TRUE;
  }

  if (len == 0) {
    if (maxLen == 0)
      return _ERROR_;
    len = ((int) RANDOM () % maxLen);
    if (len < 5 && maxLen >= 5)
      len = 5;
  }

  Assert (str != 0);
  if (str == 0)
    return _ERROR_;
  for (i = 0; i < len; i++) {
    str[i] = ((int) RANDOM () % CHAR_MOD) + 'a';
  }
  str[len] = '\0';
  return _OK_;
}

/*
 * [BeginDoc]
 * 
 * int testUpperString (char *str, int len, int maxLen);
 * 
 * testUpperString() functions the same as testString() except the
 * string generated is all upper case.
 * 
 * =================================================================
 * [EndDoc]
 */

int testUpperString (char *str, int len, int maxLen)
{
  time_t seed;
  int i;

  static int firstTime = FALSE;

  if (firstTime == FALSE) {
    time (&seed);
    SRANDOM (seed);
    firstTime = TRUE;
  }

  if (len == 0) {
    if (maxLen == 0)
      return _ERROR_;
    len = ((int) RANDOM () % maxLen);
    if (len < 5 && maxLen >= 5)
      len = 5;
  }

  Assert (str != 0);
  if (str == 0)
    return _ERROR_;
  for (i = 0; i < len; i++) {
    str[i] = ((int) RANDOM () % CHAR_MOD) + 'A';
  }
  str[len] = '\0';
  return _OK_;
}

/*
 * [BeginDoc]
 * 
 * int testMixedString (char *str, int len, int maxLen);
 * 
 * testMixedString() functions the same as testString() and
 * testUpperString() except it generates a string of upper and
 * lower case letters.  The upper-case letters are dispursed in
 * the string in starting at location 0 and traversing constant
 * lengths throughout the string.
 * 
 * =================================================================
 * [EndDoc]
 */

int testMixedString (char *str, int len, int maxLen)
{
  time_t seed;
  int i;
  int upper;
  int modulus = 7;

  static int firstTime = FALSE;

  if (firstTime == FALSE) {
    time (&seed);
    SRANDOM (seed);
    firstTime = TRUE;
  }

  if (len == 0) {
    if (maxLen == 0)
      return _ERROR_;
    len = ((int) RANDOM () % maxLen);
    if (len < 5 && maxLen >= 5)
      len = 5;
  }
  if (maxLen < modulus)
    modulus = maxLen / 2;
  upper = ((int) RANDOM () % 7);
  if (upper < 3)
    upper = 3;

  Assert (str != 0);
  if (str == 0)
    return _ERROR_;
  for (i = 0; i < len; i++) {
    if (!(i % upper))
      str[i] = ((int) RANDOM () % CHAR_MOD) + 'A';
    else
      str[i] = ((int) RANDOM () % CHAR_MOD) + 'a';
  }
  str[len] = '\0';
  return _OK_;
}

/*
 * [BeginDoc]
 * 
 * int testNumber (char *str, int len, int maxLen);
 * 
 * testNumber() works the same as testString, except it generates
 * a string of numerical digits from 0-9.
 * 
 * =================================================================
 * [EndDoc]
 */

int testNumber (char *str, int len, int maxLen)
{
  time_t seed;
  int i;

  static int firstTime = FALSE;

  if (firstTime == FALSE) {
    time (&seed);
    SRANDOM (seed);
    firstTime = TRUE;
  }

  if (len == 0) {
    if (maxLen == 0)
      return _ERROR_;
    len = ((int) RANDOM () % maxLen);
    if (len < 2 && maxLen >= 2)
      len = 2;
  }

  Assert (str != 0);
  if (str == 0)
    return _ERROR_;
  for (i = 0; i < len; i++) {
    str[i] = ((int) RANDOM () % NUMBER_MOD) + '0';
  }
  if (str[0] == '0')
    str[0] = '1';
  str[len] = '\0';
  return _OK_;
}

/*
 * [BeginDoc]
 * 
 * int testNumString (char *str, int len, int maxLen);
 * 
 * testNumString() functions the same as testString() and
 * testUpperString() except it generates a string of upper
 * case letters and numbers.  The numbers are dispursed in
 * the string in starting at location 0 and traversing constant
 * lengths throughout the string.
 * 
 * =================================================================
 * [EndDoc]
 */
int testNumString (char *str, int len, int maxLen)
{
  time_t seed;
  int i;
  int number;
  int modulus = 4;

  static int firstTime = FALSE;

  if (firstTime == FALSE) {
    time (&seed);
    SRANDOM (seed);
    firstTime = TRUE;
  }

  if (len == 0) {
    if (maxLen == 0)
      return _ERROR_;
    len = ((int) RANDOM () % maxLen);
    if (len < 5 && maxLen >= 5)
      len = 5;
  }
  if (maxLen < modulus)
    modulus = maxLen / 2;
  number = ((int) RANDOM () % 4);
  if (number < 3)
    number = 3;

  Assert (str != 0);
  if (str == 0)
    return _ERROR_;
  for (i = 0; i < len; i++) {
    if (!(i % number))
      str[i] = ((int) RANDOM () % NUMBER_MOD) + '0';
    else
      str[i] = ((int) RANDOM () % CHAR_MOD) + 'A';
  }
  str[len] = '\0';
  return _OK_;
}

/*
 * [BeginDoc]
 * 
 * To test the test functions, compile test.c with TEST_TEST defined
 * and an executable binary will be generated that will exercise
 * all the test functions in a minimal way.
 * 
 * [EndDoc]
 */
#ifdef	TEST_TEST

#include <stdio.h>

int main (void)
{
  char str[100];
  int status;
  int i;

  memset (str, 0, 100);
  printf ("\n10 constant length numbers\n");
  for (i = 0; i < 10; i++) {
    status = testNumber (str, 20, 0);
    if (_ERROR_ == status) {
      printf ("\n\n***Error returned by testNumber\n");
      return _ERROR_;
    }
    printf ("%s\n", str);
  }
  printf ("\n10 variable length numbers\n");
  for (i = 0; i < 10; i++) {
    status = testNumber (str, 0, 20);
    if (_ERROR_ == status) {
      printf ("\n\n***Error returned by testNumber\n");
      return _ERROR_;
    }
    printf ("%s\n", str);
  }
  memset (str, 0, 100);
  printf ("\n10 constant length lower case strings\n");
  for (i = 0; i < 10; i++) {
    status = testString (str, 40, 0);
    if (_ERROR_ == status) {
      printf ("\n\n***Error returned by testString\n");
      return _ERROR_;
    }
    printf ("%s\n", str);
  }
  printf ("\n10 variable length lower case strings\n");
  for (i = 0; i < 10; i++) {
    status = testString (str, 0, 40);
    if (_ERROR_ == status) {
      printf ("\n\n***Error returned by testString\n");
      return _ERROR_;
    }
    printf ("%s\n", str);
  }
  memset (str, 0, 100);
  printf ("\n10 constant length upper case strings\n");
  for (i = 0; i < 10; i++) {
    status = testUpperString (str, 40, 0);
    if (_ERROR_ == status) {
      printf ("\n\n***Error returned by testUpperString\n");
      return _ERROR_;
    }
    printf ("%s\n", str);
  }
  printf ("\n10 variable length upper case strings\n");
  for (i = 0; i < 10; i++) {
    status = testUpperString (str, 0, 40);
    if (_ERROR_ == status) {
      printf ("\n\n***Error returned by testUpperString\n");
      return _ERROR_;
    }
    printf ("%s\n", str);
  }
  memset (str, 0, 100);
  printf ("\n10 constant length mixed case strings\n");
  for (i = 0; i < 10; i++) {
    status = testMixedString (str, 40, 0);
    if (_ERROR_ == status) {
      printf ("\n\n***Error returned by testMixedString\n");
      return _ERROR_;
    }
    printf ("%s\n", str);
  }
  printf ("\n10 variable length mixed case strings\n");
  for (i = 0; i < 10; i++) {
    status = testMixedString (str, 0, 40);
    if (_ERROR_ == status) {
      printf ("\n\n***Error returned by testMixedString\n");
      return _ERROR_;
    }
    printf ("%s\n", str);
  }
  return _OK_;
}

#endif /* TEST_TEST */
