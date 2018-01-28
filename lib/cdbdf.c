/*	Source File:	cdbdf.c	*/

/*
 * [BeginDoc]
 * 
 * \section{DCDB Definition File Functions}
 * 
 * A definition file is a text file (similar to a script) that tells
 * \index{definition file}
 * the definition processor what to do.  The allowable actions are:
 * 
 * \begin{itemize}
 * 
 * \item \emph{create table}
 * Creates a table and the associated indexes.  The generic syntax is
 * as follows:
 * [Verbatim] *

create table "tablename.db"
  info "Info string - application dependent"
{
   "charfield" char (num);
   "numfield" number (num:dec);
   "logicalfield" logical;
   "datefield" date;
   "timestamp" time;
} indexed {
   idx "idxname" blksz:[case]nocase:[dup]unique "fname";
   midx "midxname" blksz:[case]nocase "fld1","fld2",...;
};

 * [EndDoc] */

/*
 * [BeginDoc]
 * 
 * Note:  if there weren't any indexes for this table, the user would
 * simply replace the `indexed' keyword with `;' and that would be the
 * end of the table definition.
 * 
 * \item \emph{create workspace}
 * Creates a workspace.  The syntax for this command is as follows:
 * [Verbatim]

create workspace "workspacename.ws";

 * [EndDoc] */

/*
 * [BeginDoc]
 * 
 * \item \emph{add}
 * Adds a table to a workspace.  The syntax for this command is as
 * follows:
 * [Verbatim]

add "tablename.db" to "workspace.ws" index "currentidx";

 * [EndDoc] */
/*
 * [BeginDoc]
 * This adds ``tablename.db'' to the workspace ``workspace.ws'' with
 * the index ``currentidx'' the current index.
 * 
 * \item comments
 * Comments are allowed anywhere in the definition file.  Both C and
 * C++ comments are recognized and supported.
 * 
 * \end{itemize}
 * 
 * The test program ``flogws2.c'' works identically to ``flogws.c''
 * except it uses a definition file to create the tables, indexes and
 * workspace and bind them together.  Then, it opens the workspace
 * file, gets table information from it and proceeds to work.  Following
 * is the definition file used to create the objects:
 * [Verbatim]

//	Definition File:	flogws2.df
//
//	Creates tables, indexes and the 
//  workspace for flogws2 test routine.

create table "users.db"
  info "Primary:Logistical information for users"
{
	"ssnumber" char (9);
	"dob" date;
	"age" number (3:0);
	"street" char (65);
	"street2" char (65);
	"city" char (50);
	"state" char (2);
	"zip" char (10);
} indexed {
	idx "userssnidx" 256:case:unique "ssnumber";
};

create table "comments.db"
  info "Secondary:Text information for comments on users"
{
	"ssnumber" char (9);
	"lineNum" number (4:0);
	"cline" char (80);
} indexed {
	midx "commentsidx" 256:case "ssnumber", "lineNum";
};

create workspace "flogws2.ws";

add "users.db" to "flogws2.ws" index "userssnidx";
add "comments.db" to "flogws2.ws" index "commentsidx";

 * [EndDoc] */

/*
 * [BeginDoc]
 * When the parser has gone through the first ``create table'' ex\-pres\-sion,
 * the table ``users.db'' and the index file ``userssnidx.idx'' would
 * have been created.  After the second ``create table'' expression,
 * ``comments.db'' and ``commentsidx.idx'' will have been created.  Then,
 * the workspace file ``flogws2.ws'' is created by the ``create workspace''
 * command.  Finally, ``users.db'' is added to ``flogws2.ws'' with the
 * index ``userssnidx'' the current index and ``comments.db'' is added
 * to ``flogws2.ws'' with ``commentsidx'' as the current index.
 * 
 * After this definition file is parsed and executed, the programmer
 * need only open ``flogws2.ws'' to get access to the tables and indexes
 * that are added to the workspace.
 * 
 * [EndDoc]
 */

#include <cdb.h>
#include <mtoken.h>

#ifndef	TRUE
#define	TRUE	1
#endif

#ifndef	FALSE
#define	FALSE	0
#endif

/* DFM - 20030316 */
/*
 * Define a global instance of syntaxError and change cdb.h to refer to that
 * global instance.
 */
char syntaxError[SYNTAX_ERROR_WIDTH + 1];

typedef struct _field_item {
  char name[MAX_FIELD_WIDTH + 1];
  fieldType ftype;
  int flen;
  int declen;
} fieldItem;

typedef struct _index_item {
  char idxname[TABLE_NAME_WIDTH + 1];
  char fldname[MAX_FIELD_WIDTH + 1];
  int isCase;
  int isUnique;
  int blksize;
} indexItem;

int defCreate (parseItem * pi);
int defCreateWorkspace (parseItem * pi);
int defCreateTable (parseItem * pi);
int defAdd (parseItem * pi);
dbTable *dbBuildTable (char *tblname, char *info, fieldItem ** flds);

/*
 * [BeginDoc]
 * \subsection{parseDBDef}
 * \index{parseDBDef}
 * [Verbatim] */

int parseDBDef (const char *file)

/* [EndDoc] */
/*
 * [BeginDoc]
 * parseDBDef() parses the definition file given by ``file''.  If it
 * succeeds it returns _OK_ and the objects specified in the definition
 * file will have been created.  If it fails, it returns _ERROR_.
 * Because parseDBDef() performs the actions as it proceeds (as an
 * interpreter would), it is possible for parseDBDef() to have created
 * some of the objects in the definition file at the point when a
 * syntax error occurs.  Any clean up that is required is the responsibility
 * of the caller.
 * 
 * [EndDoc]
 */
{
  parseItem *pi;
  CDBTokenType tkn;
  int status;

  if (!fexists ((char *) file)) {
    dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH, "parse file \"%s\" does not exist", file);
#else
    sprintf (syntaxError, "parse file \"%s\" does not exist", file);
#endif
  }

  pi = initParser (file);
  if (0 == pi) {
    dbError = DB_PARSE;
    return _ERROR_;
  }
  tkn = getToken (pi);
  if (tkn != IDENT) {
    dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	     "expected identifier, not \"%s\", line %d of file \"%s\"",
	     pi->id, pi->lnctr, pi->tokenFile);
#else
    sprintf (syntaxError,
	     "expected identifier, not \"%s\", line %d of file \"%s\"",
	     pi->id, pi->lnctr, pi->tokenFile);
#endif
    deleteParser (pi);
    return _ERROR_;
  }
  while (tkn == IDENT) {
    if (!strcmp (pi->id, "create")) {
      status = defCreate (pi);
      if (status == _ERROR_)
	return _ERROR_;
    }
    else if (!strcmp (pi->id, "add")) {
      status = defAdd (pi);
      if (status == _ERROR_)
	return _ERROR_;
    }
    /* add new def items here */
    else {
      dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
      snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	       "expected \"create\" or \"add\", not \"%s\", line %d of file \"%s\"",
	       pi->id, pi->lnctr, pi->tokenFile);
#else
      sprintf (syntaxError,
	       "expected \"create\" or \"add\", not \"%s\", line %d of file \"%s\"",
	       pi->id, pi->lnctr, pi->tokenFile);
#endif
      deleteParser (pi);
      return _ERROR_;
    }
    tkn = getToken (pi);
  }
  if (tkn != DONE) {
    dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	"expected DONE, not \"%s\", line %d of file \"%s\"",
	     pi->id, pi->lnctr, pi->tokenFile);
#else
    sprintf (syntaxError,
	"expected DONE, not \"%s\", line %d of file \"%s\"",
	     pi->id, pi->lnctr, pi->tokenFile);
#endif
    deleteParser (pi);
    return _ERROR_;
  }
  deleteParser (pi);
  return _OK_;
}

int defCreate (parseItem * pi)
{
  int status;
  CDBTokenType tkn;

  tkn = getToken (pi);
  if (tkn != IDENT) {
    dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	     "expected identifier, not \"%s\", line %d of file \"%s\"",
	     pi->id, pi->lnctr, pi->tokenFile);
#else
    sprintf (syntaxError,
	     "expected identifier, not \"%s\", line %d of file \"%s\"",
	     pi->id, pi->lnctr, pi->tokenFile);
#endif
    deleteParser (pi);
    return _ERROR_;
  }
  if (!strcmp (pi->id, "workspace")) {
    status = defCreateWorkspace (pi);
    if (status == _ERROR_)
      return _ERROR_;
  }
  else if (!strcmp (pi->id, "table")) {
    status = defCreateTable (pi);
    if (status == _ERROR_)
      return _ERROR_;
  }
  /* add new create items here */
  else {
    /* syntax error */
    dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	     "expected \"workspace\" or \"table\", not \"%s\", line %d of file \"%s\"",
	     pi->id, pi->lnctr, pi->tokenFile);
#else
    sprintf (syntaxError,
	     "expected \"workspace\" or \"table\", not \"%s\", line %d of file \"%s\"",
	     pi->id, pi->lnctr, pi->tokenFile);
#endif
    deleteParser (pi);
    return _ERROR_;
  }
  return _OK_;
}

/*
 * Syntax:
 * create workspace "workspacename.ws";
 */
int defCreateWorkspace (parseItem * pi)
{
  CDBTokenType tkn;
  workSpace *ws;

  tkn = getToken (pi);
  if (tkn != STRING) {
    dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	     "expecting a string, not \"%s\", line %d of file \"%s\"", pi->id,
	     pi->lnctr, pi->tokenFile);
#else
    sprintf (syntaxError,
	     "expecting a string, not \"%s\", line %d of file \"%s\"", pi->id,
	     pi->lnctr, pi->tokenFile);
#endif
    deleteParser (pi);
    return _ERROR_;
  }
  ws = wsCreate (pi->id);
  if (isDBError ()) {
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	     "\"%s\" - error creating workspace, line %d of file \"%s\"",
	     dberror (), pi->lnctr, pi->tokenFile);
#else
    sprintf (syntaxError,
	     "\"%s\" - error creating workspace, line %d of file \"%s\"",
	     dberror (), pi->lnctr, pi->tokenFile);
#endif
    dbError = DB_SYNTAX;
    deleteParser (pi);
    return _ERROR_;
  }
  wsClose (ws);
  if (isDBError ()) {
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	     "\"%s\" - error closing workspace, line %d of file \"%s\"",
	     dberror (), pi->lnctr, pi->tokenFile);
#else
    sprintf (syntaxError,
	     "\"%s\" - error closing workspace, line %d of file \"%s\"",
	     dberror (), pi->lnctr, pi->tokenFile);
#endif
    dbError = DB_SYNTAX;
    deleteParser (pi);
    return _ERROR_;
  }
  tkn = getToken (pi);
  if (tkn != OPERATOR || pi->id[0] != ';') {
    dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	"expected ';', not \"%s\", line %d of file \"%s\"",
	pi->id, pi->lnctr, pi->tokenFile);
#else
    sprintf (syntaxError,
	"expected ';', not \"%s\", line %d of file \"%s\"",
	pi->id, pi->lnctr, pi->tokenFile);
#endif
    deleteParser (pi);
    return _ERROR_;
  }
  return _OK_;
}

/*
 * syntax:
 * create table "tablename.db" 
 * [info "table info string"]
 * {
 *   "charfieldname" char(size);
 *   "numberfieldname" number(size:dec);
 *   "logicalfieldname" logical;
 *   "datefieldname" date;
 *   "timestampfieldname" time;
 * }[;] or
 * [indexed {
 *   idx "indexname" blksize:[no]case:[dup]unique "fieldname";
 *   midx "indexname" blksize:[no]case "fieldname1", "fieldname2", "fieldname3"...;
 * };]
 */

int defCreateTable (parseItem * pi)
{
  fieldItem **flds;
  indexItem *idx = 0;
  indexItem *midx = 0;
  char *names[MAX_FIELD + 1];
  char *info;
  char *tblname;
  CDBTokenType tkn;
  int inx;
  dbTable *tbl;

  tkn = getToken (pi);
  if (tkn != STRING) {
    dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	     "expected a string, not \"%s\", line %d of file \"%s\"", pi->id,
	     pi->lnctr, pi->tokenFile);
#else
    sprintf (syntaxError,
	     "expected a string, not \"%s\", line %d of file \"%s\"", pi->id,
	     pi->lnctr, pi->tokenFile);
#endif
    deleteParser (pi);
    return _ERROR_;
  }
  tblname = (char *) malloc (TABLE_NAME_WIDTH + 1);
  if (0 == tblname) {
    dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	"memory error parsing \"%s\", line %d",
	pi->tokenFile, pi->lnctr);
#else
    sprintf (syntaxError,
	"memory error parsing \"%s\", line %d",
	pi->tokenFile, pi->lnctr);
#endif
    deleteParser (pi);
    return _ERROR_;
  }
  memset (tblname, 0, TABLE_NAME_WIDTH + 1);
  check_pointer (tblname);
  info = (char *) malloc (TABLE_INFO_WIDTH + 1);
  if (info == 0) {
    dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	"memory error parsing \"%s\", line %d",
	pi->tokenFile, pi->lnctr);
#else
    sprintf (syntaxError, 
	"memory error parsing \"%s\", line %d",
	pi->tokenFile, pi->lnctr);
#endif
    deleteParser (pi);
    free (tblname);
    return _ERROR_;
  }
  memset (info, 0, TABLE_INFO_WIDTH + 1);
  check_pointer (info);
  flds = (fieldItem **) malloc ((MAX_FIELD + 1) * sizeof (fieldItem));
  if (flds == 0) {
    dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	"memory error parsing \"%s\", line %d",
	pi->tokenFile, pi->lnctr);
#else
    sprintf (syntaxError,
	"memory error parsing \"%s\", line %d",
	pi->tokenFile, pi->lnctr);
#endif
    deleteParser (pi);
    free (tblname);
    free (info);
    return _ERROR_;
  }
  memset (flds, 0, (MAX_FIELD + 1) * sizeof (fieldItem));
  check_pointer (flds);
  strncpy (tblname, pi->id, TABLE_NAME_WIDTH);

  tkn = getToken (pi);
  if (tkn != OPERATOR) {
    if (tkn == IDENT && !strcmp (pi->id, "info")) {
      /* get the info string */
      tkn = getToken (pi);
      if (tkn != STRING) {
	dbError = DB_SYNTAX;
#ifdef HAVE_SPRINTF
	snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	    "expected a string, not \"%s\", line %d of file \"%s\"",
	    pi->id, pi->lnctr, pi->tokenFile);
#else
	sprintf (syntaxError,
	    "expected a string, not \"%s\", line %d of file \"%s\"",
	    pi->id, pi->lnctr, pi->tokenFile);
#endif
	deleteParser (pi);
	free (tblname);
	free (info);
	free (flds);
	return _ERROR_;
      }
      strncpy (info, pi->id, TABLE_INFO_WIDTH);
      tkn = getToken (pi);
    }
    else {
      dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
      snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	  "expected \"info\", not \"%s\", line %d of file \"%s\"",
	  pi->id, pi->lnctr, pi->tokenFile);
#else
      sprintf (syntaxError,
	  "expected \"info\", not \"%s\", line %d of file \"%s\"",
	  pi->id, pi->lnctr, pi->tokenFile);
#endif
      deleteParser (pi);
      free (tblname);
      free (info);
      free (flds);
      return _ERROR_;
    }
  }
  if (tkn != OPERATOR && pi->id[0] != '{') {
    dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	"expected '{', not \"%s\", line %d of file \"%s\"",
	pi->id, pi->lnctr, pi->tokenFile);
#else
    sprintf (syntaxError,
	"expected '{', not \"%s\", line %d of file \"%s\"",
	pi->id, pi->lnctr, pi->tokenFile);
#endif
    deleteParser (pi);
    free (tblname);
    free (info);
    free (flds);
    return _ERROR_;
  }
  tkn = getToken (pi);
  for (inx = 0; tkn == STRING; inx++) {
    if (tkn != STRING) {
      dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
      snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	  "expected field name, not \"%s\", line %d, file \"%s\"",
	  pi->id, pi->lnctr, pi->tokenFile);
#else
      sprintf (syntaxError,
	  "expected field name, not \"%s\", line %d, file \"%s\"",
	  pi->id, pi->lnctr, pi->tokenFile);
#endif
      deleteParser (pi);
      free (tblname);
      free (info);
      free (flds);
      return _ERROR_;
    }
    flds[inx] = (fieldItem *) malloc (sizeof (fieldItem));
    if (flds[inx] == 0) {
      dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
      snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	  "memory error while parsing, line %d, file \"%s\"",
	  pi->lnctr, pi->tokenFile);
#else
      sprintf (syntaxError,
	  "memory error while parsing, line %d, file \"%s\"",
	  pi->lnctr, pi->tokenFile);
#endif
      deleteParser (pi);
      free (tblname);
      free (info);
      free (flds);
      return _ERROR_;
    }
    memset (flds[inx], 0, sizeof (fieldItem));
    check_pointer (flds[inx]);
    flds[inx + 1] = 0;
    strncpy (flds[inx]->name, pi->id, MAX_FIELD_WIDTH);
    tkn = getToken (pi);
    if (tkn != IDENT) {
      dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
      snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	  "expected field type, not \"%s\", line %d, file \"%s\"",
	  pi->id, pi->lnctr, pi->tokenFile);
#else
      sprintf (syntaxError,
	  "expected field type, not \"%s\", line %d, file \"%s\"",
	  pi->id, pi->lnctr, pi->tokenFile);
#endif
      deleteParser (pi);
      free (tblname);
      free (info);
      free (flds);
      return _ERROR_;
    }

    if (!strcmp (pi->id, "char")) {
      /* it's a char field */
      flds[inx]->ftype = FTYPE_CHAR;
      tkn = getToken (pi);
      if (tkn != OPERATOR && pi->id[0] != '(') {
	dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	    "expected '(', not \"%s\", line %d, file \"%s\"",
	    pi->id, pi->lnctr, pi->tokenFile);
#else
	sprintf (syntaxError,
	    "expected '(', not \"%s\", line %d, file \"%s\"",
	    pi->id, pi->lnctr, pi->tokenFile);
#endif
	deleteParser (pi);
	free (tblname);
	free (info);
	free (flds);
	return _ERROR_;
      }
      tkn = getToken (pi);
      if (tkn != NUMBER) {
	dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		 "expected a number, not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#else
	sprintf (syntaxError,
		 "expected a number, not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#endif
	deleteParser (pi);
	free (tblname);
	free (info);
	free (flds);
	return _ERROR_;
      }
      flds[inx]->flen = atoi (pi->id);
      flds[inx]->declen = 0;
      tkn = getToken (pi);
      if (tkn != OPERATOR && pi->id[0] != ')') {
	dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		 "expected ')', not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#else
	sprintf (syntaxError,
		 "expected ')', not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#endif
	deleteParser (pi);
	free (tblname);
	free (info);
	free (flds);
	return _ERROR_;
      }
      tkn = getToken (pi);
      if (tkn != OPERATOR || pi->id[0] != ';') {
	dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		 "expected ';', not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#else
	sprintf (syntaxError,
		 "expected ';', not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#endif
	deleteParser (pi);
	free (tblname);
	free (info);
	free (flds);
	return _ERROR_;
      }
    }
    else if (!strcmp (pi->id, "number")) {
      /* number field */
      flds[inx]->ftype = FTYPE_NUMBER;
      tkn = getToken (pi);
      if (tkn != OPERATOR && pi->id[0] != '(') {
	dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		 "expected '(', not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#else
	sprintf (syntaxError,
		 "expected '(', not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#endif
	deleteParser (pi);
	free (tblname);
	free (info);
	free (flds);
	return _ERROR_;
      }
      tkn = getToken (pi);
      if (tkn != NUMBER) {
	dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		 "expected a number, not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#else
	sprintf (syntaxError,
		 "expected a number, not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#endif
	deleteParser (pi);
	free (tblname);
	free (info);
	free (flds);
	return _ERROR_;
      }
      flds[inx]->flen = atoi (pi->id);
      tkn = getToken (pi);
      if (tkn != OPERATOR && pi->id[0] != ':') {
	dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		 "expected ':', not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#else
	sprintf (syntaxError,
		 "expected ':', not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#endif
	deleteParser (pi);
	free (tblname);
	free (info);
	free (flds);
	return _ERROR_;
      }
      tkn = getToken (pi);
      if (tkn != NUMBER) {
	dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		 "expected a number, not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#else
	sprintf (syntaxError,
		 "expected a number, not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#endif
	deleteParser (pi);
	free (tblname);
	free (info);
	free (flds);
	return _ERROR_;
      }
      flds[inx]->declen = atoi (pi->id);
      tkn = getToken (pi);
      if (tkn != OPERATOR && pi->id[0] != ')') {
	dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		 "expected ')', not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#else
	sprintf (syntaxError,
		 "expected ')', not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#endif
	deleteParser (pi);
	free (tblname);
	free (info);
	free (flds);
	return _ERROR_;
      }
      tkn = getToken (pi);
      if (!(tkn == OPERATOR && pi->id[0] == ';')) {
	dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		 "expected ';', not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#else
	sprintf (syntaxError,
		 "expected ';', not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#endif
	deleteParser (pi);
	free (tblname);
	free (info);
	free (flds);
	return _ERROR_;
      }
    }
    else if (!strcmp (pi->id, "logical")) {
      /* logical field */
      flds[inx]->ftype = FTYPE_LOGICAL;
      flds[inx]->flen = 1;
      flds[inx]->declen = 0;
      tkn = getToken (pi);
      if (!(tkn == OPERATOR && pi->id[0] == ';')) {
	dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		 "expected ';', not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#else
	sprintf (syntaxError,
		 "expected ';', not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#endif
	deleteParser (pi);
	free (tblname);
	free (info);
	free (flds);
	return _ERROR_;
      }
    }
    else if (!strcmp (pi->id, "date")) {
      /* date field */
      flds[inx]->ftype = FTYPE_DATE;
      flds[inx]->flen = 8;
      flds[inx]->declen = 0;
      tkn = getToken (pi);
      if (tkn != OPERATOR || pi->id[0] != ';') {
	dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		 "expected ';', not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#else
	sprintf (syntaxError,
		 "expected ';', not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#endif
	deleteParser (pi);
	free (tblname);
	free (info);
	free (flds);
	return _ERROR_;
      }
    }
    else if (!strcmp (pi->id, "time")) {
      /* time field */
      flds[inx]->ftype = FTYPE_TIME;
      flds[inx]->flen = 26;
      flds[inx]->declen = 0;
      tkn = getToken (pi);
      if (tkn != OPERATOR || pi->id[0] != ';') {
	dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		 "expected ';', not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#else
	sprintf (syntaxError,
		 "expected ';', not \"%s\", line %d, file \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
#endif
	deleteParser (pi);
	free (tblname);
	free (info);
	free (flds);
	return _ERROR_;
      }
    }
    else {
      dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
      snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	       "expected a valid field type, not \"%s\", line %d, file \"%s\"",
	       pi->id, pi->lnctr, pi->tokenFile);
#else
      sprintf (syntaxError,
	       "expected a valid field type, not \"%s\", line %d, file \"%s\"",
	       pi->id, pi->lnctr, pi->tokenFile);
#endif
      deleteParser (pi);
      free (tblname);
      free (info);
      free (flds);
      return _ERROR_;
    }
    tkn = getToken (pi);
  }
  /* now, get index information, if there is any. */
  if (tkn != OPERATOR && pi->id[0] != '}') {
    dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	"expected '}', not \"%s\", line %d, file %s",
	pi->id, pi->lnctr, pi->tokenFile);
#else
    sprintf (syntaxError,
	"expected '}', not \"%s\", line %d, file %s",
	pi->id, pi->lnctr, pi->tokenFile);
#endif
    deleteParser (pi);
    for (inx = 0; flds[inx] != 0; inx++) {
      free (flds[inx]);
    }
    free (flds);
    free (tblname);
    free (info);
    return _ERROR_;
  }
  if (info[0] == '\0')
    tbl = dbBuildTable (tblname, 0, flds);
  else
    tbl = dbBuildTable (tblname, info, flds);
  if (isDBError ()) {
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	     "error creating table while parsing %s, line %d: %s\n",
	     pi->tokenFile, pi->lnctr, dberror ());
#else
    sprintf (syntaxError,
	     "error creating table while parsing %s, line %d: %s\n",
	     pi->tokenFile, pi->lnctr, dberror ());
#endif
    dbError = DB_SYNTAX;
    deleteParser (pi);
    for (inx = 0; flds[inx] != 0; inx++)
      free (flds[inx]);
    free (flds);
    free (tblname);
    free (info);
    return _ERROR_;
  }
  tkn = getToken (pi);
  if (tkn == OPERATOR && pi->id[0] == ';') {
    closeTable (tbl);
    for (inx = 0; flds[inx] != 0; inx++)
      free (flds[inx]);
    free (flds);
    free (tblname);
    free (info);
    return _OK_;
  }
  else if (tkn == IDENT && !strcmp (pi->id, "indexed")) {
    /* Get index information and then create the table and index(es). */
    tkn = getToken (pi);
    if (tkn != OPERATOR || pi->id[0] != '{') {
      dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
      snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	  "expected '{', not \"%s\", line %d, file %s",
	  pi->id, pi->lnctr, pi->tokenFile);
#else
      sprintf (syntaxError,
	  "expected '{', not \"%s\", line %d, file %s",
	  pi->id, pi->lnctr, pi->tokenFile);
#endif
      deleteParser (pi);
      for (inx = 0; flds[inx] != 0; inx++) {
	free (flds[inx]);
      }
      free (flds);
      free (tblname);
      free (info);
      return _ERROR_;
    }
    tkn = getToken (pi);
    if (tkn != IDENT) {
      dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
      snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	       "expected an identifier, not \"%s\", line %d, file %s", pi->id,
	       pi->lnctr, pi->tokenFile);
#else
      sprintf (syntaxError,
	       "expected an identifier, not \"%s\", line %d, file %s", pi->id,
	       pi->lnctr, pi->tokenFile);
#endif
      deleteParser (pi);
      for (inx = 0; flds[inx] != 0; inx++) {
	free (flds[inx]);
      }
      free (flds);
      free (tblname);
      free (info);
      return _ERROR_;
    }
    while (tkn == IDENT) {
      if (!strcmp (pi->id, "idx")) {
	/* get idx info */
	tkn = getToken (pi);
	if (tkn != STRING) {
	  dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	  snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		   "expected index name, not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#else
	  sprintf (syntaxError,
		   "expected index name, not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#endif
	  deleteParser (pi);
	  for (inx = 0; flds[inx] != 0; inx++) {
	    free (flds[inx]);
	  }
	  free (flds);
	  free (tblname);
	  free (info);
	  return _ERROR_;
	}
	idx = (indexItem *) malloc (sizeof (indexItem));
	if (idx == 0) {
	  dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	  snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		   "memory error parsing definition file, line %d, file %s",
		   pi->lnctr, pi->tokenFile);
#else
	  sprintf (syntaxError,
		   "memory error parsing definition file, line %d, file %s",
		   pi->lnctr, pi->tokenFile);
#endif
	  deleteParser (pi);
	  for (inx = 0; flds[inx] != 0; inx++) {
	    free (flds[inx]);
	  }
	  free (flds);
	  free (tblname);
	  free (info);
	  return _ERROR_;
	}
	memset (idx, 0, sizeof (indexItem));
	check_pointer (idx);
	strncpy (idx->idxname, pi->id, TABLE_NAME_WIDTH);
	tkn = getToken (pi);
	/* looking for blksize:[no]case:[dup]unique */
	if (tkn != NUMBER) {
	  dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	  snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		   "expected block size, not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#else
	  sprintf (syntaxError,
		   "expected block size, not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#endif
	  deleteParser (pi);
	  for (inx = 0; flds[inx] != 0; inx++) {
	    free (flds[inx]);
	  }
	  free (idx);
	  free (flds);
	  free (tblname);
	  free (info);
	  return _ERROR_;
	}
	idx->blksize = atoi (pi->id);
	tkn = getToken (pi);
	if (tkn != OPERATOR && pi->id[0] != ':') {
	  dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	  snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		   "expected ':', not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#else
	  sprintf (syntaxError,
		   "expected ':', not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#endif
	  deleteParser (pi);
	  for (inx = 0; flds[inx] != 0; inx++) {
	    free (flds[inx]);
	  }
	  free (idx);
	  free (flds);
	  free (tblname);
	  free (info);
	  return _ERROR_;
	}
	tkn = getToken (pi);
	if (tkn != IDENT) {
	  dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	  snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		   "expected identifier (case/nocase), not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#else
	  sprintf (syntaxError,
		   "expected identifier (case/nocase), not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#endif
	  deleteParser (pi);
	  for (inx = 0; flds[inx] != 0; inx++) {
	    free (flds[inx]);
	  }
	  free (idx);
	  free (flds);
	  free (tblname);
	  free (info);
	  return _ERROR_;
	}
	if (!strcmp (pi->id, "case")) {
	  idx->isCase = TRUE;
	}
	else if (!strcmp (pi->id, "nocase")) {
	  idx->isCase = FALSE;
	}
	else {
	  dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	  snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		   "expected identifier (case/nocase), not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#else
	  sprintf (syntaxError,
		   "expected identifier (case/nocase), not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#endif
	  deleteParser (pi);
	  for (inx = 0; flds[inx] != 0; inx++) {
	    free (flds[inx]);
	  }
	  free (idx);
	  free (flds);
	  free (tblname);
	  free (info);
	  return _ERROR_;
	}
	tkn = getToken (pi);
	if (tkn != OPERATOR && pi->id[0] != ':') {
	  dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	  snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		   "expected ':', not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#else
	  sprintf (syntaxError,
		   "expected ':', not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#endif
	  deleteParser (pi);
	  for (inx = 0; flds[inx] != 0; inx++) {
	    free (flds[inx]);
	  }
	  free (idx);
	  free (flds);
	  free (tblname);
	  free (info);
	  return _ERROR_;
	}
	tkn = getToken (pi);
	if (tkn != IDENT) {
	  dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	  snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		   "expected identifier (dup/unique), not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#else
	  sprintf (syntaxError,
		   "expected identifier (dup/unique), not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#endif
	  deleteParser (pi);
	  for (inx = 0; flds[inx] != 0; inx++) {
	    free (flds[inx]);
	  }
	  free (idx);
	  free (flds);
	  free (tblname);
	  free (info);
	  return _ERROR_;
	}
	if (!strcmp (pi->id, "unique")) {
	  idx->isUnique = TRUE;
	}
	else if (!strcmp (pi->id, "dup")) {
	  idx->isUnique = FALSE;
	}
	else {
	  dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	  snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		   "expected identifier (dup/unique), not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#else
	  sprintf (syntaxError,
		   "expected identifier (dup/unique), not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#endif
	  deleteParser (pi);
	  for (inx = 0; flds[inx] != 0; inx++) {
	    free (flds[inx]);
	  }
	  free (idx);
	  free (flds);
	  free (tblname);
	  free (info);
	  return _ERROR_;
	}
	tkn = getToken (pi);
	if (tkn != STRING) {
	  dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	  snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		   "expected field name, not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#else
	  sprintf (syntaxError,
		   "expected field name, not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#endif
	  deleteParser (pi);
	  for (inx = 0; flds[inx] != 0; inx++) {
	    free (flds[inx]);
	  }
	  free (idx);
	  free (flds);
	  free (tblname);
	  free (info);
	  return _ERROR_;
	}
	strncpy (idx->fldname, pi->id, MAX_FIELD_WIDTH);
	tkn = getToken (pi);
	if (tkn != OPERATOR || pi->id[0] != ';') {
	  dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	  snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		   "expected ';', not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#else
	  sprintf (syntaxError,
		   "expected ';', not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#endif
	  deleteParser (pi);
	  for (inx = 0; flds[inx] != 0; inx++) {
	    free (flds[inx]);
	  }
	  free (idx);
	  free (flds);
	  free (tblname);
	  free (info);
	  return _ERROR_;
	}
	tkn = getToken (pi);
	/*
	 * Now, create the index.
	 */
	createDBIndex (tbl, idx->idxname, idx->fldname,
		       idx->isCase, idx->isUnique, idx->blksize);
	if (isTableError (tbl)) {
#ifdef HAVE_SNPRINTF
	  snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		   "error creating index while parsing %s, line %d: %s\n",
		   pi->tokenFile, pi->lnctr, dbtblerror (tbl));
#else
	  sprintf (syntaxError,
		   "error creating index while parsing %s, line %d: %s\n",
		   pi->tokenFile, pi->lnctr, dbtblerror (tbl));
#endif
	  dbError = DB_SYNTAX;
	  deleteParser (pi);
	  for (inx = 0; flds[inx] != 0; inx++)
	    free (flds[inx]);
	  free (flds);
	  free (tblname);
	  free (info);
	  return _ERROR_;
	}
	free (idx);
	if (tkn != IDENT)
	  goto gotBrace;	/* go forward */
      }
      else if (!strcmp (pi->id, "midx")) {
	/* get and create multi index */
	tkn = getToken (pi);
	if (tkn != STRING) {
	  dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	  snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		   "expected index name, not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#else
	  sprintf (syntaxError,
		   "expected index name, not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#endif
	  deleteParser (pi);
	  for (inx = 0; flds[inx] != 0; inx++) {
	    free (flds[inx]);
	  }
	  free (flds);
	  free (tblname);
	  free (info);
	  return _ERROR_;
	}
	midx = (indexItem *) malloc (sizeof (indexItem));
	if (midx == 0) {
	  dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	  snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		   "memory error parsing definition file, line %d, file %s",
		   pi->lnctr, pi->tokenFile);
#else
	  sprintf (syntaxError,
		   "memory error parsing definition file, line %d, file %s",
		   pi->lnctr, pi->tokenFile);
#endif
	  deleteParser (pi);
	  for (inx = 0; flds[inx] != 0; inx++) {
	    free (flds[inx]);
	  }
	  free (flds);
	  free (tblname);
	  free (info);
	  return _ERROR_;
	}
	memset (midx, 0, sizeof (indexItem));
	check_pointer (midx);
	strncpy (midx->idxname, pi->id, TABLE_NAME_WIDTH);
	tkn = getToken (pi);
	/* looking for blksize:[no]case */
	if (tkn != NUMBER) {
	  dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	  snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		   "expected block size, not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#else
	  sprintf (syntaxError,
		   "expected block size, not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#endif
	  deleteParser (pi);
	  for (inx = 0; flds[inx] != 0; inx++) {
	    free (flds[inx]);
	  }
	  free (midx);
	  free (flds);
	  free (tblname);
	  free (info);
	  return _ERROR_;
	}
	midx->blksize = atoi (pi->id);
	tkn = getToken (pi);
	if (tkn != OPERATOR && pi->id[0] != ':') {
	  dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	  snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		   "expected ':', not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#else
	  sprintf (syntaxError,
		   "expected ':', not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#endif
	  deleteParser (pi);
	  for (inx = 0; flds[inx] != 0; inx++) {
	    free (flds[inx]);
	  }
	  free (midx);
	  free (flds);
	  free (tblname);
	  free (info);
	  return _ERROR_;
	}
	tkn = getToken (pi);
	if (tkn != IDENT) {
	  dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	  snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		   "expected identifier (case/nocase), not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#else
	  sprintf (syntaxError,
		   "expected identifier (case/nocase), not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#endif
	  deleteParser (pi);
	  for (inx = 0; flds[inx] != 0; inx++) {
	    free (flds[inx]);
	  }
	  free (midx);
	  free (flds);
	  free (tblname);
	  free (info);
	  return _ERROR_;
	}
	if (!strcmp (pi->id, "case")) {
	  midx->isCase = TRUE;
	}
	else if (!strcmp (pi->id, "nocase")) {
	  midx->isCase = FALSE;
	}
	else {
	  dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	  snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		   "expected identifier (case/nocase), not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#else
	  sprintf (syntaxError,
		   "expected identifier (case/nocase), not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#endif
	  deleteParser (pi);
	  for (inx = 0; flds[inx] != 0; inx++) {
	    free (flds[inx]);
	  }
	  free (midx);
	  free (flds);
	  free (tblname);
	  free (info);
	  return _ERROR_;
	}
	/* field names */
	inx = 0;
	do {
	  tkn = getToken (pi);
	  if (tkn != STRING) {
	    dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		     "expected field name, not \"%s\", line %d, file %s",
		     pi->id, pi->lnctr, pi->tokenFile);
#else
	    sprintf (syntaxError,
		     "expected field name, not \"%s\", line %d, file %s",
		     pi->id, pi->lnctr, pi->tokenFile);
#endif
	    deleteParser (pi);
	    for (inx = 0; flds[inx] != 0; inx++) {
	      free (flds[inx]);
	    }
	    free (midx);
	    free (flds);
	    free (tblname);
	    free (info);
	    return _ERROR_;
	  }
	  names[inx] = (char *) malloc (MAX_FIELD_WIDTH + 1);
	  if (0 == names[inx]) {
	    dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		     "memory error while parsing def file, line %d, file %s",
		     pi->lnctr, pi->tokenFile);
#else
	    sprintf (syntaxError,
		     "memory error while parsing def file, line %d, file %s",
		     pi->lnctr, pi->tokenFile);
#endif
	    deleteParser (pi);
	    for (inx = 0; flds[inx] != 0; inx++) {
	      free (flds[inx]);
	    }
	    free (midx);
	    free (flds);
	    free (tblname);
	    free (info);
	    return _ERROR_;
	  }
	  memset (names[inx], 0, MAX_FIELD_WIDTH + 1);
	  strncpy (names[inx], pi->id, MAX_FIELD_WIDTH);
	  check_pointer (names[inx]);
	  names[inx + 1] = 0;
	  inx++;
	  tkn = getToken (pi);
	  if (tkn == OPERATOR && pi->id[0] == ';')
	    break;
	  if (tkn != OPERATOR || pi->id[0] != ',') {
	    dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		     "expected ',', not \"%s\", line %d, file %s",
		     pi->id, pi->lnctr, pi->tokenFile);
#else
	    sprintf (syntaxError,
		     "expected ',', not \"%s\", line %d, file %s",
		     pi->id, pi->lnctr, pi->tokenFile);
#endif
	    deleteParser (pi);
	    for (inx = 0; flds[inx] != 0; inx++) {
	      free (flds[inx]);
	    }
	    free (midx);
	    free (flds);
	    free (tblname);
	    free (info);
	    return _ERROR_;
	  }
	} while (tkn == OPERATOR && pi->id[0] == ',');
	if (!(tkn == OPERATOR && pi->id[0] == ';')) {
	  dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	  snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		   "expected ';', not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#else
	  sprintf (syntaxError,
		   "expected ';', not \"%s\", line %d, file %s",
		   pi->id, pi->lnctr, pi->tokenFile);
#endif
	  deleteParser (pi);
	  for (inx = 0; flds[inx] != 0; inx++) {
	    free (flds[inx]);
	  }
	  free (midx);
	  free (flds);
	  free (tblname);
	  free (info);
	  return _ERROR_;
	}
	/*
	 * Now, create the index.
	 */
	createMultiIndex (tbl, midx->idxname, names,
			  midx->isCase, midx->blksize);
	if (isTableError (tbl)) {
#ifdef HAVE_SNPRINTF
	  snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		   "error creating index while parsing %s, line %d: %s\n",
		   pi->tokenFile, pi->lnctr, dbtblerror (tbl));
#else
	  sprintf (syntaxError,
		   "error creating index while parsing %s, line %d: %s\n",
		   pi->tokenFile, pi->lnctr, dbtblerror (tbl));
#endif
	  dbError = DB_SYNTAX;
	  deleteParser (pi);
	  for (inx = 0; flds[inx] != 0; inx++)
	    free (flds[inx]);
	  free (flds);
	  free (tblname);
	  free (info);
	  return _ERROR_;
	}
	free (midx);
	tkn = getToken (pi);
	if (tkn != IDENT) {
	  for (inx = 0; names[inx] != 0; inx++)
	    free (names[inx]);
	  goto gotBrace;
	}
      }
      else {
	/* error */
	dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		 "expected identifier (idx/midx), not \"%s\", line %d, file %s",
		 pi->id, pi->lnctr, pi->tokenFile);
#else
	sprintf (syntaxError,
		 "expected identifier (idx/midx), not \"%s\", line %d, file %s",
		 pi->id, pi->lnctr, pi->tokenFile);
#endif
	deleteParser (pi);
	for (inx = 0; flds[inx] != 0; inx++) {
	  free (flds[inx]);
	}
	free (midx);
	free (flds);
	free (tblname);
	free (info);
	return _ERROR_;
      }
    }				/* end of while */
    for (inx = 0; names[inx] != 0; inx++)
      free (names[inx]);
    /* looking for '}' */
    tkn = getToken (pi);
  gotBrace:
    if (tkn != OPERATOR || pi->id[0] != '}') {
      dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
      snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	       "expected '}', not \"%s\", line %d, file %s",
	       pi->id, pi->lnctr, pi->tokenFile);
#else
      sprintf (syntaxError,
	       "expected '}', not \"%s\", line %d, file %s",
	       pi->id, pi->lnctr, pi->tokenFile);
#endif
      deleteParser (pi);
      for (inx = 0; flds[inx] != 0; inx++) {
	free (flds[inx]);
      }
      free (flds);
      free (tblname);
      free (info);
      return _ERROR_;
    }
    /* looking for ';' */
    tkn = getToken (pi);
    if (tkn != OPERATOR || pi->id[0] != ';') {
      dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
      snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	       "expected '}', not \"%s\", line %d, file %s",
	       pi->id, pi->lnctr, pi->tokenFile);
#else
      sprintf (syntaxError,
	       "expected '}', not \"%s\", line %d, file %s",
	       pi->id, pi->lnctr, pi->tokenFile);
#endif
      deleteParser (pi);
      for (inx = 0; flds[inx] != 0; inx++)
	free (flds[inx]);
      free (flds);
      free (tblname);
      free (info);
      return _ERROR_;
    }
  }				/* end of indexed */
  else {
    dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	     "expected ';' or 'indexed', not \"%s\", line %d, file %s",
	     pi->tokenFile, pi->lnctr, pi->tokenFile);
#else
    sprintf (syntaxError,
	     "expected ';' or 'indexed', not \"%s\", line %d, file %s",
	     pi->tokenFile, pi->lnctr, pi->tokenFile);
#endif
    deleteParser (pi);
    for (inx = 0; flds[inx] != 0; inx++) {
      free (flds[inx]);
    }
    free (flds);
    free (tblname);
    free (info);
    return _ERROR_;
  }
  /* clean up */
  closeTable (tbl);
  for (inx = 0; flds[inx] != 0; inx++) {
    free (flds[inx]);
  }
  free (flds);
  free (tblname);
  free (info);
  return _OK_;
}

/*
 * syntax:
 * add "tablename" to "workspacename" [index "indexname"] ;
 */
int defAdd (parseItem * pi)
{
  CDBTokenType tkn;
  char tblname[TABLE_NAME_WIDTH + 1];
  char wsname[TABLE_NAME_WIDTH + 1];
  char idxname[TABLE_NAME_WIDTH + 1];
  int idx = FALSE;
  dbTable *tbl;
  workSpace *ws;

  tkn = getToken (pi);
  if (tkn != STRING) {
    dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	     "expected a table name, not \"%s\", line %d, file %s",
	     pi->tokenFile, pi->lnctr, pi->tokenFile);
#else
    sprintf (syntaxError,
	     "expected a table name, not \"%s\", line %d, file %s",
	     pi->tokenFile, pi->lnctr, pi->tokenFile);
#endif
    deleteParser (pi);
    return _ERROR_;
  }
  memset (tblname, 0, TABLE_NAME_WIDTH + 1);
  strncpy (tblname, pi->id, TABLE_NAME_WIDTH);
  tkn = getToken (pi);
  if (tkn != IDENT || strcmp (pi->id, "to")) {
    dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	"expected 'to', not \"%s\", line %d, file %s",
	pi->id, pi->lnctr, pi->tokenFile);
#else
    sprintf (syntaxError,
	"expected 'to', not \"%s\", line %d, file %s",
	pi->id, pi->lnctr, pi->tokenFile);
#endif
    deleteParser (pi);
    return _ERROR_;
  }
  tkn = getToken (pi);
  if (tkn != STRING) {
    dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	     "expected a workspace name, not \"%s\", line %d, file %s",
	     pi->id, pi->lnctr, pi->tokenFile);
#else
    sprintf (syntaxError,
	     "expected a workspace name, not \"%s\", line %d, file %s",
	     pi->id, pi->lnctr, pi->tokenFile);
#endif
    deleteParser (pi);
    return _ERROR_;
  }
  memset (wsname, 0, TABLE_NAME_WIDTH + 1);
  strncpy (wsname, pi->id, TABLE_NAME_WIDTH);
  tkn = getToken (pi);
  if (tkn == IDENT) {
    if (!strcmp (pi->id, "index")) {
      tkn = getToken (pi);
      if (tkn != STRING) {
	dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
	snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
		 "expected index name, not \"%s\", line %d, file %s",
		 pi->id, pi->lnctr, pi->tokenFile);
#else
	sprintf (syntaxError,
		 "expected index name, not \"%s\", line %d, file %s",
		 pi->id, pi->lnctr, pi->tokenFile);
#endif
	deleteParser (pi);
	return _ERROR_;
      }
      memset (idxname, 0, TABLE_NAME_WIDTH + 1);
      strncpy (idxname, pi->id, TABLE_NAME_WIDTH);
      idx = TRUE;
    }
    else {
      dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
      snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	       "expected 'index' keyword, not \"%s\", line %d, file %s",
	       pi->id, pi->lnctr, pi->tokenFile);
#else
      sprintf (syntaxError,
	       "expected 'index' keyword, not \"%s\", line %d, file %s",
	       pi->id, pi->lnctr, pi->tokenFile);
#endif
      deleteParser (pi);
      return _ERROR_;
    }
    tkn = getToken (pi);
  }
  if (tkn != OPERATOR || pi->id[0] != ';') {
    dbError = DB_SYNTAX;
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	"expected ';', not \"%s\", line %d, file %s",
	pi->id, pi->lnctr, pi->tokenFile);
#else
    sprintf (syntaxError,
	"expected ';', not \"%s\", line %d, file %s",
	pi->id, pi->lnctr, pi->tokenFile);
#endif
    deleteParser (pi);
    return _ERROR_;
  }
  /* do it */
  ws = wsOpen (wsname);
  if (isDBError ()) {
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	"opening workspace, line %d, file %s: %s",
	pi->lnctr, pi->tokenFile, dberror ());
#else
    sprintf (syntaxError,
	"opening workspace, line %d, file %s: %s",
	pi->lnctr, pi->tokenFile, dberror ());
#endif
    deleteParser (pi);
    dbError = DB_SYNTAX;
    return _ERROR_;
  }
  tbl = openTable (tblname);
  if (isDBError ()) {
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	"opening table, line %d, file %s: %s",
	pi->lnctr, pi->tokenFile, dberror ());
#else
    sprintf (syntaxError,
	"opening table, line %d, file %s: %s",
	pi->lnctr, pi->tokenFile, dberror ());
#endif
    deleteParser (pi);
    dbError = DB_SYNTAX;
    return _ERROR_;
  }
  if (idx == TRUE) {
    setCurrentIndex (tbl, idxname);
    if (isTableError (tbl)) {
#ifdef HAVE_SNPRINTF
      snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	       "setting current index to %s in %s, line %d, file %s: %s",
	       idxname, tblname, pi->lnctr, pi->tokenFile, dbtblerror (tbl));
#else
      sprintf (syntaxError,
	       "setting current index to %s in %s, line %d, file %s: %s",
	       idxname, tblname, pi->lnctr, pi->tokenFile, dbtblerror (tbl));
#endif
      dbError = DB_SYNTAX;
      deleteParser (pi);
      return _ERROR_;
    }
  }
  wsAddTable (ws, tbl, 0);
  if (isDBError ()) {
#ifdef HAVE_SNPRINTF
    snprintf (syntaxError, SYNTAX_ERROR_WIDTH,
	"adding table to workspace, line %d, file %s: %s",
	pi->lnctr, pi->tokenFile, dberror ());
#else
    sprintf (syntaxError,
	"adding table to workspace, line %d, file %s: %s",
	pi->lnctr, pi->tokenFile, dberror ());
#endif
    deleteParser (pi);
    dbError = DB_SYNTAX;
    return _ERROR_;
  }
  wsClose (ws);
  return _OK_;
}

dbTable *dbBuildTable (char *tblname, char *info, fieldItem ** flds)
{
  int i;
  fieldType ftype[MAX_FIELD];
  char *names[MAX_FIELD];
  int flens[MAX_FIELD];
  int declens[MAX_FIELD];
  dbTable *tbl;

  for (i = 0; i < MAX_FIELD && flds[i] != 0; i++) {
    names[i] = (char *) malloc (MAX_FIELD_WIDTH + 1);
    if (0 == names[i]) {
      for (i = 0; i < MAX_FIELD && names[i] != 0; i++)
	free (names[i]);
      dbError = DB_NOMEMORY;
      return 0;
    }
    memset (names[i], 0, MAX_FIELD_WIDTH + 1);
    strncpy (names[i], flds[i]->name, MAX_FIELD_WIDTH);
    names[i + 1] = 0;
    ftype[i] = (fieldType) flds[i]->ftype;
    ftype[i + 1] = (fieldType) 0;
    flens[i] = flds[i]->flen;
    flens[i + 1] = 0;
    declens[i] = flds[i]->declen;
    declens[i + 1] = 0;
  }
  tbl = buildTable (tblname, info, ftype, names, flens, declens);
  if (isDBError ()) {
    for (i = 0; i < MAX_FIELD && names[i] != 0; i++)
      free (names[i]);
    return 0;
  }
  for (i = 0; i < MAX_FIELD && names[i] != 0; i++)
    free (names[i]);
  return tbl;
}
