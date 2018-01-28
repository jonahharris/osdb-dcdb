/*  Header File: container.h */

/*
 * [BeginDoc]
 *
 * \section{Container Module}
 * \index{container}
 *
 * The container module is designed to provide a high-performance, generic
 * sorted data structure to scripted languages like Tcl/Tk or Perl.  I am sure
 * there are already such beasts for these languages, but as I am
 * unfamiliar with them, I decided to implement one myself.  This uses
 * the shell module, a high-performance general purpose sorted module designed
 * for quick adds, quick searches  and indexed-sequential retrieval.
 *
 * The container module is part of the DCDB shared object.  If you load the
 * DCDB shared object in a Tcl/Tk or Perl script, you have access to the
 * commands represented here.  The documentation here shows examples that are
 * designed for Tcl.  If you are using Perl, you need to follow the syntax
 * requirements for Perl.
 *
 * The container is not very useful in pure C or C++ programs in that you
 * can simply use a shell for these.  You loose some performance with the
 * overhead in a container that you would not have with a pure shell, but the
 * performance is good enough overall to make the container worth using with a
 * scripting language.
 *
 * The basic things you can do with a container are as follows:
 *
 * \begin{itemize}
 * \item You can create a container with ``containerInit''.
 * \index{containerInit}
 * \item You can set field values with the ``containerSet'' functions.
 * \index{containerSetInt} \index{containerSetLong} \index{containerSetFloat}
 * \index{containerSetDouble} \index{containerSetString}
 * \item You can add a record to a container.
 * \index{containerAddRecord}
 * \item You can delete a record from a container.
 * \index{containerDeleteRecord}
 * \item You can traverse the records in a container, starting at the top and going
 * down or starting at the bottom and going up.
 * \index{containerFirst} \index{containerLast} \index{containerNext}
 * \index{containerPrev} \index{containerBOF} \index{containerEOF}
 * \item You can search for a specific item with the ``containerSearch'' function.
 * \index{containerSearch}
 * \item You can retrieve information from a container record with the
 * ``containerGetField'' function.
 * \index{containerGetField}
 * \item You can get information about a container and clear errors for a container.
 * \index{containerNumRecords} \index{containerClearError}
 * \item You can delete a container.
 * \index{containerDelete}
 * \end{itemize}
 *
 * The container module is also provided in the mycint binary when you do a
 * ``make cint'' in the primary source directory.  This builds a special C/C++
 * interpreter (CINT) that provides DCDB and the container module.  You can use
 * the C prototype and the samples provided to get a feel for how to use the
 * container module with CINT.  For more information on the CINT bindings, see
 * the primary README file and the README file in the cdb-1.1/src/cint directory.
 *
 * [EndDoc]
 */

#include <sort.h>
#include <mtoken.h>

#define	CONTAINER_FIELD_NAME_LENGTH 128
#define CONTAINER_MAX_NUM_FIELDS 16
#define	MAX_CONTAINER_HANDLES 20
#define CONTAINER_RESULT_SIZE 1024

extern char rslt[];
extern int num_container_handles;

typedef enum __contError {
  CONT_NOERROR,			/* no error */
  CONT_MEMORY,			/* error allocating memory */
  CONT_PARAM,			/* bad parameter to container function */
  CONT_INVALID,			/* invalid container handle */
  CONT_NOINDEX,			/* no index in the container */
  CONT_PARSE,			/* parsing container definition file */
  CONT_SYNTAX,			/* syntax error in container definition file */
  CONT_CORRUPT,			/* memory values corrupt */
  CONT_SHELL,			/* shell error */
  CONT_UNIQUE,			/* unique constraint violated */
  /* Add new ones here. */
  CONT_UNSPECIFIED		/* unspecified error */
} contError;

extern contError containerError;
extern char *containerErrorString[];

/*
 * [BeginDoc]
 *
 * \subsubsection{Container Fields}
 * \index{container!field types}
 *
 * The container field types are stored in a C enumerated type as follows:
 * [Verbatim] */
typedef enum __contType {
  CTYPE_INT,
  CTYPE_LONG,
  CTYPE_FLOAT,
  CTYPE_DOUBLE,
  CTYPE_STRING,
  /* Add new ones here. */
  CTYPE_UNDEFINED = 0xffffffff
} contType;
/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * You can add new ones if you want, although you will need to add the functionality
 * to set values of that type, etc.
 *
 * [EndDoc]
 */

/*
 * [BeginDoc]
 *
 * \index{container!types}
 *
 * Container fields are stored in a header and are accessible via the container handle
 * in the interface functions.  There is no other access provided to scripting languages.
 * The container handle is returned by a call to ``containerInit'' and is used for all
 * other functions in the container module.
 * \index{container!handle}
 * 
 * [EndDoc]
 */
typedef struct __contField {
  contType type;
  int offset;
  int isIndexed;
  int size;
  char name [CONTAINER_FIELD_NAME_LENGTH+1];
} contField;

typedef struct __contHandle {
  contType index_type;
  int index_offset;
  int numRecords;
  int numFields;
  int isIndexed;
  int isUnique;
  int size;
  Link *current;
  Link *head;
  Link *tail;
  void *data;
  shellHeader *shl;
  contField **fields;
  char name[CONTAINER_FIELD_NAME_LENGTH+1];
} contHandle;

extern char container_error[];

contHandle *parseContainerDefinitionFile (char *fname);
void handleDelete (contHandle *hdl);

/*
 * @DocInclude ../lib/container.c
 */
char *containerInit (char *fname);
int containerDelete (char *handle);
int containerClearError (void);
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
int containerRestructureIndex (char *handle);
