/* Source File: test_number.c */

/*
 * Designed to test the bc number.c library.  My intent is to separate it from
 * bc and use it in my library.
 */

#include <stdio.h>
#include <string.h>
#include <bcnum.h>

int bcnum_outstr_ptr = 0;
char bcnum_outstr[BCNUM_OUTSTRING_SIZE+1];
int bcnum_is_init = 0;

bcnumErrorType bcnumError = BCNUM_NOERROR;
char *bcnumErrMsg[] = {
  "no bcnum error",                           /* BCNUM_NOERROR */
  "memory exhausted",                         /* BCNUM_MEMORY */
  "output string is too small",               /* BCNUM_TOOSMALL */
  /* new ones here */
  "unspecified bcnum error",                  /* BCNUM_UNSPECIFIED */
  NULL
};

void out_char_str (int c)
{
  bcnum_outstr[bcnum_outstr_ptr] = (char)c;
  bcnum_outstr_ptr++;
  if (bcnum_outstr_ptr >= BCNUM_OUTSTRING_SIZE) {
    bcnumError = BCNUM_TOOSMALL;
    memset (bcnum_outstr, 0, BCNUM_OUTSTRING_SIZE);
    bcnum_outstr_ptr = 0;
    return;
  }
  bcnum_outstr[bcnum_outstr_ptr] = '\0';
}

void pn_str (bc_num num)
{
  bc_out_num (num, 10, out_char_str, 0);
}

void rt_error (char *mesg, ...)
{
  int len;
  va_list args;

#ifdef HAVE_SNPRINTF
  snprintf (bcnum_outstr, BCNUM_OUTSTRING_SIZE, "***Error: %s: ",
      bcnumErrMsg[bcnumError]);
#else
  sprintf (bcnum_outstr, "***Error: %s: ", bcnumErrMsg[bcnumError]);
#endif
  va_start (args, mesg);
  len = strlen (bcnum_outstr);
#ifdef HAVE_SNPRINTF
  vsnprintf (bcnum_outstr+len, BCNUM_OUTSTRING_SIZE-10, mesg, args);
#else
  sprintf (bcnum_outstr+len, mesg, args);
#endif
  va_end (args);
}

void rt_warn (char *mesg, ...)
{
  int len;
  va_list args;

#ifdef HAVE_SNPRINTF
  snprintf (bcnum_outstr, BCNUM_OUTSTRING_SIZE, "***Warning: %s: ",
      bcnumErrMsg[bcnumError]);
#else
  sprintf (bcnum_outstr, "***Warning: %s: ", bcnumErrMsg[bcnumError]);
#endif
  va_start (args, mesg);
  len = strlen (bcnum_outstr);
#ifdef HAVE_SNPRINTF
  vsnprintf (bcnum_outstr+len, BCNUM_OUTSTRING_SIZE-12, mesg, args);
#else
  vsprintf (bcnum_outstr+len, mesg, args);
#endif
  va_end (args);
}

void out_of_memory (void)
{
  bcnumError = BCNUM_MEMORY;
}

void bcnum_init (void)
{
  bc_init_numbers();
}

char *bcnum_add (char *n1, char *n2, int scale)
{
  bc_num num1, num2, sum;

  if (bcnum_is_init == 0) {
    bcnum_init ();
    bcnum_is_init = 1;
  }
  bcnum_outstr_ptr = 0;
  num1 = bc_copy_num (_zero_);
  num2 = bc_copy_num (_zero_);
  sum = bc_copy_num (_zero_);

  bc_str2num (&num1, n1, scale);
  bc_str2num (&num2, n2, scale);

  bc_add (num1, num2, &sum, scale);
  pn_str (sum);

  bc_free_num (&num1);
  bc_free_num (&num2);
  bc_free_num (&sum);

  if (bcnumError != BCNUM_NOERROR)
    return 0;
  return bcnum_outstr;
}

char *bcnum_sub (char *n1, char *n2, int scale)
{
  bc_num num1, num2, sum;

  if (bcnum_is_init == 0) {
    bcnum_init ();
    bcnum_is_init = 1;
  }
  bcnum_outstr_ptr = 0;
  num1 = bc_copy_num (_zero_);
  num2 = bc_copy_num (_zero_);
  sum = bc_copy_num (_zero_);

  bc_str2num (&num1, n1, scale);
  bc_str2num (&num2, n2, scale);

  bc_sub (num1, num2, &sum, scale);
  pn_str (sum);

  bc_free_num (&num1);
  bc_free_num (&num2);
  bc_free_num (&sum);

  if (bcnumError != BCNUM_NOERROR)
    return 0;
  return bcnum_outstr;
}

int bcnum_compare (char *n1, char *n2, int scale)
{
  int status;
  bc_num num1, num2;

  if (bcnum_is_init == 0) {
    bcnum_init ();
    bcnum_is_init = 1;
  }
  bcnum_outstr_ptr = 0;
  num1 = bc_copy_num (_zero_);
  num2 = bc_copy_num (_zero_);

  bc_str2num (&num1, n1, scale);
  bc_str2num (&num2, n2, scale);

  status = bc_compare (num1, num2);

  bc_free_num (&num1);
  bc_free_num (&num2);

  return status;
}

char *bcnum_multiply (char *n1, char *n2, int scale)
{
  bc_num num1, num2, prod;

  if (bcnum_is_init == 0) {
    bcnum_init ();
    bcnum_is_init = 1;
  }
  bcnum_outstr_ptr = 0;
  num1 = bc_copy_num (_zero_);
  num2 = bc_copy_num (_zero_);
  prod = bc_copy_num (_zero_);

  bc_str2num (&num1, n1, scale);
  bc_str2num (&num2, n2, scale);

  bc_multiply (num1, num2, &prod, scale);
  pn_str (prod);

  bc_free_num (&num1);
  bc_free_num (&num2);
  bc_free_num (&prod);

  if (bcnumError != BCNUM_NOERROR)
    return 0;
  return bcnum_outstr;
}

char *bcnum_divide (char *n1, char *n2, int scale)
{
  bc_num num1, num2, div;

  if (bcnum_is_init == 0) {
    bcnum_init ();
    bcnum_is_init = 1;
  }
  bcnum_outstr_ptr = 0;
  num1 = bc_copy_num (_zero_);
  num2 = bc_copy_num (_zero_);
  div = bc_copy_num (_zero_);

  bc_str2num (&num1, n1, scale);
  bc_str2num (&num2, n2, scale);

  bc_divide (num1, num2, &div, scale);
  pn_str (div);

  bc_free_num (&num1);
  bc_free_num (&num2);
  bc_free_num (&div);

  if (bcnumError != BCNUM_NOERROR)
    return 0;
  return bcnum_outstr;
}

char *bcnum_raise (char *n1, char *n2, int scale)
{
  bc_num num1, num2, npow;

  if (bcnum_is_init == 0) {
    bcnum_init ();
    bcnum_is_init = 1;
  }
  bcnum_outstr_ptr = 0;
  num1 = bc_copy_num (_zero_);
  num2 = bc_copy_num (_zero_);
  npow = bc_copy_num (_zero_);

  bc_str2num (&num1, n1, scale);
  bc_str2num (&num2, n2, scale);

  bc_raise (num1, num2, &npow, scale);
  pn_str (npow);

  bc_free_num (&num1);
  bc_free_num (&num2);
  bc_free_num (&npow);

  if (bcnumError != BCNUM_NOERROR)
    return 0;
  return bcnum_outstr;
}

int bcnum_iszero (char *n1)
{
  int status;
  bc_num num1;

  if (bcnum_is_init == 0) {
    bcnum_init ();
    bcnum_is_init = 1;
  }
  bcnum_outstr_ptr = 0;
  num1 = bc_copy_num (_zero_);
  bc_str2num (&num1, n1, 0);
  status = bc_is_zero (num1);
  bc_free_num (&num1);
  return status;
}

int bcnum_isnearzero (char *n1, int scale)
{
  int status;
  bc_num num1;

  if (bcnum_is_init == 0) {
    bcnum_init ();
    bcnum_is_init = 1;
  }
  bcnum_outstr_ptr = 0;
  num1 = bc_copy_num (_zero_);
  bc_str2num (&num1, n1, scale);
  status = bc_is_near_zero (num1, scale);
  bc_free_num (&num1);
  return status;
}

int bcnum_isneg (char *n1)
{
  int status;
  bc_num num1;

  if (bcnum_is_init == 0) {
    bcnum_init ();
    bcnum_is_init = 1;
  }
  bcnum_outstr_ptr = 0;
  num1 = bc_copy_num (_zero_);
  bc_str2num (&num1, n1, 0);
  status = bc_is_neg (num1);
  bc_free_num (&num1);
  return status;
}

void bcnum_uninit (void)
{
  if (bcnum_is_init == 1) {
    if (_zero_->n_ptr)
      free (_zero_->n_ptr);
    free (_zero_);
    _zero_ = 0;
    if (_one_->n_ptr)
      free (_one_->n_ptr);
    free (_one_);
    _one_ = 0;
    if (_two_->n_ptr)
      free(_two_->n_ptr);
    free (_two_);
    _two_ = 0;
    bcnum_is_init = 0;
  }
}  

#ifdef TEST_BCNUM
char *invals[] = {
  "20000.00",
  "-1847.20",     /* 18152.80 */
  "-911.00",      /* 17241.80 */
  "-335.48",      /* 16906.32 */
  "-2238.67",     /* 14667.65 */
  "-10897.60",    /* 3770.05 */
  0
};

char *outvals[] = {
  "",
  "18152.80",
  "17241.80",
  "16906.32",
  "14667.65",
  "3770.05",
  0
};

int main (void)
{
  int status;
  char *v;
  char val[BCNUM_OUTSTRING_SIZE+1];
  int i;

  strcpy (val, invals[0]);
  for (i = 1; invals[i] != 0; i++) {
    printf ("Adding %s and %s:", val, invals[i]);
    v = bcnum_add (val, invals[i], 2);
    strcpy (val, v);
    if (bcnumError != BCNUM_NOERROR) {
      printf ("\n\n***Error: bcnum_sum(%s,%s): %s\n",invals[i-1], invals[i],
	  bcnumErrMsg[bcnumError]);
      return -1;
    }
    status = bcnum_compare (val, outvals[i], 2);
    if (bcnumError != BCNUM_NOERROR) {
      printf ("\n\n***Error: bcnum_sum(%s,%s): %s\n",invals[i-1], invals[i],
	  bcnumErrMsg[bcnumError]);
      return -1;
    }
    if (status) {
      printf ("\n\n***Warning: values %s and %s should be equal, but they are not\n",
	  val, outvals[i]);
    }
    printf ("result = %s\n", val);
  }
  return 0;
}
#endif
