/*	Header File:	cdbpp.h	*/

/*
 * Implements the class wrappers for the DCDB Database Engine.
 */

#ifndef	__CDBPP_H__
#define	__CDBPP_H__

#include <cdb.h>
#include <fsystem.h>

#ifdef	NEW_STANDARD
#include <sstream>
#endif

#ifndef	NEW_STANDARD
#include <strstream>
#endif

#include <iostream>
#include <iomanip>

extern "C++" {

/*
 * [BeginDoc]
 * 
 * \section{DCDB C++ Wrappers}
 * 
 * The header file cdbpp (a forward include file for cdbpp.h)
 * provides a wrapper class for the DCDB database engine.
 * This class simply wraps the functionality surrounding a table
 * (creating tables, opening tables, adding records, etc.).
 * 
 * \subsection{table: Data Members}
 * 
 * The class that encapsulates the data and functionality
 * necessary to manipulate a table is called ``class table''.
 * The private members of the class are all data members and
 * are as follows:
 * [EndDoc]
 */

class table 
{
	/*
	 * [BeginDoc]
	 * [Verbatim] */

	dbTable *tbl;
	bool iserror;
	dbErrorType dberr;
	char error[MAX_ERROR_STRING+1];

	/* [EndDoc] */

	/*
	 * [BeginDoc]
	 * 
	 * The ``tbl'' member is of type dbTable (a table handle
	 * that is used by the DCDB Database Engine to work with
	 * tables).  ``iserror'' will be true if an error has
	 * occurred.  Should an error occur, ``dberr'' will indicate
	 * what the error is and ``error'' will be a string that
	 * describes the error.
	 * [EndDoc]
	 */
  public:

	/*
	 * [BeginDoc]
	 * 
	 * \subsection{table: Constructors and Destructor}
	 * 
	 * The public members of the table class are all functions
	 * that allow access to the private data and support data
	 * hiding.  There are several constructors that allow for
	 * \index{constructor}
	 * the creation of tables or the opening of existing tables.
	 * \index{table::table(const char *)}
	 * [Verbatim]	 */

	table (const char *name)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * This constructor opens a table that already exists.  It is an
	 * error if the table doesn't exists or if the DCDB routines cannot
	 * successfully open it.
	 * 
	 * If an error occurs and ``obj'' is an instantiation of a table for
	 * which the error occurred, then obj.isError() will
	 * return true, obj.getError() will return the error code (of
	 * type ``dbErrorType'', see cdb.h for details) and 
	 * obj.getErrorStr() will return a pointer to a string
	 * that describes the error.
	 * 
	 * [EndDoc]
	 */
	{
		iserror = false;
		dberr = DB_NOERROR;
		tbl = openTable (name);
		globalError();
	}

	/*
	 * [BeginDoc]
	 * \index{table::table(const char *,const char *,int)}
	 * [Verbatim] */

	table (const char *parsfile,
		   const char *tblname,
		   int rebuild = true)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * This constructor creates a table if it doesn't already exist
	 * or ``rebuild'' is true by parsing a definition file ``parsfile''.
	 * The definition file is assumed to create a table with the name
	 * ``tblname'' and it is an error if it does not.  Also, it is an
	 * error if ``parsfile'' doesn't exist or can't be opened for reading.
	 * 
	 * If an error occurs and ``obj'' is an instantiation of a table for
	 * which the error occurred, then obj.isError() will
	 * return true, obj.getError() will return the error code (of
	 * type ``dbErrorType'', see cdb.h for details) and 
	 * obj.getErrorStr() will return a pointer to a string
	 * that describes the error.
	 * 
	 * [EndDoc]
	 */
	{
		iserror = false;
		dberr = DB_NOERROR;
		tbl = 0;
		std::ostrstream omem (syntaxError, SYNTAX_ERROR_WIDTH+1);
		int table_exists = fexists ((char *)tblname);
		/* make sure the silly parsfile is there. */
		if (!fexists ((char *)parsfile))	{
			dbError = DB_SYNTAX;
		/*	sprintf (syntaxError, "parse file \"%s\" does not exist",
					(char *)parsfile);*/
			omem << "parse file " << (char *)parsfile << " does not exist" << std::endl;
			return;
		}
	/*	if (rebuild || !fexists ((char *)tblname) )	{*/
		if ((table_exists && rebuild) || !table_exists) {
			parseDBDef (parsfile);
			globalError();
			if (isError())	{
				tbl = 0;
				return;
			}
		}
		if (fexists ((char *)tblname))
			tbl = openTable (tblname);
		globalError();
	}

	/*
	 * [BeginDoc]
	 * \index{table::table(dbTable *)}
	 * [Verbatim] */

	table (dbTable *table)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * This constructor takes an open table given by the handle
	 * ``tbl'' and wraps it with an instantiation of the table
	 * class.
	 * 
	 * If an error occurs and ``obj'' is an instantiation of a table for
	 * which the error occurred, then obj.isError() will
	 * return true, obj.getError() will return the error code (of
	 * type ``dbErrorType'', see cdb.h for details) and 
	 * obj.getErrorStr() will return a pointer to a string
	 * that describes the error.
	 * 
	 * [EndDoc]
	 */
	{
		iserror = false;
		dberr = DB_NOERROR;
		tbl = table;
	}

	/*
	 * [BeginDoc]
	 * \index{destructor}
	 * [Verbatim] */

	~table (void)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * The destructor for the table class merely closes the table using
	 * the DCDB closeTable() function.  This insures that the data is
	 * flushed to disk before the table class object is destroyed.
	 * 
	 * [EndDoc]
	 */
	{
		if (tbl != 0)
			closeTable (tbl);
		tbl = 0;
	}

	/*
	 * [BeginDoc]
	 * \subsection{Table information functions}
	 * 
	 * The following functions provide information about the table
	 * that is wrapped by the instance of the table class.
	 * \index{table::numRecords()}
	 * [Verbatim] */
	
	const int numRecords(void)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * This function returns the number of records currently managed
	 * by the table.
	 * [EndDoc]
	 */

	{
		return tbl->hdr->numRecords;
	}

	const int numFields (void)
	{
		return tbl->hdr->numFields;
	}
	/*
	 * [BeginDoc]
	 * \index{table::isBOF()}
	 * [Verbatim] */

	const int isBOF (void)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * isBOF() returns true if the beginning of the table has been reached
	 * in a sequential move, either using an index or the physical order
	 * of the records.
	 * [EndDoc]
	 */
	{
		return tbl->bof;
	}

	/*
	 * [BeginDoc]
	 * \index{table::isEOF}
	 * [Verbatim] */

	const int isEOF (void)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * isEOF() returns true if the end of the table has been reached
	 * in a sequential move, either using an index or the physical order
	 * of the records.
	 * [EndDoc]
	 */
	{
		return tbl->eof;
	}

	/*
	 * [BeginDoc]
	 * \index{table::field(int)}
	 * [Verbatim] */

	const char *field (int i)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * field(int i) returns the value of the ``i''th field stored in the
	 * internal table buffer (literally, tbl->fields[i]).  ``i'' is
	 * constrained to be >= 0 and < tbl->numFields.
	 * [EndDoc]
	 */
	{
		if (i < 0 || i >numFields()-1)	{
			tbl->dbError = DB_PARAM;
			tableError();
			return 0;
		}
		return tbl->fields[i];
	}

	/*
	 * [BeginDoc]
	 * \index{table::field(char *)}
	 * [Verbatim] */

	const char *field (char *fieldName)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * field (char *) returns the value of the field with the name
	 * ``fieldName'' that is stored in the internal table buffer.
	 * [EndDoc]
	 */
	{
		char *fld = 0;
		fld = getField (tbl, fieldName);
		tableError();
		return fld;
	}

	/*
	 * [BeginDoc]
	 * \index{table::flen(int)}
	 * [Verbatim] */

	const int flen (int i)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * flen(int) returns the tbl->flens value for the ``i''th field
	 * of the table tbl.  ``i'' is constrained to be >= 0 and
	 * < tbl->numFields.
	 * [EndDoc]
	 */
	{
		if (i < 0 || i >numFields()-1)	{
			tbl->dbError = DB_PARAM;
			tableError();
			return 0;
		}
		return tbl->flens[i];
	}

	/*
	 * [BeginDoc]
	 * \index{table::flen(char *)}
	 * [Verbatim] */

	const int flen (const char *fieldName)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * flen(char *) returns the tbl->flens value for the field with
	 * the name ``fieldName''.  A bad parameter error is generated
	 * if ``fieldName'' is not a valid name in the field.
	 * [EndDoc]
	 */
	{
		int i = getFieldNum (tbl, fieldName);
		if (i == _ERROR_)	{
			tbl->dbError = DB_PARAM;
			tableError ();
			return 0;
		}
		return tbl->flens[i];
	}
	/*
	 * [BeginDoc]
	 * \index{table::declen(int)}
	 * [Verbatim] */

	const int declen (int i)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * declen() returns the tbl->declens value for the ``i''th field
	 * of the table tbl.  ``i'' is constrained to be >= 0 and
	 * < tbl->numFields.
	 * [EndDoc]
	 */
	{
		if (i < 0 || i >tbl->hdr->numFields-1)	{
			tbl->dbError = DB_PARAM;
			tableError();
			return 0;
		}
		return tbl->declens[i];
	}

	/*
	 * [BeginDoc]
	 * \index{table::declen(char *)}
	 * [Verbatim] */

	const int declen (const char *fieldName)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * declen(char *) returns the tbl->declens value for the field with
	 * the name ``fieldName''.  A bad parameter error is generated if
	 * ``fieldName'' is not the valid name of a field.
	 * [EndDoc]
	 */
	{
		int i = getFieldNum (tbl, fieldName);
		if (i == _ERROR_)	{
			tbl->dbError = DB_FIELD;
			tableError ();
			return 0;
		}
		return tbl->declens[i];
	}

	// error handling
	/*
	 * [BeginDoc]
	 * \subsection{Table error handling functions}
	 * 
	 * The following functions provide error handling support for the
	 * table class.  Essentially, this is a wrapper over the extensive
	 * support provided by the DCDB Database Engine error handling
	 * infrastructure.
	 * 
	 * \index{table::isError()}
	 * [Verbatim] */

	int isError (void)
	
	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * isError() returns true if an error condition has occurred, false
	 * otherwise.
	 * [EndDoc]
	 */
	{
		if (iserror)
		  return true;
		else
		  return false;
	}

	/*
	 * The following 2 are not user level programs.  Don't document
	 * them.
	 */
	void tableError (void)
	{
		if (tbl == 0)
			return;
		if (isTableError(tbl))	{
			iserror = true;
			dberr = tbl->dbError;
			memset (error, 0, MAX_ERROR_STRING+1);
			strncpy (error, dbtblerror(tbl), MAX_ERROR_STRING);
		}
	}
	void globalError (void)
	{
		if (isDBError())	{
			iserror = true;
			dberr = dbError;
			memset (error, 0, MAX_ERROR_STRING+1);
			strncpy (error, dberror(), MAX_ERROR_STRING);
		}
	}

	/*
	 * [BeginDoc]
	 * \index{table::dbErrorType()}
	 * [Verbatim] */

	const dbErrorType getError(void)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * getError() returns the value of the private data member table::dberr.
	 * This is the member that tracks error conditions.  If an error has not
	 * occurred, the value of table::dberr will be DB_NOERROR.
	 * [EndDoc]
	 */
	{
		return dberr;
	}
	const char *getErrorStr (void)
	{
		return error;
	}
	void clearErrors (void)
	{
		iserror = false;
		dberr = DB_NOERROR;
	}

	// Move functions
	/*
	 * [BeginDoc]
	 * \subsection{Cursor handling functions}
	 * 
	 * The following functions provide support for moving the current
	 * pointer in the table, either by index or by physical record order.
	 * These are basically wrappers over the DCDB movement functions.
	 * 
	 * \index{table::current}
	 * [Verbatim] */

	void current (char *idxName)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * current() is a wrapper on the DCDB function setCurrentIndex()
	 * with the internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		setCurrentIndex (tbl, idxName);
		tableError();
	}

	/*
	 * [BeginDoc]
	 * \index{table::go}
	 * [Verbatim] */

	void go (int32 recno)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * go() is a wrapper on the DCDB function gotoRecord() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		gotoRecord (tbl, recno);
		tableError();
	}

	/*
	 * [BeginDoc]
	 * \index{table::next}
	 * [Verbatim] */

	void next (void)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * next() is a wrapper on the DCDB function nextRecord() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		nextRecord (tbl);
		tableError();
	}
	/*
	 * [BeginDoc]
	 * \index{table::nextIndex}
	 * [Verbatim] */

	void nextIndex (void)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * nextIndex() is a wrapper on the DCDB function nextIndexRecord() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		nextIndexRecord (tbl);
		tableError();
	}
	/*
	 * [BeginDoc]
	 * \index{table::prev}
	 * [Verbatim] */

	void prev (void)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * prev() is a wrapper on the DCDB function prevRecord() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		prevRecord (tbl);
		tableError();
	}

	/*
	 * [BeginDoc]
	 * \index{table::prevIndex}
	 * [Verbatim] */

	void prevIndex (void)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * prevIndex() is a wrapper on the DCDB function prevIndexRecord() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		prevIndexRecord (tbl);
		tableError();
	}

	/*
	 * [BeginDoc]
	 * \index{table::head}
	 * [Verbatim] */

	void head (void)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * head() is a wrapper on the DCDB function headRecord() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		headRecord (tbl);
		tableError();
	}

	/*
	 * [BeginDoc]
	 * \index{table::headIndex}
	 * [Verbatim] */

	void headIndex (void)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * headIndex() is a wrapper on the DCDB function headIndexRecord() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		headIndexRecord (tbl);
		tableError();
	}

	/*
	 * [BeginDoc]
	 * \index{table::tail}
	 * [Verbatim] */

	void tail (void)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * tail() is a wrapper on the DCDB function tailRecord() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		tailRecord (tbl);
		tableError();
	}

	/*
	 * [BeginDoc]
	 * \index{table::tailIndex}
	 * [Verbatim] */

	void tailIndex (void)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * tailIndex() is a wrapper on the DCDB function tailIndexRecord() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		tailIndexRecord (tbl);
		tableError();
	}

	/*
	 * [BeginDoc]
	 * \index{table::searchIndex}
	 * [Verbatim] */

	int searchIndex (char *fieldValue)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * searchIndex() is a wrapper on the DCDB function searchIndexRecord() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		int status = searchIndexRecord (tbl, fieldValue);
		tableError();
		return status;
	}

	/*
	 * [BeginDoc]
	 * \index{table::searchExactIndex}
	 * [Verbatim] */

	int searchExactIndex (char *fieldValue)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * searchExactIndex() is a wrapper on the DCDB function searchExactIndexRecord() 
	 * with the internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		int status = searchExactIndexRecord (tbl, fieldValue);
		tableError();
		return status;
	}

	// Add functions
	/*
	 * [BeginDoc]
	 * \subsection{Table add functions}
	 * 
	 * These functions allow data to be added to a table.  These are wrappers
	 * over the DCDB add functions.
	 * 
	 * \index{table::add}
	 * [Verbatim] */

	void add (void)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * add() is a wrapper on the DCDB function addRecord() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		addRecord (tbl);
		tableError();
	}
	/*
	 * [BeginDoc]
	 * \index{table::massAdd}
	 * [Verbatim] */

	ListHeader *massAdd (ListHeader *lh)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * massAdd() is a wrapper on the DCDB function massAddRecord() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		ListHeader *tmp = massAddRecords (tbl, lh);
		tableError();
		return tmp;
	}

	// Edit functions
	/*
	 * [BeginDoc]
	 * \subsection{Table edit functions}
	 * 
	 * These functions allow the editing/deletion of records in a table.
	 * These are just wrappers over the existing DCDB edit functions.
	 * 
	 * \index{table::retrieve}
	 * [Verbatim] */

	void retrieve (void)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * retrieve() is a wrapper on the DCDB function retrieveRecord() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		retrieveRecord (tbl);
		tableError();
	}
	/*
	 * [BeginDoc]
	 * \index{table::update}
	 * [Verbatim] */

	void update (void)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * update() is a wrapper on the DCDB function updateRecord() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		updateRecord (tbl);
		tableError();
	}
	/*
	 * [BeginDoc]
	 * \index{table::del}
	 * [Verbatim] */

	void del (void)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * del() is a wrapper on the DCDB function deleteRecord() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		deleteRecord (tbl);
		tableError();
	}
	/*
	 * [BeginDoc]
	 * \index{table::undel}
	 * [Verbatim] */

	void undel (void)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * undel() is a wrapper on the DCDB function undeleteRecord() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		undeleteRecord (tbl);
		tableError();
	}
	/*
	 * [BeginDoc]
	 * \index{table::isDeleted}
	 * [Verbatim] */

	int isDeleted (void)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * isDeleted() is a wrapper on the DCDB function isRecordDeleted() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		return isRecordDeleted (tbl);
	}

	// User interface functions
	/*
	 * [BeginDoc]
	 * \subsection{Table user interface functions}
	 * 
	 * These functions provide an interface to the fields of the table.
	 * These, again, are just wrappers of the DCDB user interface functions.
	 * 
	 * \index{table::setChar}
	 * [Verbatim] */

	void setChar (const char *fieldName,
				 const char *value)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * setChar() is a wrapper on the DCDB function setCharField() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		setCharField (tbl, fieldName, value);
		tableError();
	}
	/*
	 * [BeginDoc]
	 * \index{table::setNumber}
	 * [Verbatim] */

	void setNumber (const char *fieldName,
				   double value)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * setNumber() is a wrapper on the DCDB function setNumberField() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		setNumberField (tbl, fieldName, value);
		tableError();
	}
	/*
	 * [BeginDoc]
	 * \index{table::setInt}
	 * [Verbatim] */

	void setInt (const char *fieldName,
				int value)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * setInt() is a wrapper on the DCDB function setIntField() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		setIntField (tbl, fieldName, value);
		tableError();
	}
	/*
	 * [BeginDoc]
	 * \index{table::setDate}
	 * [Verbatim] */

	void setDate (const char *fieldName, 
				 const char *value)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * setDate() is a wrapper on the DCDB function setDateField() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		setDateField (tbl, fieldName, value);
		tableError();
	}
	/*
	 * [BeginDoc]
	 * \index{table::setLogical}
	 * [Verbatim] */

	void setLogical (const char *fieldName,
					const char value)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * setLogical() is a wrapper on the DCDB function setLogicalField() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		setLogicalField (tbl, fieldName, value);
		tableError();
	}
	/*
	 * [BeginDoc]
	 * \index{table::setTime}
	 * [Verbatim] */

	void setTime (const char *fieldName,
				  const char *value)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * setTime() is a wrapper on the DCDB function setTimeStampField ()
	 * with the internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		setTimeStampField (tbl, fieldName, value);
		tableError();
	}
	/*
	 * [BeginDoc]
	 * \index{table::setDefault}
	 * [Verbatim] */

	void setDefault (const char *fieldName)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * setDefault() is a wrapper on the DCDB function setDefaultField() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		setDefaultField (tbl, fieldName);
		tableError();
	}

	/*
	 * [BeginDoc]
	 * \index{table::pack}
	 * [Verbatim] */

	void pack (int save)

	/* [EndDoc] */
	/*
	 * [BeginDoc]
	 * pack() is a wrapper on the DCDB function packTable() with the
	 * internal table ``tbl'' passed as the table parameter.
	 * [EndDoc]
	 */
	{
		tbl = packTable (tbl, save);
		globalError();
	}
};

}
#endif	// __CDBPP_H__
