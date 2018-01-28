/* Header File: cint_cdb.h */

#ifndef	__CINT_CDB_H__
#define	__CINT_CDB_H__

/*
 * prototypes for C command handling functions
 */

#ifdef __cplusplus__
extern "C" {
#endif

/*
 * [BeginDoc]
 *
 * \section{CINT Bindings}
 * \index{CINT}
 * 
 * CINT is a C/C++ interpreter that is available for numerous architectures under
 * the GPL license.  It is a fairly excellent piece of work that provides a simple
 * interface for embedding libraries like DCDB.  The advantage of using CINT is that
 * the development that is done, if done carefully, can be compiled into native
 * binaries using the C/C++ compiler.  This is quite an advantage.
 *
 * The interface provided for CINT is almost identical to that provided for Tcl/Tk
 * and Perl, except for the syntax differences between the languages.  The function
 * calls have the same arguments.  The primary difference between CINT and
 * Tcl/Tk/Perl is that when an error occurs, there is no exception raised in CINT;
 * an error message is merely returned.
 *
 * There is a README in the cdb-1.1/src/cint directory which has information on
 * compiling CINT for the supported platforms.  The main DCDB README file
 * (cdb-1.1/README) has information on creating a ``mycint'' binary with the DCDB
 * extensions and the container.  There are example scripts (with a .c extension) in
 * cdb-1.1/src/cint/scripts which can be interpreted by CINT or compiled into
 * native binaries.
 *
 * [EndDoc]
 */

/*
 * cdb stuff.
 */
/*
 * [BeginDoc]
 *
 * Error conditions are communicated via function return values.  If an error
 * occurs, as indicated by the return value, the variable cdberror will have
 * a description of the error.  cdberror is defined as follows:
 * [Verbatim] */

extern char cdberror[];

/* [EndDoc] */

/*
 * [BeginDoc]
 * \subsection{dbcreate}
 * \index{dbcreate}
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
 * dbcreate creates the tables and workspaces defined in the definition file given
 * by ``deffile.df''.  If any error occurs, an error (-1) is returned.  If
 * dbcreate is successful, 0 is returned.
 *
 * [EndDoc]
 */
int dbcreate (char *arg);

/*
 * [BeginDoc]
 * \subsection{dbopen}
 * \index{dbopen}
 *
 * [Verbatim]

#define SIZE 115
char handle[SIZE+1];
char *cp;
...
cp = dbopen ("database.db");
if (0 == cp) {
  Process error
}
strcpy (handle, cp);

 * [EndDoc] */

/*
 * [BeginDoc]
 * dbopen opens the table given by ``database.db''.  It is an error if the table
 * doesn't exist.  Upon success, dbopen returns a read-only pointer to the name
 * of a handle.  That handle is to be used for all other table access functions.  This
 * value must be copied to a char array, because many of the access functions use
 * the same static storage space to return results as the array the handle name is
 * returned in.  The handle is really just the name of the table, so in a pinch you
 * can replace the handle name with ``database.db'' (in this example).
 *
 * If an error occurs, dbopen returns a NULL pointer and cdberror contains
 * a description of the error.
 * 
 * [EndDoc]
 */
const char *dbopen (char *arg);

/*
 * [BeginDoc]
 *
 * \subsection{dbclose}
 * \index{dbclose}
 *
 * [Verbatim] 

int status;
...
status = dbclose ("table.db");
if (status == -1)
  Handle error...

 * [EndDoc] */

/*
 * [BeginDoc]
 * dbclose closes the table given by the argument (``table.db'' in this 
 * example), which is actually a table handle.  It
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
int dbclose (char *arg);

/*
 * [BeginDoc]
 * 
 * \subsection{dbnumrecs}
 * \index{dbnumrecs}
 *
 * [Verbatim]

int numrecs;
...
numrecs = dbnumrecs ("table.db");
if (numrecs == -1)
  handle the error...

 * [EndDoc] */

/*
 * [BeginDoc]
 * dbnumrecs returns the number of records currently being managed in the table.
 * It is an error if the table is not open yet.  Upon successful completion,
 * dbnumrecs restores the number of records.  If an error occurs, an exception
 * is raised and a string is returned that describes the error.
 *
 * [EndDoc]
 */
int dbnumrecs (char *arg);

/*
 * [BeginDoc]
 *
 * \subsection{dbnumfields}
 * \index{dbnumfields}
 *
 * [Verbatim]

int numfields;
...
numfields = dbnumfields ("table.db");
if (numfields == -1)
  handle the error...

 * [EndDoc] */

/*
 * [BeginDoc]
 * dbnumfields returns the number of fields in the table given by ``table.db''
 * in this example (this is a table handle).  It is an error 
 * if the table is not open yet.  Upon successful completion, dbnumfields
 * returns the number of fields.  If an error occurs, dbnumfields returns -1 and
 * cdberror contains a read-only string that describes the error.
 *
 * [EndDoc]
 */
int dbnumfields (char *arg);

/*
 * [BeginDoc]
 *
 * \subsection{dbseq}
 * \index{dbseq}
 *
 * [Verbatim]

 int seq;
 ...
 seq = dbseq ("table.db");
 if (seq == -1)
   handle the error...

 * [EndDoc] */

/*
 * [BeginDoc]
 * dbseq returns the ``nextSequence'' value stored in the table header of the table
 * given by ``table.db'' (a table handle).  It then increments the ``nextSequence''
 * value.  It is an error if the table is not open.  Upon successful completion,
 * dbseq returns the sequence value stored in the table before the call was made.
 * On error, a -1 is returned and cdberror contains a read-only string that describes
 * the error.
 *
 * [EndDoc]
 */
int dbseq (char *arg);

/*
 * [BeginDoc]
 *
 * \subsection{dbiseof}
 * \index{dbiseof}
 *
 * [Verbatim]

dbhead ("table.db");
while (! dbiseof ("table.db")) {
  dbretrieve ("table.db");
  do something with the data...
  dbnext ("table.db");
}

 * [EndDoc] */

/*
 * [BeginDoc]
 * dbiseof returns 1 if the end of file indicator is TRUE for the table given
 * by ``table.db'' (a table handle) and 0 if it is FALSE.  It is an error if the
 * table is not open.  On error, -1 is returned and cdberror contains a read-only
 * string that describes the error.
 *
 * [EndDoc]
 */
int dbiseof (char *arg);

/*
 * [BeginDoc]
 *
 * \subsection{dbisbof}
 * \index{dbisbof}
 *
 * [Verbatim]

dbtail ("table.db");
while (! dbiseof ("table.db")) {
  dbretrieve ("table.db");
  do something with the data...
  dbprev ("table.db");
}

 * [EndDoc] */

/*
 * [BeginDoc]
 * dbisof returns 1 if the beginning of file indicator is TRUE for the table
 * given by ``table.db'' (a table handle) and 0 if it is FALSE.  It is an error
 * if the table is not open.  On error, -1 is returned and cdberror contains a
 * read-only string that describes the error.
 *
 * [EndDoc]
 */
int dbisbof (char *arg);

/*
 * [BeginDoc]
 * \subsection{dbshow}
 * \index{dbshow}
 * [Verbatim]

char *field;
...
field = dbshow ("table.db", "SSNumber");
if (field == 0)
  handle the error...
printf ("\nSSNumber in table.db is %s\n", field);

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbshow is used to get the value of the field in the table ``table.db'' (a table
 * handle).  The field is given by ``SSNumber''.  If successful, dbshow returns the
 * information in a read-only static char string that is overwritten after each
 * call to dbshow.  If there is an error, dbshow returns a NULL pointer and cdberror
 * contains a string that describes the error.
 *
 * [EndDoc]
 */
char *dbshow (char *arg1, char *arg2);

/*
 * [BeginDoc]
 * \subsection{dbflen}
 * \index{dbflen}
 * [Verbatim]

int flen;
...
flen = dbflen ("table.db", "SSNumber");
if (flen == -1)
  handle the error...
printf (\nLength of SSNumber in table.db is %d\n", flen);

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbflen returns the length of the field ``SSNumber'' in the table ``table.db'' 
 * (a table handle).  If an error occurs, dbflen returns -1 and cdberror is a
 * read-only string that describes the error.
 *
 * [EndDoc]
 */
int dbflen (char *arg1, char *arg2);
/*
 * [BeginDoc]
 * \subsection{dbdeclen}
 * \index{dbdeclen}
 * [Verbatim]

int declen;
...
declen = dbdeclen ("table.db", "SSNumber");
if (declen == -1)
  handle error here...

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbdeclen returns the length of decimal portion for the field given by 
 * ``SSNumber''. If the field is not numeric, this will always be 0.
 * It is an error if the table is not open or ``SSNumber'' is not
 * a valid field name.  If an error occurs, an error is raised and
 * a string describing the error is returned.  The argument ``table.db'' is
 * a table handle.
 *
 * [EndDoc]
 */
int dbdeclen (char *arg1, char *arg2);
/*
 * [BeginDoc]
 * \subsection{dbsetint}
 * \index{dbsetint}
 * [Verbatim]

int status;
....
status = dbsetint ("table.db", "Age", 42);
if (status == -1)
  handle the error

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbsetint sets the tbl->fields[] value for the field given by ``Age''
 * in the table given by ``table.db'' (this is a table handle)
 * to ``42'' in this example.  The table must be open and ``Age'' must
 * be a valid numerical field.  Upon successful completion,
 * the field is set and 0 is returned.
 * If an error occurs, -1 is returned and cdberror contains a read-only
 * string describing the error.
 *
 * [EndDoc]
 */
int dbsetint (char *arg1, char *arg2, int arg3);
/*
 * [BeginDoc]
 * \subsection{dbsetnum}
 * \index{dbsetnum}
 * [Verbatim]

int status;
...
status = dbsetnum ("table.db", "Salary", 95550.00);
if (status == -1)
  handle the error 

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbsetnum works as dbsetint except it accepts a double value field.
 * If an error occurs, -1 is returned and cdberror is a read-only
 * string describing the error.
 *
 * [EndDoc]
 */
int dbsetnum (char *arg1, char *arg2, double arg3);
/*
 * [BeginDoc]
 * \subsection{dbsetlog}
 * \index{dbsetlog}
 * [Verbatim]

int status;
...
status = dbsetlog ("table.db", "isMarried", 'Y');
if (status == -1)
  handle the error

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbsetlog is used to set a logical field in the
 * tbl->fields[] array of the table ``table.db'' (a table handle).  
 * Other than that, it functions
 * identically to dbsetint.
 * If an error occurs, -1 is returned and cdberror is a read-only
 * string describing the error.
 * [EndDoc]
 */
int dbsetlog (char *arg1, char *arg2, char arg3);
/*
 * [BeginDoc]
 * \subsection{dbsetchar}
 * \index{dbsetchar}
 * [Verbatim]

int status;
...
status = dbsetchar ("table.db", "LName", "May");
if (status == -1)
  handle the error

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbsetchar works as dbsetint except it accepts a string value field.
 * If an error occurs, -1 is returned and cdberror is a read-only
 * string describing the error.
 * [EndDoc]
 */
int dbsetchar (char *arg1, char *arg2, char *arg3);
/*
 * [BeginDoc]
 * \subsection{dbsetdate}
 * \index{dbsetdate}
 * [Verbatim]

int status;
...
status = dbsetdate ("table.db", "dateApplied", "20031105");
if (status == -1)
  handle the error

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbsetdate works as dbsetint except it accepts a date value field.
 * If an error occurs, -1 is returned and cdberror is a read-only
 * string describing the error.
 *
 * [EndDoc]
 */
int dbsetdate (char *arg1, char *arg2, char *arg3);
/*
 * [BeginDoc]
 * \subsection{dbsettime}
 * \index{dbsettime}
 * [Verbatim]

int status;
...
status = dbsettime ("table.db", "timeApplied", "Wed Nov  5 15:23:40 2003");
if (status == -1)
  handle the error

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbsettime works as dbsetint except it accepts a time stamp value.
 * If an error occurs, -1 is returned and cdberror is a read-only
 * string describing the error.
 * [EndDoc]
 */
int dbsettime (char *arg1, char *arg2, char *arg3);
/*
 * [BeginDoc]
 * \subsection{dbadd}
 * \index{dbadd}
 * [Verbatim]

int status;
...
status = dbadd ("table.db");
if (status == -1) {
  if (! strcmp (cdberror, "adding record: unique index constraint violated"))
    deal with unique constraint violations
  else
    handle the error
}

 * [EndDoc] */
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
 * [EndDoc]
 */
int dbadd (char *arg);
/*
 * [BeginDoc]
 * \subsection{dbretrieve}
 * \index{dbretrieve}
 * [Verbatim]

int status;
...
status = dbretrieve ("table.db");
if (status == -1)
  handle the error
if (status == 0)
  skip a deleted field
Here, do something with the data retrieved

 * [EndDoc] */
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
 * [EndDoc]
 */
int dbretrieve (char *arg);
/*
 * BeginDoc
 * \subsection{dbupdate}
 * \index{dbupdate}
 * [Verbatim]

 * EndDoc */
/*
 * BeginDoc
 * EndDoc
 */
int dbupdate (char *arg);
/*
 * [BeginDoc]
 * \subsection{dbdel}
 * \index{dbdel}
 * [Verbatim]

int status;
status = dbdel ("table.db");
if (status == -1)
  handle the error

 * [EndDoc] */
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
 * [EndDoc]
 */
int dbdel (char *arg);
/*
 * [BeginDoc]
 * \subsection{dbundel}
 * \index{dbundel}
 * [Verbatim]

int status;
...
status = dbundel ("table.db");
if (status == -1)
  handle the error

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbundel unmarks the current record for deletion, if it
 * is marked.  If it is not marked, dbundel does nothing.
 * It is an error if the table is not open.
 * If an error occurs, -1 is returned and cdberror is a read-only
 * string describing the error.
 * [EndDoc]
 */
int dbundel (char *arg);
/*
 * [BeginDoc]
 * \subsection{dbisdeleted}
 * \index{dbisdeleted}
 * [Verbatim]

int isdel = 0;
...
isdel = dbisdeleted ("table.db");
if (isdel == -1)
  handle the error
if (isdel)
  dbundel ("table.db");

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbisdeleted returns ``1'' if the current record in the
 * table is marked for deletion, ``0'' otherwise.  It is an error
 * if the table is not open.
 * If an error occurs, -1 is returned and cdberror is a read-only
 * string describing the error.
 * [EndDoc]
 */
int dbisdeleted (char *arg);
/*
 * [BeginDoc]
 * \subsection{dbdbfldname}
 * \index{dbdbfldname}
 * [Verbatim]

#define SIZE 64
char *cp;
char fldnm[SIZE+1];
cp = dbfldname ("table.db", 0);
if (cp == 0)
  handle the error
strcpy (fldnm, cp);

 * [EndDoc] */
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
char *dbfldname (char *arg, int fldnum);
/*
 * [BeginDoc]
 * \subsection{dbfldtype}
 * \index{dbfldtype}
 * [Verbatim]

#define SIZE 10
char *cp;
char fldtyp[SIZE+1];
cp = dbfldtype ("table.db", 0);
if (cp == 0)
  handle the error
strcpy (fldtyp, cp);

 * [EndDoc] */
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
char *dbfldtype (char *arg, int fldnum);
/*
 * [BeginDoc]
 * \subsection{dbfldlen}
 * \index{dbfldlen}
 * [Verbatim]

int fldlen;
...
fldlen = dbfldlen ("table.db", 0);
if (fldlen == -1)
  handle the error

 * [EndDoc] */
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
int dbfldlen (char *arg, int fldnum);
/*
 * [BeginDoc]
 * \subsection{dbflddec}
 * \index{dbflddec}
 * [Verbatim]

int flddec;
...
flddec = dbflddec ("table.db", 0);
if (fldlen == -1)
  handle the error

 * [EndDoc] */
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
int dbflddec (char *arg, int fldnum);
/*
 * [BeginDoc]
 * \subsection{dbcurrent}
 * \index{dbcurrent}
 * [Verbatim]

int status;
...
status = dbcurrent ("table.db", "indexname");
if (status == -1)
  handle the error

 * [EndDoc] */
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
int dbcurrent (char *arg1, char *arg2);
/*
 * [BeginDoc]
 * \subsection{dbgo}
 * \index{dbgo}
 * [Verbatim]

int recno;
...
recno = dbgo ("table.db", recno+1);
if (recno == -1)
  handle the error

 * [EndDoc] */
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
int dbgo (char *arg, int recno);
/*
 * [BeginDoc]
 * \subsection{dbnext}
 * \index{dbnext}
 * [Verbatim]

int status;
...
status = dbnext ("table.db");
if (status == -1)
  handle the error
if (dbiseof ("table.db"))
  deal with end of table condition

 * [EndDoc] */
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
int dbnext (char *arg);
/*
 * [BeginDoc]
 * \subsection{dbnextindex}
 * \index{dbnextindex}
 * [Verbatim]

int status;
...
status = dbnextindex ("table.db");
if (status == -1)
  handle the error
if (dbiseof ("table.db")
  deal with the end of table condition

 * [EndDoc] */
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
 * [EndDoc]
 */
int dbnextindex (char *arg);
/*
 * [BeginDoc]
 * \subsection{dbprev}
 * \index{dbprev}
 * [Verbatim]

int status;
...
status = dbprev ("table.db");
if (status == -1)
  handle the error
if (dbisbof ("table.db"))
  deal with the end of table condition

 * [EndDoc] */
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
 * [EndDoc]
 */
int dbprev (char *arg);
/*
 * [BeginDoc]
 * \subsection{dbprevindex}
 * \index{dbprevindex}
 * [Verbatim]

int status;
...
status = dbprevindex ("table.db");
if (status == -1)
  handle the error
if (dbisbof ("table.db");
  deal with the end of table condition

 * [EndDoc] */
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
 * [EndDoc]
 */
int dbprevindex (char *arg);
/*
 * [BeginDoc]
 * \subsection{dbhead}
 * \index{dbhead}
 * [Verbatim]

int status;
...
status = dbhead ("table.db");
if (status == -1)
  handle the error

 * [EndDoc] */
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
int dbhead (char *arg);
/*
 * [BeginDoc]
 * \subsection{dbheadindex}
 * \index{dbheadindex}
 * [Verbatim]

int status;
...
status = dbheadindex ("table.db");
if (status == -1)
  handle the error

 * [EndDoc] */
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
 * [EndDoc]
 */
int dbheadindex (char *arg);
/*
 * [BeginDoc]
 * \subsection{dbtail}
 * \index{dbtail}
 * [Verbatim]

int status;
...
status = dbtail ("table.db");
if (status == -1)
  handle the error

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbtail sets the current record in the table to
 * the last physical record.  It is an error if
 * the table is not open.  dbtail clears the
 * BOF and EOF markers for the table (sets them
 * to false).
 * If an error occurs, an error is returned and
 * cdberror contains a string describing the error.
 * [EndDoc]
 */
int dbtail (char *arg);
/*
 * [BeginDoc]
 * \subsection{dbtailindex}
 * \index{dbtailindex}
 * [Verbatim]

int status;
...
status = dbtailindex ("table.db");
if (status == -1)
  handle the error

 * [EndDoc] */
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
int dbtailindex (char *arg);
/*
 * [BeginDoc]
 * \subsection{dbsearchindex}
 * \index{dbsearchindex}
 * [Verbatim]

int status;
...
status = dbsearchindex ("table.db", "111111111");
if (status == 0 || status == -1)
  handle the error
if (status == 1)
  found it, do something with it

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbsearchindex searches using the current index for
 * the value given by ``111111111''.  dbsearchindex does a
 * best fit search and only returns ``0'' if an error occurs.
 * So, for example, if ``111111112'' is in the table, the current
 * record pointer would point to that record after this call
 * to dbsearchindex.  dbsearchindex clears the end of file and beginning
 * of file conditions.
 * It is an error if the table is not open or it doesn't have
 * a current index set.  Also, you will get an error condition if you
 * call dbsearchindex on an empty table.
 * If an error occurs, an error is returned and
 * cdberror contains a string that describes the error.
 * [EndDoc]
 */
int dbsearchindex (char *arg1, char *arg2);
/*
 * [BeginDoc]
 * \subsection{dbsearchexact}
 * \index{dbsearchexact}
 * [Verbatim]

int status;
...
status = dbsearchexact ("table.db", "111111111");
if (status == -1)
  handle the error
if (status == 0)
  didn't find exact match
if (status == 1)
  found it, do something with it

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbsearchexact searches using the current index for
 * the value given by ``111111111'', in this example.  This is a
 * bogus Social Security Number.  dbsearchexact does an
 * exact search and returns ``0'' if the item is not found.
 * It is an error if the table is not open or it doesn't have
 * a current index set.  If the item is found, a 1 is returned.
 * If the item is not found or the table is empty, a 0 is returned.
 * If there is an error, -1 is returned.
 * If an error occurs, an error is returned and
 * cdberror contains a string that describes the error.
 * [EndDoc]
 */
int dbsearchexact (char *arg1, char *arg2);
/*
 * [BeginDoc]
 * \subsection{dbpack}
 * \index{dbpack}
 * [Verbatim]

int status;
...
status = dbpack ("table.db");
if (status == -1)
  handle the error

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbpack packs the open table given by the handle ``table.db'' and
 * returns 0 if successful.
 * If an error occurs, an error is returned (-1) and
 * cdberror contains a string describing the error.
 * [EndDoc]
 */
int dbpack (char *arg);
/*
 * [BeginDoc]
 * \subsection{dbreindex}
 * \index{dbreindex}
 * [Verbatim]

#define SIZE 115
int status;
char *cp, table[SIZE+1];
...
status = dbreindex ("table.db");
if (status == -1)
  handle the error
cp = dbopen ("table.db");
if (cp == 0)
  handle the error
strcpy (table, cp);

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbreindex reindexes the table given by tablename and returns
 * 0 if successful.  It is an error if the table \emph{is}
 * open.  So, ``table.db'' in the dbreindex call is not a table handle.
 * If an error occurs, an error is returned (-1) and
 * cdberror contains a string that describes the error.
  * [EndDoc]
 */
int dbreindex (char *arg);
/*
 * [BeginDoc]
 * \subsection{dbexit}
 * \index{dbexit}
 * [Verbatim]


int status;
...
status = dbexit ();
if (status == -1)
  handle the error

 * [EndDoc] */
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
int dbexit (void);
/*
 * [BeginDoc]
 * \subsection{dbtime}
 * \index{dbtime}
 * [EndDoc] 
 */
/*
 * [BeginDoc]
 * dbtime returns a double value that represents the number of seconds since
 * the epoch in fairly high resolution (if your architecture supports it).
 *
 * [EndDoc]
 */
double dbtime (void);

/*
 * [BeginDoc]
 * \subsection{md5sum}
 * \index{md5sum}
 * [Verbatim]

char *md5;

md5 = md5sum ("test.txt");
if (md5 == 0)
  handle the error
printf ("\nmd5sum of test.txt = %s\n", md5);

 * [EndDoc] */
/*
 * [BeginDoc]
 * md5sum will get an md5 hash on the file given by ``test.txt''.  If the
 * user doesn't have the permission to open the file or an error occurs,
 * 0 (NULL) is returned and cdberror contains a string describing the error.
 * [EndDoc]
 */
unsigned char *md5sum (char *fname);
/*
 * [BeginDoc]
 * \subsection{sha1sum}
 * \index{sha1sum}
 * [Verbatim]

char *sha1;

sha1 = sha1sum ("test.txt");
if (sha1 == 0)
  handle the error
printf ("\nsha1sum of test.txt = %s\n", sha1);

 * [EndDoc] */
/*
 * [BeginDoc]
 * sha1sum will get an sha1 hash on the file given by ``test.txt''.  If the
 * user doesn't have the permission to open the file or an error occurs,
 * 0 (NULL) is returned and cdberror contains a string describing the error.
 * [EndDoc]
 */
unsigned char *sha1sum (char *fname);
/*
 * [BeginDoc]
 * \subsection{rmd160sum}
 * \index{rmd160sum}
 * [Verbatim]

char *rmd;

rmd = rmd160sum ("test.txt");
if (rmd == 0)
  handle the error
printf ("\nrmd160sum of test.txt = %s\n", rmd);

 * [EndDoc] */
/*
 * [BeginDoc]
 * rmd160sum will get an rmd160 hash on the file given by ``test.txt''.  If the
 * user doesn't have the permission to open the file or an error occurs,
 * 0 (NULL) is returned and cdberrror contains a string describing the error.
 * [EndDoc]
 */
unsigned char *rmd160sum (char *fname);
/*
 * [BeginDoc]
 * 
 * \index{dbtime}
 * \subsection{dbtime}
 * 
 * [Verbatim]

double t1, t2;

t1 = dbtime ();
... do some time consuming stuff here ...
t2 = dbtime ();

printf ("\nIt took %6.4f seconds to do that\n", t2-t1);

 * [EndDoc] */
/*
 * [BeginDoc] 
 * dbtime will return a double value that is a fairly high resolution
 * timing value (if your architecture supports it).  It uses the
 * elapsed() function defined in btime.c.
 * 
 * [EndDoc]
 */
double dbtime (void);
/*
 * [BeginDoc]
 * \subsection{dbnumidx}
 * \index{dbnumidx}
 * [Verbatim]

int numidx;
...
numidx = dbnumidx ("table.db");
if (numidx == -1)
  handle the error
if (numidx == 0)
  there are no midx's

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbnummidx returns the number of multi-field indexes in the table
 * given by tablehandle.  It is not an error if there are no multi-field indexes
 * for the table.  If an error occurs, dbnummidx returns -1 and cdberror contains
 * a string that describes the error.
 *
 * [EndDoc]
 */

int dbnummidx (char *arg);
/*
 * [BeginDoc]
 * \subsection{dbismidx}
 * \index{dbismidx}
 * [Verbatim]

int ismulti;
...
ismulti = dbismidx ("table.db");
if (ismulti == -1)
  handle the error
if (ismulti == 0)
  no multi indexes
else
  there are multi indexes

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbismidx returns TRUE (1) if there are multi-field indexes associated with
 * the table given by tablehandle table.  It returns FALSE (0) otherwise.
 *
 * [EndDoc]
 */
int dbismidx (char *arg);
/*
 * [BeginDoc]
 * \subsection{dbmidxname}
 * \index{dbmidxname}
 * [Verbatim]

int status;
char *midx;
...
midx = dbmidxname ("table.db", 0);
if (midx == 0)
  handle the error
status = dbcurrent ("table.db", midx);
if (status == -1)
  handle the error

 * [EndDoc] */
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
char *dbmidxname (char *arg, int midxnum);
/*
 * [BeginDoc]
 * \subsection{dbmidxblksz}
 * \index{dbmidxblksz}
 * [Verbatim]

int blksize;
...
blksize = dbmidxblksz ("table.db", "lname_fname");
if (blksize == -1)
  handle error

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbmidxblksz returns the blocksize of the multi-field index 
 * midxname (``lname_fname'') attached
 * to the table given by tablehandle table (``table.db'').  
 * It is an error if the table has
 * no multi-field indexes or if it has none called midxname.
 * If an error occurs, -1 is returned and cdberror contains a string
 * that describes the error.
 *
 * [EndDoc]
 */
int dbmidxblksz (char *arg1, char *arg2);
/*
 * [BeginDoc]
 * \subsection{dbmidxnumfldnames}
 * \index{dbmidxnumfldnames}
 * [Verbatim]

int numfldnms;
...
numfldnms = dbmidxnumfldnames ("table.db", "lname_fname");
if (numfldnms == -1)
  handle error

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbmidxnumfldnames returns the number of fields that are used to generate
 * the multi-field index midxname (``lname_fname'') which is attached 
 * to the table given by tablehandle table (``table.db'').  
 * It is an error if the table has no multi-field indexes
 * attached to it or if it has none called midxname.
 * If an error occurs, -1 is returned and cdberror contains a string
 * that describes the error.
 *
 * [EndDoc]
 */
int dbmidxnumfldnames (char *arg1, char *arg2);
/*
 * [BeginDoc]
 * \subsection{dbmidxfldname}
 * \index{dbmidxfldname}
 * [Verbatim]

char *fldnm;
...
fldnm = dbmidxfldname ("table.db", "l_fname", 0);
if (0 == fldnm)
  handle the error

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbmidxfldname returns the num'th field name that makes up the multi-index
 * midxname (``l_fname'') which is attached to the table given by tablehandle 
 * ``table.db''.  It is
 * an error if the table has no multi-field indexes attached to it or if it has
 * none named ``l_fname'' in this case.  It is also an error if the 
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
char *dbmidxfldname (char *arg1, char *arg2, int num);
/*
 * [BeginDoc]
 * \subsection{dbisindexed}
 * \index{dbisindexed}
 * [Verbatim]

int isidx;
...
isidx = dbisindexed ("table.db", "SSNumber");
if (isidx == -1)
  handle the error

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbisindexed returns TRUE (1) if the field name fieldname in the table given by
 * the tablehandle table is indexed.  It returns FALSE (0) otherwise.
 * It is an error if fieldname (``SSNumber'') does not exist in the table.
 * If an error occurs, -1 is returned and cdberror contains a string
 * that describes the error.
 *
 * [EndDoc]
 */
int dbisindexed (char *arg1, char *arg2);
/*
 * [BeginDoc]
 * \subsection{dbidxblksz}
 * \index{dbidxblksz}
 * [Verbatim]

int blksz;
...
blksz = dbidxblksz ("table.db", "SSNumber");
if (blksz == -1)
  handle the error

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbidxblksz returns the block size of the index generated for the field
 * ``SSNumber'' in the table given by the tablehandle ``table.db''.  
 * It is an error if
 * the field is not indexed.  You should call dbisindexed to insure that the
 * field is indexed before you call dbidxblksz.
 * If an error occurs, -1 is returned and cdberror contains a string
 * that describes the error.
 *
 * [EndDoc]
 */
int dbidxblksz (char *arg1, char *arg2);
/*
 * [BeginDoc]
 * \subsection{dbshowinfo}
 * \index{dbshowinfo}
 * [Verbatim]

char *tblinfo;
...
tblinfo = dbshowinfo ("table.db");
if (tblinfo == 0)
  handle the error
printf ("\nThe table info string for table.db is: %s\n", tblinfo);

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbshowinfo returns the contents of the info string that is stored in the
 * table header.  This is specific to the application and has no meaning as
 * far as DCDB is concerned.  It can be a string of up to 125 characters and
 * can contain any value the application programmer desires.  It is set with
 * the ``info'' portion of the ``create'' command in a .df file.
 * \emph{Note: the result of this call should be used immediately as
 * subsequent calls to certain functions will overwrite it.}
 *
 * [EndDoc]
 */
char *dbshowinfo (char *arg1);

/*
 * [BeginDoc]
 * \subsection{dbteststring}
 * \index{dbteststring}
 * [Verbatim]

char *tstr;
...
tstr = dbteststring (0, 35);
if (tstr == 0)
  handle error
printf ("%s:", tstr);

 * [EndDoc] */
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
char *dbteststring (int arg1, int arg2);
/*
 * [BeginDoc]
 * \subsection{dbtestupperstring}
 * \index{dbtestupperstring}
 * [Verbatim]

char *tstr;
...
tstr = dbtestupperstring (0, 64);
if (tstr == 0)
  handle error
printf ("%s:", tstr);

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbtestupperstring works the same as dbteststring except that the string
 * returned will consist of upper-case alphabetical digits.
 *
 * [EndDoc]
 */
char *dbtestupperstring (int arg1, int arg2);
/*
 * [BeginDoc]
 * \subsection{dbtestmixedstring}
 * \index{dbtestmixedstring}
 * [Verbatim]

char *tstr;
...
tstr = dbtestmixedstring (0, 35);
if (tstr == 0)
  handle error
printf ("%s:", tstr);

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbtestmixedstring works the same as dbteststring except that the string
 * returned will be of mixed upper and lower-case alphabetical digits.
 *
 * [EndDoc]
 */
char *dbtestmixedstring (int arg1, int arg2);
/*
 * [BeginDoc]
 * \subsection{dbtestnumber}
 * \index{dbtestnumber}
 * [Verbatim]

char *tstr;
...
tstr = dbtestnumber (9,0);
if (tstr == 0)
  handle error
printf ("%s:", tstr);

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbtestnumber works the same as dbteststring except that it returns a string
 * of numerical digits.
 *
 * [EndDoc]
 */
char *dbtestnumber (int arg1, int arg2);
/*
 * [BeginDoc]
 * \subsection{dbtestnumstring}
 * \index{dbtestnumstring}
 * [Verbatim]

char *tstr;
...
tstr = dbtestnumstring (7,0);
if (tstr == 0)
  handle error
printf ("New license plate number = %s\n", tstr);

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbtestnumstring works the same as dbteststring except that it returns a
 * string of combined numerical and upper-case alphabetical digits.
 *
 * [EndDoc]
 */
char *dbtestnumstring (int arg1, int arg2);

/*
 * [BeginDoc]
 * \subsection{dbencrypt}
 * \index{dbencrypt}
 * [Verbatim]

char *enc;
int size;
...
size = strlen (data);
enc = dbencrypt (data, size, "xOkEj30>");
if (enc == 0)
  handle the error
printf ("\n\nEncrypted data: %s\n", enc);

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbencrypt encrypts the data in the data argument using the password in the
 * passwd argument.  The size argument should be a multiple of 8, although if
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
char *dbencrypt (char *arg1, int arg2, char *arg3);
/*
 * [BeginDoc]
 * \subsection{dbdecrypt}
 * \index{dbdecrypt}
 * [Verbatim]

char *denc;
...
denc = dbdecrypt (data, size, "xOkEj30>");
if (enc == 0)
  handle the error
printf ("\n\nEncrypted data: %s\n", enc);

 * [EndDoc] */
/*
 * [BeginDoc]
 * dbdecrypt decrypts the data given by the data argument using the password
 * given by passwd.  The data argument should contain data that was encrypted
 * with dbencrypt and the passwd argument should have the same password used
 * to encrypt the data.  The size argument should also be the size of the
 * data before it was encrypted.
 *
 * [EndDoc]
 */
char *dbdecrypt (char *arg1, int arg2, char *arg3);

/*
 * [BeginDoc]
 * \subsection{crc32sum}
 * \index{crc32sum}
 * [Verbatim]

char *crcsum;
...
crcsum = crc32sum ("test.txt");
if (crcsum == 0)
  handle the error
printf ("\ncrcsum of test.txt: %s\n", crcsum);

 * [EndDoc] */
/*
 * [BeginDoc]
 * crc32sum will generate the crc32 check sum of the file given by fname and
 * return that value.  This is the 32 bit check sum code from 
 * /usr/src/linux/fs/jffs2/crc32[h,c] from the 2.4.18 kernel modified for my
 * use here.
 *
 * [EndDoc]
 */
char *crc32sum (char *fname);

/*
#include <number.h>
#include <bcnum.h>

void bc_init_numbers (void);
bc_num bc_new_num (int length, int scale);
void bc_free_num (bc_num *num);
bc_num bc_copy_num (bc_num num);
void bc_init_num (bc_num *num);
void bc_str2num (bc_num *num, char *str, int scale);
char *bc_num2str (bc_num num);
void bc_int2num (bc_num *num, int val);
long bc_num2long (bc_num num);
int bc_compare (bc_num n1, bc_num n2);
char bc_is_zero (bc_num num);
char bc_is_near_zero (bc_num num, int scale);
char bc_is_neg (bc_num num);
void bc_add (bc_num n1, bc_num n2, bc_num *result, int scale_min);
void bc_sub (bc_num n1, bc_num n2, bc_num *result, int scale_min);
void bc_multiply (bc_num n1, bc_num n2, bc_num *prod, int scale);
int bc_divide (bc_num n1, bc_num n2, bc_num *quot, int scale);
int bc_modulo (bc_num num1, bc_num num2, bc_num *result,
			   int scale);
int bc_divmod (bc_num num1, bc_num num2, bc_num *quot,
			   bc_num *rem, int scale);
int bc_raisemod (bc_num base, bc_num expo, bc_num mod,
			     bc_num *result, int scale);
void bc_raise (bc_num num1, bc_num num2, bc_num *result,
			   int scale);
int bc_sqrt (bc_num *num, int scale);
void bc_out_num (bc_num num, int o_base, void (* out_char)(int),
			     int leading_zero);

			     */
typedef enum _bcnum_error_type {
  BCNUM_NOERROR,                  /* no problema */
  BCNUM_MEMORY,                   /* memory exhausted */
  BCNUM_TOOSMALL,                 /* output string is too small */
  /* new ones here */
  BCNUM_UNSPECIFIED               /* unspecified error */
} bcnumErrorType;

extern int bcnum_outstr_ptr;
extern char bcnum_outstr[];
extern int bcnum_is_init;
extern bcnumErrorType bcnumError;
extern char *bcnumErrMsg[];

/*
 * [BeginDoc]
 *
 * \index{bcnumadd}
 * \subsection{bcnumadd}
 *
 * cp = bcnumadd ("24.70", "15.17", 2);
 *
 * bcnumadd uses the number engine from bc to add the two numbers.
 *
 * [EndDoc]
 */
char *bcnumadd (char *arg1, char *arg2, int arg3);

/*
 * [BeginDoc]
 *
 * \index{bcnumsub}
 * \subsection{bcnumsub}
 *
 * total = bcnumsub ("240.37", "23.28", 2);
 *
 * bcnumsub uses the number engine from bc to subtract the two numbers.
 *
 * [EndDoc]
 */
char *bcnumsub (char *arg1, char *arg2, int arg3);

/*
 * [BeginDoc]
 *
 * \index{bcnumcompare}
 * \subsection{bcnumcompare}
 *
 * set result [ bcnumcompare $num1 $num2 2 ]
 *
 * bcnumcompare uses the number engine from bc to compare the two numbers.
 *
 * [EndDoc]
 */
int bcnumcompare (char *arg1, char *arg2, int arg3);

/*
 * [BeginDoc]
 *
 * \index{bcnummultiply}
 * \subsection{bcnummultiply}
 *
 * total = bcnummultiply ("240.37", "-23.28", 2);
 *
 * bcnummultiply uses the number engine from bc to multiply the two numbers.
 *
 * [EndDoc]
 */
char *bcnummultiply (char *arg1, char *arg2, int arg3);

/*
 * [BeginDoc]
 *
 * \index{bcnumdivide}
 * \subsection{bcnumdivide}
 *
 * total = bcnumdivide ("240.37", "-23.28", 2);
 *
 * bcnumdivide uses the number engine from bc to divide the two numbers.
 *
 * [EndDoc]
 */
char *bcnumdivide (char *arg1, char *arg2, int arg3);

/*
 * [BeginDoc]
 *
 * \index{bcnumraise}
 * \subsection{bcnumraise}
 *
 * total = bcnumraise ("240.37", "-23.28", 2);
 *
 * bcnumraise uses the number engine from bc to raise the fist number to the
 * power of the second number.
 *
 * [EndDoc]
 */
char *bcnumraise (char *arg1, char *arg2, int arg3);

/*
 * [BeginDoc]
 * \index{bcnumiszero}
 * \subsection{bcnumiszero}
 *
 * status = bcnumiszero (result);
 *
 * bcnumiszero uses the GNU bc number engine to determine if arg1 is zero and
 * returns 1 if it is and 0 if it is not.
 *
 * [EndDoc]
 */

int bcnumiszero (char *arg1);


/*
 * [BeginDoc]
 * \index{bcnumisnearzero}
 * \subsection{bcnumisnearzero}
 *
 * status = bcnumisnearzero (result, 2);
 *
 * bcnumisnearzero uses the GNU bc number engine to determine if arg1 is near
 * zero.  It returns 1 if it is and 0 if it is not.
 *
 * [EndDoc]
 */

int bcnumisnearzero (char *arg1, int scale);

/*
 * [BeginDoc]
 * \index{bcnumisneg}
 * \subsection{bcnumisneg}
 *
 * status = bcnumisneg (result);
 *
 * bcnumisneg uses the GNU bc number engine to determine if arg1 is negative.
 * It returns 1 if it is and 0 if it is not.
 *
 * [EndDoc]
 */

int bcnumisneg (char *arg1);

/*
 * [BeginDoc]
 *
 * \index{bcnumuninit}
 * \subsection{bcnumuninit}
 *
 * bcnumuninit();
 *
 * bcnumuninit de-initializes the bcnum stuff (basically, freeing some
 * bcnum numbers that are preallocated).  This should be called when you are
 * done with the bcnum bindings.  If you don't do it manually, there is an
 * atexit() function that will do it for you.  This only returns _OK_.
 *
 * [EndDoc]
 */
int bcnumuninit (void);


/*
 * Container stuff.
 */
extern char container_error[];
extern char container_syntax_error[];

char *containerInit (char *fname);
int containerDelete (char *handle);
void containerClearError (void);
int containerSetInt (char *handle, char *field, int value);
int containerSetLong (char *handle, char *field, long value);
int containerSetFloat (char *handle, char *field, float value);
int containerSetDouble (char *handle, char *field, double value);
int containerSetString (char *handle, char *field, char *value);
char *containerGetField (char *handle, char *field);
int containerAddRecord (char *handle);
int containerSearch (char *handle, char *value);
int containerDeleteRecord (char *handle, char *value);
int containerFirst (char *handle);
int containerLast (char *handle);
int containerPrev (char *handle);
int containerNext (char *handle);
int containerBOF (char *handle);
int containerEOF (char *handle);
int containerNumRecords (char *handle);

#ifdef __cplusplus__
}
#endif

#endif	/* __CINT_CDB_H__ */
