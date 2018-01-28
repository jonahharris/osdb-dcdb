/*	Source File:	cdbws.c	*/

#include <cdb.h>

/*
 * [BeginDoc]
 * 
 * \section{DCDB Workspace Functions}
 * 
 * A workspace is basically a grouping of tables.  The workspace functionality
 * is provided as a convenience.  The user can create the tables necessary for
 * an application, create the supporting indexes and then add the tables to
 * a workspace.  Once that is done, the user only needs to open and close
 * the workspace to open and close all the tables and indexes that are part of the 
 * workspace.
 * 
 * [EndDoc]
 */

static int wsCompare (void *p1, void *p2)
{
  if (((wsField *) p1)->fieldNum == ((wsField *) p2)->fieldNum)
    return 0;
  return (((wsField *) p1)->fieldNum - ((wsField *) p2)->fieldNum);
}

/*
 * [BeginDoc]
 * \subsection{wsCreate}
 * \index{wsCreate}
 * [Verbatim] */

workSpace *wsCreate (const char *name)

/* [EndDoc] */
/*
 * [BeginDoc]
 * wsCreate() creates a workspace with name ``name''.
 * wsCreate() simply sets up the internal
 * variables for a workspace and saves them to a file.  If 
 * it is successful, wsCreate() returns a valid workSpace
 * descriptor that can be used in further workspace function calls.
 * 
 * If an error occurs, isDBError() is TRUE and dberror() returns a string
 * that describes the error.
 * 
 * [EndDoc]
 */
{
  int fd;
  off_t status = 0;
  workSpace *ws;

  ws = malloc (sizeof (workSpace));
  if (ws == 0) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (ws, 0, sizeof (workSpace));
  check_pointer (ws);

  ws->hdr = malloc (sizeof (wsHeader));
  if (ws->hdr == 0) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (ws->hdr, 0, sizeof (wsHeader));
  check_pointer (ws->hdr);

  ws->hdr->magic = DB_MAGIC;
  strncpy (ws->hdr->wsName, name, TABLE_NAME_WIDTH);
  check_pointer (ws->hdr);
  strncpy (ws->fileName, name, TABLE_NAME_WIDTH);
  check_pointer (ws);

  fd = fileCreate (name);
  if (_ERROR_ == fd) {
    dbError = fioError;
    free (ws->hdr);
    free (ws);
    return 0;
  }
  fileClose (fd);
  fd = fileOpen (name);
  if (_ERROR_ == fd) {
    dbError = fioError;
    free (ws->hdr);
    free (ws);
    return 0;
  }

  /* write the header to file */
  status = fileWrite (fd, ws->hdr, sizeof (wsHeader));
  if (status != sizeof (wsHeader)) {
    fileClose (fd);
    dbError = DB_IO_READWRITE;
    free (ws->hdr);
    free (ws);
    return 0;
  }
  fileClose (fd);
  check_pointer (ws->hdr);
  check_pointer (ws);
  return ws;
}

/*
 * [BeginDoc]
 * \subsection{wsAddTable}
 * \index{wsAddTable}
 * [Verbatim] */

int wsAddTable (workSpace * ws, dbTable * tbl, char *comment)

/* [EndDoc] */
/*
 * [BeginDoc]
 * wsAddTable() adds the table ``tbl'' to the workspace ``ws''.  The character
 * string ``comment'' allows the application designer to store information that
 * is application dependent with the table information in the workspace.  If
 * wsAddTable() is successful, the current index (tbl->current) becomes the
 * default index and is set as current each time the workspace opens the table.
 * wsAddTable() returns _OK_ if it succeeds.
 * 
 * If an error occurs, isDBError() is TRUE and dberror() returns a string
 * that describes the error.
 * 
 * [EndDoc]
 */
{
  int status;
  int fd;
  int i;
  off_t written;
  off_t offset;
  wsField *fld;
  dbIndex *idx;
  Link *lnk;

  check_pointer (ws);
  check_pointer (tbl);
  if (ws->fields == 0) {
    ws->fields = initShell (wsCompare, TRUE, TRUE);
    if (ws->fields == 0) {
      dbError = DB_SHELL;
      return _ERROR_;
    }
  }
  fld = malloc (sizeof (wsField));
  if (0 == fld) {
    dbError = DB_NOMEMORY;
    return _ERROR_;
  }
  memset (fld, 0, sizeof (wsField));
  check_pointer (fld);

  ws->number++;
  ws->hdr->numWS++;
  fld->fieldNum = ws->number;
  check_pointer (tbl);
  fld->tbl = tbl;
  ws->current = fld;
  if (comment)
    strncpy (fld->tableComment, comment, TABLE_INFO_WIDTH);
  strcpy (fld->tableName, tbl->fileName);
  if (tbl->current) {
    idx = tbl->current;
    strcpy (fld->currentIndex, idx->hdr->indexName);
  }

  lnk = malloc (sizeof (Link));
  if (0 == lnk) {
    dbError = DB_NOMEMORY;
    free (fld);
    ws->number--;
    return _ERROR_;
  }
  lnk->data = fld;
  check_pointer (lnk);
  check_pointer (ws->fields);
  status = addShellItem (ws->fields, lnk);
  if (_ERROR_ == status) {
    dbError = DB_SHELL;
    free (fld);
    ws->number--;
    return _ERROR_;
  }

  /*
   * Now, store what we've done in the file and go away.
   */
  fd = fileOpen (ws->fileName);
  if (_ERROR_ == fd) {
    dbError = fioError;
    return _ERROR_;
  }
  written = fileWrite (fd, ws->hdr, sizeof (wsHeader));
  if (written != sizeof (wsHeader)) {
    dbError = DB_IO_READWRITE;
    fileClose (fd);
    return _ERROR_;
  }
  offset = written;
  for (i = 0, lnk = ws->fields->lh->head->next;
       i < ws->number && lnk != ws->fields->lh->tail; i++, lnk = lnk->next) {
    written = fileSeekBegin (fd, offset);
    if (written != offset) {
      dbError = DB_IO_READWRITE;
      fileClose (fd);
      return _ERROR_;
    }
    fld = lnk->data;
    written = fileWrite (fd, fld, sizeof (wsField));
    if (written != sizeof (wsField)) {
      dbError = DB_IO_READWRITE;
      fileClose (fd);
      return _ERROR_;
    }
  }

  fileClose (fd);
  check_pointer (ws->fields);
  check_pointer (ws->hdr);
  check_pointer (ws);
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{wsClose}
 * \index{wsClose}
 * [Verbatim] */

int wsClose (workSpace * ws)

/* [EndDoc] */
/*
 * [BeginDoc]
 * wsClose() closes the workspace given by the descriptor ``ws''.  All the
 * tables and indexes grouped with ``ws'' are closed as well.  wsClose()
 * returns _OK_ if it is successful.
 * 
 * If an error occurs, isDBError() is TRUE and dberror() returns a string
 * that describes the error.
 * 
 * [EndDoc]
 */
{
  int fd;
  int i;
  off_t written;
  off_t offset;
  wsField *fld;
  Link *lnk;

  check_pointer (ws->hdr);
  check_pointer (ws);
  fd = fileOpen (ws->fileName);
  if (_ERROR_ == fd) {
    dbError = fioError;
    return _ERROR_;
  }
  written = fileWrite (fd, ws->hdr, sizeof (wsHeader));
  if (written != sizeof (wsHeader)) {
    dbError = DB_IO_READWRITE;
    fileClose (fd);
    return _ERROR_;
  }
  offset = written;
  if (ws->fields != 0) {
    for (i = 0, lnk = ws->fields->lh->head->next;
	 i < ws->hdr->numWS && lnk != ws->fields->lh->tail;
	 i++, lnk = lnk->next) {
      written = fileSeekBegin (fd, offset);
      if (written != offset) {
	dbError = DB_IO_READWRITE;
	fileClose (fd);
	return _ERROR_;
      }
      fld = lnk->data;
      written = fileWrite (fd, fld, sizeof (wsField));
      if (written != sizeof (wsField)) {
	dbError = DB_IO_READWRITE;
	fileClose (fd);
	return _ERROR_;
      }
      storeTableHeader (fld->tbl);
      closeTable (fld->tbl);
      fld->tbl = 0;
      if (isDBError ()) {
	return _ERROR_;
      }
      offset += written;
    }
    delShell (ws->fields, 0);
  }
  fileClose (fd);
  free (ws->hdr);
  free (ws);
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{wsOpen}
 * \index{wsOpen}
 * [Verbatim] */

workSpace *wsOpen (const char *name)

/* [EndDoc] */
/*
 * [BeginDoc]
 * wsOpen() opens the workspace with the file name ``name''.  All tables
 * and indexes associated with the workspace are opened and default indexes
 * are set for each table.  If wsOpen() succeeds, it returns a valid
 * workspace descriptor; if an error occurs, the descriptor is not valid
 * (it is NULL), isDBError() is TRUE and dberror() returns a string
 * describing the error.
 * 
 * [EndDoc]
 */
{
  int fd;
  int i;
  off_t offset = 0L;
  off_t read = 0;
  workSpace *ws;
  wsField *fld;
  Link *lnk;
  int status;

  ws = malloc (sizeof (workSpace));
  if (ws == 0) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (ws, 0, sizeof (workSpace));
  check_pointer (ws);

  ws->hdr = malloc (sizeof (wsHeader));
  if (ws->hdr == 0) {
    dbError = DB_NOMEMORY;
    return 0;
  }
  memset (ws->hdr, 0, sizeof (wsHeader));
  check_pointer (ws->hdr);

  fd = fileOpen (name);
  if (fd == _ERROR_) {
    dbError = fioError;
    free (ws->hdr);
    free (ws);
    return 0;
  }
  fileSeekBegin (fd, 0);
  read = fileRead (fd, ws->hdr, sizeof (wsHeader));
  if (read != sizeof (wsHeader)) {
    dbError = DB_IO_READWRITE;
    free (ws->hdr);
    free (ws);
    fileClose (fd);
    return 0;
  }
  strcpy (ws->fileName, ws->hdr->wsName);
  if (ws->hdr->numWS == 0) {
    fileClose (fd);
    check_pointer (ws->hdr);
    check_pointer (ws);
    return ws;
  }
  ws->number = ws->hdr->numWS;

  ws->fields = initShell (wsCompare, TRUE, TRUE);
  if (ws->fields == 0) {
    dbError = DB_SHELL;
    free (ws->hdr);
    free (ws);
    fileClose (fd);
    return 0;
  }

  offset = read;
  for (i = 0; i < ws->hdr->numWS; i++) {
    read = fileSeekBegin (fd, offset);
    if (read != offset) {
      dbError = DB_IO_READWRITE;
      fileClose (fd);
      return 0;
    }
    fld = malloc (sizeof (wsField));
    if (0 == fld) {
      dbError = DB_NOMEMORY;
      fileClose (fd);
      return 0;
    }
    memset (fld, 0, sizeof (wsField));
    check_pointer (fld);
    read = fileRead (fd, fld, sizeof (wsField));
    if (read != sizeof (wsField)) {
      dbError = DB_IO_READWRITE;
      fileClose (fd);
      return 0;
    }
    fld->tbl = openTable (fld->tableName);
    if (isDBError ()) {
      fileClose (fd);
      free (fld);
      return 0;
    }
    setCurrentIndex (fld->tbl, fld->currentIndex);
    if (isTableError (fld->tbl)) {
      dbError = fld->tbl->dbError;
      fileClose (fd);
      closeTable (fld->tbl);
      free (fld);
      return 0;
    }
    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      dbError = DB_NOMEMORY;
      fileClose (fd);
      closeTable (fld->tbl);
      free (fld);
      return 0;
    }
    lnk->data = fld;
    status = addShellItem (ws->fields, lnk);
    if (status == _ERROR_) {
      dbError = DB_SHELL;
      fileClose (fd);
      closeTable (fld->tbl);
      free (fld);
      return 0;
    }
    offset += read;
  }
  fileClose (fd);
  return ws;
}

/*
 * [BeginDoc]
 * \subsection{wsGetTable}
 * \index{wsGetTable}
 * [Verbatim] */

dbTable *wsGetTable (workSpace * ws, char *name)

/* [EndDoc] */
/*
 * [BeginDoc]
 * wsGetTable() returns a descriptor to the table with the file name ``name''
 * from the workspace ``ws''.  It is not an error if there is no table with
 * that name in the workspace; NULL is simply returned in that case.  If an
 * error occurs, isDBError() is TRUE and dberror() returns a string describing
 * the error.
 * 
 * [EndDoc]
 */
{
  wsField *fld;
  Link *lnk = 0;

  if (ws == 0 || name == 0) {
    dbError = DB_PARAM;
    return 0;
  }
  if (ws->fields != 0) {
    lnk = ws->fields->lh->head->next;
    while (lnk != ws->fields->lh->tail) {
      fld = lnk->data;
      if (!strncmp (name, fld->tableName, strlen (name))) {
	ws->current = fld;
	return fld->tbl;
      }
      lnk = lnk->next;
    }
  }
  return 0;
}
