/*	Source File:	cdbtable.c	*/  
  
/*
 * Table creation and support routines.
 */ 
  
/*
 * [BeginDoc]
 * 
 * \section{DCDB Table Functions}
 * 
 * This file includes the routines to create, open and close a table,
 * as well as the routine to save the table header.  The process that
 * a user should follow to create a table is this:
 * 
 * \begin{itemize}
 * 
 * \item create the table (see buildTable()).
 * 
 * \item create all the indexes (see createDBIndex() and
 *    createMultiIndex()).
 * 
 * \item close the table and indexes (closeTable()).
 * 
 * \item reopen the table and indexes (openTable()).
 * 
 * \end{itemize}
 * 
 * The valid table pointer returned by openTable() can then be used
 * for adding, updating, and searching table data.
 * 
 * [EndDoc]
 */ 
  
#include <cdb.h>
  
#ifdef HAVE_DIR_H
#include <dir.h>
#endif
  
#ifndef MAXPATH
#define	MAXPATH	255
#endif	/*  */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
  
#ifdef	WIN32
#undef MAXPATH
#define	MAXPATH	MAX_PATH
#endif	/*  */

dbErrorType dbError = DB_NOERROR;

/*
 * [BeginDoc]
 * \subsection{createTable}
 * \index{createTable}
 * [Verbatim] */ 
dbTable * createTable (const char *fname, dbHeader * hdr,
			  dbField * flds[])  
/* [EndDoc] */ 
 
/*
 * [BeginDoc]
 * createTable() creates a cdb table.  ``fname'' is the name of the 
 * table to create.  ``hdr'' is a pointer to a dbHeader type that should
 * have the tableInfo field set, either to all binary 0's or what
 * the application needs.
 * 
 * The ``flds'' parameter is an array of pointers to dbField types.
 * The last item in this list should be NULL.  Every member of the
 * dbField structure in each item pointed to by the ``flds[]'' array
 * should be filled.  The dbFields records pointed to by the items 
 * in the ``flds[]'' array are saved to disk unchanged, so it is
 * contingent on the caller to insure that the fields are set up
 * correctly.
 * 
 * Upon successful completion, createTable() will have created a
 * DCDB table and returns a valid pointer to a dbTable object.  This
 * object is allocated on the heap and should be freed before the
 * program exits by calling closeTable().  NULL is returned by
 * createTable() on error.  The user can use the isDBError()
 * macro to determine if an error has occurred and the function
 * dberror() to communicate the error to the user.
 * 
 * The ``hdr'' and ``flds[]'' parameters should be allocated on the heap
 * and are assumed to belong to the table structure on successful
 * completion of this call.  When you call closeTable(), it will
 * attempt to deallocate the storage with free().
 * 
 * Because of the low-level nature of the createTable() function,
 * it should not be used by the end user to create tables.  The
 * buildTable() function should be used instead.
 * 
 * [EndDoc]
 */ 
{
  unsigned i, srecs;
  dbTable * tbl;
  int fd;
  off_t written;
  char path[MAXPATH];
  char *cp;
  dbg2 (printf ("\n\ncreateTable(%s,...)\n", fname);
    );
  if (fname[0] == '\0') {
    dbError = DB_PARAM;
    return 0;
  }
  strcpy (path, fname);
  strcat (path, ".LCK");
  if (fexists (path)) {
    dbError = DB_LOCK;
    return 0;
  }
      /* FIXME */
#if 0
    
      /*
       * First, make sure the file isn't locked.
       */ 
      if (fexists ((char *) fname)) {
      fd = fileOpen (fname);
      Assert (_ERROR_ != fd);
      if (_ERROR_ == fd) {
	dbError = fioError;
	return 0;
      }
      if (fileLockTest (fd, F_WRLCK, 0, FILE_BEGINNING, sizeof (dbHeader))) {
	fileClose (fd);
	dbError = DB_LOCK;
	return 0;
      }
      fileClose (fd);
    }
    
      /*
       * it's just a lock file, trash the lock and recreate the table
       */ 
      fileRemove (path);
  }
#endif
  
    /*
     * Set up the header for the table.
     */ 
    Assert (0 != hdr);
  Assert (0 != flds);
  if (0 == hdr || 0 == flds) {
    dbError = DB_PARAM;
    return 0;
  }
  for (i = 0, srecs = 0; flds[i] != NULL; i++) {
    srecs += flds[i]->fieldLength;
  }
  hdr->numFields = i;
  hdr->sizeRecord = srecs + 1;
  hdr->magic = DB_MAGIC;
  hdr->numRecords = 0;
  hdr->sizeFields = i * sizeof (dbField);
  
    /*
     * Create the file
     */ 
    if (fexists ((char *) fname))	/* if file exists */
    fileRemove (fname);	/* delete it */
  fd = fileCreate (fname);
  Assert (_ERROR_ != fd);
  if (_ERROR_ == fd) {
    dbError = fioError;
    return 0;
  }
  fd = fileClose (fd);
  Assert (_ERROR_ != fd);
  if (_ERROR_ == fd) {
    dbError = fioError;
    return 0;
  }
  fd = fileOpen (fname);
  Assert (_ERROR_ != fd);
  if (_ERROR_ == fd) {
    dbError = fioError;
    return 0;
  }
  
    /*
     * Allocate table
     */ 
    tbl = (dbTable *) malloc (sizeof (dbTable));
  Assert (0 != tbl);
  if (0 == tbl) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl, 0, sizeof (dbTable));
  check_pointer (tbl);
  tbl->fd = fd;
  tbl->hdr = hdr;
  tbl->fldAry = flds;
  
    /*getcwd (path, MAXPATH);
       strcat (path, "/"); */ 
    strcpy (path, fname);
  tbl->fileName = malloc (strlen (path) + 1);
  Assert (0 != tbl->fileName);
  if (tbl->fileName == 0) {
    free (tbl);
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl->fileName, 0, strlen (path) + 1);
  check_pointer (tbl->fileName);
  cp = strchr (path, (int) '\\');
  while (cp != NULL) {
    *cp = '/';
    cp = strchr (path, (int) '\\');
  } strcpy (tbl->fileName, path);
  check_pointer (tbl->fileName);
  tbl->fieldNames = malloc ((hdr->numFields + 1) * sizeof (char *));
  Assert (0 != tbl->fieldNames);
  if (0 == tbl->fieldNames) {
    free (tbl->fileName);
    free (tbl);
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl->fieldNames, 0, ((hdr->numFields + 1) * sizeof (char *)));
  check_pointer (tbl->fieldNames);
  for (i = 0; flds[i] != NULL; i++)
    tbl->fieldNames[i] = flds[i]->fieldName;
  tbl->fieldNames[hdr->numFields] = NULL;
  check_pointer (tbl->fieldNames);
  tbl->offset = sizeof (dbHeader) + hdr->numFields * sizeof (dbField);
  tbl->crec = 0;
  tbl->dbError = DB_NOERROR;
  tbl->dbErrMsg = malloc (MAX_ERROR_STRING + 1);
  if (tbl->dbErrMsg == 0) {
    free (tbl->fieldNames);
    free (tbl->fileName);
    free (tbl);
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl->dbErrMsg, 0, MAX_ERROR_STRING + 1);
  check_pointer (tbl->dbErrMsg);
  tbl->data = malloc (hdr->sizeRecord);
  Assert (0 != tbl->data);
  if (0 == tbl->data) {
    free (tbl->dbErrMsg);
    free (tbl->fieldNames);
    free (tbl->fileName);
    free (tbl);
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl->data, 0, hdr->sizeRecord);
  check_pointer (tbl->data);
  tbl->fields = malloc ((hdr->numFields + 1) * sizeof (char *));
  Assert (0 != tbl->fields);
  if (0 == tbl->fields) {
    free (tbl->data);
    free (tbl->dbErrMsg);
    free (tbl->fieldNames);
    free (tbl->fileName);
    free (tbl);
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl->fields, 0, ((hdr->numFields + 1) * sizeof (char *)));
  check_pointer (tbl->fields);
  tbl->flens = malloc ((hdr->numFields + 1) * sizeof (int));
  Assert (0 != tbl->flens);
  if (0 == tbl->flens) {
    free (tbl->fields);
    free (tbl->data);
    free (tbl->dbErrMsg);
    free (tbl->fieldNames);
    free (tbl->fileName);
    free (tbl);
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl->flens, 0, (hdr->numFields + 1) * sizeof (int));
  check_pointer (tbl->flens);
  tbl->declens = malloc ((hdr->numFields + 1) * sizeof (int));
  Assert (0 != tbl->declens);
  if (0 == tbl->declens) {
    free (tbl->flens);
    free (tbl->fields);
    free (tbl->data);
    free (tbl->dbErrMsg);
    free (tbl->fieldNames);
    free (tbl->fileName);
    free (tbl);
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl->flens, 0, (hdr->numFields + 1) * sizeof (int));
  check_pointer (tbl->flens);
  for (i = 0; flds[i] != NULL; i++) {
    tbl->fields[i] = malloc (flds[i]->fieldLength + 1);
    Assert (0 != tbl->fields[i]);
    if (0 == tbl->fields[i]) {
      
	/* deallocate what is convenient... */ 
	free (tbl->flens);
      free (tbl->fields);
      free (tbl->data);
      free (tbl->dbErrMsg);
      free (tbl->fieldNames);
      free (tbl->fileName);
      free (tbl);
      dbError = DB_NOMEMORY;
      return 0;
    }
    memset (tbl->fields[i], 0, flds[i]->fieldLength + 1);
    check_pointer (tbl->fields[i]);
    tbl->flens[i] = flds[i]->fieldLength;
    tbl->declens[i] = flds[i]->decLength;
    check_pointer (tbl->fields[i]);
    check_pointer (tbl->flens);
    check_pointer (tbl->declens);
  }
  check_pointer (tbl->fields);
  tbl->ctype = 0;
  tbl->entryCache = tbl->searchCache = tbl->queryCache = 0;
  
    /*
     * Write the data
     */ 
    written = fileSeekBegin (tbl->fd, 0L);
  Assert (_ERROR_ != written);
  if (_ERROR_ == written) {
    dbError = fioError;
    return 0;
  }
  strcpy (tbl->hdr->timeStamp, getTimeStamp ());
  written = fileWrite (tbl->fd, tbl->hdr, sizeof (dbHeader));
  Assert (written == sizeof (dbHeader));
  if (written != sizeof (dbHeader)) {
    dbError = fioError;
    return 0;
  }
  for (i = 0; flds[i] != NULL; i++) {
    written = fileWrite (tbl->fd, flds[i], sizeof (dbField));
    Assert (written == sizeof (dbField));
    if (written != sizeof (dbField)) {
      dbError = fioError;
      return 0;
    }
  }
  check_pointer (tbl);
  dbg2 (printf ("\n\ncreateTable() end\n");
    );
  
    /*
     * Create a table lock.
     */ 
    strcpy (path, tbl->fileName);
  strcat (path, ".LCK");
  fileClose (fileCreate (path));
  /* FIXME */
#if 0
  status =
    fileLockRegion (tbl->fd, F_SETLK, F_WRLCK, 0, FILE_BEGINNING,
		    sizeof (dbHeader));
  if (status == _ERROR_) {
    dbError = fioError;
    return 0;
  }
#endif
  return tbl;
}


/*
 * [BeginDoc]
 * \subsection{openTable}
 * \index{openTable}
 * [Verbatim] */ 
  dbTable * openTable (const char *fname) 
/* [EndDoc] */ 
  
/*
 * [BeginDoc]
 * openTable() opens a table that already exists.  ``fname'' is
 * the name of the table on the file system.
 * A valid pointer
 * to a dbTable structure is returned upon success, NULL is returned
 * otherwise.  All indexes for the table are opened when the table
 * is opened and are managed by the table members idxList and
 * midxList.
 * 
 * If an error occurs, isDBError() will be true and dberror() will
 * return a string that describes the error.
 * 
 * [EndDoc]
 */ 
{
  dbHeader * hdr;
  dbField ** flds;
  dbTable * tbl;
  int fd, i;
  off_t rd;
  char path[MAXPATH];
  char *cp;
  dbg2 (printf ("\n\nopenTable(%s)\n", fname);
    );
  strcpy (path, fname);
  if (!fexists (path)) {
    dbError = DB_INVALIDFILE;
    return 0;
  }
  strcat (path, ".LCK");
  if (fexists (path)) {
    dbError = DB_LOCK;
    return 0;
  }
    /* FIXME */
    /* The locking code isn't working - fix it. */
#if 0
    fd = fileOpen (fname);
    if (fd < 0) {
      dbError = fioError;
      return 0;
    }
    if (fileLockTest (fd, F_WRLCK, 0, FILE_BEGINNING, sizeof (dbHeader))) {
      fileClose (fd);
      dbError = DB_LOCK;
      return 0;
    }
    
    else {
      
	/* it's just a file lock...remove it */ 
	fileClose (fd);
      fileRemove (path);
    }
  }
#endif
  fd = fileOpen (fname);
  if (_ERROR_ == fd) {
    dbError = fioError;
    return 0;
  }
  
    /*
     * Allocate the structures
     */ 
    tbl = malloc (sizeof (dbTable));
  Assert (0 != tbl);
  if (0 == tbl) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl, 0, sizeof (dbTable));
  check_pointer (tbl);
  hdr = malloc (sizeof (dbHeader));
  Assert (0 != hdr);
  if (0 == hdr) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (hdr, 0, sizeof (dbHeader));
  check_pointer (hdr);
  tbl->fd = fd;
  tbl->hdr = hdr;
  rd = fileRead (tbl->fd, (void *) hdr, sizeof (dbHeader));
  check_pointer (hdr);
  Assert (sizeof (dbHeader) == rd);
  if (rd != sizeof (dbHeader)) {
    dbError = DB_IO_READWRITE;
    return 0;
  }
  Assert (DB_MAGIC == hdr->magic);
  if (DB_MAGIC != hdr->magic) {
    dbError = DB_INVALIDFILE;
    return 0;
  }
  flds = malloc ((hdr->numFields + 1) * (sizeof (dbField *)));
  Assert (0 != flds);
  if (0 == flds) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (flds, 0, ((hdr->numFields + 1) * (sizeof (dbField *))));
  check_pointer (flds);
  for (i = 0; i < hdr->numFields; i++) {
    flds[i] = malloc (sizeof (dbField));
    Assert (0 != flds[i]);
    if (0 == flds[i]) {
      dbError = DB_NOMEMORY;
      return 0;
    }
    memset (flds[i], 0, sizeof (dbField));
    check_pointer (flds[i]);
  }
  flds[hdr->numFields] = NULL;
  check_pointer (flds);
  tbl->fieldNames = malloc ((hdr->numFields + 1) * sizeof (char *));
  Assert (0 != tbl->fieldNames);
  if (0 == tbl->fieldNames) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl->fieldNames, 0, ((hdr->numFields + 1) * (sizeof (char *))));
  check_pointer (tbl->fieldNames);
  for (i = 0; i < hdr->numFields; i++) {
    rd = fileRead (tbl->fd, (void *) flds[i], sizeof (dbField));
    Assert (rd == sizeof (dbField));
    if (rd != sizeof (dbField)) {
      dbError = DB_IO_READWRITE;
      return 0;
    }
    tbl->fieldNames[i] = flds[i]->fieldName;
    check_pointer (flds[i]->fieldName);
    check_pointer (tbl->fieldNames);
  }
  tbl->fldAry = flds;
  
    /*
     * Fill in the rest of tbl structure
     */ 
    /*getcwd (path, MAXPATH);
       strcat (path, "/"); */ 
    strcpy (path, fname);
  tbl->fileName = malloc (strlen (path) + 1);
  Assert (0 != tbl->fileName);
  if (0 == tbl->fileName) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl->fileName, 0, strlen (path) + 1);
  check_pointer (tbl->fileName);
  cp = strchr (path, (int) '\\');
  while (cp != NULL) {
    *cp = '/';
    cp = strchr (path, (int) '\\');
  } strcpy (tbl->fileName, path);
  check_pointer (tbl->fileName);
  tbl->offset = sizeof (dbHeader) + hdr->numFields * sizeof (dbField);
  tbl->crec = 0;
  tbl->dbError = DB_NOERROR;
  tbl->dbErrMsg = malloc (MAX_ERROR_STRING + 1);
  if (tbl->dbErrMsg == 0) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl->dbErrMsg, 0, MAX_ERROR_STRING + 1);
  check_pointer (tbl->dbErrMsg);
  tbl->data = malloc (hdr->sizeRecord);
  Assert (0 != tbl->data);
  if (0 == tbl->data) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl->data, 0, hdr->sizeRecord);
  check_pointer (tbl->data);
  tbl->fields = malloc ((hdr->numFields + 1) * sizeof (char *));
  Assert (0 != tbl->fields);
  if (0 == tbl->fields) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl->fields, 0, (hdr->numFields + 1) * sizeof (char *));
  check_pointer (tbl->fields);
  tbl->flens = malloc ((hdr->numFields + 1) * sizeof (int));
  Assert (0 != tbl->flens);
  if (0 == tbl->flens) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl->flens, 0, (hdr->numFields + 1) * sizeof (int));
  tbl->declens = malloc ((hdr->numFields + 1) * sizeof (int));
  Assert (0 != tbl->declens);
  if (0 == tbl->declens) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl->declens, 0, (hdr->numFields + 1) * sizeof (int));
  check_pointer (tbl->declens);
  for (i = 0; flds[i] != NULL; i++) {
    tbl->fields[i] = malloc (flds[i]->fieldLength + 1);
    Assert (0 != tbl->fields[i]);
    if (0 == tbl->fields[i]) {
      dbError = DB_NOMEMORY;
      return 0;
    }
    memset (tbl->fields[i], 0, flds[i]->fieldLength + 1);
    check_pointer (tbl->fields[i]);
    tbl->flens[i] = flds[i]->fieldLength;
    tbl->declens[i] = flds[i]->decLength;
    check_pointer (tbl->fields[i]);
    check_pointer (tbl->flens);
    check_pointer (tbl->declens);
  }
  tbl->ctype = 0;
  tbl->entryCache = tbl->searchCache = tbl->queryCache = 0;
  i = openDBIndexes (tbl);
  Assert (i != _ERROR_);
  if (_ERROR_ == i) {
    dbError = tbl->dbError;
    closeTable (tbl);
    return 0;
  }
  i = openMultiIndexes (tbl);
  Assert (i != _ERROR_);
  if (_ERROR_ == i) {
    dbError = tbl->dbError;
    closeTable (tbl);
    return 0;
  }
  strcpy (path, tbl->fileName);
  strcat (path, ".LCK");
  fileClose (fileCreate (path));
  /* FIXME */
#if 0
  i =
    fileLockRegion (tbl->fd, F_SETLK, F_WRLCK, 0, FILE_BEGINNING,
		    sizeof (dbHeader));
  if (i == _ERROR_) {
    dbError = fioError;
    return 0;
  }
#endif
  dbg2 (printf ("\n\nopenTable() end\n");
    );
  return tbl;
}


/*
 * [BeginDoc]
 * \subsection{openTableNoIndexes}
 * \index{openTableNoIndexes}
 * [Verbatim] */ 
  dbTable * openTableNoIndexes (const char *fname) 
/* [EndDoc] */ 
  
/*
 * [BeginDoc]
 * openTableNoIndexes() opens the table with the name ``fname'' without
 * opening the indexes associated with the table.  This is useful for
 * applications in which tables have corrupted indexes and the application
 * still has to open the tables.
 * 
 * Upon success, openTableNoIndexes() returns a valid table descriptor.  If
 * an error occurs, isDBError() will be TRUE and dberror() will return a
 * description of the error that occurred.
 * 
 * [EndDoc]
 */ 
{
  dbHeader * hdr;
  dbField ** flds;
  dbTable * tbl;
  int fd, i;
  off_t rd;
  char path[MAXPATH];
  char *cp;
  dbg2 (printf ("\n\nopenTableNoIndexes(%s)\n", fname);
    );
  strcpy (path, fname);
  strcat (path, ".LCK");
  if (fexists (path)) {
    dbError = DB_LOCK;
    return 0;
  }
  /* FIXME */
#if 0
    fd = fileOpen (fname);
    if (fd < 0) {
      dbError = fioError;
      return 0;
    }
    if (fileLockTest (fd, F_WRLCK, 0, FILE_BEGINNING, sizeof (dbHeader))) {
      fileClose (fd);
      dbError = DB_LOCK;
      return 0;
    }
    
    else {
      fileClose (fd);
      fileRemove (path);
    }
  }
#endif
  fd = fileOpen (fname);
  if (_ERROR_ == fd) {
    dbError = fioError;
    return 0;
  }
  
    /*
     * Allocate the structures
     */ 
    tbl = malloc (sizeof (dbTable));
  Assert (0 != tbl);
  if (0 == tbl) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl, 0, sizeof (dbTable));
  check_pointer (tbl);
  hdr = malloc (sizeof (dbHeader));
  Assert (0 != hdr);
  if (0 == hdr) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (hdr, 0, sizeof (dbHeader));
  check_pointer (hdr);
  tbl->fd = fd;
  tbl->hdr = hdr;
  rd = fileRead (tbl->fd, (void *) hdr, sizeof (dbHeader));
  check_pointer (hdr);
  Assert (sizeof (dbHeader) == rd);
  if (rd != sizeof (dbHeader)) {
    dbError = DB_IO_READWRITE;
    return 0;
  }
  Assert (DB_MAGIC == hdr->magic);
  if (DB_MAGIC != hdr->magic) {
    dbError = DB_INVALIDFILE;
    return 0;
  }
  flds = malloc ((hdr->numFields + 1) * (sizeof (dbField *)));
  Assert (0 != flds);
  if (0 == flds) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (flds, 0, ((hdr->numFields + 1) * (sizeof (dbField *))));
  check_pointer (flds);
  for (i = 0; i < hdr->numFields; i++) {
    flds[i] = malloc (sizeof (dbField));
    Assert (0 != flds[i]);
    if (0 == flds[i]) {
      dbError = DB_NOMEMORY;
      return 0;
    }
    memset (flds[i], 0, sizeof (dbField));
    check_pointer (flds[i]);
  }
  flds[hdr->numFields] = NULL;
  check_pointer (flds);
  tbl->fieldNames = malloc ((hdr->numFields + 1) * sizeof (char *));
  Assert (0 != tbl->fieldNames);
  if (0 == tbl->fieldNames) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl->fieldNames, 0, ((hdr->numFields + 1) * sizeof (char *)));
  check_pointer (tbl->fieldNames);
  for (i = 0; i < hdr->numFields; i++) {
    rd = fileRead (tbl->fd, (void *) flds[i], sizeof (dbField));
    Assert (rd == sizeof (dbField));
    if (rd != sizeof (dbField)) {
      dbError = DB_IO_READWRITE;
      return 0;
    }
    tbl->fieldNames[i] = flds[i]->fieldName;
    check_pointer (flds[i]->fieldName);
    check_pointer (tbl->fieldNames);
  }
  tbl->fldAry = flds;
  
    /*
     * Fill in the rest of tbl structure
     */ 
    /*getcwd (path, MAXPATH);
       strcat (path, "/"); */ 
    strcpy (path, fname);
  tbl->fileName = malloc (strlen (path) + 1);
  Assert (0 != tbl->fileName);
  if (0 == tbl->fileName) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl->fileName, 0, strlen (path) + 1);
  check_pointer (tbl->fileName);
  cp = strchr (path, (int) '\\');
  while (cp != NULL) {
    *cp = '/';
    cp = strchr (path, (int) '\\');
  } strcpy (tbl->fileName, path);
  check_pointer (tbl->fileName);
  tbl->offset = sizeof (dbHeader) + hdr->numFields * sizeof (dbField);
  tbl->crec = 0;
  tbl->dbError = DB_NOERROR;
  tbl->dbErrMsg = malloc (MAX_ERROR_STRING + 1);
  if (tbl->dbErrMsg == 0) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl->dbErrMsg, 0, MAX_ERROR_STRING + 1);
  check_pointer (tbl->dbErrMsg);
  tbl->data = malloc (hdr->sizeRecord);
  Assert (0 != tbl->data);
  if (0 == tbl->data) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl->data, 0, hdr->sizeRecord);
  check_pointer (tbl->data);
  tbl->fields = malloc ((hdr->numFields + 1) * sizeof (char *));
  Assert (0 != tbl->fields);
  if (0 == tbl->fields) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl->fields, 0, (hdr->numFields + 1) * sizeof (char *));
  check_pointer (tbl->fields);
  tbl->flens = malloc ((hdr->numFields + 1) * sizeof (int));
  Assert (0 != tbl->flens);
  if (0 == tbl->flens) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl->flens, 0, (hdr->numFields + 1) * sizeof (int));
  check_pointer (tbl->flens);
  tbl->declens = malloc ((hdr->numFields + 1) * sizeof (int));
  Assert (0 != tbl->declens);
  if (0 == tbl->declens) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (tbl->declens, 0, (hdr->numFields + 1) * sizeof (int));
  check_pointer (tbl->declens);
  for (i = 0; flds[i] != NULL; i++) {
    tbl->fields[i] = malloc (flds[i]->fieldLength + 1);
    Assert (0 != tbl->fields[i]);
    if (0 == tbl->fields[i]) {
      dbError = DB_NOMEMORY;
      return 0;
    }
    memset (tbl->fields[i], 0, flds[i]->fieldLength + 1);
    check_pointer (tbl->fields[i]);
    tbl->flens[i] = flds[i]->fieldLength;
    tbl->declens[i] = flds[i]->decLength;
    check_pointer (tbl->fields[i]);
    check_pointer (tbl->flens);
    check_pointer (tbl->declens);
  }
  tbl->ctype = 0;
  tbl->entryCache = tbl->searchCache = tbl->queryCache = 0;
  strcpy (path, tbl->fileName);
  strcat (path, ".LCK");
  fileClose (fileCreate (path));
  dbg2 (printf ("\n\nopenTableNoIndexes(%s)\n", fname);
    );
  /* FIXME */
#if 0
  i =
    fileLockRegion (tbl->fd, F_SETLK, F_WRLCK, 0, FILE_BEGINNING,
		    sizeof (dbHeader));
  if (i == _ERROR_) {
    dbError = fioError;
    return 0;
  }
#endif
  return tbl;
}


/*
 * [BeginDoc]
 * \subsection{closeTable}
 * \index{closeTable}
 * [Verbatim] */ 
int closeTable (dbTable * tbl)  
/* [EndDoc] */ 

/*
 * [BeginDoc] 
 * closeTable() closes all open indexes associated with the table,
 * writes the table header information, flushes the table file,
 * closes the table and frees all the dynamically allocated memory
 * ``owned'' by the table structure.  Because of the caching
 * that occurs with table files and index files, it is very important
 * that the user close all open tables before the application exits.
 * If a table is not closed, it is very likely that data corruption will
 * occur.  If the indexes are not closed, it is almost certain that
 * index corruption will occur.
 * 
 * closetable() returns _OK_ on success.  If an error occurs,
 * isDBError() is TRUE and dberror() will return a string describing
 * the error which occurred.  closeTable() will make every attempt to
 * continue closing a table even when errors occur, to insure that
 * data is preserved if possible.  Therefore, if multiple errors
 * occur, the error indicated by dberror() will be the last one
 * to have occurred.
 * 
 * [EndDoc]
 */ 
{
  int i;
  unsigned numFlds;
  unsigned sizeRec;
  char path[MAXPATH];
  int ret = _OK_;
  dbg2 (printf ("\n\ncloseTable()\n");
    );
  
    /*
     * tbl->fd
     */ 
    i = storeTableHeader (tbl);
  Assert (_ERROR_ != i);
  if (_ERROR_ == i)		/* Error already set */
    ret = _ERROR_;
  /* FIXME */
#if 0
  i =
    fileLockRegion (tbl->fd, F_SETLK, F_UNLCK, 0, FILE_BEGINNING,
		    sizeof (dbHeader));
  if (i == _ERROR_) {
    dbError = fioError;
    return 0;
  }
#endif
  fileClose (tbl->fd);
  strcpy (path, tbl->fileName);
  strcat (path, ".LCK");
  if (fexists (path))
    fileRemove (path);
  
    /*
     * tbl->idxList
     */ 
    if (tbl->idxList != 0)
    if (tbl->idxList->number > 0)
      closeDBIndexes (tbl);
  if (dbError != DB_NOERROR)
    ret = _ERROR_;
  if (tbl->midxList != 0)
    if (tbl->midxList->number > 0)
      closeMultiIndexes (tbl);
  if (dbError != DB_NOERROR)
    ret = _ERROR_;;
  check_pointer (tbl->fields);
  
    /*
     * tbl->dbh, tbl->fldAry, tbl->fieldNames, tbl->fileName
     */ 
    numFlds = tbl->hdr->numFields;
  sizeRec = tbl->hdr->sizeRecord;
  check_pointer (tbl->hdr);
  free (tbl->hdr);
  for (i = 0; tbl->fldAry[i] != NULL; i++) {
    check_pointer (tbl->fldAry[i]);
    free (tbl->fldAry[i]);
  }
  check_pointer (tbl->fldAry);
  free (tbl->fldAry);
  check_pointer (tbl->fieldNames);
  free (tbl->fieldNames);
  check_pointer (tbl->fileName);
  free (tbl->fileName);
  
    /*
     * tbl->dbErrMsg, tbl->data, tbl->fields, tbl->flens
     */ 
    check_pointer (tbl->dbErrMsg);
  free (tbl->dbErrMsg);
  check_pointer (tbl->data);
  free (tbl->data);
  check_pointer (tbl->fields);
  for (i = 0; i < (int)numFlds; i++) {
    check_pointer (tbl->fields[i]);
    free (tbl->fields[i]);
  }
  check_pointer (tbl->fields);
  check_pointer (tbl->fields);
  free (tbl->fields);
  check_pointer (tbl->flens);
  free (tbl->flens);
  check_pointer (tbl->declens);
  free (tbl->declens);
  
    /*
     * tbl
     */ 
    check_pointer (tbl);
  free (tbl);
  dbg2 (printf ("\n\ncloseTable() end\n");
    );
  return ret;
}


/*
 * [BeginDoc]
 * \subsection{storeTableHeader}
 * \index{storeTableHeader}
 * [Verbatim] */ 
int storeTableHeader (dbTable * tbl)  
/* [EndDoc] */ 

/*
 * [BeginDoc]
 * storeTableHeader() stores the table header.  This stores the
 * table header to disk.  This is done before the table is closed
 * to insure that the memory data gets flushed to disk, but it can
 * also be done by the user at any time during execution.  The time
 * stamp is updated and the header is copied to the beginning of the
 * file.
 * 
 * The table header is stored to disk if this function is successful.
 * If not, isTableError(tbl) will be TRUE and dbtblerror() will return
 * a string that describes the error.
 * 
 * [EndDoc]
 */ 
{
  off_t loc;
  off_t written;
  if (tbl == 0)
    return _ERROR_;
  strcpy (tbl->hdr->timeStamp, getTimeStamp ());
  loc = fileSeekBegin (tbl->fd, 0L);
  Assert (0 == loc);
  if (loc != 0) {
    tbl->dbError = DB_UNSPECIFIED;
    return _ERROR_;
  }
  written = fileWrite (tbl->fd, tbl->hdr, sizeof (dbHeader));
  Assert (sizeof (dbHeader) == written);
  if (sizeof (dbHeader) != written) {
    tbl->dbError = DB_IO_READWRITE;
    return _ERROR_;
  }
  
    /*
     * return to the current offset in the file
     */ 
    /*fileFlush (tbl->fd); *//* flush file */ 
    loc = fileSeekBegin (tbl->fd, tbl->offset);
  Assert (loc == tbl->offset);
  if (loc != tbl->offset) {
    tbl->dbError = DB_UNSPECIFIED;
    return _ERROR_;
  }
  return _OK_;
}


/*
 * [BeginDoc]
 * \subsection{storeFieldHeader}
 * \index{storeFieldHeader}
 * [Verbatim] */ 
int storeFieldHeader (int num, dbTable * tbl)  
/* [EndDoc] */ 
/*
 * [BeginDoc]
 * storeFieldHeader() stores the ``num'' field header of table 
 * ``tbl'' .  ``num'' is assumed to hold the field number (0-based,
 * so 0 => first field, etc.).  ``num'' should be passed to the 
 * function such that tbl->fldAry[num] == field to be updated.
 * If storeFieldHeader() is successful, it returns _OK_.  If not,
 * isTableError(tbl) will be TRUE for the table and dbtblerror()
 * will return a description of the error.
 * 
 * [EndDoc]
 */ 
{
  off_t loc;
  off_t written;
  off_t here;
  here = sizeof (dbHeader) + (num * sizeof (dbField));
  loc = fileSeekBegin (tbl->fd, here);
  Assert (here == loc);
  if (loc != here) {
    tbl->dbError = DB_UNSPECIFIED;
    return _ERROR_;
  }
  written = fileWrite (tbl->fd, tbl->fldAry[num], sizeof (dbField));
  Assert (sizeof (dbField) == written);
  if (sizeof (dbField) != written) {
    tbl->dbError = DB_IO_READWRITE;
    return _ERROR_;
  }
  
    /*
     * return to the current offset in the file
     */ 
    /*fileFlush (tbl->fd); *//* flush file */ 
    loc = fileSeekBegin (tbl->fd, tbl->offset);
  Assert (loc == tbl->offset);
  if (loc != tbl->offset) {
    tbl->dbError = DB_UNSPECIFIED;
    return _ERROR_;
  }
  return _OK_;
}


#ifdef	DEBUG
  
# include <test.h>
  
/*
 * Don't bother to document for now.
 */ 
int DebugCheckTable (dbTable * tbl) 
{
  int i;
  check_pointer (tbl->hdr);
  check_pointer (tbl->fldAry);
  check_pointer (tbl->data);
  check_pointer (tbl->dbErrMsg);
  for (i = 0; i < tbl->hdr->numFields; i++) {
    check_pointer (tbl->fldAry[i]);
    check_pointer (tbl->fields[i]);
    check_pointer (tbl->flens);
    check_pointer (tbl->declens);
  }
  check_pointer (tbl->fileName);
  check_pointer (tbl);
  return _OK_;
}


#endif	/* DEBUG */
  
