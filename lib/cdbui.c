/*	Source File:	cdbui.c	*/

#include <cdb.h>
#include <stdio.h>
#include <string.h>

#define	MAXPATH			255
#define	FORMAT_LENGTH	512

/*
 * [BeginDoc]
 * 
 * \section{DCDB User Interface Functions}
 * 
 * These are the user interface functions of the DCDB library, functions that
 * are designed purely as candy for the end-user.  These functions are not
 * necessary to using DCDB and the functionality provided here can be had
 * through other means.
 * 
 * [EndDoc]
 */

/*
 * [BeginDoc]
 * \subsection{buildTable}
 * \index{buildTable}
 * [Verbatim] */

dbTable *buildTable (const char *tblname, const char *tblinfo,
		     fieldType * ftype, char **names,
		     int *flens, int *declens)

/* [EndDoc] */
/*
 * [BeginDoc]
 * buildTable() performs an identical function to createTable().  It does
 * not, however, require a person to create on the heap and initialize a
 * dbHeader record and a dbField array.
 * 
 * ``tblname'' is the name of the table to create.  ``tblinfo'' is a 
 * string of data that will be saved in the table of max length
 * TABLE_INFO_WIDTH.  This allows the user to store in the table
 * information that is specific to the application.  ``ftype'' is an
 * array of field types, each describing what type of field is represented
 * in the table.  ``names'' is an array of character strings that contains
 * names for each field.  ``flens'' is an array of integers that gives the
 * length for each field.  And, ``declens'' is the length of the decimal
 * part of each field, if that is applicable (only applies to NUMBER fields).
 * The ``ftype'', ``names'', ``flens'' and ``declens'' arrays contain the
 * information that fully identifies each field.  These must have the same
 * number of items and must correspond with each other to describe a field.
 * The ``names'' array \emph{must} be NULL terminated.  If it isn't, you
 * will probably get a protection fault on a call to buildTable().
 * 
 * Upon success, buildTable() will return the descriptor for the open table.
 * If an error occurs, isDBError() will be TRUE and dberror() will return a
 * string which describes the error.  If that happens, buildTable() will
 * return NULL.
 * 
 * It is recommended that upon successful completion of the buildTable()
 * function that the user then create all indexes for the table, close
 * the table (with a call to closeTable()) and then reopen the table with
 * a call to openTable().  This should be done before any data is added to the
 * table.  Closing the table and reopening it is a defensive technique that
 * will insure that all the basic file structures are flushed to disk and
 * reinitialized at reopening.
 * 
 * Note: this function is deprecated, as is createTable(), by the parseDBDef()
 * function.  Tables, indexes and workspaces should be created by creating a
 * .df file (either in the application or manually) and then calling
 * parseDBDef() with the name of the .df file as an argument.
 *
 * [EndDoc]
 */
{
  dbHeader *dbh;
  dbField **flds;
  dbTable *tbl;
  int i, fd;
  int numFields;
  char path[255];

  strcpy (path, tblname);
  strcat (path, ".LCK");
  if (fexists (path)) {
    /*
     * First, make sure the file isn't locked.
     */
    if (fexists ((char *) tblname)) {
      fd = fileOpen (tblname);
      Assert (_ERROR_ != fd);
      if (_ERROR_ == fd) {
        dbError = fioError;
        return 0;
      }
#ifndef __MINGW32__
      if (fileLockTest (fd, F_WRLCK, 0, FILE_BEGINNING, sizeof (dbHeader))) {
        fileClose (fd);
        dbError = DB_LOCK;
        return 0;
      }
#endif /* __MINGW32__ */
      fileClose (fd);
      /*
       * it's just a lock file, trash the lock and recreate the table
       */
    }
    fileRemove (path);
  }
  for (i = 0; names[i] != 0; i++);
  numFields = i;
  dbh = malloc (sizeof (dbHeader));
  Assert (0 != dbh);
  if (0 == dbh) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (dbh, 0, sizeof (dbHeader));
  check_pointer (dbh);

  strcpy (dbh->timeStamp, getTimeStamp ());
  if (tblinfo != 0)
    strncpy (dbh->tableInfo, tblinfo, TABLE_INFO_WIDTH);

  flds = malloc ((numFields + 1) * sizeof (dbField *));
  Assert (0 != flds);
  if (0 == flds) {
    dbError = DB_NOMEMORY;
    free (dbh);
    return 0;
  }
  memset (flds, 0, (numFields + 1) * sizeof (dbField *));
  check_pointer (flds);
  for (i = 0; i < numFields; i++) {
    flds[i] = malloc (sizeof (dbField));
    Assert (0 != flds[i]);
    if (0 == flds[i])
      return 0;
    memset (flds[i], 0, sizeof (dbField));
    check_pointer (flds[i]);
    strcpy (flds[i]->fieldName, names[i]);
    flds[i]->ftype = ftype[i];
    flds[i]->indexed = ITYPE_NOINDEX;
    flds[i]->fieldLength = flens[i];
    flds[i]->decLength = declens[i];
    check_pointer (flds[i]);
  }
  flds[numFields] = 0;
  tbl = createTable (tblname, dbh, flds);
  strcpy (path, tbl->fileName);
  strcat (path, ".LCK");
  fileClose (fileCreate (path));
  return tbl;
}

/*
 * [BeginDoc]
 * \subsection{getFieldNum}
 * \index{getFieldNum}
 * [Verbatim] */

int getFieldNum (dbTable * tbl, const char *fieldName)

/* [EndDoc] */
/*
 * [BeginDoc]
 * getFieldNum() is a simple function that is used to convert a field name
 * to a numerical offset into the tbl->fields array.  ``tbl'' is the table
 * to work with and ``fieldName'' is the name of the field.  If the call is
 * successful, getFieldNum() returns an integer value ``i'' such that
 * tbl->fields[i] is the storage space for the field, tbl->flens[i] is the
 * length and tbl->declens[i] is the length of the decimal part, if
 * if applicable.  It should be noted that getFieldNum() uses strncmp()
 * to compare ``fieldName'' with the names of the fields, so the field
 * doesn't have to match exactly.  Therefore, if you have a field named
 * ``line'' and one named ``lineNum'' and ``fieldName'' is ``line'', you
 * may not get what you expect from getFieldNum().  It is better to make
 * your fields distinct within the first few characters to avoid such
 * ambiguities.  At the least, you should have the ``line'' field in
 * the table before the ``lineNum'' field, otherwise you will never get
 * a valid return from getFieldNum() for the ``line'' field.
 * 
 * If an error occurs, isTableError(tbl) is TRUE for this table and
 * dbtblerror(tbl) returns a string that describes the error.
 * 
 * [EndDoc]
 */
{
  int i;

  for (i = 0; i < tbl->hdr->numFields; i++) {
    if (!strncmp (tbl->fldAry[i]->fieldName, fieldName, strlen (fieldName)))
      break;
  }
  if (i >= tbl->hdr->numFields) {
    tbl->dbError = DB_PARAM;
    return _ERROR_;
  }
  return i;
}

/* internal function */
void timeStamp2Date (char *date, const char *timeStamp)
{
  char month[3], year[5], day[3];

  /* this is cumbersome, but it is just as fast as a table lookup. */
  if (!strncmp (timeStamp + 4, "Jan", 3)) {
    strcpy (month, "01");
    goto doneMonth;
  }
  if (!strncmp (timeStamp + 4, "Feb", 3)) {
    strcpy (month, "02");
    goto doneMonth;
  }
  if (!strncmp (timeStamp + 4, "Mar", 3)) {
    strcpy (month, "03");
    goto doneMonth;
  }
  if (!strncmp (timeStamp + 4, "Apr", 3)) {
    strcpy (month, "04");
    goto doneMonth;
  }
  if (!strncmp (timeStamp + 4, "May", 3)) {
    strcpy (month, "05");
    goto doneMonth;
  }
  if (!strncmp (timeStamp + 4, "Jun", 3)) {
    strcpy (month, "06");
    goto doneMonth;
  }
  if (!strncmp (timeStamp + 4, "Jul", 3)) {
    strcpy (month, "07");
    goto doneMonth;
  }
  if (!strncmp (timeStamp + 4, "Aug", 3)) {
    strcpy (month, "08");
    goto doneMonth;
  }
  if (!strncmp (timeStamp + 4, "Sep", 3)) {
    strcpy (month, "09");
    goto doneMonth;
  }
  if (!strncmp (timeStamp + 4, "Oct", 3)) {
    strcpy (month, "10");
    goto doneMonth;
  }
  if (!strncmp (timeStamp + 4, "Nov", 3)) {
    strcpy (month, "11");
    goto doneMonth;
  }
  /* Here, assume december is all that's left */
  strcpy (month, "12");

doneMonth:

  strncpy (day, timeStamp + 8, 2);
  day[2] = '\0';
  if (day[0] == ' ')
    day[0] = '0';

  strncpy (year, timeStamp + 20, 4);
  year[4] = '\0';

  sprintf (date, "%s%s%s", year, month, day);
  return;
}

/*
 * [BeginDoc]
 * \subsection{setCharField}
 * \index{setCharField}
 * [Verbatim] */

int setCharField (dbTable * tbl, const char *fieldName, const char *value)

/* [EndDoc] */
/*
 * [BeginDoc]
 * setCharField() sets the field given by ``fieldName'' in the table ``tbl'' to
 * ``value''.  It is an error if the field is not a character field or if there
 * is no field called ``fieldName''.  setCharField() insures that the data copied
 * to the field buffer is of tbl->flens[] length for that field, at most.  If
 * ``value'' is NULL, the characters in the field are set to all binary 0 values.
 * 
 * If an error occurs, isTableError(tbl) is true for this table and dbtblerror(tbl)
 * returns a string describing the error.
 * 
 * [EndDoc]
 */
{
  int i;

  i = getFieldNum (tbl, fieldName);
  if (i == _ERROR_) {
    tbl->dbError = DB_FIELD;
    return _ERROR_;
  }
  /* check the field type */
  if (tbl->fldAry[i]->ftype != FTYPE_CHAR) {
    tbl->dbError = DB_FIELD;
    return _ERROR_;
  }
  /* do it! */
  memset (tbl->fields[i], 0, tbl->flens[i] + 1);
  if (value == 0)
    return _OK_;
  strncpy (tbl->fields[i], value, tbl->flens[i]);
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{setNumberField}
 * \index{setNumberField}
 * [Verbatim] */

int setNumberField (dbTable * tbl, const char *fieldName, double value)

/* [EndDoc] */
/*
 * [BeginDoc]
 * setNumberField() sets the NUMBER field given by ``fieldName'' in the table
 * ``tbl'' to the value ``value''.  It is an error if the field with the
 * name ``fieldName'' is not a numeric field or if there is no field with
 * that name.  The data is copied to the field right justified and padded with
 * spaces on the left.  If ``value'' is 0, the field is set to ``0'', right justified
 * and padded with spaces.
 * 
 * If an error occurs, isTableError(tbl) is true for this table and dbtblerror(tbl)
 * returns a string describing the error.
 * 
 * [EndDoc]
 */
{
  int i;
  char format[FORMAT_LENGTH + 1];
  char test_field[MAX_FIELD + 1];

  i = getFieldNum (tbl, fieldName);
  if (i == _ERROR_) {
    tbl->dbError = DB_FIELD;
    return _ERROR_;
  }
  /* check the field type */
  if (tbl->fldAry[i]->ftype != FTYPE_NUMBER) {
    tbl->dbError = DB_FIELD;
    return _ERROR_;
  }
  /* build the format string - make it arbitrarily long. */
  /*
     format = malloc (FORMAT_LENGTH);
     if (0 == format)     {
     tbl->dbError = DB_NOMEMORY;
     return _ERROR_;
     }
     memset (format, 0, FORMAT_LENGTH);
     check_pointer (format);
   */
  sprintf (format, "%%%d.%df", tbl->flens[i] - tbl->declens[i],
	   tbl->declens[i]);
  /* now, write it, but carefully. */
  sprintf (test_field, format, value);
  if (strlen (test_field) > (size_t)tbl->flens[i]) {
    tbl->dbError = DB_INPUT;
    return _ERROR_;
  }
  memset (tbl->fields[i], 0, tbl->flens[i] + 1);
  strcpy (tbl->fields[i], test_field);
  /*
     check_pointer (format);
     free (format);
   */
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{setIntField}
 * \index{setIntField}
 * [Verbatim] */

int setIntField (dbTable * tbl, const char *fieldName, int value)

/* [EndDoc] */
/*
 * [BeginDoc]
 * setIntField() sets the NUMBER field given by ``fieldName'' in the table
 * ``tbl'' to the value ``value''.  It is an error if the field with the
 * name ``fieldName'' is not a numeric field or if there is no field with
 * that name.  The data is copied to the field right justified and padded with
 * spaces on the left.  If ``value'' is 0, the field is set to ``0'', right justified
 * and padded with spaces.
 * 
 * If an error occurs, isTableError(tbl) is true for this table and dbtblerror(tbl)
 * returns a string describing the error.
 * 
 * [EndDoc]
 */
{
  int i;
  char format[FORMAT_LENGTH + 1];
  char test_field[MAX_FIELD + 1];

  i = getFieldNum (tbl, fieldName);
  if (i == _ERROR_) {
    tbl->dbError = DB_FIELD;
    return _ERROR_;
  }
  /* check the field type */
  if (tbl->fldAry[i]->ftype != FTYPE_NUMBER) {
    tbl->dbError = DB_FIELD;
    return _ERROR_;
  }
  /* build the format string - make it arbitrarily long. */
  /*
     format = malloc (FORMAT_LENGTH);
     if (0 == format)     {
     tbl->dbError = DB_NOMEMORY;
     return _ERROR_;
     }
     memset (format, 0, FORMAT_LENGTH);
     check_pointer (format);
   */
  sprintf (format, "%%%dd", tbl->flens[i]);
  /* now, write it, but carefully. */
  sprintf (test_field, format, value);
  if (strlen (test_field) > (size_t)tbl->flens[i]) {
    tbl->dbError = DB_INPUT;
    return _ERROR_;
  }
  memset (tbl->fields[i], 0, tbl->flens[i] + 1);
  strcpy (tbl->fields[i], test_field);
  /*
     check_pointer (format);
     free (format);
   */
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{setDateField}
 * \index{setDateField}
 * [Verbatim] */

int setDateField (dbTable * tbl, const char *fieldName, const char *value)

/* [EndDoc] */
/*
 * [BeginDoc]
 * setDateField() sets the DATE field given by ``fieldName'' in the table
 * ``tbl'' to the value ``value''.  It is an error if the field with the
 * name ``fieldName'' is not a date field, if there is no field with
 * that name or if the date field is not 8 digits long.  If ``value'' is
 * NULL, the date field is given today's date.
 * 
 * If an error occurs, isTableError(tbl) is true for this table and dbtblerror(tbl)
 * returns a string describing the error.
 * 
 * 
 * [EndDoc]
 */
{
  int i;

  i = getFieldNum (tbl, fieldName);
  if (i == _ERROR_) {
    tbl->dbError = DB_FIELD;
    return _ERROR_;
  }
  if (tbl->fldAry[i]->ftype != FTYPE_DATE) {
    tbl->dbError = DB_FIELD;
    return _ERROR_;
  }
  if (value == 0) {
    memset (tbl->fields[i], 0, tbl->flens[i] + 1);
    timeStamp2Date (tbl->fields[i], getTimeStamp ());
    return _OK_;
  }
  if (strlen (value) != 8) {
    tbl->dbError = DB_FIELD;
    return _ERROR_;
  }
  memset (tbl->fields[i], 0, tbl->flens[i] + 1);
  strcpy (tbl->fields[i], value);
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{setDateField2TimeStamp}
 * \index{setDateField2TimeStamp}
 * [Verbatim] */

int setDateField2TimeStamp (dbTable * tbl, const char *fieldName,
			    const char *timeStamp)

/* [EndDoc] */
/*
 * [BeginDoc]
 * setDateField2TimeStamp() sets the DATE field given by ``fieldName'' in the table
 * ``tbl'' to the value in ``timeStamp''.  setDateField2TimeStamp() converts the
 * time stamp to an 8 digit date in the form YYYYMMDD before it sets the field.
 * It is an error if the field with the
 * name ``timeStamp'' is not a valid time stamp or if there is no field with
 * that name.
 * 
 * If an error occurs, isTableError(tbl) is true for this table and dbtblerror(tbl)
 * returns a string describing the error.
 * 
 * [EndDoc]
 */
{
  int i;

  if (strlen (timeStamp) != 25) {
    tbl->dbError = DB_FIELD;
    return _ERROR_;
  }
  i = getFieldNum (tbl, fieldName);
  if (i == _ERROR_) {
    tbl->dbError = DB_FIELD;
    return _ERROR_;
  }
  if (tbl->fldAry[i]->ftype != FTYPE_DATE) {
    tbl->dbError = DB_FIELD;
    return _ERROR_;
  }
  memset (tbl->fields[i], 0, tbl->flens[i] + 1);
  timeStamp2Date (tbl->fields[i], getTimeStamp ());
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{setLogicalField}
 * \index{setLogicalField}
 * [Verbatim] */

int setLogicalField (dbTable * tbl, const char *fieldName, const char value)

/* [EndDoc] */
/*
 * [BeginDoc]
 * setLogicalField() sets the LOGICAL field given by ``fieldName'' in the table
 * ``tbl'' to the value ``value''.  It is an error if the field with the
 * name ``fieldName'' is not a numeric field, if there is no field with
 * that name, or if value is not one of `y', `Y', `t', `T', `n', `N', `f', `F'.
 * If ``value'' is 0, the field is set to `T'.
 * 
 * If an error occurs, isTableError(tbl) is true for this table and dbtblerror(tbl)
 * returns a string describing the error.
 * 
 * [EndDoc]
 */
{
  char *cp;
  int i;

  cp = strchr ("yYtTnNfF", value);
  if (0 == cp) {
    tbl->dbError = DB_FIELD;
    return _ERROR_;
  }
  i = getFieldNum (tbl, fieldName);
  if (i == _ERROR_) {
    tbl->dbError = DB_FIELD;
    return _ERROR_;
  }
  if (tbl->fldAry[i]->ftype != FTYPE_LOGICAL) {
    tbl->dbError = DB_FIELD;
    return _ERROR_;
  }
  memset (tbl->fields[i], 0, tbl->flens[i] + 1);
  if (value == '\0') {
    tbl->fields[i][0] = 'T';
    return _OK_;
  }
  tbl->fields[i][0] = value;
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{setTimeStampField}
 * \index{setTimeStampField}
 * [Verbatim] */

int setTimeStampField (dbTable * tbl, const char *fieldName,
		       const char *timeStamp)

/* [EndDoc] */
/*
 * [BeginDoc]
 * setTimeStampField() sets the TIMESTAMP field given by ``fieldName'' in the table
 * ``tbl'' to the value ``timeStamp''.
 * A time stamp is a string like that returned from asctime(), as follows:
 * ``Tue Mar 21 11:10:42 MST 2000''  This gives the date/time information for
 * a particular moment in time.
 * It is an error if the field with the
 * name ``timeStamp'' is not a timeStamp field, if there is no field with
 * that name or if the timeStamp field is not 25 digits long.  If ``timeStamp'' is
 * NULL, the timestamp is set to now.
 * 
 * If an error occurs, isTableError(tbl) is true for this table and dbtblerror(tbl)
 * returns a string describing the error.
 * 
 * [EndDoc] */
{
  int i;

  if (timeStamp != 0) {
    if (strlen (timeStamp) != 25) {
      tbl->dbError = DB_FIELD;
      return _ERROR_;
    }
  }
  i = getFieldNum (tbl, fieldName);
  if (i == _ERROR_) {
    tbl->dbError = DB_FIELD;
    return _ERROR_;
  }
  if (tbl->fldAry[i]->ftype != FTYPE_TIME) {
    tbl->dbError = DB_FIELD;
    return _ERROR_;
  }
  memset (tbl->fields[i], 0, tbl->flens[i] + 1);
  if (timeStamp == 0) {
    strncpy (tbl->fields[i], getTimeStamp (), tbl->flens[i]);
    return _OK_;
  }
  strncpy (tbl->fields[i], timeStamp, tbl->flens[i]);
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{setDefaultField}
 * \index{setDefaultField}
 * [Verbatim] */

int setDefaultField (dbTable * tbl, const char *fieldName)

/* [EndDoc] */
/*
 * [BeginDoc]
 * setDefaultField() sets the field given by ``fieldName'' in the table
 * ``tbl'' to a default value.  What a ``default'' value is depends on the
 * field.  This is described in setCharField(), setNumberField(),
 * setLogicalField(), setDateField() and setTimeStampField() for calls
 * with the ``value'' or ``timeStamp'' parameter set to NULL.
 * 
 * If an error occurs, isTableError(tbl) is true for this table and dbtblerror(tbl)
 * returns a string describing the error.
 * 
 * [EndDoc]
 */
{
  int i;

  i = getFieldNum (tbl, fieldName);
  if (i == _ERROR_) {
    tbl->dbError = DB_FIELD;
    return _ERROR_;
  }
  switch (tbl->fldAry[i]->ftype) {
  case FTYPE_CHAR:
    setCharField (tbl, fieldName, 0);
    break;
  case FTYPE_NUMBER:
    setNumberField (tbl, fieldName, 0);
    break;
  case FTYPE_LOGICAL:
    setLogicalField (tbl, fieldName, 0);
    break;
  case FTYPE_DATE:
    setDateField (tbl, fieldName, 0);
    break;
  case FTYPE_TIME:
    setTimeStampField (tbl, fieldName, 0);
    break;
  default:
    tbl->dbError = DB_FIELD;
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{getField}
 * \index{getField}
 * [Verbatim] */

char *getField (dbTable * tbl, const char *fieldName)

/* [EndDoc] */
/*
 * [BeginDoc]
 * getField() gets the value that is currently set in the field
 * given by ``fieldName'' in the table given by ``tbl''.
 * The value is returned in a pointer to the field.  This should be
 * considered a constant value to the calling program.
 * It is an
 * error if ``fieldName'' is not the name of a valid field in ``tbl''.
 * 
 * If an error occurs, isTableError(tbl) is true for this table and dbtblerror(tbl)
 * returns a string describing the error.
 * 
 * [EndDoc]
 */
{
  int i;
  i = getFieldNum (tbl, fieldName);
  if (i == _ERROR_) {
    tbl->dbError = DB_FIELD;
    return 0;
  }
  return tbl->fields[i];
}
