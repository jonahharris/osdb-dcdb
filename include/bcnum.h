/* Source File: bcnum.h */

#ifndef __BCNUM_H__
#define __BCNUM_H__

#include <number.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>

#include <dcdb_config.h>

#define BCNUM_OUTSTRING_SIZE 128
extern int bcnum_outstr_ptr;
extern char bcnum_outstr[];
extern int bcnum_is_init;

typedef enum _bcnum_error_type {
  BCNUM_NOERROR,                  /* no problema */
  BCNUM_MEMORY,                   /* memory exhausted */
  BCNUM_TOOSMALL,                 /* output string is too small */
  /* new ones here */
  BCNUM_UNSPECIFIED               /* unspecified error */
} bcnumErrorType;

extern bcnumErrorType bcnumError;
extern char *bcnumErrMsg[];

void out_char_str (int c);
void pn_str (bc_num num);
void rt_error (char *mesg, ...);
void rt_warn (char *mesg, ...);
void out_of_memory (void);

char *bcnum_add (char *n1, char *n2, int scale);
char *bcnum_sub (char *n1, char *n2, int scale);
int bcnum_compare (char *n1, char *n2, int scale);
char *bcnum_multiply (char *n1, char *n2, int scale);
char *bcnum_divide (char *n1, char *n2, int scale);
char *bcnum_raise (char *n1, char *n2, int scale);
int bcnum_iszero (char *n1);
int bcnum_isnearzero (char *n1, int scale);
int bcnum_isneg (char *n1);
void bcnum_uninit (void);

#endif /* __BCNUM_H__ */
