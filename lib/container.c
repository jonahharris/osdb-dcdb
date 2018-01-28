/*  Source File: container.c */

#include <container.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifdef HAVE_SNPRINTF
#define SPRINTF(str,sz,rest...) \
  snprintf (str,sz,rest)
#else
#define SPRINTF(str,sz,rest...) \
  sprintf (str,rest)
#endif

/*
 * [BeginDoc]
 *
 * \subsubsection{Container Error Handling}
 *
 * If an error occurs in a container function, an exception is raised in the scripting
 * language and the error is returned.
 * \index{container error}
 *
 * There are occasions when an error is not critical (like a unique constraint
 * violation).  In that case, you should clear the error condition with
 * ``containerClearError'' before proceeding to call container functions.
 * \index{container error!clearing}
 *
 * [EndDoc]
 */
contError containerError = CONT_NOERROR;
char *containerErrorString[] = {
  "no error",
  "memory exhausted",
  "bad parameter to container function",
  "invalid container handle",
  "no index in container",
  "parsing container definition file",
  "syntax error in container definition file",
  "memory values corrupt",
  "shell error",
  "unique constraint violated",
  /* Add new ones here. */
  "unspecified error"
};

// BUGBUG - I need to use mutexes to make this thread safe.
int num_container_handles = 0;
contHandle *container_handles[MAX_CONTAINER_HANDLES] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

char container_error[CONTAINER_RESULT_SIZE + 1];
char container_syntax_error[CONTAINER_RESULT_SIZE + 1];

char rslt[CONTAINER_RESULT_SIZE + 1];

#define cont_comp(p1,p2) ((p1)<(p2)?-1:((p1)==(p2)?0:-1))

/*
 * Internal function.
 * Provides comparison functionality for containers.
 */
static int container_compare (void *p1, void *p2)
{
  /*
   * Each data item has to have 2 ints as the first items:
   * 1) a ctype value which shows what type it is, and
   * 2) an offset into the data pointer where the comparison value is
   */

#define	type1 ((int)*((char*)p1))
#define	offset1 ((int)*(char*)(p1+sizeof(int)))
#define	offset2 ((int)*(char*)(p2+sizeof(int)))

  Assert (p1 != 0);
  Assert (p2 != 0);
  switch (type1) {
  case CTYPE_STRING:
    return (strcmp ((char *) p1 + offset1, (char *) p2 + offset2));
    break;
  case CTYPE_INT:
    return (cont_comp ((*(int *) p1 + offset1), (*(int *) p2 + offset2)));
    break;
  case CTYPE_LONG:
    return (cont_comp ((*(long *) p1 + offset1), (*(long *) p2 + offset2)));
    break;
  case CTYPE_FLOAT:
    return (cont_comp ((*(float *) p1 + offset1), (*(float *) p2 + offset2)));
    break;
  case CTYPE_DOUBLE:
    return (cont_comp
	    ((*(double *) p1 + offset1), (*(double *) p2 + offset2)));
    break;
  case CTYPE_UNDEFINED:
    break;
  }
  return 0;
}

/*
 * Internal functions.
 * Process container handle requests.
 */
static contHandle *findContainerHandle (char *handle)
{
  int i;

  if (handle == 0 || *handle == '\0') {
    containerError = CONT_PARAM;
    return 0;
  }
  if (num_container_handles == 0 || handle[0] == 0) {
    containerError = CONT_INVALID;
    return 0;
  }
  for (i = 0; i < num_container_handles; i++) {
    if (!strcmp (container_handles[i]->name, handle))
      return container_handles[i];
  }
  return 0;
}

static int findContainerHandleNumber (char *handle)
{
  int i;

  if (handle == 0 || *handle == '\0') {
    containerError = CONT_PARAM;
    return _ERROR_;
  }
  if (num_container_handles == 0 || handle[0] == 0) {
    containerError = CONT_INVALID;
    return _ERROR_;
  }
  for (i = 0; i < num_container_handles; i++) {
    if (!strcmp (container_handles[i]->name, handle))
      return i;
  }
  return _ERROR_;
}

/*
 * Internal function.
 * Parses the container definition file.
 */
contHandle *parseContainerDefinitionFile (char *fname)
{
  contHandle *hdl = 0;
  contField *fld = 0;
  contField **flds = 0;
  parseItem *pi = 0;
  CDBTokenType tkn;
  int fldnum = 0;
  int i;
  int offset = 2 * sizeof (int);
  int wordsize = sizeof (long);

  if (fname == 0 || *fname == '\0') {
    containerError = CONT_PARAM;
    return 0;
  }
  flds = malloc (CONTAINER_MAX_NUM_FIELDS * sizeof (contHandle *));
  if (flds == 0) {
    containerError = CONT_MEMORY;
    return 0;
  }
  memset (flds, 0, CONTAINER_MAX_NUM_FIELDS * sizeof (contHandle *));
  check_pointer (flds);

  /* initialize the token parser. */
  pi = initParser (fname);
  if (0 == pi) {
    containerError = CONT_PARSE;
    free (flds);
    return 0;
  }
  /* expect identifier: container */
  tkn = getToken (pi);
  if (tkn != IDENT || strcmp (pi->id, "container")) {
    containerError = CONT_SYNTAX;
    SPRINTF (container_syntax_error, CONTAINER_RESULT_SIZE,
	     "expected 'container', not \"%s\", line %d of file \"%s\"",
	     pi->id, pi->lnctr, pi->tokenFile);
    goto syntaxErrorReturn;
  }
  /* expect string: container_name */
  tkn = getToken (pi);
  if (tkn != STRING) {
    containerError = CONT_SYNTAX;
    SPRINTF (container_syntax_error, CONTAINER_RESULT_SIZE,
	     "expected a string with 'container name', not \"%s\", line %d of file \"%s\"",
	     pi->id, pi->lnctr, pi->tokenFile);
    goto syntaxErrorReturn;
  }
  /* allocate a handle and set it's name */
  hdl = malloc (sizeof (contHandle));
  if (0 == hdl) {
    containerError = CONT_MEMORY;
    goto syntaxErrorReturn;
  }
  memset (hdl, 0, sizeof (contHandle));
  check_pointer (hdl);
  strncpy (hdl->name, pi->id, CONTAINER_FIELD_NAME_LENGTH);
  hdl->name[CONTAINER_FIELD_NAME_LENGTH] = '\0';

  /* expect operator: '{' */
  tkn = getToken (pi);
  if (tkn != OPERATOR || pi->id[0] != '{') {
    containerError = CONT_SYNTAX;
    SPRINTF (container_syntax_error, CONTAINER_RESULT_SIZE,
	     "expected '{', not \"%s\", line %d of file \"%s\"",
	     pi->id, pi->lnctr, pi->tokenFile);
    goto syntaxErrorReturn;
  }
  /* expect string: fldname */
  tkn = getToken (pi);
  if (tkn != STRING) {
    containerError = CONT_SYNTAX;
    SPRINTF (container_syntax_error, CONTAINER_RESULT_SIZE,
	     "expected a string with 'field name', not \"%s\", line %d of \"%s\"",
	     pi->id, pi->lnctr, pi->tokenFile);
    goto syntaxErrorReturn;
  }
  while (tkn != OPERATOR) {
    /* allocate a field and set it's values */
    fld = malloc (sizeof (contField));
    if (0 == fld) {
      containerError = CONT_MEMORY;
      goto syntaxErrorReturn;
    }
    memset (fld, 0, sizeof (contField));
    check_pointer (fld);
    strncpy (fld->name, pi->id, CONTAINER_FIELD_NAME_LENGTH);
    fld->name[CONTAINER_FIELD_NAME_LENGTH] = '\0';
    tkn = getToken (pi);
    if (tkn != IDENT) {
      containerError = CONT_SYNTAX;
      SPRINTF (container_syntax_error, CONTAINER_RESULT_SIZE,
	       "expected an identifier (int, long, float, double, or string), not \"%s\", "
	       "line %d of \"%s\"", pi->id, pi->lnctr, pi->tokenFile);
      goto syntaxErrorReturn;
    }
    if (!strcmp (pi->id, "int")) {
      fld->type = CTYPE_INT;
      fld->offset = offset;
      fld->size = sizeof (int);
      offset += sizeof (int);
    }
    else if (!strcmp (pi->id, "long")) {
      fld->type = CTYPE_LONG;
      fld->offset = offset;
      fld->size = sizeof (long);
      offset += sizeof (long);
    }
    else if (!strcmp (pi->id, "float")) {
      fld->type = CTYPE_FLOAT;
      fld->offset = offset;
      fld->size = sizeof (float);
      offset += sizeof (float);
      offset += wordsize - (offset % wordsize);
    }
    else if (!strcmp (pi->id, "double")) {
      fld->type = CTYPE_DOUBLE;
      fld->offset = offset;
      fld->size = sizeof (double);
      offset += sizeof (double);
      offset += wordsize - (offset % wordsize);
    }
    else if (!strcmp (pi->id, "string")) {
      fld->type = CTYPE_STRING;
      tkn = getToken (pi);
      if (!(tkn == OPERATOR && pi->id[0] == '(')) {
	containerError = CONT_SYNTAX;
	SPRINTF (container_syntax_error, CONTAINER_RESULT_SIZE,
		 "expected '(', not \"%s\", line %d of \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
	goto syntaxErrorReturn;
      }
      tkn = getToken (pi);
      if (!(tkn == NUMBER)) {
	containerError = CONT_SYNTAX;
	SPRINTF (container_syntax_error, CONTAINER_RESULT_SIZE,
		 "expected a number, not \"%s\", line %d of \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
	goto syntaxErrorReturn;
      }
      fld->offset = offset;
      fld->size = atoi (pi->id);
      fld->size++;		/* don't forget the '\0'! */
      offset += fld->size;
      offset += wordsize - (offset % wordsize);
      tkn = getToken (pi);
      if (!(tkn == OPERATOR && pi->id[0] == ')')) {
	containerError = CONT_SYNTAX;
	SPRINTF (container_syntax_error, CONTAINER_RESULT_SIZE,
		 "expected ')', not \"%s\", line %d of \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
	goto syntaxErrorReturn;
      }
    }
    else {
      containerError = CONT_SYNTAX;
      SPRINTF (container_syntax_error, CONTAINER_RESULT_SIZE,
	       "expected one of int, long, float, double, or string, not \"%s\", "
	       "line %d of \"%s\"", pi->id, pi->lnctr, pi->tokenFile);
      goto syntaxErrorReturn;
    }
    /* expect ident (unique|dup) OR operator ';' */
    tkn = getToken (pi);
    if (tkn == IDENT) {
      if (!strcmp (pi->id, "unique")) {
	if (hdl->isIndexed == TRUE) {
	  containerError = CONT_SYNTAX;
	  SPRINTF (container_syntax_error, CONTAINER_RESULT_SIZE,
		   "only one field in a container can be indexed, line %d of \"%s\"",
		   pi->lnctr, pi->tokenFile);
	  goto syntaxErrorReturn;
	}
	hdl->isIndexed = TRUE;
	hdl->isUnique = TRUE;
	hdl->index_offset = fld->offset;
	hdl->index_type = fld->type;
	fld->isIndexed = TRUE;
      }
      else if (!strcmp (pi->id, "dup")) {
	if (hdl->isIndexed == TRUE) {
	  containerError = CONT_SYNTAX;
	  SPRINTF (container_syntax_error, CONTAINER_RESULT_SIZE,
		   "only one field in a container can be indexed, line %d of \"%s\"",
		   pi->lnctr, pi->tokenFile);
	  goto syntaxErrorReturn;
	}
	hdl->isIndexed = TRUE;
	hdl->isUnique = FALSE;
	hdl->index_offset = fld->offset;
	hdl->index_type = fld->type;
	fld->isIndexed = TRUE;
      }
      else {
	containerError = CONT_SYNTAX;
	SPRINTF (container_syntax_error, CONTAINER_RESULT_SIZE,
		 "expected one of unique or dup, not \"%s\", line %d of \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
	goto syntaxErrorReturn;
      }
      tkn = getToken (pi);
      if (!(tkn == OPERATOR || pi->id[0] == ';')) {
	containerError = CONT_SYNTAX;
	SPRINTF (container_syntax_error, CONTAINER_RESULT_SIZE,
		 "expected ';', not \"%s\", line %d of \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
	goto syntaxErrorReturn;
      }
    }
    else {
      if (!(tkn == OPERATOR || pi->id[0] == ';')) {
	containerError = CONT_SYNTAX;
	SPRINTF (container_syntax_error, CONTAINER_RESULT_SIZE,
		 "expected ';', not \"%s\", line %d of \"%s\"",
		 pi->id, pi->lnctr, pi->tokenFile);
	goto syntaxErrorReturn;
      }
    }
    flds[fldnum] = fld;
    fldnum++;
    tkn = getToken (pi);
    if (tkn == OPERATOR)
      break;
    if (CONTAINER_MAX_NUM_FIELDS < fldnum)
      break;
  }
  if (!(tkn == OPERATOR || pi->id[0] == '}')) {
    containerError = CONT_SYNTAX;
    SPRINTF (container_syntax_error, CONTAINER_RESULT_SIZE,
	     "expected '}', not \"%s\", line %d of \"%s\"",
	     pi->id, pi->lnctr, pi->tokenFile);
    goto syntaxErrorReturn;
  }
  /* ignore everything else */
  deleteParser (pi);
  hdl->fields = flds;
  hdl->numFields = fldnum;
  hdl->numRecords = 0;
  hdl->size = offset;

  return hdl;

syntaxErrorReturn:
  if (pi != 0)
    deleteParser (pi);
  for (i = 0; i < CONTAINER_MAX_NUM_FIELDS; i++) {
    if (flds[i] != 0)
      free (flds[i]);
    else
      break;
  }
  free (flds);
  if (fld != 0)
    free (fld);
  if (hdl != 0)
    free (hdl);
  return 0;
}

/*
 * [BeginDoc]
 *
 * \subsection{containerInit}
 * \index{containerInit}
 * [Verbatim] */

char *containerInit (char *fname)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * containerInit cdf_file_name
 *
 * The ``containerInit'' function initializes a container.  The argument
 * given by ``fname'' should be a \emph{container definition file (.cdf)}.
 * \index{container!cdf file} \index{container!container definition file}
 * The following is an example cdf file:
 * [EndDoc]
 * @DocInclude ../include/cdf_example.txt
 * [BeginDoc]
 *
 * The cdf file supports a C-like syntax.  You can use C or C++ comments and whitespace
 * is not relevant.  It is an error if you don't use the ';' at the end of each line.
 * Anything after the last close curly brace is ignored.
 *
 * ``containerInit'' will return a handle that will be used by all other container
 * routines.\index{container!handle}  This handle should be stored in a variable, as
 * follows:
 *
 * [Verbatim]
if [ catch { set cont1 [ containerInit container.cdf ] } result ] {
  puts "containerInit container.cdf: $result"
  exit 1
}
 * [EndDoc] */
{
  int i;
  contHandle *hdl;

  hdl = parseContainerDefinitionFile (fname);
  if (0 == hdl) {
    if (containerError == CONT_PARSE) {
      SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	  "couldn't parse \"%s\"", fname);
    }
    else if (containerError == CONT_SYNTAX) {
      SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	  "syntax error: %s", container_syntax_error);
    }
    else {
      SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	  "container error: %s", containerErrorString[containerError]);
    }
    return 0;
  }
  /* now, make sure that one of the fields is indexed */
  for (i = 0; i < hdl->numFields && hdl->fields[i] != 0; i++)
    if (hdl->fields[i]->isIndexed == TRUE)
      break;
  if (i >= hdl->numFields) {
    containerError = CONT_NOINDEX;
    handleDelete (hdl);
    strcpy (container_error, containerErrorString[containerError]);
    return 0;
  }
  /* initiate the shell */
  hdl->shl = initShell (container_compare, hdl->isUnique, TRUE);
  if (hdl->shl == 0) {
    containerError = CONT_SHELL;
    handleDelete (hdl);
    strcpy (container_error, containerErrorString[containerError]);
    return 0;
  }
  hdl->head = hdl->current = hdl->shl->lh->head;
  hdl->tail = hdl->shl->lh->tail;
  /* allocate the data variable */
  hdl->data = malloc (hdl->size);
  if (0 == hdl->data) {
    containerError = CONT_MEMORY;
    handleDelete (hdl);
    strcpy (container_error, containerErrorString[containerError]);
    return 0;
  }
  memset (hdl->data, 0, hdl->size);
  check_pointer (hdl->data);
  /* finally, add the handle to the handle array */
  if (container_handles[num_container_handles] == 0) {
    container_handles[num_container_handles] = hdl;
    num_container_handles++;
  }
  else {
    containerError = CONT_CORRUPT;
    handleDelete (hdl);
    strcpy (container_error, containerErrorString[containerError]);
    return 0;
  }
  return (hdl->name);
}

/*
 * [BeginDoc]
 *
 * \subsection{containerDelete}
 * \index{containerDelete}
 *
 * [Verbatim] */

int containerDelete (char *handle)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * containerDelete handle
 *
 * The ``containerDelete'' function deletes the container given by the handle
 * ``handle''.  On error, it throws an exception and returns an error.  Otherwise,
 * it clears the container from memory and the handle ``handle'' becomes invalid.
 *
 * [EndDoc]
 */
{
  int i, j;
  contHandle *hdl;

  hdl = findContainerHandle (handle);
  if (hdl == 0) {
    strcpy (container_error, containerErrorString[containerError]);
    return _ERROR_;
  }
  i = findContainerHandleNumber (handle);
  if (i == _ERROR_) {
    strcpy (container_error, containerErrorString[containerError]);
    return _ERROR_;
  }
  for (j = i; j < num_container_handles && container_handles[j] != 0; j++)
    container_handles[j] = container_handles[j + 1];
  if (num_container_handles > 0)
    num_container_handles--;
  handleDelete (hdl);
  return _OK_;
}

void handleDelete (contHandle * hdl)
{
  int i;
  contField **flds;

  if (hdl->shl != 0)
    delShell (hdl->shl, 0);
  if (hdl->fields != 0) {
    flds = hdl->fields;
    for (i = 0; i < hdl->numFields; i++) {
      if (flds[i] != 0)
	free (flds[i]);
    }
    free (flds);
  }
  if (hdl->data != 0)
    free (hdl->data);
  free (hdl);
}

/*
 * [BeginDoc]
 *
 * \subsection{containerClearError}
 * \index{containerClearError}
 * [Verbatim] */

int containerClearError (void)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * containerClearError
 *
 * The ``containerClearError function clears the global error condition for all
 * containers.  It takes no arguments and only returns an OK value.  Note: the way
 * errors are handled by the container module makes is thread-unsafe.
 *
 * [EndDoc]
 */
{
  int i;

  memset (container_error, 0, CONTAINER_RESULT_SIZE);
  containerError = CONT_NOERROR;
  set_shlError(0, SHELL_NOERR);
  for (i = 0; i < num_container_handles; i++) {
    set_shlError(container_handles[i]->shl,SHELL_NOERR);
    set_ListError(container_handles[i]->shl->lh,LIST_NOERROR);
  }
  set_ListError(0, LIST_NOERROR);
  return _OK_;
}

/*
 * [BeginDoc]
 *
 * \subsection{containerSetInt}
 * \index{containerSetInt}
 * [Verbatim] */

int containerSetInt (char *handle, char *field, int value)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * containerSetInt handle field value
 *
 * The ``containerSetInt'' function allows you to set an integer field ``field'' in the
 * container given by ``handle'' to ``value''.  It is an error if the handle is not valid
 * or if the field does not exist.  Also, if ``value'' doesn't \emph{look like} an
 * integer to Tcl or Perl, you may get an error from the scripting language.  On error,
 * ``containerSetInt'' throws and exception and returns the error to the scripting
 * language.  On success, the field is set to the value.  The programmer must set all
 * the fields that she wants populated using the ``containerSet'' functions before she
 * calls ``containerAddRecord''.\index{containerAddRecord}
 *
 * [EndDoc]
 */
{
  contHandle *hdl;
  int i;

  hdl = findContainerHandle (handle);
  if (0 == hdl) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container handle %s is invalid", handle);
    return _ERROR_;
  }
  for (i = 0; i < CONTAINER_MAX_NUM_FIELDS && hdl->fields[i] != 0; i++)
    if (!strcmp (hdl->fields[i]->name, field))
      break;
  if (hdl->fields[i]->type != CTYPE_INT) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"field %s in container %s is not an int", field, handle);
    return _ERROR_;
  }
  *((int *) ((char *) (hdl->data + hdl->fields[i]->offset))) = value;
  return _OK_;
}

/*
 * [BeginDoc]
 *
 * \subsection{containerSetLong}
 * \index{containerSetLong}
 * [Verbatim] */

int containerSetLong (char *handle, char *field, long value)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * containerSetLong handle field value
 *
 * The ``containerSetLong'' function allows you to set a long integer field ``field'' in the
 * container given by ``handle'' to ``value''.  It is an error if the handle is not valid
 * or if the field does not exist.  Also, if ``value'' doesn't \emph{look like} a long
 * integer to Tcl or Perl, you may get an error from the scripting language.  On error,
 * ``containerSetLong'' throws and exception and returns the error to the scripting
 * language.  On success, the field is set to the value.  The programmer must set all
 * the fields that she wants populated using the ``containerSet'' functions before she
 * calls ``containerAddRecord''.\index{containerAddRecord}
 *
 * [EndDoc]
 */
{
  contHandle *hdl;
  int i;

  hdl = findContainerHandle (handle);
  if (0 == hdl) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container handle %s is invalid", handle);
    return _ERROR_;
  }
  for (i = 0; i < CONTAINER_MAX_NUM_FIELDS && hdl->fields[i] != 0; i++)
    if (!strcmp (hdl->fields[i]->name, field))
      break;
  if (hdl->fields[i]->type != CTYPE_LONG) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"field %s in container %s is not a long", field, handle);
    return _ERROR_;
  }
  *((long *) ((char *) (hdl->data + hdl->fields[i]->offset))) = value;
  return _OK_;
}

/*
 * [BeginDoc]
 *
 * \subsection{containerSetFloat}
 * \index{containerSetFloat}
 * [Verbatim] */

int containerSetFloat (char *handle, char *field, float value)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * containerSetFloat handle field value
 *
 * The ``containerSetFloat'' function allows you to set a float field ``field'' in the
 * container given by ``handle'' to ``value''.  It is an error if the handle is not valid
 * or if the field does not exist.  Also, if ``value'' doesn't \emph{look like} a
 * float to Tcl or Perl, you may get an error from the scripting language.  On error,
 * ``containerSetFloat'' throws and exception and returns the error to the scripting
 * language.  On success, the field is set to the value.  The programmer must set all
 * the fields that she wants populated using the ``containerSet'' functions before she
 * calls ``containerAddRecord''.\index{containerAddRecord}
 *
 * [EndDoc]
 */
{
  contHandle *hdl;
  int i;

  hdl = findContainerHandle (handle);
  if (0 == hdl) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container handle %s is invalid", handle);
    return _ERROR_;
  }
  for (i = 0; i < CONTAINER_MAX_NUM_FIELDS && hdl->fields[i] != 0; i++)
    if (!strcmp (hdl->fields[i]->name, field))
      break;
  if (hdl->fields[i]->type != CTYPE_FLOAT) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"field %s in container %s is not a float", field, handle);
    return _ERROR_;
  }
  *((float *) ((char *) (hdl->data + hdl->fields[i]->offset))) = value;
  return _OK_;
}

/*
 * [BeginDoc]
 *
 * \subsection{containerSetDouble}
 * \index{containerSetDouble}
 * [Verbatim] */

int containerSetDouble (char *handle, char *field, double value)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * containerSetDouble handle field value
 *
 * The ``containerSetDouble'' function allows you to set a double field ``field'' in the
 * container given by ``handle'' to ``value''.  It is an error if the handle is not valid
 * or if the field does not exist.  Also, if ``value'' doesn't \emph{look like} a
 * double to Tcl or Perl, you may get an error from the scripting language.  On error,
 * ``containerSetInt'' throws and exception and returns the error to the scripting
 * language.  On success, the field is set to the value.  The programmer must set all
 * the fields that she wants populated using the ``containerSet'' functions before she
 * calls ``containerAddRecord''.\index{containerAddRecord}
 *
 * [EndDoc]
 */
{
  contHandle *hdl;
  int i;

  hdl = findContainerHandle (handle);
  if (0 == hdl) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container handle %s is invalid", handle);
    return _ERROR_;
  }
  for (i = 0; i < CONTAINER_MAX_NUM_FIELDS && hdl->fields[i] != 0; i++)
    if (!strcmp (hdl->fields[i]->name, field))
      break;
  if (hdl->fields[i]->type != CTYPE_DOUBLE) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"field %s in container %s is not a double", field, handle);
    return _ERROR_;
  }
  *((double *) ((char *) (hdl->data + hdl->fields[i]->offset))) = value;
  return _OK_;
}

/*
 * [BeginDoc]
 *
 * \subsection{containerSetString}
 * \index{containerSetString}
 * [Verbatim] */

int containerSetString (char *handle, char *field, char *value)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * containerSetString handle field string_value
 *
 * The ``containerSetString'' function sets the field ``field'' to ``value'' in the
 * container given by ``handle''.  It is an error if the handle is not valid or if the
 * field does not exist.  On error, ``containerSetString'' throws an exception and
 * returns the error to the scripting language.  On success, the field is set to the
 * value up to the max length of the field, if value is longer.  In other words, if the
 * string given by ``value'' is longer than the field length, it will get truncated.
 *
 * [EndDoc]
 */
{
  contHandle *hdl;
  int i;

  hdl = findContainerHandle (handle);
  if (0 == hdl) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container handle %s is invalid", handle);
    return _ERROR_;
  }
  for (i = 0; i < CONTAINER_MAX_NUM_FIELDS && hdl->fields[i] != 0; i++)
    if (!strcmp (hdl->fields[i]->name, field))
      break;
  if (hdl->fields[i]->type != CTYPE_STRING) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"field %s in container %s is not a string", field, handle);
    return _ERROR_;
  }
  strncpy ((char *) (hdl->data + hdl->fields[i]->offset), value,
	   hdl->fields[i]->size);
  *(char*)(hdl->data + hdl->fields[i]->offset + hdl->fields[i]->size - 1) = '\0';
  return _OK_;
}

/*
 * [BeginDoc]
 *
 * \subsection{containerGetField}
 * \index{containerGetField}
 * [Verbatim] */

char *containerGetField (char *handle, char *field)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * containerGetField handle field
 *
 * containerGetField returns the field value for the field given by ``field'' for the
 * handle ``handle''.  It is an error if the ``handle'' or the ``field'' is invalid.
 * On error, an exception is thrown and the error is returned to the scripting language.
 *
 * [EndDoc]
 */
{
  int i;
  contHandle *hdl;
  int intval;
  long longval;
  float floatval;
  double doubleval;

  hdl = findContainerHandle (handle);
  if (0 == hdl) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container handle %s is invalid", handle);
    return 0;
  }
  for (i = 0; i < CONTAINER_MAX_NUM_FIELDS && hdl->fields[i] != 0; i++)
    if (!strcmp (hdl->fields[i]->name, field))
      break;
  if (hdl->fields[i] == 0) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"field %s is invalid", field);
    return 0;
  }
  if (hdl->data != 0 && hdl->current->data != 0)
    memcpy (hdl->data, hdl->current->data, hdl->size);
  check_pointer (hdl->data);
  /*memset (rslt, 0, CONTAINER_RESULT_SIZE); */
  switch (hdl->fields[i]->type) {
  case CTYPE_INT:
    intval = *((int *) ((char *) hdl->data + hdl->fields[i]->offset));
    SPRINTF (rslt, CONTAINER_RESULT_SIZE, "%d", intval);
    return rslt;
  case CTYPE_LONG:
    longval = *((long *) ((char *) hdl->data + hdl->fields[i]->offset));
    SPRINTF (rslt, CONTAINER_RESULT_SIZE, "%ld", longval);
    return rslt;
  case CTYPE_FLOAT:
    floatval = *((float *) ((char *) hdl->data + hdl->fields[i]->offset));
    SPRINTF (rslt, CONTAINER_RESULT_SIZE, "%f", floatval);
    return rslt;
  case CTYPE_DOUBLE:
    doubleval = *((double *) ((char *) hdl->data + hdl->fields[i]->offset));
    SPRINTF (rslt, CONTAINER_RESULT_SIZE, "%lf", doubleval);
    return rslt;
  case CTYPE_STRING:
    SPRINTF (rslt, CONTAINER_RESULT_SIZE, "%s",
	((char *) hdl->data + hdl->fields[i]->offset));
    return rslt;
  default:
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	     "type of field %s in container %s not determinable",
	     field, handle);
    return 0;
  }
  /* shouldn't get here, but gcc can be a little fussy */
  return 0;
}

/*
 * [BeginDoc]
 *
 * \subsection{containerAddRecord}
 * \index{containerAddRecord}
 * [Verbatim] */

int containerAddRecord (char *handle)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The ``containerAddRecord'' function adds the data stored in the container by calls
 * to the ``containerSet'' functions to the container given by ``handle''.  On success,
 * the record is added to the container and a new internal buffer is allocated and
 * initialized to all zero values.  On failure, the record is not added, the buffer
 * remains unchanged, an exception is thrown and the error is returned to the scripting
 * language.  Also, if ``containerAddRecord'' is successful, the current pointer points
 * to the new record in the container.
 *
 * [EndDoc]
 */
{
  int status;
  contHandle *hdl;
  Link *lnk;

  hdl = findContainerHandle (handle);
  if (0 == hdl) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container handle %s is invalid", handle);
    return _ERROR_;
  }
  *((char *) hdl->data) = (int) hdl->index_type;
  *((char *) hdl->data + sizeof (int)) = (int) hdl->index_offset;
  lnk = malloc (sizeof (Link));
  if (0 == lnk) {
    containerError = CONT_MEMORY;
    return _ERROR_;
  }
/*  memset (lnk, 0, sizeof(Link));*/
  check_pointer (lnk);
  hdl->current = lnk;
  lnk->data = hdl->data;
  status = addShellItem (hdl->shl, lnk);
  if (status == _ERROR_) {
    if (hdl->shl->shlError == SHELL_UNIQUE)
      containerError = CONT_UNIQUE;
    else
      containerError = CONT_SHELL;
    hdl->current = hdl->shl->lh->current;
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"unique constraint violated");
    return _ERROR_;
  }
  hdl->numRecords = hdl->shl->lh->number;
  hdl->current = hdl->shl->lh->current;
  hdl->data = malloc (hdl->size);
  if (0 == hdl->data) {
    containerError = CONT_MEMORY;
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	containerErrorString[containerError]);
    return _ERROR_;
  }
  memset (hdl->data, 0, hdl->size);
  check_pointer (hdl->data);
  return (_OK_);
}

/*
 * [BeginDoc]
 *
 * \subsection{containerSearch}
 * \index{containerSearch}
 * [Verbatim] */

int containerSearch (char *handle, char *value)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The ``containerSearch'' function searches the container given by ``handle'' for the
 * value ``value'' in the sorted field.  When you define the container,
 * you specify which field is sorted with the ``dup'' or ``unique'' key word.  
 * This is the field that is used to search on for ``value''.  If an
 * error occurs, it raises an error and returns the error to the scripting language.
 * Otherwise, it returns false (0) if it doesn't find the value and true (1) if it does.
 * The current container pointer remains unchanged if the ``value'' isn't found and it
 * points to the found record if ``value'' is found.
 *
 * [EndDoc]
 */
{
  int i;
  contHandle *hdl;
  void *data;
  Link *lnk;
  Link *found = 0;
  int intval;
  long longval;
  float floatval;
  double doubleval;

  hdl = findContainerHandle (handle);
  if (0 == hdl) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container handle %s is invalid", handle);
    return _ERROR_;
  }
  lnk = malloc (sizeof (Link));
  if (0 == lnk) {
    containerError = CONT_MEMORY;
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container error: %s", containerErrorString[containerError]);
    return _ERROR_;
  }
  memset (lnk, 0, sizeof (Link));
  check_pointer (lnk);
  data = malloc (hdl->size);
  if (0 == data) {
    containerError = CONT_MEMORY;
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container error: %s", containerErrorString[containerError]);
    free (lnk);
    return _ERROR_;
  }
  memset (data, 0, hdl->size);
  check_pointer (data);
  *((char *) data) = (int) hdl->index_type;
  *((char *) data + sizeof (int)) = (int) hdl->index_offset;
  /* determine which field is indexed and search on that */
  for (i = 0; i < hdl->numFields && hdl->fields[i] != 0; i++) {
    if (hdl->fields[i]->isIndexed == TRUE)
      break;
  }
  if (i >= hdl->numFields) {
    containerError = CONT_NOINDEX;
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container error: %s", containerErrorString[containerError]);
    free (data);
    free (lnk);
    return _ERROR_;
  }
  switch (hdl->fields[i]->type) {
  case CTYPE_INT:
    intval = atoi (value);
    *((char *) data + hdl->index_offset) = intval;
    break;
  case CTYPE_LONG:
    longval = atol (value);
    *((char *) data + hdl->index_offset) = longval;
    break;
  case CTYPE_FLOAT:
    floatval = (float) atof (value);
    *((char *) data + hdl->index_offset) = floatval;
    break;
  case CTYPE_DOUBLE:
    doubleval = atof (value);
    *((char *) data + hdl->index_offset) = doubleval;
    break;
  case CTYPE_STRING:
    strncpy ((char *) (data + hdl->index_offset), value,
	     hdl->fields[i]->size);
    break;
  case CTYPE_UNDEFINED:
    break;
  }
  lnk->data = data;
  found = findShellItem (hdl->shl, lnk);
  /* found it */
  if (found != 0) {
    free (lnk);
    free (data);
    hdl->current = found;
    return TRUE;
  }
  /* didn't find it - check for error */
  if (found == 0 && hdl->shl->shlError == SHELL_NOERR) {
    free (lnk);
    free (data);
    return FALSE;
  }
  else {			/* shlError != SHELL_NOERR */
    free (lnk);
    free (data);
    containerError = CONT_SHELL;
    if (hdl->shl->shlError == SHELL_UNIQUE)
      SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	       "container error: unique contstraint violated");
    else if (hdl->shl->shlError == SHELL_LIST)
      SPRINTF (container_error,  CONTAINER_RESULT_SIZE,
	  "container error: list error: %s", ListErrorString[ListError]);
    else
      SPRINTF (container_error, CONTAINER_RESULT_SIZE, "container error: %s",
	       containerErrorString[containerError]);
    return _ERROR_;
  }
  return FALSE;			/* keep gcc happy */
}

/*
 * [BeginDoc]
 *
 * \subsection{containerQuery}
 * \index{containerQuery}
 * [Verbatim] */

int containerQuery (char *handle, char *value)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The ``containerQuery'' function searches the container given by ``handle'' for the
 * value ``value'' in the sorted field.  This works similarly to containerSearch()
 * except that it always resets the current pointer unless there's an error.  If it
 * can't find an exact match, it will match on the item before the point in the
 * sorted list where the searched for item would be logically added.  This only
 * returns 0 (FALSE) on error.
 *
 * [EndDoc]
 */
{
  int i;
  contHandle *hdl;
  void *data;
  Link *lnk;
  Link *found = 0;
  int intval;
  long longval;
  float floatval;
  double doubleval;

  hdl = findContainerHandle (handle);
  if (0 == hdl) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container handle %s is invalid", handle);
    return _ERROR_;
  }
  lnk = malloc (sizeof (Link));
  if (0 == lnk) {
    containerError = CONT_MEMORY;
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container error: %s", containerErrorString[containerError]);
    return _ERROR_;
  }
  memset (lnk, 0, sizeof (Link));
  check_pointer (lnk);
  data = malloc (hdl->size);
  if (0 == data) {
    containerError = CONT_MEMORY;
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container error: %s", containerErrorString[containerError]);
    free (lnk);
    return _ERROR_;
  }
  memset (data, 0, hdl->size);
  check_pointer (data);
  *((char *) data) = (int) hdl->index_type;
  *((char *) data + sizeof (int)) = (int) hdl->index_offset;
  /* determine which field is indexed and search on that */
  for (i = 0; i < hdl->numFields && hdl->fields[i] != 0; i++) {
    if (hdl->fields[i]->isIndexed == TRUE)
      break;
  }
  if (i >= hdl->numFields) {
    containerError = CONT_NOINDEX;
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container error: %s", containerErrorString[containerError]);
    free (data);
    free (lnk);
    return _ERROR_;
  }
  switch (hdl->fields[i]->type) {
  case CTYPE_INT:
    intval = atoi (value);
    *((char *) data + hdl->index_offset) = intval;
    break;
  case CTYPE_LONG:
    longval = atol (value);
    *((char *) data + hdl->index_offset) = longval;
    break;
  case CTYPE_FLOAT:
    floatval = (float) atof (value);
    *((char *) data + hdl->index_offset) = floatval;
    break;
  case CTYPE_DOUBLE:
    doubleval = atof (value);
    *((char *) data + hdl->index_offset) = doubleval;
    break;
  case CTYPE_STRING:
    strncpy ((char *) (data + hdl->index_offset), value,
	     hdl->fields[i]->size);
    break;
  case CTYPE_UNDEFINED:
    break;
  }
  lnk->data = data;
  found = queryShellItem (hdl->shl, lnk);
  /* found it */
  if (found != 0) {
    free (lnk);
    free (data);
    hdl->current = found;
    return TRUE;
  }
  else {			/* ! found ==> error */
    free (lnk);
    free (data);
    containerError = CONT_SHELL;
    if (hdl->shl->shlError == SHELL_UNIQUE)
      SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	       "container error: unique contstraint violated");
    else if (hdl->shl->shlError == SHELL_LIST)
      SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	  "container error: list error: %s", ListErrorString[ListError]);
    else
      SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	  "container error: %s", containerErrorString[containerError]);
    return _ERROR_;
  }
  return FALSE;			/* keep gcc happy */
}

/*
 * [BeginDoc]
 *
 * \subsection{containerDeleteRecord}
 * \index{containerDeleteRecord}
 * [Verbatim] */

int containerDeleteRecord (char *handle, char *value)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The ``containerDeleteRecord'' function deletes the current record.  The current
 * record can be set by calls to the traversal functions, including ``containerSearch'',
 * ``containerFirst'', ``containerLast'', ``containerNext'', ``containerPrev'', etc.
 * On success, the record is deleted from the container.  On error, an exception is
 * thrown and an error is returned to the scripting language.
 *
 * [EndDoc]
 */
{
  Link *lnk;
  contHandle *hdl;
  int status;

  status = containerSearch (handle, value);
  if (status == _ERROR_)
    return _ERROR_;
  if (status == FALSE)
    return FALSE;
  hdl = findContainerHandle (handle);
  if (0 == hdl) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container handle %s is invalid", handle);
    return _ERROR_;
  }
  lnk = hdl->current;
  lnk = removeShellItem (hdl->shl, lnk);
  if (lnk == 0) {
    containerError = CONT_SHELL;
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container error: %s", containerErrorString[containerError]);
    return _ERROR_;
  }
  free (lnk->data);
  free (lnk);
  hdl->numRecords = hdl->shl->lh->number;
  hdl->current = hdl->shl->lh->current;
  return TRUE;
}

/*
 * [BeginDoc]
 *
 * \subsection{containerFirst}
 * \index{containerFirst}
 * [Verbatim] */

int containerFirst (char *handle)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The ``containerFirst'' function sets the current record in the container given by
 * ``handle'' to the first record.  It's an error if ``handle'' is an invalid
 * container handle; if so, an exception is thrown and an error returned to the scripting
 * language.  Otherwise, the current record is set.  It is not an error to call this
 * function on an empty container, but it is meaningless.
 *
 * [EndDoc]
 */
{
  contHandle *hdl;

  hdl = findContainerHandle (handle);
  if (0 == hdl) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container handle %s is invalid", handle);
    return _ERROR_;
  }
  if (hdl->numRecords == 0)
    hdl->current = hdl->shl->lh->head;
  else
    hdl->current = hdl->shl->lh->head->next;
  return _OK_;
}

/*
 * [BeginDoc]
 *
 * \subsection{containerLast}
 * \index{containerLast}
 * [Verbatim] */

int containerLast (char *handle)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The ``containerLast'' function sets the current record in the container given by
 * ``handle'' to the last record.  It's an error if ``handle'' is an invalid
 * container handle; if so, an exception is thrown and an error returned to the scripting
 * language.  Otherwise, the current record is set.  It is not an error to call this
 * function on an empty container, but it is meaningless.
 *
 * [EndDoc]
 */
{
  contHandle *hdl;

  hdl = findContainerHandle (handle);
  if (0 == hdl) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container handle %s is invalid", handle);
    return _ERROR_;
  }
  if (hdl->numRecords == 0)
    hdl->current = hdl->shl->lh->tail;
  else
    hdl->current = hdl->shl->lh->tail->prev;
  return _OK_;
}

/*
 * [BeginDoc]
 *
 * \subsection{containerNext}
 * \index{containerNext}
 * [Verbatim] */

int containerNext (char *handle)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The ``containerNext'' function sets the current record in the container given by
 * ``handle'' to the first record.  It's an error if ``handle'' is an invalid
 * container handle; if so, an exception is thrown and an error returned to the scripting
 * language.  Otherwise, the current record is set.  It is not an error to call this
 * function on an empty container, but it is meaningless.
 *
 * [EndDoc]
 */
{
  contHandle *hdl;

  hdl = findContainerHandle (handle);
  if (0 == hdl) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container handle %s is invalid", handle);
    return _ERROR_;
  }
  if (hdl->numRecords == 0)
    hdl->current = hdl->shl->lh->head;
  else if (hdl->current != hdl->shl->lh->tail)
    hdl->current = hdl->current->next;
  return _OK_;
}

/*
 * [BeginDoc]
 *
 * \subsection{containerPrev}
 * \index{containerPrev}
 * [Verbatim] */

int containerPrev (char *handle)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The ``containerPrev'' function sets the current record in the container given by
 * ``handle'' to the first record.  It's an error if ``handle'' is an invalid
 * container handle; if so, an exception is thrown and an error returned to the scripting
 * language.  Otherwise, the current record is set.  It is not an error to call this
 * function on an empty container, but it is meaningless.
 *
 * [EndDoc]
 */
{
  contHandle *hdl;

  hdl = findContainerHandle (handle);
  if (0 == hdl) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container handle %s is invalid", handle);
    return _ERROR_;
  }
  if (hdl->numRecords == 0)
    hdl->current = hdl->shl->lh->head;
  else if (hdl->current != hdl->shl->lh->head)
    hdl->current = hdl->current->prev;
  return _OK_;
}

/*
 * [BeginDoc]
 *
 * \subsection{containerBOF}
 * \index{containerBOF}
 * [Verbatim] */

int containerBOF (char *handle)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The ``containerBOF'' function returns true if the current record in the container
 * given by ``handle'' is the first one in the container.  If the container is empty or
 * if the current pointer is not on the first record, it returns false.  If ``handle''
 * is not a valid container handle, ``containerBOF'' throws an exception and returns
 * an error condition to the scripting language.
 *
 * [EndDoc]
 */
{
  contHandle *hdl;

  hdl = findContainerHandle (handle);
  if (0 == hdl) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container handle %s is invalid", handle);
    return _ERROR_;
  }
  if (hdl->numRecords == 0)
    return FALSE;
  if (hdl->current == hdl->shl->lh->head->next)
    return TRUE;
  else
    return FALSE;
}

/*
 * [BeginDoc]
 *
 * \subsection{containerEOF}
 * \index{containerEOF}
 * [Verbatim] */

int containerEOF (char *handle)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The ``containerEOF'' function returns true if the current record in the container
 * given by ``handle'' is the last one in the container.  If the container is empty or
 * if the current pointer is not on the last record, it returns false.  If ``handle''
 * is not a valid container handle, ``containerEOF'' throws an exception and returns
 * an error condition to the scripting language.
 *
 * [EndDoc]
 */
{
  contHandle *hdl;

  hdl = findContainerHandle (handle);
  if (0 == hdl) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container handle %s is invalid", handle);
    return _ERROR_;
  }
  if (hdl->numRecords == 0)
    return FALSE;
  if (hdl->current == hdl->shl->lh->tail->prev)
    return TRUE;
  else
    return FALSE;
}

/*
 * [BeginDoc]
 *
 * \subsection{containerNumRecords}
 * \index{containerNumRecords}
 * [Verbatim] */

int containerNumRecords (char *handle)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The ``containerNumRecords'' function returns the number of records stored by the
 * container given by ``handle''.  It is an error if ``handle'' is an invalid
 * container handle, in which case an exception is thrown and an error is returned to the
 * scripting language.  Otherwise, the number of records in the container is returned.
 * The current record pointer remains unchanged by a call to ``containerNumRecords''.
 *
 * [EndDoc]
 */
{
  contHandle *hdl;

  hdl = findContainerHandle (handle);
  if (0 == hdl) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container handle %s is invalid", handle);
    return _ERROR_;
  }
  return hdl->numRecords;
}

/*
 * [BeginDoc]
 *
 * \subsection{containerRestructureIndex}
 * \index{containerRestructureIndex}
 * [Verbatim] */

int containerRestructureIndex (char *handle)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The ``containerRestructureIndex'' function restructures the in-memory
 * index of the container given by ``handle''.  It is an error if ``handle''
 * is invalid, in which case an exception is thrown and an error is returned
 * to the scripting language.
 *
 * [EndDoc]
 */
{
  contHandle *hdl;
  int status;

  hdl = findContainerHandle (handle);
  if (0 == hdl) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"container handle %s is invalid", handle);
    return _ERROR_;
  }
  status = restructureShellNodes (hdl->shl);
  if (status == _ERROR_) {
    SPRINTF (container_error, CONTAINER_RESULT_SIZE,
	"error returned from restructureShellNodes()");
    return _ERROR_;
  }
  return _OK_;
}
