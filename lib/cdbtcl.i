/*	Interface file:	cdbtcl.i	*/

/*
 * [BeginDoc]
 * 
 * \section{Tcl/Tk Interface Bindings}
 * 
 * Tcl bindings are provided through the interface file cdbtcl.i in the src
 * directory of the cdb distribution.  This file is a SWIG interface file that
 * allows SWIG to set up the hooks with the C functions in DCDB.
 * 
 * There are three \emph{typemaps} provided in cdbtcl.i (for an explanation of
 * typemaps, see the SWIG documentation).  These essentially cause all error
 * conditions returned from DCDB functions to cause an exception to be raised.
 * The implication of this is that your tcl script will exit with an exception
 * condition (leaving tables open, etc) if you don't handle exceptions in your
 * Tcl code.  Look at at the scripts in (topdir)/tests/tcl for examples of how
 * to use Tcl to do real work with DCDB.
 * 
 * The bindings for Tcl and Tk are identical and are both documented in this
 * portion of the documentation.
 * 
 * [EndDoc]
 */

%module cdbtcl
%{

#include <interface.h>

/*
 * 20021205 - DFM
 * The default for this is dumb, I think.
 */
#define	SWIG_RcFileName	"~/.wishrc"
%}

%include tclsh.i

%typemap (out) char * {
	Tcl_Obj *tcl_result = Tcl_GetObjResult(interp);
	if (result == NULL)	{
		/*tcl_result = Tcl_GetObjResult(interp);*/
		Tcl_SetStringObj (tcl_result, cdberror, -1);
		return TCL_ERROR;
	}
	else	{
		Tcl_SetStringObj (tcl_result, result, -1);
		return TCL_OK;
	}
}

%typemap (out) unsigned char * {
	Tcl_Obj *tcl_result = Tcl_GetObjResult(interp);
	if (result == NULL)	{
		Tcl_SetStringObj (tcl_result, " ", -1);
		return TCL_ERROR;
	}
	else	{
		Tcl_SetStringObj (tcl_result, result, -1);
		return TCL_OK;
	}
}

%typemap (out) int {
	Tcl_Obj *tcl_result = Tcl_GetObjResult(interp);
	if (result == _ERROR_)	{
		Tcl_SetStringObj (tcl_result, cdberror, -1);
		return TCL_ERROR;
	}
	else	{
		Tcl_SetObjResult (interp, Tcl_NewIntObj((long) result));
		return TCL_OK;
	}
}

/*
 * [BeginDoc]
 * 
 * \section{DCDB Tcl/Tk Bindings}
 * 
 * The Tcl/Tk bindings for DCDB provide simple wrappers for the
 * functions that serve as the core of DCDB.  These functions
 * allow the Tcl developer to use the complete capabilities of
 * DCDB with the simplicity and quick development of Tcl scripting.
 * 
 * Internally, all tables opened in Tcl scripts are maintained in a
 * simple array.  Each time a table is accessed via a table
 * handle, the array is searched for the associated ``dbTable'' item.
 * Random searches through an array like this are inherently
 * inefficient.  For the sake of performance, therefore, it is preferable
 * to keep the number of concurrently open tables down to fewer than
 * twenty or so.  If more tables are needed concurrently, the developer should
 * consider using a different data structure to store table handles
 * (a hash would provide better performance for more items, for example).
 * Such a change would require a change to the cdb.c file in the DCDB
 * lib directory.  There is a limit currently set on the number of
 * concurrently open tables.  That limit is given by the MAX_HANDLES
 * constant.  Change this constant to increase this or decrease it.
 * 
 * The developer should read the README file for information on
 * creating the ``libcdbtcl.so'' and ``libconttcl.so'' shared objects.
 * Both of these are needed to execute the test scripts in Tcl.
 *
 * All of the Tcl bindings for DCDB require arguments except dbexit.  If the
 * command is entered without an argument, a ``usage'' string
 * is returned and an error is raised.  Care should be taken to insure
 * that production tables are not corrupted during script development.
 * 
 * The DCDB Tcl commands that require a table handle all operate
 * on an open table.  If an error occurs, it is possible that the
 * script will abort without cleaning up any open tables.  To
 * avoid that possibility in production code, the developer
 * should call the commands in a Tcl ``catch'' construct.
 * This will allow the script to call ``dbexit'' before
 * terminating to properly close all open tables and indexes.
 * \emph{Failure to do so will result in data corruption in your
 * indexes and possibly your tables.}
 *
 * See the sample scripts for examples of how to protect your tables
 * while manipulating them.  The following documentation should be
 * considered reference material only.  It \emph{will not} show you how to
 * use the Tcl bindings to do real work with DCDB.  You should study the
 * test scripts and use them as examples of how to write code using the
 * DCDB bindings and Tcl.
 * 
 * [EndDoc]
 */

/*
 * [BeginDoc]
 * \subsection{dbcreate}
 * \index{dbcreate}
 * dbcreate definitionfile
 *
 * dbcreate creates the tables and workspaces defined in
 * the definition file given by ``definitionfile''.  If an
 * error occurs, an error is raised and a string explaining
 * the error is returned.  If dbcreate is successful,
 * ``OK'' is returned.
 * 
 * For examples of using dbcreate, look at the scripts in
 * (topdir)/tests/tcl.  Basically, all of these scripts do the
 * following:
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
 * This is the simplest way to insure that the tables and indexes
 * needed by your script are created.
 * [EndDoc]
 */
int dbcreate (char *arg);

/*
 * [BeginDoc]
 * \index{dbopen}
 * \subsection{dbopen}
 *
 * set table [ dbopen tablename ]
 *
 * dbopen opens the table given by ``tablename''.  It is an
 * error if the table doesn't exist.  Upon success, dbopen
 * returns a table handle (really, just the table name) that
 * is used in subsequent DCDB Tcl commands.  This value should
 * be stored in a variable for further use.  If an error occurs,
 * dbopen raises an error and returns a string that describes
 * the error.  If other tables are open, the developer should
 * execute the dbopen in a Tcl ``catch'' statement to allow
 * the script to properly close the other open tables before
 * exiting.
 *
 * [EndDoc]
 */

const char *dbopen (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbclose}
 * \subsection{dbclose}
 *
 * dbclose $tablehandle
 *
 * dbclose closes the table given by ``tablehandle''.  It is
 * an error if the table is not open.  Upon successful completion,
 * a table and all it's associated indexes will be properly flushed
 * to disk and closed and ``OK'' is returned.  If an error occurs,
 * dbclose raises an error and returns a string that describes
 * the error.
 * 
 * It's actually more common to use dbexit to close all open tables
 * than to close tables individually.
 *
 * [EndDoc]
 */

int dbclose (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbnumrecs}
 * \subsection{dbnumrecs}
 *
 * set numrecs [ dbnumrecs $tablehandle ]
 *
 * dbnumrecs returns the number of records currently being
 * managed in the table.  It is an error if the table is
 * not open yet.  Upon successful completion, dbnumrecs
 * returns the number of records.  If an error occurs,
 * an exception is raised and a string is returned that
 * describes the error.
 *
 * [EndDoc]
 */

int dbnumrecs (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbnumfields}
 * \subsection{dbnumfields}
 *
 * set numfieds [ dbnumfields $tablehandle ]
 *
 * dbnumfields returns the number of fields
 * in the table.  It is an error if the table is
 * not open yet.  Upon successful completion, dbnumfields
 * returns the number of fields.  If an error occurs,
 * an exception is raised and a string is returned that
 * describes the error.
 *
 * [EndDoc]
 */

int dbnumfields (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbseq}
 * \subsection{dbseq}
 *
 * set seq [ dbseq $tablehandle ]
 *
 * dbseq returns the ``nextSequence'' value stored in the
 * table header of the table given by ``tablehandle'', and then
 * increments the ``nextSequence'' value.  It is an error if
 * the table is not open.  Upon successful completion, dbseq
 * returns the sequence value stored in the table before the
 * call was made.  If an error occurs, an error is raised
 * and a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbseq (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbiseof}
 * \subsection{dbiseof}
 *
 * set status [ dbiseof $tablehandle ]
 *
 * dbiseof returns ``1'' if the end of file indicator for the
 * table is TRUE and ``0'' if it is FALSE.  It is an error if
 * the table is not open.  If an error occurs, an error is
 * raised and a string is returned that describes the error.
 *
 * [EndDoc]
 */

int dbiseof (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbisbof}
 * \subsection{dbisbof}
 *
 * set status [ dbisbof $tablehandle ]
 *
 * dbisbof returns ``1'' if the beginning of file indicator for the
 * table is TRUE and ``0'' if it is FALSE.  It is an error if
 * the table is not open.  If an error occurs, an error is
 * raised and a string is returned that describes the error.
 *
 * [EndDoc]
 */

int dbisbof (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbshow}
 * \subsection{dbshow}
 *
 * set fldval [ dbshow $tablehandle field ]
 *
 * dbshow returns the contents of ``field'' if successful.  If
 * dbretrieve wasn't called before the call to dbshow, the return
 * value will be meaningless.  It is an error if the table
 * is not open or ``field'' is not the name of a valid field in
 * the table.  If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

char *dbshow (char *arg1, char *arg2);

/*
 * [BeginDoc]
 *
 * \index{dbflen}
 * \subsection{dbflen}
 *
 * set fldlen [ dbflen $tablehandle field ]
 *
 * dbflen returns the length of ``field'' for the table.
 * It is an error if the table is not open or ``field'' is not
 * a valid field name.  If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbflen (char *arg1, char *arg2);

/*
 * [BeginDoc]
 *
 * \index{dbdeclen}
 * \subsection{dbdeclen}
 *
 * set declen [ dbdeclen $tablehandle field ]
 *
 * dbdeclen returns the length of decimal portion for ``field''.
 * If the field is not numeric, this will always be 0.
 * It is an error if the table is not open or ``field'' is not
 * a valid field name.  If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbdeclen (char *arg1, char *arg2);

/*
 * [BeginDoc]
 *
 * \index{dbsetint}
 * \subsection{dbsetint}
 *
 * set status [ dbsetint $tablehandle field intval ]
 *
 * dbsetint sets the tbl->fields[] value for ``field''
 * to ``intval''.  The table must be open and ``field'' must
 * be a valid numerical field.  Upon successful completion,
 * the field is set and ``OK'' is returned.
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 * [EndDoc]
 */

int dbsetint (char *arg1, char *arg2, int arg3);

/*
 * [BeginDoc]
 *
 * \index{dbsetnum}
 * \subsection{dbsetnum}
 *
 * set status [ dbsetnum $tablehandle field dblval ]
 *
 * dbsetnum works as dbsetint except it accepts double
 * values, as well.
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbsetnum (char *arg1, char *arg2, double arg3);

/*
 * [BeginDoc]
 *
 * \index{bdsetlog}
 * \subsection{dbsetlog}
 *
 * set status [ dbsetlog $tablehandle field logval ]
 *
 * dbsetlog is used to set a logical field in the
 * tbl->fields[] array.  Other than that, it functions
 * identically to dbsetint.
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbsetlog (char *arg1, char *arg2, char arg3);

/*
 * [BeginDoc]
 *
 * \index{dbsetchar}
 * \subsection{dbsetchar}
 *
 * set status [ dbsetchar $tablehandle field charval ]
 *
 * dbsetchar is used to set character fields to a
 * value.  Other than that, it works identically to
 * dbsetint.
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbsetchar (char *arg1, char *arg2, char *arg3);

/*
 * [BeginDoc]
 *
 * \index{dbsetdate}
 * \subsection{dbsetdate}
 *
 * set status [ dbsetdate $tablehandle field dateval ]
 *
 * dbsetdate is used to set the value of date fields.
 * Other than that, it works identically to dbsetint.
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbsetdate (char *arg1, char *arg2, char *arg3);

/*
 * [BeginDoc]
 *
 * \index{dbsettime}
 * \subsection{dbsettime}
 *
 * set status [ dbsettime $tablehandle field timeval ]
 *
 * dbsettime is used to set the value of a time
 * stamp field.  Other than that, it works identically to
 * dbsetint.
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbsettime (char *arg1, char *arg2, char *arg3);

/*
 * [BeginDoc]
 *
 * \index{dbadd}
 * \subsection{dbadd}
 *
 * set status [ dbadd $tablehandle ]
 *
 * dbadd adds the data placed in the tbl->fields[] array
 * using the dbsetxxx calls to the table.  It is an error
 * if a unique index constraint would be violated by the
 * add.  It is an error if the table is not open.  Upon
 * successful completion, the data will have been added to
 * the table.
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbadd (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbretrieve}
 * \subsection{dbretrieve}
 *
 * set status [ dbretrieve $tablehandle ]
 *
 * dbretrieve retrieves the data for the current record
 * into the tbl->fields[] array where it is accessible
 * via calls to dbshow.  It is an error if the table is
 * not open.  If the current record is marked for
 * deletion, ``0'' is returned.
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbretrieve (char *arg);

/*
 * Note: This is disabled until I can redesign it.
 *
 * [BeginDoc]
 *
 * \index{dbupdate}
 * \subsection{dbupdate}
 *
 * set status [ dbupdate $tablehandle ]
 *
 * dbupdate replaces the contents of the current
 * record with the values stored in the tbl->fields[]
 * array.  It is an error if the table is not open.
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbupdate (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbdel}
 * \subsection{dbdel}
 *
 * set status [ dbdel $tablehandle ]
 *
 * dbdel marks the current record in the table for
 * deletion and returns ``OK'' if successful.
 * The record is not really deleted until dbpack
 * is run on the table, at which point the deletion
 * is irreversible.  It is an error if the table
 * is not open.
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbdel (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbundel}
 * \subsection{cdbundel}
 *
 * set status [ dbundel $tablehandle ]
 *
 * dbundel unmarks the current record for deletion, if it
 * is marked.  If it is not marked, dbundel does nothing.
 * It is an error if the table is not open.
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbundel (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbisdeleted}
 * \subsection{dbisdeleted}
 *
 * set isdel [ dbisdeleted $tablehandle ]
 *
 * dbisdeleted returns ``1'' if the current record in the
 * table is marked for deletion, ``0'' otherwise.  It is an error
 * if the table is not open.
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbisdeleted (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbfldname}
 * \subsection{dbfldname}
 *
 * set fldnm [ dbfldname $tablehandle fldnum ]
 *
 * dbfldname returns the name of the field given by fldnum.  fldnum
 * is a zero-based offset into an array of names (the first item is 0,
 * for example).  If fldnum is less than 0 or greater than the number
 * of fields, an error is raised.  An error is likewise
 * raised if there are no open tables, if the file handle is invalid or
 * on some other critical error.
 *
 * [EndDoc]
 */

char *dbfldname (char *arg, int fldnum);

/*
 * [BeginDoc]
 *
 * \index{dbfldtype}
 * \subsection{dbfldtype}
 *
 * set fldtype [ dbfldtype $tablehandle fldnum ]
 *
 * dbfldtype returns the type of the field given by fldnum.  fldnum
 * is a zero-based offset into an array of names (the first item is 0,
 * for example).  If fldnum is less than 0 or greater than the number
 * of fields, an error is raised.  An error is likewise
 * raised if there are no open tables, if the file handle is invalid or
 * on some other critical error.
 *
 * [EndDoc]
 */

char *dbfldtype (char *arg, int fldnum);

/*
 * [BeginDoc]
 *
 * \index{dbfldlen}
 * \subsection{dbfldlen}
 *
 * set fldlen [ dbfldlen $tablehandle fldnum ]
 *
 * dbfldlen returns the length of the field given by fldnum.  fldnum
 * is a zero-based offset into an array of names (the first item is 0,
 * for example).  If fldnum is less than 0 or greater than the number
 * of fields, an error is raised.  An error is likewise
 * raised if there are no open tables, if the file handle is invalid or
 * on some other critical error.
 *
 * [EndDoc]
 */

int dbfldlen (char *arg, int fldnum);

/*
 * [BeginDoc]
 *
 * \index{dbflddec}
 * \subsection{dbflddec}
 *
 * set flddec [ dbflddec $tablehandle fldnum ]
 *
 * dbflddec returns the length of the field given by fldnum.  fldnum
 * is a zero-based offset into an array of names (the first item is 0,
 * for example).  If fldnum is less than 0 or greater than the number
 * of fields, an error is raised.  An error is likewise
 * raised if there are no open tables, if the file handle is invalid or
 * on some other critical error.
 *
 * [EndDoc]
 */

int dbflddec (char *arg, int fldnum);

/*
 * [BeginDoc]
 *
 * \index{dbcurrent}
 * \subsection{dbcurrent}
 *
 * set status [ dbcurrent $tablehandle indexname ]
 *
 * dbcurrent sets the current index for the table to
 * ``indexname'' and returns ``OK'' on success.
 * It is an error if the table is not open
 * or if ``indexname'' is not a valid index for the table.
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbcurrent (char *arg1, char *arg2);

/*
 * [BeginDoc]
 *
 * \index{dbgo}
 * \subsection{dbgo}
 *
 * set recno [ dbgo tablehandle recno ]
 *
 * dbgo sets the current record to the record numbered
 * ``recno'' (a 32-bit integer value) on success and returns
 * the record number of the current record.  It is an error
 * if the table is not open or ``recno'' is less than
 * one or greater than the number of records.
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbgo (char *arg, int recno);

/*
 * [BeginDoc]
 *
 * \index{dbnext}
 * \subsection{dbnext}
 *
 * set status [ dbnext $tablehandle ]
 *
 * dbnext sets the current record in the table to
 * the next physical record.  It is an error if
 * the table is not open.  If the current record
 * is already the last record when dbnext is called,
 * dbnext sets the end of file marker on the table to
 * TRUE (which can be checked with a call to dbiseof).
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbnext (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbnextindex}
 * \subsection{dbnextindex}
 *
 * set status [ dbnextindex $tablehandle ]
 *
 * dbnextindex sets the current record in the table to
 * the next record by the current index.  It is an error if
 * the table is not open.  If there is no current index,
 * dbnextindex decays to a call to dbnext.
 * If the current record
 * is already the last record when dbnextindex is called,
 * dbnextindex sets the end of file marker on the table to
 * TRUE (which can be checked with a call to dbiseof).
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbnextindex (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbprev}
 * \subsection{dbprev}
 *
 * set status [ dbprev $tablehandle ]
 *
 * dbprev sets the current record in the table to
 * the previous physical record.  It is an error if
 * the table is not open.  If the current record
 * is already the first record when dbprev is called,
 * dbprev sets the beginning of file marker on the table to
 * TRUE (which can be checked with a call to dbisbof).
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbprev (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbprevindex}
 * \subsection{dbprevindex}
 *
 * set status [ dbprevindex $tablehandle ]
 *
 * dbprevindex sets the current record in the table to
 * the previous  record by the current index.  It is an error if
 * the table is not open.  If the current index is not set,
 * dbprevindex decays to a call to dbprev.
 * If the current record
 * is already the first record when dbprevindex is called,
 * dbprevindex sets the beginning of file marker on the table to
 * TRUE (which can be checked with a call to dbisbof).
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbprevindex (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbhead}
 * \subsection{dbhead}
 *
 * set status [ dbhead $tablehandle ]
 *
 * dbhead sets the current record in the table to
 * the first physical record.  It is an error if
 * the table is not open.  dbhead clears the
 * BOF and EOF markers for the table (sets them
 * to false).
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbhead (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbheadindex}
 * \subsection{dbheadindex}
 *
 * set status [ dbheadindex $tablehandle ]
 *
 * dbheadindex sets the current record in the table to
 * the first record by the current index.  It is an error if
 * the table is not open.  If there is no current
 * index set, dbheadindex decays to a call to dbhead.
 * dbheadindex clears the
 * BOF and EOF markers for the table (sets them
 * to false).
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbheadindex (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbtail}
 * \subsection{dbtail}
 *
 * set status [ dbtail $tablehandle ]
 *
 * dbtail sets the current record in the table to
 * the last physical record.  It is an error if
 * the table is not open.  dbtail clears the
 * BOF and EOF markers for the table (sets them
 * to false).
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbtail (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbtailindex}
 * \subsection{dbtailindex}
 *
 * set status [ dbtailindex $tablehandle ]
 *
 * dbtailindex sets the current record in the table to
 * the last record by the current index.  It is an error if
 * the table is not open.  If there is no current index,
 * dbtailindex decays to a call to dbtail.
 * dbtailindex clears the
 * BOF and EOF markers for the table (sets them
 * to false).
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbtailindex (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbsearchindex}
 * \subsection{dbsearchindex}
 *
 * set status [ dbsearchindex $tablehandle fieldvalue ]
 *
 * dbsearchindex searches using the current index for
 * the value given by ``fieldvalue''.  dbsearchindex does a
 * best fit search and only returns ``0'' if an error occurs.
 * It is an error if the table is not open or it doesn't have
 * a current index set.
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbsearchindex (char *arg1, char *arg2);

/*
 * [BeginDoc]
 *
 * \index{dbsearchexact}
 * \subsection{dbsearchexact}
 *
 * set status [ dbsearchexact $tablehandle fieldvalue ]
 *
 * dbsearchexact searches using the current index for
 * the value given by ``fieldvalue''.  dbsearchexact does an
 * exact search and returns ``0'' if the item is not found.
 * It is an error if the table is not open or it doesn't have
 * a current index set.
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * \emph{Note:} You should take extra measures when searching for
 * numerical data based on how it is stored.  See test3.tcl for a
 * discussion of this issue.
 *
 * [EndDoc]
 */

int dbsearchexact (char *arg1, char *arg2);

/*
 * [BeginDoc]
 *
 * \index{dbpack}
 * \subsection{dbpack}
 *
 * set status [ dbpack $tablehandle ]
 *
 * dbpack packs the open table given by the handle and
 * returns ``OK'' if successful.
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbpack (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbreindex}
 * \subsection{dbreindex}
 *
 * set status [ dbreindex tablename ]
 *
 * dbreindex reindexes the table given by tablename and returns
 * ``OK'' if successful.  It is an error if the table \emph{is}
 * open.
 * If an error occurs, an error is raised and
 * a string describing the error is returned.
 *
 * [EndDoc]
 */

int dbreindex (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbexit}
 * \subsection{dbexit}
 *
 * set status [ dbexit ]
 *
 * dbexit is the only DCDB Tcl command that does not take any
 * arguments.  To get a usage string, simply use ``dbexit help''.
 * If there are no open tables, it simply returns
 * ``OK''.  If there are open tables, it closes all of them,
 * cleans up the list used to store table handles and returns
 * ``OK''.
 *
 * [EndDoc]
 */

int dbexit (void);

/*
 * [BeginDoc]
 * 
 * \index{md5sum}
 * \subsection{md5sum}
 * 
 * set md5val [ md5sum fspec ]
 * 
 * md5sum will get an md5 hash on the file given by fspec.  If the
 * user doesn't have the permission to open the file or an error occurs,
 * 0 is returned and an error is raised.
 * 
 * [EndDoc]
 */
unsigned char *md5sum (char *fname);

/*
 * [BeginDoc]
 * 
 * \index{sha1sum}
 * \subsection{sha1sum}
 * 
 * set sha1val [ sha1sum fspec ]
 * 
 * sha1sum will get an sha1 hash on the file given by fspec.  If the
 * user doesn't have the permission to open the file or an error occurs,
 * 0 is returned and an error is raised.
 * 
 * [EndDoc]
 */
unsigned char *sha1sum (char *fname);

/*
 * [BeginDoc]
 * 
 * \index{rmd160sum}
 * \subsection{rmd160sum}
 * 
 * set rmdval [ rmd160sum fspec ]
 * 
 * rmd160sum will get an rmd160 hash on the file given by fspec.  If the
 * user doesn't have the permission to open the file or an error occurs,
 * 0 is returned and an error is raised.
 * 
 * [EndDoc]
 */

unsigned char *rmd160sum (char *fname);

/*
 * [BeginDoc]
 * 
 * \index{dbtime}
 * \subsection{dbtime}
 * 
 * set clock1 [ dbtime ]
 * 
 * dbtime will return a double value that is a fairly high resolution
 * timing value (if your architecture supports it).  It uses the
 * elapsed() function defined in btime.c.
 * 
 * [EndDoc]
 */

double dbtime (void);

/*
 * [BeginDoc]
 *
 * \index{dbnummidx}
 * \subsection{dbnummidx}
 *
 * set number [ dbnummidx $table ]
 *
 * dbnummidx returns the number of multi-field indexes in the table
 * given by tablehandle.  It is not an error if there are no multi-field indexes
 * for the table.
 *
 * [EndDoc]
 */
int dbnummidx (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbismidx}
 * \subsection{dbismidx}
 *
 * set ismultiindexes [ dbismidx $table ]
 *
 * dbismidx returns TRUE if there are multi-field indexes associated with
 * the table given by tablehandle table.  It returns FALSE otherwise.
 *
 * [EndDoc]
 */

int dbismidx (char *arg);

/*
 * [BeginDoc]
 *
 * \index{dbmidxname}
 * \subsection{dbmidxname}
 *
 * set fieldname [ dbmidxname $table $num ]
 *
 * dbmidxname returns the name of the num'th multi-field index for the table
 * given by tablehandle.  It is an error if there are no multi-field indexes
 * for the table.  Use dbismidx first to determine if there are multi-field
 * indexes for the table.
 *
 * [EndDoc]
 */
char *dbmidxname (char *arg, int midxnum);

/*
 * [BeginDoc]
 *
 * \index{dbmidxblksz}
 * \subsection{dbmidxblksz}
 *
 * set blksize [ dbmidxblksz $table $midxname ]
 *
 * dbmidxblksz returns the blocksize of the multi-field index midxname attached
 * to the table given by tablehandle table.  It is an error if the table has
 * no multi-field indexes or if it has none called midxname.
 *
 * [EndDoc]
 */
int dbmidxblksz (char *arg1, char *arg2);

/*
 * [BeginDoc]
 *
 * \index{dbmidxnumfldnames}
 * \subsection{dbmidxnumfldnames}
 *
 * set numfldnms [ dbmidxnumfldnames $table $midxname ]
 *
 * dbmidxnumfldnames returns the number of fields that are used to generate
 * the multi-field index midxname which is attached to the table given by
 * tablehandle table.  It is an error if the table has no multi-field indexes
 * attached to it or if it has none called midxname.
 *
 * [EndDoc]
 */
int dbmidxnumfldnames (char *arg1, char *arg2);

/*
 * [BeginDoc]
 *
 * \index{dbmidxfldname}
 * \subsection{dbmidxfldname}
 *
 * set fldname [ dbmidxfldname $table $midxname $num ]
 * 
 * dbmidxfldname returns the num'th field name that makes up the multi-index
 * midxname which is attached to the table given by tablehandle table.  It is
 * an error if the table has no multi-field indexes attached to it or if it has
 * none named midxname.  It is also an error if the multi-field index given by
 * midxname doesn't have num fields.  You should call dbmidxnumfldnames to
 * determine how many field names are used in generating the multi-field index.
 *
 * [EndDoc]
 */
char *dbmidxfldname (char *arg1, char *arg2, int num);

/*
 * [BeginDoc]
 *
 * \index{dbisindexed}
 * \subsection{dbisindexed}
 *
 * set isindexed [ dbisindexed $table $fieldname ]
 *
 * dbisindexed returns TRUE if the field name fieldname in the table given by
 * the tablehandle table is indexed.  It returns FALSE otherwise.
 *
 * [EndDoc]
 */
int dbisindexed (char *arg1, char *arg2);

/*
 * [BeginDoc]
 *
 * \index{dbidxblksz}
 * \subsection{dbidxblksz}
 *
 * set blksize [ dbidxblksz $table $fieldname ]
 *
 * dbidxblksz returns the block size of the index generated for the field
 * fieldname in the table given by the tablehandle table.  It is an error if
 * the field is not indexed.  You should call dbisindexed to insure that the
 * field is indexed before you call dbidxblksz.
 *
 * [EndDoc]
 */
int dbidxblksz (char *arg1, char *arg2);

/*
 * [BeginDoc]
 *
 * \index{dbshowinfo}
 * \subsection{dbshowinfo}
 *
 * set tableinfo [ dbshowinfo $table ]
 *
 * dbshowinfo returns the contents of the info string that is stored in the
 * table header.  This is specific to the application and has no meaning as
 * far as DCDB is concerned.  It can be a string of up to 125 characters and
 * can contain any value the application programmer desires.  It is set with
 * the ``info'' portion of the ``create'' command in a .df file.
 *
 * [EndDoc]
 */
char *dbshowinfo (char *arg1);

/*
 * [BeginDoc]
 *
 * \index{dbteststring}
 * \subsection{dbteststring}
 *
 * set LName [ dbteststring 0 64 ]
 *
 * dbteststring returns a psuedo-random string of characters, all lower-case.
 * It takes 2 arguments, both integers.  If the first argument is set, the
 * second must be 0.  Conversely, if the second argument is set, the first must
 * be 0.  Setting the first argument gives you a string of that length.  For
 * example, ``dbteststring 10 0'' will return a 10 character string.  Setting
 * the second argument will return a string of somewhat random length between
 * 5 and the length specified.  So, ``dbteststring 0 10'' gives you a random
 * string of lower-case letters between 5 and 10 digits long.  It is an error
 * if both arguments are 0 or if both are non-zero.
 *
 * [EndDoc]
 */
char *dbteststring (int arg1, int arg2);

/*
 * [BeginDoc]
 *
 * \index{dbtestupperstring}
 * \subsection{dbtestupperstring}
 *
 * set LName [ dbtestupperstring 0 64 ]
 *
 * dbtestupperstring works the same as dbteststring except that the string
 * returned will consist of upper-case alphabetical digits.
 *
 * [EndDoc]
 */
char *dbtestupperstring (int arg1, int arg2);

/*
 * [BeginDoc]
 *
 * \index{dbtestmixedstring}
 * \subsection{dbtestmixedstring}
 *
 * set teststr [ dbtestmixedstring 0 35 ]
 *
 * dbtestmixedstring works the same as dbteststring except that the string
 * returned will be of mixed upper and lower-case alphabetical digits.
 *
 * [EndDoc]
 */
char *dbtestmixedstring (int arg1, int arg2);

/*
 * [BeginDoc]
 *
 * \index{dbtestnumber}
 * \subsection{dbtestnumber}
 * 
 * set SSN [ dbtestnumber 9 0 ]
 *
 * dbtestnumber works the same as dbteststring except that it returns a string
 * of numerical digits.
 *
 * [EndDoc]
 */
char *dbtestnumber (int arg1, int arg2);

/*
 * [BeginDoc]
 *
 * \index{dbtestnumstring}
 * \subsection{dbtestnumstring}
 *
 * set LicensePlate [ dbtestnumstring 7 0 ]
 *
 * dbtestnumstring works the same as dbteststring except that it returns a
 * string of combined numercal and upper-case alphabetical digits.
 *
 * [EndDoc]
 */
char *dbtestnumstring (int arg1, int arg2);

/*
 * [BeginDoc]
 *
 * \index{dbencrypt}
 * \subsection{dbencrypt}
 *
 * set encrypted [ dbencrypt $data $size $password ]
 *
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
 *
 * \index{dbdecrypt}
 * \subsection{dbdecrypt}
 *
 * set decrypted [ dbdecrypt $data $size $passwd ]
 *
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
 *
 * \index{crc32sum}
 * \subsection{crc32sum}
 *
 * set crcsum [ crc32sum $fname ]
 *
 * crc32sum will generate the crc32 check sum of the file given by fname and
 * return that value.  This is the 32 bit check sum code from 
 * (LINUX_SRC)/fs/jffs2/crc32* from the 2.4.18 kernel modified for my
 * my use here.
 *
 * [EndDoc]
 */
char *crc32sum (char *arg1);

/*
 * [BeginDoc]
 *
 * \index{bcnumadd}
 * \subsection{bcnumadd}
 *
 * set total [ bcnumadd 240.37 23.28 2 ]
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
 * set total [ bcnumsub 240.37 23.28 2 ]
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
 * set total [ bcnummultiply 240.37 -23.28 2 ]
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
 * set total [ bcnumdivide 240.37 -23.28 2 ]
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
 * set total [ bcnumraise 240.37 -23.28 2 ]
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
 * set status [bcnumiszero $result]
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
 * set status [bcnumisnearzero $result 2]
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
 * set status [bcnumisneg $result]
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
 * bcnumuninit
 *
 * bcnumuninit de-initializes the bcnum stuff (basically, freeing some
 * bcnum numbers that are preallocated).  This should be called when you are
 * done with the bcnum bindings.  If you don't do it manually, there is an
 * atexit() function that will do it for you.  This only returns _OK_.
 *
 * [EndDoc]
 */
int bcnumuninit (void);

%include conttcl.i
