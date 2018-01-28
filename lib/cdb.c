/* Source File: cdb.c */

#include <interface.h>
#include <stdio.h>
#ifndef	WIN32
#include <unistd.h>
#endif
#include <sort.h>

extern int bcnum_is_init;
void bcnum_uninit(void);

/*
 * [BeginDoc]
 *
 * \section{High-level C API Functions}
 *
 * This section discusses the high-level interface functions for C
 * programmers.  This is the recommended method for working with DCDB
 * using C programs.  There are numerous examples of using DCDB with
 * this Application Programmer Interface (API) in (topdir)/tests/c.
 * You should study this C code to see how things are done using the
 * high-level APIs.  What follows should just be considered reference
 * information.
 *
 * [EndDoc]
 */

/*
 ******************************************************************
 * Private data items
 ******************************************************************
 */

#define	MAX_DIR_SIZE	1024
#define	RESULT_SIZE	MAX_DIR_SIZE
#define	MAX_BLOWFISH	2048

typedef struct cdb_table_handle_ {
  char handle[TABLE_NAME_WIDTH + 1];
  char cwd[MAX_DIR_SIZE + 1];
  dbTable *tbl;
} cdbTableHandle;		/* handle to an open table */

				/*ListHeader *handles;*//* list of open tables */

#define	MAX_HANDLES 20

int num_handles = 0;
static cdbTableHandle *handles[MAX_HANDLES] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static char rslt[RESULT_SIZE + 1];
static char bf_rslt[MAX_BLOWFISH * 2 + 1];

/*
 ******************************************************************
 * Global data items
 ******************************************************************
 */

/*
 * [BeginDoc]
 *
 * Error conditions are communicated via function return values.  If an error
 * occurs, as indicated by the return value, the variable cdberror will have
 * a description of the error.  cdberror is defined as follows:
 * [Verbatim] */

char cdberror[RESULT_SIZE + 1];

/* [EndDoc] */

/*
 ******************************************************************
 */

/* 
 * internal - don't document 
 */
static void my_atexit (void)
{
  (void)dbexit();
  if (bcnum_is_init != 0)
    bcnum_uninit();
}

/*
 * internal - don't document
 */
static cdbTableHandle *findHandle (const char *handle)
{
  int i;

  if (handle == 0 || *handle == '\0')	/* bad parameter */
    return 0;
  if (num_handles == 0 || handles[0] == 0)	/* array empty */
    return 0;
  for (i = 0; i < num_handles; i++) {
    if (!strcmp (handles[i]->handle, handle))
      return handles[i];
  }
  return 0;
}

char *dbgeterror (void)
{
  // just return the global error condition.
  return cdberror;
}

/*
 * [BeginDoc]
 * \subsection{dbcreate}
 * \index{dbcreate}
 *
 * [Verbatim] */

int dbcreate (char *dffile)

/* [EndDoc] */
/*
 * [BeginDoc]
 * The following is an example that shows how you would use dbcreate() to
 * create tables and indexes:
 *
 * [Verbatim] 
int status;
status = dbcreate("deffile.df");
if (status == -1) {
  Process the error...
}

 [EndDoc] */
/*
 * [BeginDoc]
 * dbcreate creates the tables and workspaces defined
 * in the definition file given
 * by ``deffile.df''.  If any error occurs, an error (-1) is returned.  If
 * dbcreate is successful, 0 is returned.
 *
 * For practical examples of using dbcreate, you should study the code
 * in (topdir)/tests/c.  Basically, the scripts do the following:
 *
 * \begin{itemize}
 * \item open a file (flogadd1.df, for example),
 * \item write the code into the file to create the needed tables
 * and indexes,
 * \item close the file,
 * \item call dbcreate with the file name as an argument (
 * ``dbcreate flogadd1.df'', for example),
 * \item and remove the .df file
 * \end{itemize}
 *
 * Embedding this logic into your program is the simplest way to insure
 * that the .df is created when needed.
 *
 * [EndDoc]
 */
{
  if (dffile == 0 || dffile[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbcreate definitionfile");
#else
    (void)sprintf (cdberror, "Usage: dbcreate definitionfile");
#endif
    return _ERROR_;
  }
  if (!fexists (dffile)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "File %s doesn't exist", dffile);
#else
    (void)sprintf (cdberror, "File %s doesn't exist", dffile);
#endif
    return _ERROR_;
  }
  /*
   * Parse the def file and handle errors.
   */
  parseDBDef (dffile);
  if (isDBError ()) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Parsing %s: %s", dffile, dberror ());
#else
    (void)sprintf (cdberror, "Parsing %s: %s", dffile, dberror ());
#endif
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{dbopen}
 * \index{dbopen}
 *
 * [Verbatim] */

const char *dbopen (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbopen opens the table given by ``tbl''.  It is an error if the table
 * doesn't exist.  Upon success, dbopen returns a read-only pointer to the name
 * of a handle.  That handle is to be used for all other table access functions.  This
 * value must be copied to a char array, because many of the access functions use
 * the same static storage space to return results as the array the handle name is
 * returned in.  The handle is really just the name of the table, so in a pinch you
 * can replace the handle name with ``tbl'' (in this example).
 *
 * If an error occurs, dbopen returns a NULL pointer and cdberror contains
 * a description of the error.
 * 
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  char *pwd;
  static int atexit_set = FALSE;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbopen tablename");
#else
    (void)sprintf (cdberror, "Usage: dbopen tablename");
#endif
    return NULL;
  }
  if (!fexists (tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "File %s doesn't exist", tbl);
#else
    (void)sprintf (cdberror, "File %s doesn't exist", tbl);
#endif
    return NULL;
  }
  /*
   * Open the table and return the handle or deal with errors.
   */
  hdl = (cdbTableHandle *) malloc (sizeof (cdbTableHandle));
  if (hdl == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "cdbopen: Memory error allocating a handle");
#else
    (void)sprintf (cdberror, "cdbopen: Memory error allocating a handle");
#endif
    return NULL;
  }
  memset (hdl, 0, sizeof (cdbTableHandle));
  check_pointer (hdl);
  hdl->tbl = openTable (tbl);
  if (isDBError ()) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Opening table %s: %s", tbl, dberror ());
#else
    (void)sprintf (cdberror, "Opening table %s: %s", tbl, dberror ());
#endif
    return NULL;
  }
  if (!atexit_set) {
    atexit (my_atexit);
    atexit_set = TRUE;
  }
  strncpy (hdl->handle, tbl, TABLE_NAME_WIDTH);
  pwd = getcwd (hdl->cwd, MAX_DIR_SIZE);
  if (pwd == 0)
    memset (hdl->cwd, 0, MAX_DIR_SIZE);
  handles[num_handles] = hdl;
  num_handles++;
  strncpy (rslt, tbl, 1020);
  return rslt;
}

/*
 * [BeginDoc]
 *
 * \subsection{dbclose}
 * \index{dbclose}
 *
 * [Verbatim] */

int dbclose (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbclose closes the table given by the argument ``tbl'',
 * which is actually a table handle.  It
 * is an error if the table is not open.  Upon successful completion, a table and
 * all it's associated indexes will be properly flushed to disk and closed and 0 is
 * returned.  If an error occurs, dbclose returns -1 and cdberror will contain a
 * read-only string that describes the error.
 *
 * It is entirely permissible to use dbexit to close all open tables right
 * before exiting, should you choose to do so.
 * 
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  char *pwd, cwd[MAX_DIR_SIZE + 1];
  int status;
  int must_close_table = FALSE;
  int i, j;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbclose tablehandle");
#else
    (void)sprintf (cdberror, "Usage: dbclose tablehandle");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    (void)sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  /*
   * Remove the table from the array and close it.
   */
  for (i = 0; i < num_handles; i++) {
    if (!strcmp (handles[i]->handle, tbl))
      break;
  }
  hdl = handles[i];
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "closing table %s: table handle not found", tbl);
#else
    (void)sprintf (cdberror, "closing table %s: table handle not found", tbl);
#endif
    return _ERROR_;
  }
  handles[i] = 0;
  for (j = i; j < num_handles; j++) {
    handles[j] = handles[j + 1];
    if (handles[j] == 0)
      break;
  }
  num_handles--;

  if (hdl->cwd[0] != 0) {
    pwd = getcwd (cwd, MAX_DIR_SIZE);
    if (pwd != 0) {
      status = chdir (hdl->cwd);
      if (status != _ERROR_) {
	closeTable (hdl->tbl);
	if (isDBError ()) {
#ifdef HAVE_SNPRINTF
	  snprintf (cdberror, RESULT_SIZE, "closing table %s: %s", tbl, dberror ());
#else
	  (void)sprintf (cdberror, "closing table %s: %s", tbl, dberror ());
#endif
	  return _ERROR_;
	}
      }
      else {
	must_close_table = TRUE;
      }
      chdir (cwd);
    }
    else {
      must_close_table = TRUE;
    }
  }
  else {
    must_close_table = TRUE;
  }

  if (must_close_table == TRUE) {
    closeTable (hdl->tbl);
    if (isDBError ()) {
#ifdef HAVE_SNPRINTF
      snprintf (cdberror, RESULT_SIZE, "closing table %s: %s", tbl, dberror ());
#else
      (void)sprintf (cdberror, "closing table %s: %s", tbl, dberror ());
#endif
      return _ERROR_;
    }
  }
  check_pointer (hdl);
  free (hdl);
  return _OK_;
}

/*
 * [BeginDoc]
 * 
 * \subsection{dbnumrecs}
 * \index{dbnumrecs}
 *
 * [Verbatim] */

int dbnumrecs (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbnumrecs returns the number of records currently being managed in the table.
 * It is an error if the table is not open yet.  Upon successful completion,
 * dbnumrecs restores the number of records.  If an error occurs, an exception
 * is raised and a string is returned that describes the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbnumrecs tablehandle");
#else
    (void)sprintf (cdberror, "Usage: dbnumrecs tablehandle");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    (void)sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "dbnumrecs: %s not in handle list", tbl);
#else
    (void)sprintf (cdberror, "dbnumrecs: %s not in handle list", tbl);
#endif
    return _ERROR_;
  }
  return hdl->tbl->hdr->numRecords;
}

/*
 * [BeginDoc]
 *
 * \subsection{dbnumfields}
 * \index{dbnumfields}
 *
 * [Verbatim] */

int dbnumfields (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbnumfields returns the number of fields in the table given by ``tbl''
 * (this is a table handle).  It is an error 
 * if the table is not open yet.  Upon successful completion, dbnumfields
 * returns the number of fields.  If an error occurs, dbnumfields returns -1 and
 * cdberror contains a read-only string that describes the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbnumfields tablehandle");
#else
    (void)sprintf (cdberror, "Usage: dbnumfields tablehandle");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    (void)sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "dbnumfields: %s not in handle list", tbl);
#else
    (void)sprintf (cdberror, "dbnumfields: %s not in handle list", tbl);
#endif
    return _ERROR_;
  }
  return hdl->tbl->hdr->numFields;
}

/*
 * [BeginDoc]
 *
 * \subsection{dbseq}
 * \index{dbseq}
 *
 * [Verbatim] */

int dbseq (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbseq returns the ``nextSequence'' value stored in the table header of the table
 * given by ``tbl'' (a table handle).  It then increments the ``nextSequence''
 * value.  It is an error if the table is not open.  Upon successful completion,
 * dbseq returns the sequence value stored in the table before the call was made.
 * On error, a -1 is returned and cdberror contains a read-only string that describes
 * the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  int rtn;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbseq tablehandle");
#else
    (void)sprintf (cdberror, "Usage: dbseq tablehandle");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    (void)sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "dbseq: %s not in handle list", tbl);
#else
    (void)sprintf (cdberror, "dbseq: %s not in handle list", tbl);
#endif
    return _ERROR_;
  }
  rtn = hdl->tbl->hdr->nextSequence;
  hdl->tbl->hdr->nextSequence++;
  storeTableHeader (hdl->tbl);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "dbseq, saving table header: %s",
	dbtblerror (hdl->tbl));
#else
    (void)sprintf (cdberror, "dbseq, saving table header: %s",
	dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return rtn;
}

/*
 * [BeginDoc]
 *
 * \subsection{dbiseof}
 * \index{dbiseof}
 *
 * [Verbatim] */

int dbiseof (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbiseof returns 1 if the end of file indicator is TRUE for the table given
 * by ``tbl'' (a table handle) and 0 if it is FALSE.  It is an error if the
 * table is not open.  On error, -1 is returned and cdberror contains a read-only
 * string that describes the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbiseof tablehandle");
#else
    (void)sprintf (cdberror, "Usage: dbiseof tablehandle");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    (void)sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    (void)sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  return hdl->tbl->eof;
}

/*
 * [BeginDoc]
 *
 * \subsection{dbisbof}
 * \index{dbisbof}
 *
 * [Verbatim] */

int dbisbof (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbisof returns 1 if the beginning of file indicator is TRUE for the table
 * given by ``tbl'' (a table handle) and 0 if it is FALSE.  It is an error
 * if the table is not open.  On error, -1 is returned and cdberror contains a
 * read-only string that describes the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbisbof tablehandle");
#else
    (void)sprintf (cdberror, "Usage: dbisbof tablehandle");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    (void)sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    (void)sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  return hdl->tbl->bof;
}

/*
 * [BeginDoc]
 * \subsection{dbshow}
 * \index{dbshow}
 * [Verbatim] */

char *dbshow (const char *tbl, const char *fldnm)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbshow is used to get the value of the field in the table ``tbl'' (a table
 * handle).  The field is given by ``fldnm''.  If successful, dbshow returns the
 * information in a read-only static char string that is overwritten after each
 * call to dbshow.  If there is an error, dbshow returns a NULL pointer and cdberror
 * contains a string that describes the error.
 *
 * [EndDoc] 
 */
{
  cdbTableHandle *hdl;
  int i;

  if (tbl == 0 || tbl[0] == 0 || fldnm == 0 || fldnm[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbshow tablehandle field");
#else
    (void)sprintf (cdberror, "Usage: dbshow tablehandle field");
#endif
    return NULL;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    (void)sprintf (cdberror, "No tables open yet");
#endif
    return NULL;
  }
  hdl = findHandle ((char *) tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    (void)sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return NULL;
  }
  i = getFieldNum (hdl->tbl, fldnm);
  if (i == _ERROR_) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "invalid field, %s", fldnm);
#else
    (void)sprintf (cdberror, "invalid field, %s", fldnm);
#endif
    return NULL;
  }
#ifdef HAVE_SNPRINTF
  snprintf (rslt, RESULT_SIZE, "%s", hdl->tbl->fields[i]);
#else
  (void)sprintf (rslt, "%s", hdl->tbl->fields[i]);
#endif
  return rslt;
}

/*
 * [BeginDoc]
 * \subsection{dbflen}
 * \index{dbflen}
 * [Verbatim] */

int dbflen (char *tbl, char *fldnm)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbflen returns the length of the field ``fldnm'' in the table ``tbl'' 
 * (a table handle).  If an error occurs, dbflen returns -1 and cdberror is a
 * read-only string that describes the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  int i;

  if (tbl == 0 || tbl[0] == 0 || fldnm == 0 || fldnm[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbflen tablehandle field");
#else
    (void)sprintf (cdberror, "Usage: dbflen tablehandle field");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    (void)sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    (void)sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  i = getFieldNum (hdl->tbl, fldnm);
  if (i == _ERROR_) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "invalid field, %s", fldnm);
#else
    (void)sprintf (cdberror, "invalid field, %s", fldnm);
#endif
    return _ERROR_;
  }
  return hdl->tbl->flens[i];
}

/*
 * [BeginDoc]
 * \subsection{dbdeclen}
 * \index{dbdeclen}
 * [Verbatim] */

int dbdeclen (char *tbl, char *fldnm)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbdeclen returns the length of the decimal portion for the field given by 
 * ``fldnm''. If the field is not numeric, this will always be 0.
 * It is an error if the table is not open or ``fldnm'' is not
 * a valid field name.  If an error occurs, an error is raised and
 * a string describing the error is returned.  The argument ``tbl'' is
 * a table handle.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  int i;

  if (tbl == 0 || tbl[0] == 0 || fldnm == 0 || fldnm[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbdeclen tablehandle field");
#else
    (void)sprintf (cdberror, "Usage: dbdeclen tablehandle field");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    (void)sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    (void)sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  i = getFieldNum (hdl->tbl, fldnm);
  if (i == _ERROR_) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "invalid field, %s", fldnm);
#else
    (void)sprintf (cdberror, "invalid field, %s", fldnm);
#endif
    return _ERROR_;
  }
  return hdl->tbl->declens[i];
}

/*
 * [BeginDoc]
 * \subsection{dbsetint}
 * \index{dbsetint}
 * [Verbatim] */

int dbsetint (char *tbl, char *fldnm, int val)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbsetint sets the tbl->fields[] value for the field given by ``fldnm''
 * in the table given by ``tbl'' (this is a table handle)
 * to ``val'' in this example.  The table must be open and ``fldnm'' must
 * be a valid numerical field.  Upon successful completion,
 * the field is set and 0 is returned.
 * If an error occurs, -1 is returned and cdberror contains a read-only
 * string describing the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  int i;

  if (tbl == 0 || tbl[0] == 0 || fldnm == 0 || fldnm[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbsetint tablehandle field intval");
#else
    (void)sprintf (cdberror, "Usage: dbsetint tablehandle field intval");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    (void)sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    (void)sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  i = getFieldNum (hdl->tbl, fldnm);	/* validate the field name */
  if (i == _ERROR_) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "invalid field, %s", fldnm);
#else
    (void)sprintf (cdberror, "invalid field, %s", fldnm);
#endif
    return _ERROR_;
  }
  setIntField (hdl->tbl, fldnm, val);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "setting int field: %s", dbtblerror (hdl->tbl));
#else
    (void)sprintf (cdberror, "setting int field: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{dbsetnum}
 * \index{dbsetnum}
 * [Verbatim] */

int dbsetnum (char *tbl, char *fldnm, double val)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbsetnum works as dbsetint except it accepts a double value field.
 * If an error occurs, -1 is returned and cdberror is a read-only
 * string describing the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  int i;

  if (tbl == 0 || tbl[0] == 0 || fldnm == 0 || fldnm[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbsetnum tablehandle field dblval");
#else
    (void)sprintf (cdberror, "Usage: dbsetnum tablehandle field dblval");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    (void)sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    (void)sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  i = getFieldNum (hdl->tbl, fldnm);	/* validate the field name */
  if (i == _ERROR_) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "invalid field, %s", fldnm);
#else
    (void)sprintf (cdberror, "invalid field, %s", fldnm);
#endif
    return _ERROR_;
  }
  setNumberField (hdl->tbl, fldnm, val);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "setting num field: %s", dbtblerror (hdl->tbl));
#else
    (void)sprintf (cdberror, "setting num field: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{dbsetlog}
 * \index{dbsetlog}
 * [Verbatim] */

int dbsetlog (char *tbl, char *fldnm, char val)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbsetlog is used to set a logical field in the
 * tbl->fields[] array of the table ``tbl'' (a table handle).  
 * Other than that, it functions
 * identically to dbsetint.
 * If an error occurs, -1 is returned and cdberror is a read-only
 * string describing the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  int i;

  if (tbl == 0 || tbl[0] == 0 || fldnm == 0 || fldnm[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbsetlog tablehandle field logval");
#else
    (void)sprintf (cdberror, "Usage: dbsetlog tablehandle field logval");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    (void)sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    (void)sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  i = getFieldNum (hdl->tbl, fldnm);	/* validate the field name */
  if (i == _ERROR_) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "invalid field, %s", fldnm);
#else
    (void)sprintf (cdberror, "invalid field, %s", fldnm);
#endif
    return _ERROR_;
  }
  setLogicalField (hdl->tbl, fldnm, val);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "setting num field: %s", dbtblerror (hdl->tbl));
#else
    (void)sprintf (cdberror, "setting num field: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{dbsetchar}
 * \index{dbsetchar}
 * [Verbatim] */

int dbsetchar (char *tbl, char *fldnm, char *val)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbsetchar works as dbsetint except it accepts a string value field.
 * If an error occurs, -1 is returned and cdberror is a read-only
 * string describing the error.  If the ``val'' variable is set to ``0'',
 * The value of ``fldnm'' is set to "" (empty string).
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  int i;

  if (tbl == 0 || tbl[0] == 0 || fldnm == 0 || fldnm[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "dbsetchar tablehandle field charval");
#else
    (void)sprintf (cdberror, "dbsetchar tablehandle field charval");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    (void)sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    (void)sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  i = getFieldNum (hdl->tbl, fldnm);	/* validate the field name */
  if (i == _ERROR_) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "invalid field, %s", fldnm);
#else
    (void)sprintf (cdberror, "invalid field, %s", fldnm);
#endif
    return _ERROR_;
  }
  /*
   * Look here for "0", not just '0' as the first character.
   */
  if (val[0] == '0' && val[1] == '\0')
    setCharField (hdl->tbl, fldnm, 0);
  else
    setCharField (hdl->tbl, fldnm, val);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "setting char field: %s", dbtblerror (hdl->tbl));
#else
    (void)sprintf (cdberror, "setting char field: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{dbsetdate}
 * \index{dbsetdate}
 * [Verbatim] */

int dbsetdate (char *tbl, char *fldnm, char *val)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbsetdate works as dbsetint except it accepts a date value field.
 * If an error occurs, -1 is returned and cdberror is a read-only
 * string describing the error.  If the ``val'' argument is ``0'', the
 * date value is set to today's date.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  int i;

  if (tbl == 0 || tbl[0] == 0 || fldnm == 0 || fldnm[0] == 0 ||
      val == 0 || val[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbsetdate tablehandle field dateval");
#else
    (void)sprintf (cdberror, "Usage: dbsetdate tablehandle field dateval");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    (void)sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    (void)sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  i = getFieldNum (hdl->tbl, fldnm);	/* validate the field name */
  if (i == _ERROR_) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "invalid field, %s", fldnm);
#else
    (void)sprintf (cdberror, "invalid field, %s", fldnm);
#endif
    return _ERROR_;
  }
  if (!strcmp (val, "0"))
    setDateField (hdl->tbl, fldnm, 0);
  else
    setDateField (hdl->tbl, fldnm, val);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "setting num field: %s", dbtblerror (hdl->tbl));
#else
    (void)sprintf (cdberror, "setting num field: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{dbsettime}
 * \index{dbsettime}
 * [Verbatim] */

int dbsettime (char *tbl, char *fldnm, char *val)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbsettime works as dbsetint except it accepts a time stamp value.
 * If an error occurs, -1 is returned and cdberror is a read-only
 * string describing the error.  If ``val'' is ``0'', the current date
 * and time is used to generate the timestamp.
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  int i;

  if (tbl == 0 || tbl[0] == 0 || fldnm == 0 || fldnm[0] == 0 ||
      val == 0 || val[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbsettime tablehandle field timeval");
#else
    (void)sprintf (cdberror, "Usage: dbsettime tablehandle field timeval");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    (void)sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    (void)sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  i = getFieldNum (hdl->tbl, fldnm);	/* validate the field name */
  if (i == _ERROR_) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "invalid field, %s", fldnm);
#else
    (void)sprintf (cdberror, "invalid field, %s", fldnm);
#endif
    return _ERROR_;
  }
  if (!strcmp (val, "0"))
    setTimeStampField (hdl->tbl, fldnm, 0);
  else
    setTimeStampField (hdl->tbl, fldnm, val);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "setting num field: %s", dbtblerror (hdl->tbl));
#else
    (void)sprintf (cdberror, "setting num field: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{dbadd}
 * \index{dbadd}
 * [Verbatim] */

int dbadd (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbadd adds the data placed in the tbl->fields[] array
 * using the dbsetxxx function calls.  It is an error
 * if a unique index constraint would be violated by the
 * add; however, it is application dependent whether this should result in
 * the program exiting.  It is an error if the table is not open.  Upon
 * successful completion, the data will have been added to
 * the table.
 * If an error occurs, -1 is returned and cdberror is a read-only
 * string describing the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbadd tablehandle");
#else
    (void)sprintf (cdberror, "Usage: dbadd tablehandle");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    (void)sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    (void)sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  addRecord (hdl->tbl);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "adding record: %s", dbtblerror (hdl->tbl));
#else
    (void)sprintf (cdberror, "adding record: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{dbretrieve}
 * \index{dbretrieve}
 * [Verbatim] */

int dbretrieve (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbretrieve retrieves the data for the current record
 * into the tbl->fields[] array where it is accessible
 * via calls to dbshow.  It is an error if the table is
 * not open.  If the current record is marked for
 * deletion, ``0'' is returned.  You should be aware that because you
 * move to a record doesn't mean the record is retrieved.  You must
 * call dbretrieve after movement functions in order to get the data from
 * the table record.
 * If an error occurs, -1 is returned and cdberror is a read-only
 * string describing the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  int status;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbretrieve tablehandle");
#else
    (void)sprintf (cdberror, "Usage: dbretrieve tablehandle");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    (void)sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    (void)sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  status = retrieveRecord (hdl->tbl);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "retrieving record: %s", dbtblerror (hdl->tbl));
#else
    (void)sprintf (cdberror, "retrieving record: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return status;
}

/*
 * BeginDoc
 * \subsection{dbupdate}
 * \index{dbupdate}
 * [Verbatim] */

int dbupdate (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbupdate replaces the contents of the current
 * record with the values stored in the tbl->fields[]
 * array of the table given by the tablehandle ``tbl''.
 * It is an error if the table is not open.
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  char handle[TABLE_NAME_WIDTH + 1];
  int status;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbupdate tablehandle");
#else
    (void)sprintf (cdberror, "Usage: dbupdate tablehandle");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    (void)sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    (void)sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  strcpy (handle, hdl->handle);
  status = updateRecord (hdl->tbl);
  if (status == _ERROR_) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "updating record: %s", dbtblerror (hdl->tbl));
#else
    (void)sprintf (cdberror, "updating record: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{dbdel}
 * \index{dbdel}
 * [Verbatim] */

int dbdel (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbdel marks the current record in the table for
 * deletion and returns 0 if successful.
 * The record is not really deleted until dbpack
 * is run on the table, at which point the deletion
 * is irreversible.  It is an error if the table
 * is not open.
 * If an error occurs, -1 is returned and cdberror is a read-only
 * string describing the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbdel tablehandle");
#else
    (void)sprintf (cdberror, "Usage: dbdel tablehandle");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    (void)sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    (void)sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  deleteRecord (hdl->tbl);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "deleting record: %s", dbtblerror (hdl->tbl));
#else
    (void)sprintf (cdberror, "deleting record: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{dbundel}
 * \index{dbundel}
 * [Verbatim] */

int dbundel (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbundel unmarks the current record for deletion, if it
 * is marked.  If it is not marked, dbundel does nothing.
 * It is an error if the table is not open.
 * If an error occurs, -1 is returned and cdberror is a read-only
 * string describing the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbundel tablehandle");
#else
    (void)sprintf (cdberror, "Usage: dbundel tablehandle");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    (void)sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    (void)sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  undeleteRecord (hdl->tbl);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "undeleting record: %s", dbtblerror (hdl->tbl));
#else
    (void)sprintf (cdberror, "undeleting record: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{dbisdeleted}
 * \index{dbisdeleted}
 * [Verbatim] */

int dbisdeleted (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbisdeleted returns ``1'' if the current record in the
 * table is marked for deletion, ``0'' otherwise.  It is an error
 * if the table is not open.
 * If an error occurs, -1 is returned and cdberror is a read-only
 * string describing the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  int i;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbisdeleted tablehandle");
#else
    (void)sprintf (cdberror, "Usage: dbisdeleted tablehandle");
#endif
    return _ERROR_;
  }
  if (handles == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    (void)sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    (void)sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  i = isRecordDeleted (hdl->tbl);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "checking deleted record: %s", dbtblerror (hdl->tbl));
#else
    (void)sprintf (cdberror, "checking deleted record: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{dbdbfldname}
 * \index{dbdbfldname}
 * [Verbatim] */

char *dbfldname (char *table, int fldnum)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbfldname returns the name of the field given by fldnum.  fldnum
 * is a zero-based offset into an array of names (the first item is 0,
 * for example).  If fldnum is less than 0 or greater than the number
 * of fields, an error is returned.  An error is likewise
 * returned if there are no open tables, if the file handle is invalid or
 * on some other critical error.  If there is an error, a NULL pointer
 * is returned and cdberror contains a read-only string describing the
 * error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  dbField *fld;
  dbTable *tbl;

  if (table == 0 || table[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbfldname tablehandle fldnum");
#else
    (void)sprintf (cdberror, "Usage: dbfldname tablehandle fldnum");
#endif
    return 0;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    (void)sprintf (cdberror, "No tables open yet");
#endif
    return 0;
  }
  hdl = findHandle (table);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", table);
#else
    (void)sprintf (cdberror, "Couldn't find %s in handle list", table);
#endif
    return 0;
  }
  tbl = hdl->tbl;
  if (fldnum < 0 || fldnum >= tbl->hdr->numFields) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "%d out of bounds for table %s", fldnum, table);
#else
    sprintf (cdberror, "%d out of bounds for table %s", fldnum, table);
#endif
    return 0;
  }
  fld = tbl->fldAry[fldnum];
  strncpy (rslt, fld->fieldName, RESULT_SIZE-2);
  return rslt;
}

/*
 * [BeginDoc]
 * \subsection{dbfldtype}
 * \index{dbfldtype}
 * [Verbatim] */

char *dbfldtype (char *table, int fldnum)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbfldtype returns the type of the field given by fldnum.  fldnum
 * is a zero-based offset into an array of names (the first item is 0,
 * for example).  If fldnum is less than 0 or greater than the number
 * of fields, an error is returned.  An error is likewise
 * returned if there are no open tables, if the file handle is invalid or
 * on some other critical error.  If there is any error, a NULL pointer
 * is returned and cdberror contains a read-only string describing the
 * error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  dbField *fld;
  dbTable *tbl;

  if (table == 0 || table[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbfldtype tablehandle fldnum");
#else
    sprintf (cdberror, "Usage: dbfldtype tablehandle fldnum");
#endif
    return NULL;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    sprintf (cdberror, "No tables open yet");
#endif
    return NULL;
  }
  hdl = findHandle (table);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", table);
#else
    sprintf (cdberror, "Couldn't find %s in handle list", table);
#endif
    return NULL;
  }
  tbl = hdl->tbl;
  if (fldnum < 0 || fldnum >= tbl->hdr->numFields) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "%d out of bounds for table %s", fldnum, table);
#else
    sprintf (cdberror, "%d out of bounds for table %s", fldnum, table);
#endif
    return NULL;
  }
  fld = tbl->fldAry[fldnum];
  switch (fld->ftype) {
  case FTYPE_CHAR:
    strncpy (rslt, "char", RESULT_SIZE-2);
    break;
  case FTYPE_NUMBER:
    strncpy (rslt, "number", RESULT_SIZE-2);
    break;
  case FTYPE_LOGICAL:
    strncpy (rslt, "logical", RESULT_SIZE-2);
    break;
  case FTYPE_DATE:
    strncpy (rslt, "date", RESULT_SIZE-2);
    break;
  case FTYPE_TIME:
    strncpy (rslt, "time stamp", RESULT_SIZE-2);
    break;
  default:
    strncpy (rslt, "none", RESULT_SIZE-2);
    break;
  }
  return rslt;
}

/*
 * [BeginDoc]
 * \subsection{dbfldlen}
 * \index{dbfldlen}
 * [Verbatim] */

int dbfldlen (char *table, int fldnum)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbfldlen returns the length of the field given by fldnum.  fldnum
 * is a zero-based offset into an array of names (the first item is 0,
 * for example).  If fldnum is less than 0 or greater than the number
 * of fields, an error is returned.  An error is likewise
 * returned if there are no open tables, if the file handle is invalid or
 * on some other critical error.  If there is an error, -1 is returned and
 * cdberror contains a read-only string describing the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  dbField *fld;
  dbTable *tbl;

  if (table == 0 || table[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbfldlen tablehandle fldnum");
#else
    sprintf (cdberror, "Usage: dbfldlen tablehandle fldnum");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (table);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", table);
#else
    sprintf (cdberror, "Couldn't find %s in handle list", table);
#endif
    return _ERROR_;
  }
  tbl = hdl->tbl;
  if (fldnum < 0 || fldnum >= tbl->hdr->numFields) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "%d out of bounds for table %s", fldnum, table);
#else
    sprintf (cdberror, "%d out of bounds for table %s", fldnum, table);
#endif
    return _ERROR_;
  }
  fld = tbl->fldAry[fldnum];
  return fld->fieldLength;
}

/*
 * [BeginDoc]
 * \subsection{dbflddec}
 * \index{dbflddec}
 * [Verbatim] */

int dbflddec (char *table, int fldnum)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbflddec returns the length of the field given by fldnum.  fldnum
 * is a zero-based offset into an array of names (the first item is 0,
 * for example).  If fldnum is less than 0 or greater than the number
 * of fields, an error is returned.  An error is likewise
 * returned if there are no open tables, if the file handle is invalid or
 * on some other critical error.  If there is an error, -1 is returned and
 * cdberror contains a read-only string describing the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  dbField *fld;
  dbTable *tbl;

  if (table == 0 || table[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbflddec tablehandle fldnum");
#else
    sprintf (cdberror, "Usage: dbflddec tablehandle fldnum");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (table);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", table);
#else
    sprintf (cdberror, "Couldn't find %s in handle list", table);
#endif
    return _ERROR_;
  }
  tbl = hdl->tbl;
  if (fldnum < 0 || fldnum >= tbl->hdr->numFields) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "%d out of bounds for table %s", fldnum, table);
#else
    sprintf (cdberror, "%d out of bounds for table %s", fldnum, table);
#endif
    return _ERROR_;
  }
  fld = tbl->fldAry[fldnum];
  return fld->decLength;
}

/*
 * [BeginDoc]
 * \subsection{dbcurrent}
 * \index{dbcurrent}
 * [Verbatim] */

int dbcurrent (char *tbl, char *idxnm)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbcurrent sets the current index for the table to
 * ``indexname'' and returns 0 on success.
 * It is an error if the table is not open
 * or if ``indexname'' is not a valid index for the table.
 * If an error occurs, an error is returned and
 * a string describing the error is made available in cdberror.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;

  if (tbl == 0 || tbl[0] == 0 || idxnm == 0 || idxnm[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbcurrent tablehandle indexname");
#else
    sprintf (cdberror, "Usage: dbcurrent tablehandle indexname");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  setCurrentIndex (hdl->tbl, idxnm);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "setting current index: %s", dbtblerror (hdl->tbl));
#else
    sprintf (cdberror, "setting current index: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{dbgo}
 * \index{dbgo}
 * [Verbatim] */

int dbgo (char *tbl, int recno)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbgo sets the current record to the record numbered
 * ``recno'' (a 32-bit integer value) on success and returns
 * the record number of the current record.  It is an error
 * if the table is not open or ``recno'' is less than
 * one or greater than the number of records.
 * If an error occurs, an error is returned and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbgo tablehandle recno");
#else
    sprintf (cdberror, "Usage: dbgo tablehandle recno");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  if (recno < 0 || (recno == 0 && tbl[0] != '0')
      || (unsigned long) recno > hdl->tbl->hdr->numRecords) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "record number is out of bounds");
#else
    sprintf (cdberror, "record number is out of bounds");
#endif
    return _ERROR_;
  }
  gotoRecord (hdl->tbl, recno);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "going to a record: %s", dbtblerror (hdl->tbl));
#else
    sprintf (cdberror, "going to a record: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return recno;
}

/*
 * [BeginDoc]
 * \subsection{dbnext}
 * \index{dbnext}
 * [Verbatim] */

int dbnext (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbnext sets the current record in the table to
 * the next physical record without regard for indexing.  It is an error if
 * the table is not open.  If the current record
 * is already the last record when dbnext is called,
 * dbnext sets the end of file marker on the table to
 * TRUE (which can be checked with a call to dbiseof).
 * If an error occurs, an error is returned and
 * cdberror contains a string describing the error.
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbnext tablehandle");
#else
    sprintf (cdberror, "Usage: dbnext tablehandle");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  nextRecord (hdl->tbl);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "next record: %s", dbtblerror (hdl->tbl));
#else
    sprintf (cdberror, "next record: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{dbnextindex}
 * \index{dbnextindex}
 * [Verbatim] */

int dbnextindex (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbnextindex sets the current record in the table to
 * the next record by the current index.  It is an error if
 * the table is not open.  If there is no current index,
 * dbnextindex decays to a call to dbnext.
 * If the current record
 * is already the last record when dbnextindex is called,
 * dbnextindex sets the end of file marker on the table to
 * TRUE (which can be checked with a call to dbiseof).
 * If an error occurs, an error is returned and
 * cdberror contains a string describing the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbnextindex tablehandle");
#else
    sprintf (cdberror, "Usage: dbnextindex tablehandle");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  nextIndexRecord (hdl->tbl);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "next index record: %s", dbtblerror (hdl->tbl));
#else
    sprintf (cdberror, "next index record: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{dbprev}
 * \index{dbprev}
 * [Verbatim] */

int dbprev (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbprev sets the current record in the table to
 * the previous physical record.  It is an error if
 * the table is not open.  If the current record
 * is already the first record when dbprev is called,
 * dbprev sets the beginning of file marker on the table to
 * TRUE (which can be checked with a call to dbisbof).
 * If an error occurs, an error is returned and
 * cdberror contains a string that describes the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbprev tablehandle");
#else
    sprintf (cdberror, "Usage: dbprev tablehandle");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  prevRecord (hdl->tbl);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "prev record: %s", dbtblerror (hdl->tbl));
#else
    sprintf (cdberror, "prev record: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{dbprevindex}
 * \index{dbprevindex}
 * [Verbatim] */

int dbprevindex (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbprevindex sets the current record in the table to
 * the previous  record by the current index.  It is an error if
 * the table is not open.  If the current index is not set,
 * dbprevindex decays to a call to dbprev.
 * If the current record
 * is already the first record when dbprevindex is called,
 * dbprevindex sets the beginning of file marker on the table to
 * TRUE (which can be checked with a call to dbisbof).
 * If an error occurs, an error is returned and
 * cdberror contains a string that describes the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbprevindex tablehandle");
#else
    sprintf (cdberror, "Usage: dbprevindex tablehandle");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  prevIndexRecord (hdl->tbl);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "prev index record: %s", dbtblerror (hdl->tbl));
#else
    sprintf (cdberror, "prev index record: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{dbhead}
 * \index{dbhead}
 * [Verbatim] */

int dbhead (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbhead sets the current record in the table to
 * the first physical record.  It is an error if
 * the table is not open.  dbhead clears the
 * BOF and EOF markers for the table (sets them
 * to false).
 * if an error occurs, an error (-1) is returned and
 * cdberror contains a string that describes the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbhead tablehandle");
#else
    sprintf (cdberror, "Usage: dbhead tablehandle");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  headRecord (hdl->tbl);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "head record: %s", dbtblerror (hdl->tbl));
#else
    sprintf (cdberror, "head record: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{dbheadindex}
 * \index{dbheadindex}
 * [Verbatim] */

int dbheadindex (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbheadindex sets the current record in the table to
 * the first record by the current index.  It is an error if
 * the table is not open.  If there is no current
 * index set, dbheadindex decays to a call to dbhead.
 * dbheadindex clears the
 * BOF and EOF markers for the table (sets them
 * to false).
 * If an error occurs, an error is returned and
 * cdberror contains a string that describes the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbheadindex tablehandle");
#else
    sprintf (cdberror, "Usage: dbheadindex tablehandle");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  headIndexRecord (hdl->tbl);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "head index record: %s", dbtblerror (hdl->tbl));
#else
    sprintf (cdberror, "head index record: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{dbtail}
 * \index{dbtail}
 * [Verbatim] */

int dbtail (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbtail sets the current record in the table to
 * the last physical record.  It is an error if
 * the table is not open.  dbtail clears the
 * BOF and EOF markers for the table (sets them
 * to false).
 * If an error occurs, an error is returned and
 * cdberror contains a string describing the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbtail tablehandle");
#else
    sprintf (cdberror, "Usage: dbtail tablehandle");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  tailRecord (hdl->tbl);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "tail record: %s", dbtblerror (hdl->tbl));
#else
    sprintf (cdberror, "tail record: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{dbtailindex}
 * \index{dbtailindex}
 * [Verbatim] */

int dbtailindex (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbtailindex sets the current record in the table to
 * the last record by the current index.  It is an error if
 * the table is not open.  If there is no current index,
 * dbtailindex decays to a call to dbtail.
 * dbtailindex clears the
 * BOF and EOF markers for the table (sets them
 * to false).
 * If an error occurs, an error is returned and
 * cdberror contains a string that describes the error.
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbtailindex tablehandle");
#else
    sprintf (cdberror, "Usage: dbtailindex tablehandle");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  tailIndexRecord (hdl->tbl);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "tail index record: %s", dbtblerror (hdl->tbl));
#else
    sprintf (cdberror, "tail index record: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{dbsearchindex}
 * \index{dbsearchindex}
 * [Verbatim] */

int dbsearchindex (char *tbl, char *val)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbsearchindex searches the table ``tbl'' using the current index for
 * the value given by ``val''.  dbsearchindex does a
 * best fit search and only returns ``0'' if an error occurs.
 * So, for example, if ``val'' is ``111111111'' and that value
 * doesn't exist in the table but ``111111112'' does, then the current
 * record pointer would point to that record after a call
 * to dbsearchindex.  dbsearchindex clears the end of file and beginning
 * of file conditions.
 * It is an error if the table is not open or it doesn't have
 * a current index set.  Also, you will get an error condition if you
 * call dbsearchindex on an empty table.
 * If an error occurs, an error is returned and
 * cdberror contains a string that describes the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  int i;

  if (tbl == 0 || tbl[0] == 0 || val == 0 || val[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbsearchindex tablehandle fieldvalue");
#else
    sprintf (cdberror, "Usage: dbsearchindex tablehandle fieldvalue");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  i = searchIndexRecord (hdl->tbl, val);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "searching index record: %s", dbtblerror (hdl->tbl));
#else
    sprintf (cdberror, "searching index record: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return i;
}

/*
 * [BeginDoc]
 * \subsection{dbsearchexact}
 * \index{dbsearchexact}
 * [Verbatim] */

int dbsearchexact (char *tbl, char *val)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbsearchexact searches the table ``tbl'' using the current index for
 * the value given by ``val'', in this example.  This is a
 * bogus Social Security Number.  dbsearchexact does an
 * exact search and returns ``0'' if the item is not found.
 * It is an error if the table is not open or it doesn't have
 * a current index set.  If the item is found, a 1 is returned
 * and the current record is the record that has that value.
 * If the item is not found or the table is empty, a 0 is returned.
 * If there is an error, -1 is returned.
 * If an error occurs, an error is returned and
 * cdberror contains a string that describes the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  int i;

  if (tbl == 0 || tbl[0] == 0 || val == 0 || val[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbsearchexact tablehandle fieldvalue");
#else
    sprintf (cdberror, "Usage: dbsearchexact tablehandle fieldvalue");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  i = searchExactIndexRecord (hdl->tbl, val);
  if (isTableError (hdl->tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "searching exact record: %s", dbtblerror (hdl->tbl));
#else
    sprintf (cdberror, "searching exact record: %s", dbtblerror (hdl->tbl));
#endif
    return _ERROR_;
  }
  return i;
}

/*
 * [BeginDoc]
 * \subsection{dbpack}
 * \index{dbpack}
 * [Verbatim] */

int dbpack (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbpack packs the open table given by the handle ``tbl'' and
 * returns 0 if successful.
 * If an error occurs, an error is returned (-1) and
 * cdberror contains a string describing the error.
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbpack tablehandle");
#else
    sprintf (cdberror, "Usage: dbpack tablehandle");
#endif
    return _ERROR_;
  }
  if (num_handles == 0 || handles[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "No tables open yet");
#else
    sprintf (cdberror, "No tables open yet");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in handle list", tbl);
#else
    sprintf (cdberror, "Couldn't find %s in handle list", tbl);
#endif
    return _ERROR_;
  }
  hdl->tbl = packTable (hdl->tbl, FALSE);
  if (isDBError ()) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "tail record: %s", dberror ());
#else
    sprintf (cdberror, "tail record: %s", dberror ());
#endif
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{dbreindex}
 * \index{dbreindex}
 * [Verbatim] */

int dbreindex (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbreindex reindexes the table given by tablename and returns
 * 0 if successful.  It is an error if the table \emph{is}
 * open.  So, ``tbl'' in the dbreindex call is not a table handle.
 * If an error occurs, an error is returned (-1) and
 * cdberror contains a string that describes the error.
 * [EndDoc]
 */
{
  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbreindex tablename");
#else
    sprintf (cdberror, "Usage: dbreindex tablename");
#endif
    return _ERROR_;
  }
  if (!fexists (tbl)) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "File %s doesn't exist", tbl);
#else
    sprintf (cdberror, "File %s doesn't exist", tbl);
#endif
    return _ERROR_;
  }
  /*
   * Do the reindex and process any errors.
   */
  reindexTable (tbl);
  if (isDBError ()) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Reindexing %s: %s", tbl, dberror ());
#else
    sprintf (cdberror, "Reindexing %s: %s", tbl, dberror ());
#endif
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{dbexit}
 * \index{dbexit}
 * [Verbatim] */

int dbexit (void)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbexit is the only DCDB cint command that does not take any
 * arguments.
 * If there are no open tables, it simply returns
 * 0.  If there are open tables, it closes all of them,
 * cleans up the list used to store table handles and returns
 * 0.
 * [EndDoc]
 */
{
  int i;
  int handle_error = 0;

  if (num_handles == 0 || handles[0] == 0) {
    return _OK_;
  }
  for (i = 0; i < num_handles; i++) {
    if (handles[i] == 0)
      break;
    closeTable (handles[i]->tbl);
    if (isDBError ()) {
      handle_error = dbError;
#ifdef HAVE_SNPRINTF
      snprintf (cdberror, RESULT_SIZE, "dbexit: %s", dberror ());
#else
      sprintf (cdberror, "dbexit: %s", dberror ());
#endif
    }
    free (handles[i]);
    handles[i] = 0;
  }

  if (handle_error == _OK_)
    return _OK_;
  else
    return _ERROR_;
}

#ifndef __MINGW32__

/*
 * [BeginDoc]
 * \subsection{dbtime}
 * \index{dbtime}
 * [EndDoc] 
 */
double dbtime (void)
/*
 * [BeginDoc]
 * dbtime returns a double value that represents the number of seconds since
 * the epoch in fairly high resolution (if your architecture supports it).
 *
 * [EndDoc]
 */
{
  int status;
  double tm = 0.0;

  status = elapsed (&tm);
  if (status == _ERROR_)
    return 0.0;
  return tm;
}

#endif /* __MINGW32 */

/*
 * Return the number of multi-field indexes that are open by the table.
 */
/*
 * [BeginDoc]
 * \subsection{dbnumidx}
 * \index{dbnumidx}
 * [Verbatim] */

int dbnummidx (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbnummidx returns the number of multi-field indexes in the table
 * given by tablehandle.  It is not an error if there are no multi-field indexes
 * for the table.  If an error occurs, dbnummidx returns -1 and cdberror contains
 * a string that describes the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbnumidx tablehandle");
#else
    sprintf (cdberror, "Usage: dbnumidx tablehandle");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (hdl->tbl->midxList == 0)
    return 0;
  return hdl->tbl->midxList->number;
}

/*
 * Returns TRUE if there are multi-field indexes with this table, FALSE otherwise.
 */
/*
 * [BeginDoc]
 * \subsection{dbismidx}
 * \index{dbismidx}
 * [Verbatim] */

int dbismidx (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbismidx returns TRUE (1) if there are multi-field indexes associated with
 * the table given by tablehandle table.  It returns FALSE (0) otherwise.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbnumidx tablehandle");
#else
    sprintf (cdberror, "Usage: dbnumidx tablehandle");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (hdl->tbl->midxList == 0)
    return FALSE;
  return TRUE;
}

/*
 * Return the midxnum'th multi-field index.  It is an error if midxNum < 0 ||
 * midxNum > numMultiIndexes(tbl).
 */
/*
 * [BeginDoc]
 * \subsection{dbmidxname}
 * \index{dbmidxname}
 * [Verbatim] */

char *dbmidxname (char *tbl, int midxnum)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbmidxname returns the name of the num'th multi-field index for the table
 * given by tablehandle.  It is an error if there are no multi-field indexes
 * for the table.  Use dbismidx first to determine if there are multi-field
 * indexes for the table.
 * \emph{Note: the result of this call should be used immediately as
 * subsequent calls to certain functions will overwrite it.}
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  int status;
  midxField idx;
  int i;

  if (tbl == 0 || tbl[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbmidxname tablehandle midxnum");
#else
    sprintf (cdberror, "Usage: dbmidxname tablehandle midxnum");
#endif
    return 0;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in table list", tbl);
#else
    sprintf (cdberror, "Couldn't find %s in table list", tbl);
#endif
    return 0;
  }
  if (hdl->tbl->midxList == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "There are no multi-indexes in %s", tbl);
#else
    sprintf (cdberror, "There are no multi-indexes in %s", tbl);
#endif
    return 0;
  }
  status = hdl->tbl->midxList->number;
  if (status == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "There are no multi-indexes in %s", tbl);
#else
    sprintf (cdberror, "There are no multi-indexes in %s", tbl);
#endif
    return 0;
  }
  if (midxnum > status) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Number of multi-indexes = %d - couldn't get %dth",
	     status, midxnum);
#else
    sprintf (cdberror, "Number of multi-indexes = %d - couldn't get %dth",
	     status, midxnum);
#endif
    return 0;
  }
  if (hdl->tbl->hdr->midxInfo[midxnum - 1][0] == '\0') {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "There is no midx number %d in %s",
        midxnum, tbl);
#else
    sprintf (cdberror, "There is no midx number %d in %s",
        midxnum, tbl);
#endif
    return 0;
  }
  if (hdl->tbl->hdr->midxInfo[midxnum - 1][0] == '\0') {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "There is no number %d multi-field index in %s",
	     midxnum, tbl);
#else
    sprintf (cdberror, "There is no number %d multi-field index in %s",
	     midxnum, tbl);
#endif
    return 0;
  }
  memset (&idx, 0, sizeof (midxField));
  str2midxField (&idx, hdl->tbl->hdr->midxInfo[midxnum - 1]);
  strncpy (rslt, idx.indexName, RESULT_SIZE-2);
  for (i = 0; idx.names[i] != 0; i++)
    free (idx.names[i]);
  return rslt;
}

/*
 * Return blocksize of the midx.
 */
/*
 * [BeginDoc]
 * \subsection{dbmidxblksz}
 * \index{dbmidxblksz}
 * [Verbatim] */

int dbmidxblksz (char *tbl, char *midxnm)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbmidxblksz returns the blocksize of the multi-field index 
 * midxname (``midxnm'') attached
 * to the table given by tablehandle table (``tbl'').  
 * It is an error if the table has
 * no multi-field indexes or if it has none called midxname.
 * If an error occurs, -1 is returned and cdberror contains a string
 * that describes the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  midxField idx;
  int i, j;
  int status;

  if (tbl == 0 || tbl[0] == 0 || midxnm == 0 || midxnm[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbmidxblksz tablehandle midxname");
#else
    sprintf (cdberror, "Usage: dbmidxblksz tablehandle midxname");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in table list", tbl);
#else
    sprintf (cdberror, "Couldn't find %s in table list", tbl);
#endif
    return _ERROR_;
  }
  if (hdl->tbl->midxList == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "There are no multi-indexes in %s", tbl);
#else
    sprintf (cdberror, "There are no multi-indexes in %s", tbl);
#endif
    return _ERROR_;
  }
  status = hdl->tbl->midxList->number;
  if (status == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "There are no multi-indexes in %s", tbl);
#else
    sprintf (cdberror, "There are no multi-indexes in %s", tbl);
#endif
    return _ERROR_;
  }
  for (i = 0; i < MAX_MIDX; i++) {
    if (hdl->tbl->hdr->midxInfo[i][0] == '\0') {
#ifdef HAVE_SNPRINTF
      snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in table", midxnm);
#else
      sprintf (cdberror, "Couldn't find %s in table", midxnm);
#endif
      return _ERROR_;
    }
    memset (&idx, 0, sizeof (midxField));
    str2midxField (&idx, hdl->tbl->hdr->midxInfo[i]);
    if (!strncmp (midxnm, idx.indexName, strlen (midxnm))) {
      status = idx.blkSize;
      /* we're done... */
      for (j = 0; j < MAX_IDX && idx.names[j] != '\0'; j++)
	free (idx.names[j]);
      break;
    }
    for (j = 0; j < MAX_IDX && idx.names[j] != '\0'; j++)
      free (idx.names[j]);
  }
  return status;
}

/*
 * Return the number of field names in a multi-field index.
 */
/*
 * [BeginDoc]
 * \subsection{dbmidxnumfldnames}
 * \index{dbmidxnumfldnames}
 * [Verbatim] */

int dbmidxnumfldnames (char *tbl, char *midxnm)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbmidxnumfldnames returns the number of fields that are used to generate
 * the multi-field index midxname (``midxnm'') which is attached 
 * to the table given by tablehandle table (``tbl'').  
 * It is an error if the table has no multi-field indexes
 * attached to it or if it has none called midxname.
 * If an error occurs, -1 is returned and cdberror contains a string
 * that describes the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  int status;
  midxField idx;
  int i, j;

  if (tbl == 0 || tbl[0] == 0 || midxnm == 0 || midxnm[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbmidxnumfldnames tablehandle midxname");
#else
    sprintf (cdberror, "Usage: dbmidxnumfldnames tablehandle midxname");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in table list", tbl);
#else
    sprintf (cdberror, "Couldn't find %s in table list", tbl);
#endif
    return _ERROR_;
  }
  if (hdl->tbl->midxList == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "There are no multi-indexes in %s", tbl);
#else
    sprintf (cdberror, "There are no multi-indexes in %s", tbl);
#endif
    return _ERROR_;
  }
  status = hdl->tbl->midxList->number;
  if (status == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "There are no multi-indexes in %s", tbl);
#else
    sprintf (cdberror, "There are no multi-indexes in %s", tbl);
#endif
    return _ERROR_;
  }
  for (i = 0; i < MAX_MIDX; i++) {
    if (hdl->tbl->hdr->midxInfo[i][0] == '\0') {
#ifdef HAVE_SNPRINTF
      snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in table", midxnm);
#else
      sprintf (cdberror, "Couldn't find %s in table", midxnm);
#endif
      return _ERROR_;
    }
    memset (&idx, 0, sizeof (midxField));
    str2midxField (&idx, hdl->tbl->hdr->midxInfo[i]);
    if (!strncmp (midxnm, idx.indexName, strlen (midxnm))) {
      for (j = 0; j < MAX_IDX && idx.names[j] != '\0'; j++);
      status = j;
      /* we're done... */
      for (j = 0; j < MAX_IDX && idx.names[j] != '\0'; j++)
	free (idx.names[j]);
      break;
    }
  }
  return status;
}

/*
 * Return the num'th field name in the multi-field index given by midxnm.
 */
/*
 * [BeginDoc]
 * \subsection{dbmidxfldname}
 * \index{dbmidxfldname}
 * [Verbatim] */

char *dbmidxfldname (char *tbl, char *midxnm, int num)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbmidxfldname returns the num'th field name that makes up the multi-index
 * midxname (``midxnm'') which is attached to the table given by tablehandle 
 * ``tbl''.  It is
 * an error if the table has no multi-field indexes attached to it or if it has
 * none named ``midxnm'' in this case.  It is also an error if the 
 * multi-field index given by
 * midxname doesn't have num fields (0 in this example).  
 * You should call dbmidxnumfldnames to
 * determine how many field names are used in generating the multi-field index.
 * If an error occurs, 0 (NULL) is returned and cdberror contains a string
 * that describes the error.
 * \emph{Note: the result of this call should be used immediately as
 * subsequent calls to certain functions will overwrite it.}
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  int status;
  midxField idx;
  int i, j;

  if (tbl == 0 || tbl[0] == 0 || midxnm == 0 || midxnm[0] == 0
      || num < 0 || num > MAX_IDX) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbmidxfldname tablehandle midxname num");
#else
    sprintf (cdberror, "Usage: dbmidxfldname tablehandle midxname num");
#endif
    return 0;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in table list", tbl);
#else
    sprintf (cdberror, "Couldn't find %s in table list", tbl);
#endif
    return 0;
  }
  if (hdl->tbl->midxList == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "There are no multi-indexes in %s", tbl);
#else
    sprintf (cdberror, "There are no multi-indexes in %s", tbl);
#endif
    return 0;
  }
  status = hdl->tbl->midxList->number;
  if (status == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "There are no multi-indexes in %s", tbl);
#else
    sprintf (cdberror, "There are no multi-indexes in %s", tbl);
#endif
    return 0;
  }
  for (i = 0; i < MAX_MIDX; i++) {
    if (hdl->tbl->hdr->midxInfo[i][0] == '\0') {
#ifdef HAVE_SNPRINTF
      snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in table", midxnm);
#else
      sprintf (cdberror, "Couldn't find %s in table", midxnm);
#endif
      return 0;
    }
    memset (&idx, 0, sizeof (midxField));
    str2midxField (&idx, hdl->tbl->hdr->midxInfo[i]);
    if (!strncmp (midxnm, idx.indexName, strlen (midxnm))) {
      if (idx.names[num - 1] == 0 || idx.names[num - 1][0] == '\0') {
#ifdef HAVE_SNPRINTF
	snprintf (cdberror, RESULT_SIZE, "There is no number %d name in index %s", num,
		 midxnm);
#else
	sprintf (cdberror, "There is no number %d name in index %s", num,
		 midxnm);
#endif
	return 0;
      }
      memset (rslt, 0, strlen (idx.names[num - 1]) + 5);
      strcpy (rslt, idx.names[num - 1]);
      /* we're done... */
      for (j = 0; j < MAX_IDX && idx.names[j] != '\0'; j++)
	free (idx.names[j]);
      break;
    }
    for (j = 0; j < MAX_IDX && idx.names[j] != '\0'; j++)
      free (idx.names[j]);
  }
  return rslt;
}

/*
 * returns 0 (FALSE) if the field is not indexed, or 1 (TRUE) otherwise.
 */
/*
 * [BeginDoc]
 * \subsection{dbisindexed}
 * \index{dbisindexed}
 * [Verbatim] */

int dbisindexed (char *tbl, char *fldnm)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbisindexed returns TRUE (1) if the field name fieldname in the table given by
 * the tablehandle table is indexed.  It returns FALSE (0) otherwise.
 * It is an error if fieldname (``fldnm'') does not exist in the table
 * given by ``tbl''.
 * If an error occurs, -1 is returned and cdberror contains a string
 * that describes the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  int status;

  if (tbl == 0 || tbl[0] == 0 || fldnm == 0 || fldnm[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbisindexed tablehandle fieldname");
#else
    sprintf (cdberror, "Usage: dbisindexed tablehandle fieldname");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in table list", tbl);
#else
    sprintf (cdberror, "Couldn't find %s in table list", tbl);
#endif
    return _ERROR_;
  }
  status = getFieldNum (hdl->tbl, fldnm);
  if (status == _ERROR_) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't get field num of %s", tbl);
#else
    sprintf (cdberror, "Couldn't get field num of %s", tbl);
#endif
    return _ERROR_;
  }
  if (hdl->tbl->fldAry[status]->indexed == ITYPE_NOINDEX)
    return FALSE;
  else
    return TRUE;
}

/*
 * Returns the blocksize of the index on the field given by fldname.
 */
/*
 * [BeginDoc]
 * \subsection{dbidxblksz}
 * \index{dbidxblksz}
 * [Verbatim] */

int dbidxblksz (char *tbl, char *fldnm)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbidxblksz returns the block size of the index generated for the field
 * ``fldnm'' in the table given by the tablehandle ``tbl''.  
 * It is an error if
 * the field is not indexed.  You should call dbisindexed to insure that the
 * field is indexed before you call dbidxblksz.
 * If an error occurs, -1 is returned and cdberror contains a string
 * that describes the error.
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;
  int status;

  if (tbl == 0 || tbl[0] == 0 || fldnm == 0 || fldnm[0] == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbidxblksz tablehandle fieldname");
#else
    sprintf (cdberror, "Usage: dbidxblksz tablehandle fieldname");
#endif
    return _ERROR_;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in table list", tbl);
#else
    sprintf (cdberror, "Couldn't find %s in table list", tbl);
#endif
    return _ERROR_;
  }
  status = getFieldNum (hdl->tbl, fldnm);
  if (status == _ERROR_) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in the field list for %s",
	     fldnm, tbl);
#else
    sprintf (cdberror, "Couldn't find %s in the field list for %s",
	     fldnm, tbl);
#endif
    return _ERROR_;
  }
  if (hdl->tbl->fldAry[status]->indexed == ITYPE_NOINDEX) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Field %s is not indexed", fldnm);
#else
    sprintf (cdberror, "Field %s is not indexed", fldnm);
#endif
    return _ERROR_;
  }
  return hdl->tbl->fldAry[status]->indexBlkSize;
}

/*
 * [BeginDoc]
 * \subsection{dbshowinfo}
 * \index{dbshowinfo}
 * [Verbatim] */

char *dbshowinfo (char *tbl)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbshowinfo returns the contents of the info string that is stored in the
 * table header of the table given by ``tbl''.
 * This is specific to the application and has no meaning as
 * far as DCDB is concerned.  It can be a string of up to 125 characters and
 * can contain any value the application programmer desires.  It is set with
 * the ``info'' portion of the ``create'' command in a .df file.
 * \emph{Note: the result of this call should be used immediately as
 * subsequent calls to certain functions will overwrite it.}
 *
 * [EndDoc]
 */
{
  cdbTableHandle *hdl;

  if (tbl == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: cdbshowinfo tablehandle");
#else
    sprintf (cdberror, "Usage: cdbshowinfo tablehandle");
#endif
    return 0;
  }
  hdl = findHandle (tbl);
  if (0 == hdl) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Couldn't find %s in table list", tbl);
#else
    sprintf (cdberror, "Couldn't find %s in table list", tbl);
#endif
    return 0;
  }
  strncpy (rslt, hdl->tbl->hdr->tableInfo, RESULT_SIZE-2);
  return (rslt);
}

#ifndef __MINGW32__

#include <test.h>

/*
 * [BeginDoc]
 * \subsection{dbteststring}
 * \index{dbteststring}
 * [Verbatim] */

char *dbteststring (int len, int rlen)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbteststring returns a psuedo-random string of characters, all lower-case.
 * It takes 2 arguments, both integers.  If the first argument is set, the
 * second must be 0.  Conversely, if the second argument is set, the first must
 * be 0.  Setting the first argument gives you a string of that length.  For
 * example, ``dbteststring(10,0)'' will return a 10 character string.  Setting
 * the second argument will return a string of somewhat random length between
 * 5 and the length specified.  So, ``dbteststring(0,10)'' gives you a random
 * string of lower-case letters between 5 and 10 digits long.  It is an error
 * if both arguments are 0 or if both are non-zero.
 * If an error occurs, -1 is returned and cdberror contains a string
 * that describes the error.
 * \emph{Note: the result of this call should be used immediately as
 * subsequent calls to certain functions will overwrite it.}
 *
 * [EndDoc]
 */
{
  int status;

  if (len == 0 && rlen == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE,
	"Usage: dbteststring len|0, maxlen|0 where len and maxLen cannot both be 0.");
#else
    sprintf (cdberror,
	"Usage: dbteststring len|0, maxlen|0 where len and maxLen cannot both be 0.");
#endif
    return 0;
  }
  if (len >= RESULT_SIZE) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "len (%d) too big", len);
#else
    sprintf (cdberror, "len (%d) too big", len);
#endif
    return 0;
  }
  if (rlen >= RESULT_SIZE) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "rlen (%d) too big", rlen);
#else
    sprintf (cdberror, "rlen (%d) too big", rlen);
#endif
    return 0;
  }

  status = testString (rslt, len, rlen);
  if (status == _ERROR_) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "error calling testString()");
#else
    sprintf (cdberror, "error calling testString()");
#endif
    return 0;
  }
  return rslt;
}

/*
 * [BeginDoc]
 * \subsection{dbtestupperstring}
 * \index{dbtestupperstring}
 * [Verbatim] */

char *dbtestupperstring (int len, int rlen)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbtestupperstring works the same as dbteststring except that the string
 * returned will consist of upper-case alphabetical digits.
 *
 * [EndDoc]
 */
{
  int status;

  if (len == 0 && rlen == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE,
	"Usage: dbteststring len|0, maxlen|0 where len and maxLen cannot both be 0.");
#else
    sprintf (cdberror,
	"Usage: dbteststring len|0, maxlen|0 where len and maxLen cannot both be 0.");
#endif
    return 0;
  }
  if (len >= RESULT_SIZE) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "len (%d) too big", len);
#else
    sprintf (cdberror, "len (%d) too big", len);
#endif
    return 0;
  }
  if (rlen >= RESULT_SIZE) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "rlen (%d) too big", rlen);
#else
    sprintf (cdberror, "rlen (%d) too big", rlen);
#endif
    return 0;
  }

  status = testUpperString (rslt, len, rlen);
  if (status == _ERROR_) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "error calling testUpperString()");
#else
    sprintf (cdberror, "error calling testUpperString()");
#endif
    return 0;
  }
  return rslt;
}

/*
 * [BeginDoc]
 * \subsection{dbtestmixedstring}
 * \index{dbtestmixedstring}
 * [Verbatim] */

char *dbtestmixedstring (int len, int rlen)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbtestmixedstring works the same as dbteststring except that the string
 * returned will be of mixed upper and lower-case alphabetical digits.
 *
 * [EndDoc]
 */
{
  int status;

  if (len == 0 && rlen == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE,
	"Usage: dbteststring len|0, maxlen|0 where len and maxLen cannot both be 0.");
#else
    sprintf (cdberror,
	"Usage: dbteststring len|0, maxlen|0 where len and maxLen cannot both be 0.");
#endif
    return 0;
  }
  if (len >= RESULT_SIZE) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "len (%d) too big", len);
#else
    sprintf (cdberror, "len (%d) too big", len);
#endif
    return 0;
  }
  if (rlen >= RESULT_SIZE) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "rlen (%d) too big", rlen);
#else
    sprintf (cdberror, "rlen (%d) too big", rlen);
#endif
    return 0;
  }

  status = testMixedString (rslt, len, rlen);
  if (status == _ERROR_) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "error calling testMixedString()");
#else
    sprintf (cdberror, "error calling testMixedString()");
#endif
    return 0;
  }
  return rslt;
}

/*
 * [BeginDoc]
 * \subsection{dbtestnumber}
 * \index{dbtestnumber}
 * [Verbatim] */

char *dbtestnumber (int len, int rlen)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbtestnumber works the same as dbteststring except that it returns a string
 * of numerical digits.
 *
 * [EndDoc]
 */
{
  int status;

  if (len == 0 && rlen == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE,
	"Usage: dbteststring len|0, maxlen|0 where len and maxLen cannot both be 0.");
#else
    sprintf (cdberror,
	"Usage: dbteststring len|0, maxlen|0 where len and maxLen cannot both be 0.");
#endif
    return 0;
  }
  if (len >= RESULT_SIZE) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "len (%d) too big", len);
#else
    sprintf (cdberror, "len (%d) too big", len);
#endif
    return 0;
  }
  if (rlen >= RESULT_SIZE) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "rlen (%d) too big", rlen);
#else
    sprintf (cdberror, "rlen (%d) too big", rlen);
#endif
    return 0;
  }

  status = testNumber (rslt, len, rlen);
  if (status == _ERROR_) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "error calling testNumber()");
#else
    sprintf (cdberror, "error calling testNumber()");
#endif
    return 0;
  }
  return rslt;
}

/*
 * [BeginDoc]
 * \subsection{dbtestnumstring}
 * \index{dbtestnumstring}
 * [Verbatim] */

char *dbtestnumstring (int len, int rlen)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbtestnumstring works the same as dbteststring except that it returns a
 * string of combined numerical and upper-case alphabetical digits.
 *
 * [EndDoc]
 */
{
  int status;

  if (len == 0 && rlen == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE,
	"Usage: dbteststring len|0, maxlen|0 where len and maxLen cannot both be 0.");
#else
    sprintf (cdberror,
	"Usage: dbteststring len|0, maxlen|0 where len and maxLen cannot both be 0.");
#endif
    return 0;
  }
  if (len >= RESULT_SIZE) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "len (%d) too big", len);
#else
    sprintf (cdberror, "len (%d) too big", len);
#endif
    return 0;
  }
  if (rlen >= RESULT_SIZE) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "rlen (%d) too big", rlen);
#else
    sprintf (cdberror, "rlen (%d) too big", rlen);
#endif
    return 0;
  }

  status = testNumString (rslt, len, rlen);
  if (status == _ERROR_) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "error calling testNumString()");
#else
    sprintf (cdberror, "error calling testNumString()");
#endif
    return 0;
  }
  return rslt;
}

#include <blowfish.h>

/*
 * [BeginDoc]
 * \subsection{dbencrypt}
 * \index{dbencrypt}
 * [Verbatim] */

char *dbencrypt (char *data, int size, char *pwd)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbencrypt encrypts the data in the data argument (data) using the password in the
 * passwd argument (pwd).  The size argument (size) should be a multiple of 8,
 * although if
 * it is not, dbencrypt will find the closest multiple of 8 greater than size.
 * The maximum size for a data item is 2048 bytes.  The maximum length of the
 * password is 56 bytes (448 bits).
 *
 * dbencrypt uses the blowfish algorithm.  This algorithm is unencumbered with
 * patents and \emph{restrictive} copyrights.  It is an open source 
 * implementation written by
 * Bruce Schneier (thanks, Bruce).  You can use dbencrypt to create and manage
 * passwords or to encrypt data in blocks.
 *
 * [EndDoc]
 */
{
  char *cp = 0;
  if (data == 0 || size == 0 || pwd == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbencrypt string size password");
#else
    sprintf (cdberror, "Usage: dbencrypt string size password");
#endif
    return 0;
  }
  cp = blowfishEncryptString (data, size, pwd);
  if (cp == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "bad parameter to dbencrypt");
#else
    sprintf (cdberror, "bad parameter to dbencrypt");
#endif
    return 0;
  }
  memset (bf_rslt, 0, MAX_BLOWFISH * 2 + 1);
  strncpy (bf_rslt, cp, MAX_BLOWFISH * 2);
  return bf_rslt;
}

/*
 * [BeginDoc]
 * \subsection{dbdecrypt}
 * \index{dbdecrypt}
 * [Verbatim] */

char *dbdecrypt (char *data, int size, char *pwd)

/* [EndDoc] */
/*
 * [BeginDoc]
 * dbdecrypt decrypts the data given by the data argument (data) using the password
 * given by passwd (pwd).  The data argument should contain data that was encrypted
 * with dbencrypt and the passwd argument should have the same password used
 * to encrypt the data.  The size argument (size) should also be the size of the
 * data before it was encrypted.
 *
 * [EndDoc]
 */
{
  char *cp = 0;
  if (data == 0 || size == 0 || pwd == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: dbdecrypt string size [ password ]");
#else
    sprintf (cdberror, "Usage: dbdecrypt string size [ password ]");
#endif
    return 0;
  }
  cp = blowfishDecryptString (data, size, pwd);
  if (cp == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "bad parameter to dbdecrypt");
#else
    sprintf (cdberror, "bad parameter to dbdecrypt");
#endif
  }
  memset (bf_rslt, 0, MAX_BLOWFISH * 2 + 1);
  strncpy (bf_rslt, cp, MAX_BLOWFISH * 2);
  return bf_rslt;
}

#endif /* __MINGW32__ */

#include <crc32.h>

/*
 * [BeginDoc]
 * \subsection{crc32sum}
 * \index{crc32sum}
 * [Verbatim] */

char *crc32sum (char *fname)

/* [EndDoc] */
/*
 * [BeginDoc]
 * crc32sum will generate the crc32 check sum of the file given by fname and
 * return that value.  This is the 32 bit check sum code from 
 * /usr/src/linux/fs/jffs2/crc32[h,c] from the 2.4.18 kernel modified for my
 * use here.
 *
 * [EndDoc]
 */
{
  unsigned crc = 0;

  crc = crc32Sum (fname);
  if (crc == 0) {
    if (dbError == DB_NOMEMORY) {
#ifdef HAVE_SNPRINTF
      snprintf (cdberror, RESULT_SIZE, "critical memory error");
#else
      sprintf (cdberror, "critical memory error");
#endif
      return 0;
    }
    else if (dbError == DB_IO_DENIED) {
#ifdef HAVE_SNPRINTF
      snprintf (cdberror, RESULT_SIZE, "permission denied");
#else
      sprintf (cdberror, "permission denied");
#endif
      return 0;
    }
    else if (dbError == DB_IO_TOOMANY) {
#ifdef HAVE_SNPRINTF
      snprintf (cdberror, RESULT_SIZE, "too many files open at once");
#else
      sprintf (cdberror, "too many files open at once");
#endif
      return 0;
    }
    else if (dbError == DB_IO_NOFILE) {
#ifdef HAVE_SNPRINTF
      snprintf (cdberror, RESULT_SIZE, "file (%s) does not exist", fname);
#else
      sprintf (cdberror, "file (%s) does not exist", fname);
#endif
      return 0;
    }
    else if (dbError == DB_IO_READWRITE) {
#ifdef HAVE_SNPRINTF
      snprintf (cdberror, RESULT_SIZE, "critical I/O read/write error");
#else
      sprintf (cdberror, "critical I/O read/write error");
#endif
      return 0;
    }
    else {
#ifdef HAVE_SNPRINTF
      snprintf (cdberror, RESULT_SIZE, "unspecified error");
#else
      sprintf (cdberror, "unspecified error");
#endif
      return 0;
    }
  }
#ifdef HAVE_SNPRINTF
  snprintf (rslt, RESULT_SIZE, "%u", crc);
#else
  sprintf (rslt, "%u", crc);
#endif
  return rslt;
}

#include <bcnum.h>

/*
 * [BeginDoc]
 *
 * \subsection{BC number engine}
 *
 * One of the weaknesses of TCL as a scripting language is the lack of
 * a useful way to add, subtract, multiply and divide numbers in a way
 * that is friendly for dollar amount calculations.  This is necessary in
 * order to use TCL for dollar-based calculations.  The following bindings
 * are available to help solve that problem.  They use the GNU BC number
 * engine (number.h and number.c), which was written in such a way that
 * it was trivial to include it into this project.  Please read the 
 * copyright notices on the source files and redestribute it according to
 * those notices.
 *
 * [EndDoc]
 */

/*
 * [BeginDoc]
 * \index{bcnumadd}
 * \subsection{bcnumadd}
 * [Verbatim] */

char *bcnumadd (char *val1, char *val2, int scale)

/* [EndDoc] */
/*
 * [BeginDoc]
 * bcnumadd uses the GNU bc number engine to add val1 to val2 in the scale
 * given by scale.
 *
 * [EndDoc]
 */
{
  char *cp = 0;
  if (val1 == 0 || val2 == 0 || scale < 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: bcnumadd num1 num2 scale");
#else
    sprintf (cdberror, "Usage: bcnumadd num1 num2 scale");
#endif
    return 0;
  }
  cp = bcnum_add (val1, val2, scale);
  if (bcnumError != BCNUM_NOERROR) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "bcnum error: %s", bcnumErrMsg[bcnumError]);
#else
    sprintf (cdberror, "bcnum error: %s", bcnumErrMsg[bcnumError]);
#endif
    return 0;
  }
  memset (rslt, 0, BCNUM_OUTSTRING_SIZE + 1);
  strncpy (rslt, cp, BCNUM_OUTSTRING_SIZE);
  return rslt;
}

/*
 * [BeginDoc]
 * \index{bcnumsub}
 * \subsection{bcnumsub}
 * [Verbatim] */

char *bcnumsub (char *val1, char *val2, int scale)

/* [EndDoc] */
/*
 * [BeginDoc]
 * bcnumsub uses the GNU bc number engine to subtract val2 from val1 in the scale
 * given by scale.
 *
 * [EndDoc]
 */
{
  char *cp = 0;
  if (val1 == 0 || val2 == 0 || scale < 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: bcnumadd num1 num2 scale");
#else
    sprintf (cdberror, "Usage: bcnumadd num1 num2 scale");
#endif
    return 0;
  }
  cp = bcnum_sub (val1, val2, scale);
  if (bcnumError != BCNUM_NOERROR) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "bcnum error: %s", bcnumErrMsg[bcnumError]);
#else
    sprintf (cdberror, "bcnum error: %s", bcnumErrMsg[bcnumError]);
#endif
    return 0;
  }
  memset (rslt, 0, BCNUM_OUTSTRING_SIZE + 1);
  strncpy (rslt, cp, BCNUM_OUTSTRING_SIZE);
  return rslt;
}

/*
 * [BeginDoc]
 * \index{bcnumcompare}
 * \subsection{bcnumcompare}
 * [Verbatim] */

int bcnumcompare (char *val1, char *val2, int scale)

/* [EndDoc] */
/*
 * [BeginDoc]
 * bcnumcompare uses the GNU bc number engine to compare val1 to val2 
 * in the scale given by scale.
 *
 * [EndDoc]
 */
{
  int status;
  if (val1 == 0 || val2 == 0 || scale < 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: bcnumcompare num1 num2 scale");
#else
    sprintf (cdberror, "Usage: bcnumcompare num1 num2 scale");
#endif
    return _ERROR_;
  }
  status = bcnum_compare (val1, val2, scale);
  if (bcnumError != BCNUM_NOERROR) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "bcnum error: %s", bcnumErrMsg[bcnumError]);
#else
    sprintf (cdberror, "bcnum error: %s", bcnumErrMsg[bcnumError]);
#endif
    return _ERROR_;
  }
  return status;
}

/*
 * [BeginDoc]
 * \index{bcnummultiply}
 * \subsection{bcnummultiply}
 * [Verbatim] */

char *bcnummultiply (char *val1, char *val2, int scale)

/* [EndDoc] */
/*
 * [BeginDoc]
 * bcnummultiply uses the GNU bc number engine to multiply val1 by val2
 * in the scale given by scale.
 *
 * [EndDoc]
 */
{
  char *cp = 0;
  if (val1 == 0 || val2 == 0 || scale < 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: bcnummultiply num1 num2 scale");
#else
    sprintf (cdberror, "Usage: bcnummultiply num1 num2 scale");
#endif
    return 0;
  }
  cp = bcnum_multiply (val1, val2, scale);
  if (bcnumError != BCNUM_NOERROR) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "bcnum error: %s", bcnumErrMsg[bcnumError]);
#else
    sprintf (cdberror, "bcnum error: %s", bcnumErrMsg[bcnumError]);
#endif
    return 0;
  }
  memset (rslt, 0, BCNUM_OUTSTRING_SIZE + 1);
  strncpy (rslt, cp, BCNUM_OUTSTRING_SIZE);
  return rslt;
}

/*
 * [BeginDoc]
 * \index{bcnumdivide}
 * \subsection{bcnumdivide}
 * [Verbatim] */

char *bcnumdivide (char *val1, char *val2, int scale)

/* [EndDoc] */
/*
 * [BeginDoc]
 * bcnumdivide uses the GNU bc number engine to divide val1 to val2
 * in the scale given by scale.
 *
 * [EndDoc]
 */
{
  char *cp = 0;
  if (val1 == 0 || val2 == 0 || scale < 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: bcnumdivide num1 num2 scale");
#else
    sprintf (cdberror, "Usage: bcnumdivide num1 num2 scale");
#endif
    return 0;
  }
  cp = bcnum_divide (val1, val2, scale);
  if (bcnumError != BCNUM_NOERROR) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "bcnum error: %s", bcnumErrMsg[bcnumError]);
#else
    sprintf (cdberror, "bcnum error: %s", bcnumErrMsg[bcnumError]);
#endif
    return 0;
  }
  memset (rslt, 0, BCNUM_OUTSTRING_SIZE + 1);
  strncpy (rslt, cp, BCNUM_OUTSTRING_SIZE);
  return rslt;
}

/*
 * [BeginDoc]
 * \index{bcnumraise}
 * \subsection{bcnumraise}
 * [Verbatim] */

char *bcnumraise (char *val1, char *val2, int scale)

/* [EndDoc] */
/*
 * [BeginDoc]
 * bcnumraise uses the GNU bc number engine to raise val1 to the power of val2
 * in the scale given by scale.  val2 is truncated to an integer value before
 * val1 is raised to that power.
 *
 * [EndDoc]
 */
{
  char *cp = 0;
  if (val1 == 0 || val2 == 0 || scale < 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: bcnumraise num1 num2 scale");
#else
    sprintf (cdberror, "Usage: bcnumraise num1 num2 scale");
#endif
    return 0;
  }
  cp = bcnum_raise (val1, val2, scale);
  if (bcnumError != BCNUM_NOERROR) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "bcnum error: %s", bcnumErrMsg[bcnumError]);
#else
    sprintf (cdberror, "bcnum error: %s", bcnumErrMsg[bcnumError]);
#endif
    return 0;
  }
  memset (rslt, 0, BCNUM_OUTSTRING_SIZE + 1);
  strncpy (rslt, cp, BCNUM_OUTSTRING_SIZE);
  return rslt;
}

/*
 * [BeginDoc]
 * \index{bcnumiszero}
 * \subsection{bcnumiszero}
 * [Verbatim] */

int bcnumiszero (char *val1)

/* [EndDoc] */
/*
 * [BeginDoc]
 * bcnumiszero uses the GNU bc number engine to determine if val1 is zero and
 * returns 1 if it is and 0 if it is not.
 *
 * [EndDoc]
 */
{
  int status;
  if (val1 == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: bcnumiszero num1");
#else
    sprintf (cdberror, "Usage: bcnumiszero num1");
#endif
    return _ERROR_;
  }
  status = bcnum_iszero (val1);
  if (bcnumError != BCNUM_NOERROR) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "bcnum error: %s", bcnumErrMsg[bcnumError]);
#else
    sprintf (cdberror, "bcnum error: %s", bcnumErrMsg[bcnumError]);
#endif
    return 0;
  }
  return status;
}

/*
 * [BeginDoc]
 * \index{bcnumisnearzero}
 * \subsection{bcnumisnearzero}
 * [Verbatim] */

int bcnumisnearzero (char *val1, int scale)

/* [EndDoc] */
/*
 * [BeginDoc]
 * bcnumisnearzero uses the GNU bc number engine to determine if val1 is near
 * zero (determined by scale).  It returns 1 if it is and 0 if it is not.
 *
 * [EndDoc]
 */
{
  int status;
  if (val1 == 0 || scale < 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: bcnumisnearzero num1");
#else
    sprintf (cdberror, "Usage: bcnumisnearzero num1");
#endif
    return _ERROR_;
  }
  status = bcnum_isnearzero (val1, scale);
  if (bcnumError != BCNUM_NOERROR) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "bcnum error: %s", bcnumErrMsg[bcnumError]);
#else
    sprintf (cdberror, "bcnum error: %s", bcnumErrMsg[bcnumError]);
#endif
    return 0;
  }
  return status;
}

/*
 * [BeginDoc]
 * \index{bcnumisneg}
 * \subsection{bcnumisneg}
 * [Verbatim] */

int bcnumisneg (char *val1)

/* [EndDoc] */
/*
 * [BeginDoc]
 * bcnumisneg uses the GNU bc number engine to determine if val1 is negative.
 * It returns 1 if it is and 0 if it is not.
 *
 * [EndDoc]
 */
{
  int status;
  if (val1 == 0) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "Usage: bcnumisneg num1");
#else
    sprintf (cdberror, "Usage: bcnumisneg num1");
#endif
    return _ERROR_;
  }
  status = bcnum_isneg (val1);
  if (bcnumError != BCNUM_NOERROR) {
#ifdef HAVE_SNPRINTF
    snprintf (cdberror, RESULT_SIZE, "bcnum error: %s", bcnumErrMsg[bcnumError]);
#else
    sprintf (cdberror, "bcnum error: %s", bcnumErrMsg[bcnumError]);
#endif
    return 0;
  }
  return status;
}

/*
 * [BeginDoc]
 * \index{bcnumuninit}
 * \subsection{bcnumuninit}
 * [Verbatim] */

int bcnumuninit (void)

/* [EndDoc] */
/*
 * bcnumuninit de-initializes the bcnum stuff (basically, freeing some
 * bcnum numbers that are preallocated).  This should be called when you are
 * done with the bcnum bindings.  If you don't do it manually, there is an
 * atexit() function that will do it for you.  This only returns _OK_.
 *
 * [EndDoc]
 */
{
  if (bcnum_is_init)
    bcnum_uninit();

  return _OK_;
}
