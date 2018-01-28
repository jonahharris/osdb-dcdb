/*	Source File:	cdbutils.c	*/  
  
/*
 * Utilities for the cdb and cql modules.
 */ 
  
#include <cdb.h>
#include <time.h>
#include <mtoken.h>
  
/*
 * [BeginDoc]
 * 
 * \section{DCDB Utility functions}
 * 
 * This is the C file with the miscellaneous utility functions and
 * globals needed by the cdb module.
 * 
 * [EndDoc]
 */ 
  
/*
 * [BeginDoc]
 * \index{dbErrMsg}
 * The following is the definition of dbErrMsg[].  This is an
 * array that contains pointers to the strings that provide descriptions
 * for errors that can occur.
 * 
 * [Verbatim] */ 

char *dbErrMsg[] = { "no cdb error", /* DB_NOERROR */ 
    "I/O permission denied",              /* I/O error - */ 
                                          /* access denied */ 
    "I/O too many open files",            /* I/O error - */ 
                                          /* too many files opened */ 
    "I/O file or directory doesn't exist",/* I/O error - */ 
                                          /* no such file or directory */ 
    "I/O bad file descriptor",            /* I/O error - */ 
                                          /* bad file descriptor */ 
    "I/O incomplete read or write",       /* DB_IO_READWRITE */ 
    "I/O operation prohibited",           /* DB_IO_PROHIB */ 
    "I/O operation would cause a deadlock",/* DB_IO_DEADLK */ 
    "I/O locking failed", "memory error", /* Memory error */ 
    "invalid cdb file",                   /* DB_INVALIDFILE */ 
    "list related error",                 /* DB_LISTERROR */ 
    "field level error",                  /* DB_FIELD */ 
    "index related error",                /* DB_INDEX */ 
    "couldn't create the index",          /* DB_CREATEINDEX */ 
    "cdb memory variables corrupted",     /* DB_UNSTABLE */ 
    "unique index constraint violated",   /* DB_UNIQUE */ 
    "too many indexes for this table",    /* DB_TOOMANYIDX */ 
    "bad function parameter",             /* DB_PARAM */ 
    "shell related error",                /* DB_SHELL */ 
    "table locked or in use",             /* DB_LOCK */ 
    "parser related error",               /* DB_PARSE */ 
    "syntax error",                       /* DB_SYNTAX */ 
    "bad input",                          /* DB_INPUT */ 
    /* Add new ones here */ 
    "unspecified cdb error"               /* DB_UNSPECIFIED */  
};


/* [EndDoc] */ 
  
/*
 * [BeginDoc]
 * \subsection{getTimeStamp}
 * \index{getTimeStamp}
 * [Verbatim] */ 
const char *getTimeStamp (void)  
/* [EndDoc] */ 
 
/*
 * [BeginDoc]
 * getTimeStamp() calls asctime() to get a string representing the
 * date and time in a consistent way.  This is an ANSI function
 * and available across platforms.  The string returned by this
 * function is saved in the table header as is.  See the compiler
 * documentation on the asctime() function for more information.
 * 
 * [EndDoc]
 */ 
{
  char *cp;
  static char timeStamp[30];
  time_t now;
  memset (timeStamp, 0, 30);
  time (&now);
  strncpy (timeStamp, asctime (localtime (&now)), 29);
  cp = strchr (timeStamp, '\n');
  if (0 != cp)
    *cp = '\0';
  return timeStamp;
}


/*
 * [BeginDoc]
 * \subsection{sortedTimeStamp}
 * \index{sortedTimeStamp}
 * [Verbatim] */ 
const char *sortedTimeStamp (void)  
/* [EndDoc] */ 
 
/*
 * [BeginDoc]
 * sortedTimeStamp() calls asctime() to get a string representing
 * the data and time.  Then, it pulls the information out and places
 * it in the following format:
 * [Verbatim] 

    YYYYMMDDHHMMSS

   [EndDoc] */ 
 
/*
 * [BeginDoc]
 * where ``YYYY'' is year, ``MM'' is month, ``DD'' is day, ``HH'' is
 * hour, ``MM'' (the second set) is minutes and ``SS'' is seconds.
 * 
 * [EndDoc]
 */ 
{
  char *cp;
  static char timeStamp[30];
  char tmpStamp[30];
  time_t now;
  memset (timeStamp, 0, 30);
  time (&now);
  strncpy (tmpStamp, asctime (localtime (&now)), 29);
  cp = strchr (tmpStamp, '\n');
  if (0 != cp)
  *cp = '\0';
  
  /* now, copy data over */ 
  /* format:  Fri Sep 13 00:00:00 1986\n\0 */ 
  /* year */ 
  timeStamp[0] = tmpStamp[20];
  timeStamp[1] = tmpStamp[21];
  timeStamp[2] = tmpStamp[22];
  timeStamp[3] = tmpStamp[23];
  
  /* month */ 
  if (tmpStamp[4] == 'A') {	/* Aug, Apr */
    if (tmpStamp[5] == 'u') {	/* Aug */
      timeStamp[4] = '0';
      timeStamp[5] = '8';
    }
    
    else {			/* Apr */
      timeStamp[4] = '0';
      timeStamp[5] = '4';
    }
  }
  if (tmpStamp[4] == 'D') {	/* Dec */
    timeStamp[4] = '1';
    timeStamp[5] = '2';
  }
  if (tmpStamp[4] == 'F') {	/* Feb */
    timeStamp[4] = '0';
    timeStamp[5] = '2';
  }
  if (tmpStamp[4] == 'J') {	/* Jan, Jun, Jul */
    if (tmpStamp[5] == 'u') {	/* Jun, Jul */
      if (tmpStamp[6] == 'l') {	/* Jul */
	timeStamp[4] = '0';
	timeStamp[5] = '7';
      }
      
      else {			/* Jun */
	timeStamp[4] = '0';
	timeStamp[5] = '6';
      }
    }
    
    else {			/* Jan */
      timeStamp[4] = '0';
      timeStamp[5] = '1';
    }
  }
  if (tmpStamp[4] == 'M') {	/* Mar, May */
    if (tmpStamp[6] == 'r') {	/* Mar */
      timeStamp[4] = '0';
      timeStamp[5] = '3';
    }
    
    else {			/* May */
      timeStamp[4] = '0';
      timeStamp[5] = '5';
    }
  }
  if (tmpStamp[4] == 'N') {	/* Nov */
    timeStamp[4] = '1';
    timeStamp[5] = '1';
  }
  if (tmpStamp[4] == 'O') {	/* Oct */
    timeStamp[4] = '1';
    timeStamp[5] = '0';
  }
  if (tmpStamp[4] == 'S') {	/* Sep */
    timeStamp[4] = '0';
    timeStamp[5] = '9';
  }
  
  /* Day */ 
  timeStamp[6] = tmpStamp[8];
  timeStamp[7] = tmpStamp[9];
  
  /* Hour, Min, Sec */ 
  timeStamp[8] = tmpStamp[11];
  timeStamp[9] = tmpStamp[12];
  timeStamp[10] = tmpStamp[14];
  timeStamp[11] = tmpStamp[15];
  timeStamp[12] = tmpStamp[17];
  timeStamp[13] = tmpStamp[18];
  
  /* null terminator */ 
  timeStamp[14] = '\0';
  cp = strchr (timeStamp, ' ');
  while (cp != 0) {
    *cp = '0';
    cp = strchr (timeStamp, ' ');
  }
  return timeStamp;
}


/*
 * [BeginDoc]
 * \subsection{field2Record}
 * \index{field2Record}
 * [Verbatim] */ 
void field2Record (dbTable * tbl)  
/* [EndDoc] */ 
/*
 * [BeginDoc]
 * field2Record() translates the strings in the tbl->fields[] array
 * to the format expected in each record.  The data gets copied to
 * the tbl->data field of the tbl structure.
 * 
 * [EndDoc]
 */ 
{
  unsigned offset = 1;
  int i;
  if (*(((char *) tbl->data) + 1) != '\0')
    memset ((void *) ((char *) tbl->data + 1), 0, tbl->hdr->sizeRecord - 1);
  for (i = 0; i < tbl->hdr->numFields; i++) {
    memmove (tbl->data + offset, tbl->fields[i], tbl->flens[i]);
    tbl->fields[i][tbl->flens[i]] = '\0';
    offset += tbl->flens[i];
  }
  check_pointer (tbl->data);
}


/*
 * [BeginDoc]
 * \subsection{record2Field}
 * \index{record2Field}
 * [Verbatim] */ 
void record2Field (dbTable * tbl)  
/* [EndDoc] */ 
/*
 * [BeginDoc]
 * record2Field() translates the data in the tbl->data field to
 * strings.  Each string is copied into the tbl->fields[] array.
 * 
 * [EndDoc]
 */ 
{
  unsigned offset = 1;
  int i;
  for (i = 0; i < tbl->hdr->numFields; i++) {
    memset (tbl->fields[i], 0, tbl->flens[i] + 1);
    check_pointer (tbl->fields[i]);
    memmove (tbl->fields[i], tbl->data + offset, tbl->flens[i]);
    offset += tbl->flens[i];
  }
  check_pointer (tbl->data);
}


/*
 * [BeginDoc]
 * \subsection{dbtblerror}
 * \index{dbtblerror}
 * [Verbatim] */ 
const char *dbtblerror (dbTable * tbl)  
/* [EndDoc] */ 
/*
 * [BeginDoc]
 * 
 * dbtblerror() returns a constant string that describes the error
 * that has occurred.  dbtblerror() is guaranteed to return a valid
 * string even if an error did not occur, so it is safe to use it
 * in a function call.  The return value is a static string that is
 * local to the dbtblerror() function.  dbtblerror() should be used
 * in a context where an error occurred at the table level (with a
 * valid table descriptor).  However, if the error is global,
 * dbtblerror() will handle it correctly.
 * 
 * dbtblerror() clears all error conditions after it is called.
 * Therefore, an error should be processed immediately after
 * dbtblerror() is called (at least, before it or dberror() is
 * called again) or error information could be lost.
 * 
 * [EndDoc]
 */ 
{
  static char line[MAX_ERROR_STRING];
  memset (line, 0, MAX_ERROR_STRING);
  if (tbl->dbError == DB_NOERROR && dbError == DB_NOERROR) {
    strncpy (line, dbErrMsg[DB_NOERROR], MAX_ERROR_STRING);
    return line;
  }
  if (tbl->dbError == DB_INDEX || dbError == DB_INDEX) {
    if (idxError == IDX_SHELL) {
      if (shlError == SHELL_LIST) {
	if (ListError == LIST_FILE) {
	  strcpy (line, "index, shell, list, from fsystem: ");
	  strncat (line, fioErrMsg[fioError], MAX_ERROR_STRING-34);
	  set_fioError(FIO_NOERROR);
	  set_ListError(0, LIST_NOERROR);
	  set_shlError(0, SHELL_NOERR);
	  idxError = IDX_NOERR;
	  tbl->dbError = DB_NOERROR;
	  dbError = DB_NOERROR;
	  return line;
	}
	
	else {
	  strcpy (line, "index, shell, from list: ");
	  strncat (line, ListErrorString[ListError], MAX_ERROR_STRING-25);
	  set_ListError(0, LIST_NOERROR);
	  set_shlError(0, SHELL_NOERR);
	  idxError = IDX_NOERR;
	  tbl->dbError = DB_NOERROR;
	  dbError = DB_NOERROR;
	  return line;
	}
      }
      
      else {
	strcpy (line, "index, from shell: ");
	strncat (line, shlErrorStr[shlError], MAX_ERROR_STRING-19);
	set_shlError(0, SHELL_NOERR);
	idxError = IDX_NOERR;
	tbl->dbError = DB_NOERROR;
	dbError = DB_NOERROR;
	return line;
      }
    }
    
    else if (idxError == IDX_LIST) {
      if (ListError == LIST_FILE) {
	strcpy (line, "index, list, from fsystem: ");
	strncat (line, fioErrMsg[fioError], MAX_ERROR_STRING-27);
	set_fioError(FIO_NOERROR);
	set_ListError(0, LIST_NOERROR);
	idxError = IDX_NOERR;
	tbl->dbError = DB_NOERROR;
	dbError = DB_NOERROR;
	return line;
      }
      
      else {
	strcpy (line, "index, from list: ");
	strncat (line, ListErrorString[ListError], MAX_ERROR_STRING-18);
	set_ListError(0, LIST_NOERROR);
	idxError = IDX_NOERR;
	tbl->dbError = DB_NOERROR;
	dbError = DB_NOERROR;
	return line;
      }
    }
    
    else if (idxError == IDX_FSYSTEM) {
      strcpy (line, "index, from fsystem: ");
      strncat (line, fioErrMsg[fioError], MAX_ERROR_STRING-21);
      set_fioError(FIO_NOERROR);
      idxError = IDX_NOERR;
      tbl->dbError = DB_NOERROR;
      dbError = DB_NOERROR;
      return line;
    }
    
    else {
      strcpy (line, "index: ");
      strncat (line, idxErrMsg[idxError], MAX_ERROR_STRING-7);
      idxError = IDX_NOERR;
      tbl->dbError = DB_NOERROR;
      dbError = DB_NOERROR;
      return line;
    }
  }
  
  else if (tbl->dbError == DB_LISTERROR || dbError == DB_LISTERROR) {
    if (ListError == LIST_FILE) {
      strcpy (line, "list, from fsystem: ");
      strncat (line, fioErrMsg[fioError], MAX_ERROR_STRING-20);
      set_fioError(FIO_NOERROR);
      set_ListError(0, LIST_NOERROR);
      tbl->dbError = DB_NOERROR;
      dbError = DB_NOERROR;
      return line;
    }
    
    else {
      strcpy (line, "list: ");
      strncat (line, ListErrorString[ListError], MAX_ERROR_STRING-6);
      set_ListError(0, LIST_NOERROR);
      tbl->dbError = DB_NOERROR;
      dbError = DB_NOERROR;
      return line;
    }
  }
  
  else if (tbl->dbError == DB_SHELL || dbError == DB_SHELL) {
    if (shlError == SHELL_LIST) {
      if (ListError == LIST_FILE) {
	strcpy (line, "shell, list, from fsystem: ");
	strncat (line, fioErrMsg[fioError], MAX_ERROR_STRING-27);
	set_fioError(FIO_NOERROR);
	set_ListError(0, LIST_NOERROR);
	set_shlError(0, SHELL_NOERR);
	idxError = IDX_NOERR;
	tbl->dbError = DB_NOERROR;
	dbError = DB_NOERROR;
	return line;
      }
      
      else {
	strcpy (line, "shell, from list: ");
	strncat (line, ListErrorString[ListError], MAX_ERROR_STRING-18);
	set_ListError(0, LIST_NOERROR);
	set_shlError(0, SHELL_NOERR);
	idxError = IDX_NOERR;
	tbl->dbError = DB_NOERROR;
	dbError = DB_NOERROR;
	return line;
      }
    }
    
    else {
      strcpy (line, "from shell: ");
      strncat (line, shlErrorStr[shlError], MAX_ERROR_STRING-12);
      set_shlError(0, SHELL_NOERR);
      idxError = IDX_NOERR;
      tbl->dbError = DB_NOERROR;
      dbError = DB_NOERROR;
      return line;
    }
  }
  
  else if (tbl->dbError == DB_PARSE || dbError == DB_PARSE) {
    strcpy (line, "parser: ");
    strncat (line, parseErrorString[parseError], MAX_ERROR_STRING-8);
    parseError = PARS_NOERROR;
    tbl->dbError = DB_NOERROR;
    dbError = DB_NOERROR;
    return line;
  }
  
  else if (tbl->dbError == DB_SYNTAX || dbError == DB_SYNTAX) {
    strcpy (line, "syntax error: ");
    strncat (line, syntaxError, MAX_ERROR_STRING-14);
    tbl->dbError = DB_NOERROR;
    dbError = DB_NOERROR;
    return line;
  }
  
  else {
    if (tbl->dbError != DB_NOERROR)
      strcpy (line, dbErrMsg[tbl->dbError]);
    if (dbError != DB_NOERROR)
      strcpy (line, dbErrMsg[dbError]);
    tbl->dbError = DB_NOERROR;
    dbError = DB_NOERROR;
    return line;
  }
  return line;
}


/*
 * [BeginDoc]
 * \subsection{dberror}
 * \index{dberror}
 * [Verbatim] */ 
const char *dberror (void)  
/* [EndDoc] */ 
/*
 * [BeginDoc]
 * dberror() returns a constant string that describes the error
 * that has occurred.  dberror() is guaranteed to return a valid
 * string even if an error did not occur, so it is safe to use it
 * in a function call.  The return value is a static string that is
 * local to the dberror() function.  dberror() should be used in
 * a context where the error is global (where there is no valid table
 * descriptor).  It will not handle it properly if the error is
 * really at the table level as opposed to global.
 * 
 * dberror() clears all error conditions after it is called.
 * Therefore, an error should be processed immediately after
 * dberror() is called (at least, before it or dbtblerror() is
 * called again) or error information could be lost.
 * 
 * [EndDoc]
 */ 
{
  static char line[MAX_ERROR_STRING];
  memset (line, 0, MAX_ERROR_STRING);
  if (dbError == DB_NOERROR) {
    strcpy (line, dbErrMsg[DB_NOERROR]);
    return line;
  }
  if (dbError == DB_INDEX) {
    if (idxError == IDX_SHELL) {
      if (shlError == SHELL_LIST) {
	if (ListError == LIST_FILE) {
	  strcpy (line, "index, shell, list, from fsystem: ");
	  strncat (line, fioErrMsg[fioError], MAX_ERROR_STRING-34);
	  set_fioError(FIO_NOERROR);
	  set_ListError(0, LIST_NOERROR);
	  set_shlError(0, SHELL_NOERR);
	  idxError = IDX_NOERR;
	  dbError = DB_NOERROR;
	  return line;
	}
	
	else {
	  strcpy (line, "index, shell, from list: ");
	  strncat (line, ListErrorString[ListError], MAX_ERROR_STRING-25);
	  set_ListError(0, LIST_NOERROR);
	  set_shlError(0, SHELL_NOERR);
	  idxError = IDX_NOERR;
	  dbError = DB_NOERROR;
	  return line;
	}
      }
      
      else {
	strcpy (line, "index, from shell: ");
	strncat (line, shlErrorStr[shlError], MAX_ERROR_STRING-19);
	set_shlError(0, SHELL_NOERR);
	idxError = IDX_NOERR;
	dbError = DB_NOERROR;
	return line;
      }
    }
    
    else if (idxError == IDX_LIST) {
      if (ListError == LIST_FILE) {
	strcpy (line, "index, list, from fsystem: ");
	strncat (line, fioErrMsg[fioError], MAX_ERROR_STRING-27);
	set_fioError(FIO_NOERROR);
	set_ListError(0, LIST_NOERROR);
	idxError = IDX_NOERR;
	dbError = DB_NOERROR;
	return line;
      }
      
      else {
	strcpy (line, "index, from list: ");
	strncat (line, ListErrorString[ListError], MAX_ERROR_STRING-18);
	set_ListError(0, LIST_NOERROR);
	idxError = IDX_NOERR;
	dbError = DB_NOERROR;
	return line;
      }
    }
    
    else if (idxError == IDX_FSYSTEM) {
      strcpy (line, "index, from fsystem: ");
      strncat (line, fioErrMsg[fioError], MAX_ERROR_STRING-21);
      set_fioError(FIO_NOERROR);
      idxError = IDX_NOERR;
      dbError = DB_NOERROR;
      return line;
    }
    
    else {
      strcpy (line, "index: ");
      strncat (line, idxErrMsg[idxError], MAX_ERROR_STRING-7);
      idxError = IDX_NOERR;
      dbError = DB_NOERROR;
      return line;
    }
  }
  
  else if (dbError == DB_LISTERROR) {
    if (ListError == LIST_FILE) {
      strcpy (line, "list, from fsystem: ");
      strncat (line, fioErrMsg[fioError], MAX_ERROR_STRING-20);
      set_fioError(FIO_NOERROR);
      set_ListError(0, LIST_NOERROR);
      dbError = DB_NOERROR;
      return line;
    }
    
    else {
      strcpy (line, "list: ");
      strncat (line, ListErrorString[ListError], MAX_ERROR_STRING-6);
      set_ListError(0, LIST_NOERROR);
      dbError = DB_NOERROR;
      return line;
    }
  }
  
  else if (dbError == DB_SHELL) {
    if (shlError == SHELL_LIST) {
      if (ListError == LIST_FILE) {
	strcpy (line, "shell, list, from fsystem: ");
	strncat (line, fioErrMsg[fioError], MAX_ERROR_STRING-27);
	set_fioError(FIO_NOERROR);
	set_ListError(0, LIST_NOERROR);
	set_shlError(0, SHELL_NOERR);
	idxError = IDX_NOERR;
	dbError = DB_NOERROR;
	return line;
      }
      
      else {
	strcpy (line, "shell, from list: ");
	strncat (line, ListErrorString[ListError], MAX_ERROR_STRING-18);
	set_ListError(0, LIST_NOERROR);
	set_shlError(0, SHELL_NOERR);
	idxError = IDX_NOERR;
	dbError = DB_NOERROR;
	return line;
      }
    }
    
    else {
      strcpy (line, "from shell: ");
      strncat (line, shlErrorStr[shlError], MAX_ERROR_STRING-12);
      set_shlError(0, SHELL_NOERR);
      idxError = IDX_NOERR;
      dbError = DB_NOERROR;
      return line;
    }
  }
  
  else if (dbError == DB_PARSE) {
    strcpy (line, "parser: ");
    strncat (line, parseErrorString[parseError], MAX_ERROR_STRING-8);
    parseError = PARS_NOERROR;
    dbError = DB_NOERROR;
    return line;
  }
  
  else if (dbError == DB_SYNTAX) {
    strcpy (line, "syntax error: ");
    strncat (line, syntaxError, MAX_ERROR_STRING-14);
    dbError = DB_NOERROR;
    return line;
  }
  
  else {
    strcpy (line, dbErrMsg[dbError]);
    dbError = DB_NOERROR;
    return line;
  }
  return line;
}


/*
 * Don't document for now.
 */ 
void dbClearAllErrors (dbTable * tbl) 
{
  parseError = PARS_NOERROR;
  set_fioError(FIO_NOERROR);
  set_ListError(0, LIST_NOERROR);
  set_shlError(0, SHELL_NOERR);
  idxError = IDX_NOERR;
  dbError = DB_NOERROR;
  tbl->dbError = DB_NOERROR;
} 
