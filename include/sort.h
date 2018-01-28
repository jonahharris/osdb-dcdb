/* Header File: sort.h */

#ifndef __SORT_H__
#define __SORT_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Provides support for the file system, list and sort functionality.
 */

#include <dcdb_config.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef WIN32
#include <io.h>
#endif /* WIN32 */
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if ( ! __MINGW32__ && ! _CH_)
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#endif
#if (__MINGW32__ || _CH_)
#undef HAVE_PTHREAD_H
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <errno.h>

#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <string.h>

/*
 * @DocInclude ../include/disclaim.tex
 *
 * [BeginDoc]
 *
 * \section{The Sort Module}
 *
 * This is the sort module.  It includes several layers of functionality
 * that are not strictly related to sorting.  For example, there is debug
 * functionality included that provides a mechanism for easily debugging
 * dynamic memory access and utilization.  There is low-level file
 * functionality that attempts to provide file access capabilities in a
 * system independent way.  There is linked-list functionality that provides
 * support for doubly linked lists, including the ability to save them to disk
 * and retrieve them.  Finally, there is the actual sorting functionality.
 *
 * This module consists of a group of C functions that are combined to
 * provide sorting capability, including persistence. The code herein has been
 * fairly well tested and serves as the basis for other functionality.
 *
 * The sort algorithms here include quick sort and a hybrid in memory b-tree
 * \index{quick sort} \index{b*tree}
 * sort that is called a ``shell''.  This is probably misnamed in that this
 * sort is not strictly a shell.  It is more like a b-tree that provides search
 * access, sorted and reverse sorted sequential access.  A user can start at
 * the beginning of the list and traverse to the end, can start at the end of
 * the list and traverse to the beginning, or search to a location and traverse
 * forward or backward from there.  This evolved from a shell sort to what it
 * is today.
 *
 * The actual data is stored in a doubly linked list, so there are routines
 * provided for working with a list in a formal way.  These routines include
 * routines that are not necessarily used by the shell sort but have
 * proved to be useful.  The low-level data structure of the list is the
 * ``Link''.  The Link structure is defined below, but it basically contains
 * pointers to next and previous links and a pointer to a data element.  A
 * ``ListHeader'' structure defines the list proper.  It includes a ``head'' 
 * and ``tail'' element for managing the list, a ``current'' pointer by which
 * currency is maintained and other information pertinent to a list.
 *
 * The index information for the shell sort is kept in
 * a structure called a ``shellNode''.  This contains pointers to ``prev'' and
 * ``next'' items by which the nodes are kept in a list, a ``here'' pointer
 * that points to Links in a list, and a ``level'' array which is an array of
 * shellNodes and provides the basis for the high level indexing that makes
 * the shell sort efficient for adds and searches.
 *
 * There is a limitation to the sort algorithm provided here.
 * \index{sort!limitations}
 * This sort routine works better with data that is not random but is at least
 * partially ordered.  Specifically, if the data is at least partially sorted,
 * either forward or backward ordered,
 * the routine sorts the data more efficiently and the indexing is such that
 * searches are more efficient.  I am not clear on why this is the case, but it
 * can be shown emperically to be so fairly trivially.  Anyway, if you have an
 * application which needs to sort data and search through the data efficiently,
 * it is best if you can to massage the data a little to remove the randomness
 * if that is possible.  There are many ways to do this that can increase the
 * effectiveness of the sort routine.  One of those methods is displayed in
 * the shsort() routine, or more generally in the $topdir/tests/c/rough_sort.c
 * program.  These routines use a hash to initially sort the stream of strings
 * by the first two and first three characters, initially, to reduce randomness
 * in a manner that is very efficient.
 * 
 * The quick sort included here is very efficient.  It has eliminated recursion
 * and is typically 20-50% faster than system provided quick sorts.  There are
 * routines provided here to sort a list with a quick sort and return a shell
 * or a list.
 *
 * [EndDoc]
 */

/*
 *****************************************************************************
 * Debug stuff.
 *****************************************************************************
 */

/*
 * [BeginDoc]
 *
 * \subsection{Debug Code}
 *
 * Working with dynamic memory is difficult at times.  Basically, there is
 * considerable flexibility and power in being able to dynamically allocate
 * system memory off the heap, use it for programming tasks and then
 * deallocate it.  However, this power comes at the cost of greater complexity
 * and more responsibility on the part of the programmer to insure that the
 * heap data allocated is used ``properly'' (i.e., it is not over filled and
 * is properly returned back to the system when the program is finished with
 * it).  Bugs that are introduced into code by failing to adhere to strict
 * coding standards and perspectives on programming can be very difficult to
 * track down and can cause major disruption in production code.
 *
 * The following are a list of principles that are important to adhere to
 * when coding, in my opinion:
 *
 * \begin{itemize}
 *
 * \item A bug is a programming flaw created by the programmer that wrote the
 * code.  The word ``bug'' is actually a misnomer.  It connotes the idea that
 * an independent entity with volition of it's own somehow lodged itself into
 * the code and caused it to be broken.  David Thielen in his book,
 * ``No Bugs'' suggests that they should be called MFUs, which are loosely
 * translated as \emph{Massive Mess-Ups}.\footnote{
 * No bugs!: delivering error-free code in C and C++/David Thielen.
 * 1992 ISBN 0-201-60890-1}
 * The working mentality you need to develop as a programmer is that bugs are
 * in the code \emph{because you put them there}.  You, having put the
 * programming flaw there, have an obligation to find it and remove it.
 *
 * \item Debug code should be read only.  It should never alter data.
 * Debug code should \emph{never} effect the code path.  Debug code
 * should never fix problems or cause a different path to be taken than
 * is taken normally.  Debug code should be unobtrusive until it finds a
 * problem, and then it should glare at you.  It should not effect the program,
 * except possibly efficiency, unless it needs to notify you of something that
 * should be fixed.
 *
 * \item Use debug macros instead of having ``ifdef DEBUG'' everywhere wherever
 * possible.
 *
 * \item Take pains to insure that every code path is exercised.  Step through
 * new code and watch variables, function parameters, etc.  If something can
 * \emph{possibly} happen but usually doesn't, force it to during testing.
 * The aim is to try to break the code or make it mess up \emph{during testing},
 * not during use.  Whenever possible, devise a set of test cases that will
 * exercise every combination of entry data on a function.
 *
 * \item Put extra effort into ensuring that the low-level functionality is
 * solid.  This reaps benefits in debugging because you have more security
 * about the source of bugs.
 *
 * \item Debug test code does not need to be efficient.  The primary purpose of
 * using it is to debug your code.  If it is too slow, you can use conditional
 * defines to speed things up.  For example, only do a real slow test if
 * DEBUG >= 2.  If DEBUG == 1, do slow tests.  Otherwise, if DEBUG is defined,
 * do a fast test.  That way you can control how often really slow testing is
 * done.
 *
 * \item There are times in coding when you aren't sure how you should proceed.
 * For example, sometimes you write code that is dependent on some condition.
 * However, if the condition fails, the code will fail.  At times like that,
 * you will want to use a catchy word in a comment to bring it to your
 * attention.  Something like BUGBUG, FIXME or XXX.  Many good editors (like
 * vim) hilight many of those words specially in C code for you.  Then, before
 * you release the code, make sure all FIXME's or XXX's are resolved.
 *
 * \item Use prototypes extensively.  Compile your code with the highest
 * warning level turned on and \emph{heed} every warning.  Don't just do things
 * to shut off the warnings unless you \emph{know} what you are doing.  For
 * example, it may shut off a warning if you cast a void pointer to an integer;
 * it may also introduce bugs that are \emph{really} hard to track down in
 * running code.
 *
 * \item Get in the habit of placing debugging code in your program as you
 * write it and not after there are problems with it.  For example, if a
 * function parameter should never be `x', then use an assert to insure that
 * it is not `x' when testing, and then put an if statement in the code to
 * insure it is not `x'.  The assert will make the problem glaring during
 * testing and the non-debugging test will insure that it is caught in running
 * code.  Also, if you allocate memory and copy data into the buffer, do a
 * check_pointer() at that location to insure in testing that the buffer is
 * not overrun.  Finally, at the end of each program run where memory is to
 * be allocated and freed from the heap, use print_block_list() to insure that
 * you have no memory still allocated.  If the debugging code is already 
 * in your source, when you are ready to test, you just need to compile 
 * with DEBUG defined -- it will save you time during testing.
 *
 * \end{itemize}
 *
 * Wherever possible, the code herein uses these principles.
 *
 * \subsubsection{Assert}
 * \index{Assert}
 *
 * [Verbatim] */

#ifdef  DEBUG
#define Assert(b)       {if(!(b)) \
  printf ("Assert failed in file %s, line %d\n", __FILE__, __LINE__);}
#else
#define Assert(b)
#endif

/* [EndDoc] */
/* 
 * [BeginDoc]
 *
 * The ``Assert'' macro flags an error if ``b'' is not true, giving the source
 * file and the line number.  Asserts are useful in that they allow you to
 * show a condition that you want to be aware of in testing mode in a glaring
 * way.  Asserts should not be used as a substitution for checking, though.
 * For example, if a parameter to a function should never be NULL, assert that
 * it is not null and then check it in your code.
 *
 * [EndDoc]
 */

/*
 * [BeginDoc]
 *
 * \subsubsection{dbg}
 * \index{dbg}
 * [Verbatim] */

#ifdef  DEBUG
#define dbg(str)        str
#else
#define dbg(str)
#endif

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * dbg() is a macro that is used to wrap debugging code without having to 
 * use \emph{ifdef-endif's} everywhere.  dbg() only allows compilation of
 * what is inside it if DEBUG is defined.
 *
 * [EndDoc]
 */

/*
 * [BeginDoc]
 *
 * \subsubsection{dbg2}
 * \index{dbg2}
 * [Verbatim] */

#if     (DEBUG > 1)
#define dbg2(str)       str
#else
#define dbg2(str)
#endif

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * dbg2() is like dbg, except it allows what is inside it to be compiled only
 * if DEBUG is 2 or greater.
 *
 * [EndDoc]
 */

/*
 * [BeginDoc]
 *
 * \subsection{Dynamic memory debugging}
 * \index{dynamic memory!debugging}
 *
 * [Verbatim] */

#ifdef  DEBUG
#define DEBUG_FILL_CHAR         0xfe
#define DebugMemSet(p,n)        memset(p,DEBUG_FILL_CHAR,n)
#else
#define DebugMemSet(p,n)
#endif

#ifdef DEBUG

#define DBGMEM_HEADER           0xcefaedfeUL
#define DBGMEM_TRAILER          0xfecaefbeUL
#define SIZE_DEBUG_HEADER       (sizeof(DBGMEM_HEADER))
#define SIZE_DEBUG_TRAILER      (sizeof(DBGMEM_TRAILER))
#define SIZE_DEBUG                      (SIZE_DEBUG_HEADER+SIZE_DEBUG_TRAILER)

#undef malloc   /* in case they are system macros */
#undef calloc
#undef free
#undef realloc
#undef check_reference
#undef check_pointer
#undef print_block_list
#undef strdup

#define malloc(size)            mymalloc(size,__FILE__,__LINE__)
#define calloc(size,num)        mycalloc(size,num,__FILE__,__LINE__)
#define free(p)                         myfree(p,__FILE__,__LINE__)
#define realloc(p,size)         myrealloc(p,size,__FILE__,__LINE__)
#define strdup(s)                       mystrdup(s,__FILE__, __LINE__)
                /* check that p is pointing inside some block */
#define check_reference(p)      check_address(p,__FILE__,__LINE__)

                /* check that p is pointing to a block */
#define check_pointer(p)        check_valid(p,__FILE__,__LINE__,4)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The debug code included in the sort module has hooks to the standard
 * heap memory allocation routines used to help track down pointer errors.
 * If the code is compiled with DEBUG defined, the standard function calls
 * to malloc and company are changed to debugging versions of the routines.
 *
 * [EndDoc]
 */
void *mymalloc(int size,char *filename,int line);
void *mycalloc(int size,int num,char *filename,int line);
void myfree(void *p,char *filename,int line);
void *myrealloc(void *p,int newsize,char *filename,int line);
void print_block_list(void);
int check_valid(void *address,char *filename,int line,int type);
int check_address(void *address,char *filename,int line);

#else  /* not debugging; change some functions to nothing */

#define check_reference(p)      /* nothing */
#define check_pointer(p)        /* nothing */
#define print_block_list()      /* nothing */

#endif  /* ifdef DEBUG */

/*
 * File stuff
 */
#if defined(__linux__) || defined(__solaris__)
#define FILE_MODE       (O_RDWR)
#define FILE_PERMISSION (0644)
#endif
#if     defined (__CYGWIN__) || defined (WIN32)
#define FILE_MODE       (O_RDWR|O_BINARY)
#define FILE_PERMISSION (S_IREAD | S_IWRITE)
#endif

#if defined(__linux__) || defined(__solaris__) || defined(WIN32)
#define FILE_BEGINNING  SEEK_SET
#define FILE_END        SEEK_END
#define FILE_CURRENT    SEEK_CUR
#endif

#if defined(__CYGWIN__)
#define FILE_BEGINNING  SEEK_SET
#define FILE_END        SEEK_END
#define FILE_CURRENT    SEEK_CUR
#endif

/*
 * Common defines
 */
#ifdef  WIN32
#include "win32Util.h"
#endif

#ifndef TRUE
#define TRUE    1
#endif  /* TRUE */

#ifndef FALSE
#define FALSE 0
#endif /* FALSE */

#ifndef YES
#define YES             1
#define NO              2
#endif  /* YES */

#define _OK_    0
#define _ERROR_ -1

/*
 * Common macros and defines.  Don't document.
 */
#define iswhite(ch) ((ch)==' '||(ch)=='\t')

#ifdef WIN32
#define INLINE
#endif

#if defined(__GNU__) || defined(__linux__) || defined(__solaris__)
#ifndef INLINE
#define INLINE __inline__
#endif
#endif

#define MAX_SPLIT_LENGTH 1000000

#ifdef  __cplusplus
#define externC         extern "C" {
#define endC            }
#endif  /* __cplusplus */

#ifndef __cplusplus
#define externC
#define endC
#endif /* __cplusplus */

/*
 * [BeginDoc]
 *
 * \subsection{Error handling}
 * \index{sort!error hadling}
 * \label{ErrorHandling}
 *
 * In the sort module, error handling is accomplished via an error value which
 * \index{sort!error handling}
 * is an offset into an array of strings that describe the error.
 * There are three separate bodies of code for which error handling is
 * done, the file manipulation code, the list manipulation code and the sort
 * routine code.  Each of these has a separate enumerated list and global
 * variable.  The strength of this error handling methodology is that it is
 * fast.  The down side is that it is not thread safe in that one thread's
 * error information can be clobbered by another thread.
 *
 * \subsubsection{Error handling for file routines}
 *
 * For the file manipulation routines, there is an enumerated data type and
 * variables defined as follows:
 * [Verbatim] */

typedef enum __fio_error {
  FIO_NOERROR,            /* no file error */
  FIO_DENIED,             /* permission denied - EACCESS */
  FIO_TOOMANY,            /* too many open files - EMFILE */
  FIO_NOFILE,             /* file doesn't exist - ENOENT */
  FIO_BADFD,              /* bad file descriptor - EBADF */
  FIO_PROHIB,             /* operation prohibited - EAGAIN */
  FIO_DEADLK,             /* operation would cause a deadlock - EDEADLK */
  FIO_NOLOCK,             /* locking failed - ENOLCK */
  FIO_NOMEMORY,           /* memory error */
  /* Add new ones here */
  FIO_UNSPECIFIED         /* unspecified error */
} fioErrorType;

extern fioErrorType fioError;
extern char *fioErrMsg[];

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * ``fioError'' is a global variable of type ``fioErrorType''.  When an error
 * occurs in the file manipulation routines, fioError is set to the error type
 * and fioErrMsg[fioError] will provide a string that describes what went wrong.
 *
 * [EndDoc]
 */

/*
 * [BeginDoc]
 *
 * \subsubsection{Error handling for the list routines}
 *
 * Error conditions are communicated from the list routines using the same
 * mechanism as for the file system routines.
 * [Verbatim] */

/* type of error */
typedef enum __list_error_type {
  LIST_NOERROR,           /* no error */
  LIST_NOTIMPLEMENTED,    /* feature not implemented */
  LIST_UNKNOWN,           /* unknown list type */
  LIST_NOMEM,             /* memory error */
  LIST_INVALID,           /* invalid list */
  LIST_NOTEMPTY,          /* can't delete non-empty list */
  LIST_NULLCMP,           /* compare function cannot be NULL */
  LIST_PARAM,             /* bad function parameter */
  LIST_TREECORRUPT,       /* corrupted tree */
  LIST_UNIQUE,            /* can't add non-unique item to unique tree */
  LIST_FILE,              /* error opening file */
  /* Add new ones here */
  LIST_UNSPECIFIED        /* unspecified error */
} ListErrorType;

extern ListErrorType ListError;
extern char *ListErrorString[];

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * Again, if an error occurs, ``ListError'' is set to the error type and
 * ListErrorString[ListError] will provide the constant string that describes
 * the error.
 *
 * [EndDoc]
 */

/*
 * [BeginDoc]
 * \subsubsection{Error handling for the sort routines}
 *
 * The enumerated list and global errors for communicating exception conditions
 * from the sort routines are as follows:
 * [Verbatim] */

/* type of error */
typedef enum _shell_error {
  SHELL_NOERR,                /* no error */
  SHELL_NOMEM,                /* memory exhausted */
  SHELL_PARAM,                /* invalid function parameter */
  SHELL_LIST,                 /* list error */
  SHELL_CORRUPT,              /* corrupt data detected */
  SHELL_UNIQUE,               /* unique constraint violated */
  SHELL_LINK_MGR,             /* link manager error */
	SHELL_TH_CREATE,            /* error creating a thread */
	SHELL_TH_JOIN,              /* error joining a thread */
  /* Add new ones here */
  SHELL_UNSPECIFIED           /* unspecified error */
} shellError;

extern shellError shlError;
extern char *shlErrorStr[];

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The error handling for the sort routines is slightly more complex than that
 * for the file system and list routines.  Depending on the context in which
 * an error occurrs, the error information may be stored in the global
 * ``shlError'' variable or in the ``shlError'' variable that is part of the
 * shellHeader.  So, by way of example, assume your shellHeader is declared
 * as follows:
 * [Verbatim] *

shellHeader *shl;

 * [EndDoc] */

/*
 * [BeginDoc]
 *
 * Given that and depending on which shell manipulation function is being
 * called, the error will probably be set with shl->shlError.  If the error
 * occurs before there is a shellHeader, then obviously the global variable
 * will be set.  Regardless, if an error
 * occurs, ``shlError'' or ``shl->shlError'' is set to the error type indicated
 * in the enumerated list and shlErrorStr[shlError] or
 * shlErrorStr[shl->shlError] will provide a constant string
 * that describes the error, respectively.  Look at the documentation for
 * the shell function that you are calling to determine which variable to
 * look at for error handling.
 *
 * [EndDoc]
 */

/*
 *
 * List stuff
 */

/*
 * [BeginDoc]
 *
 * \subsection{List handling data structures, macros and constants}
 *
 * \index{list handling}
 * The data structures for the list handling routines are as follows:
 * \index{Link} \index{ListType} \index{ListHeader} \index{treeStore}
 * [Verbatim] */

/* Threshold for dividing a list - deprecated */
#define LIST_THRESHOLD  20

/* Default MAGIC number for list saves/gets */
#define DEFAULT_MAGIC   0xea37beefUL

/* type of list */
typedef enum __list_type {
  UNSORTED,
  SORTED,
  SLOWSORTED,
  STACKED,
  QUEUED,
  UNKNOWN_LIST_TYPE = (int) -1               /* insures int enum */
} ListType;

/* link structure */
typedef struct __link {
  struct __link *next;
  struct __link *prev;
  void *data;
} Link;

/* list header structure */
typedef struct __list_header {
  ListErrorType ListError;                  /* local ListError variable. */
  Link *head;
  Link *tail;
  Link *current;
  ListType type;
  size_t number;          /* number in this list */
  int (*compare) (const void *d1, const void *d2);
  struct __list_header *__n;
  struct __list_header *__p;
} ListHeader;

/* tree store structure */
typedef struct __tree_store {
  unsigned long thisMagic;        /* magic number - application */
                                  /*  specific */
  char timeStamp[27];             /* time stamp  - application */
  char description[128];          /* description field - app. */
                                  /*  specific */
  int isUnique;                   /* unique sorting? */
  int isCase;                     /* case sensitive? */
  int splitVal;                   /* number of items to split */
                                  /*  a node at */
  int number;                     /* number of items */
  int size;                       /* size of each item stored */
} treeStore;

/* [EndDoc] */
/*
 * [BeginDoc]
 * The ``ListType'' enumerated list specifies what kind of list is being
 * created.  This will effect how the items are added to and, possibly, removed
 * from the list.
 *
 * The ``Link'' structure is used in all the list management routines to
 * store data references to data.  The ``next'' and ``prev'' elements of
 * the Link structure are used to link the item into the list.  The 
 * ``data'' element points to the user's data item.
 *
 * The ``ListHeader'' structure is used for each list that is created.  The
 * initialization routine for lists allocates a list header and populates the
 * relevant fields.  This data item is then used in all calls to the list
 * management routines to indicate which list is being operated on.  The
 * ``__n'' and ``__p'' members of the ListHeader are used to track lists by the
 * list management routines.  The ``head'' and ``tail'' members of the
 * ListHeader structure are used to set beginning and ending markers for the
 * doubly linked list.  The ``current'' members of ListHeader will be used
 * to maintain currency for this list.  What the current item is will be
 * determined in the context of a list manipulation routine.  The ``type''
 * member shows what type of list this is.  The ``number'' member indicates
 * how many items are being contained in this list.  The ``compare'' routine
 * is used to determine the order of sort for sorted lists.
 *
 * Finally, the ``treeStore'' structure is used to store lists to disks.  This
 * is designed to accommodate the needs of routines like the shell sort
 * routines as well as just for lists.  The ``thisMagic'' member is used
 * to determine what kind of list this is.  This member can be changed to
 * meet the needs of the application.  The ``timeStamp'' member will create a
 * 26 character string that contains the date and time (to 100th of a second).
 * The ``description'' field allows an application to comment on what kind of
 * data is stored and is application specific.  The ``isUnique'' field is TRUE
 * if unique constraints are managed for the sort; FALSE otherwise.  The
 * ``isCase'' field is TRUE is case sensitivity is maintained and FALSE if
 * sorting is done without regard for case.  The ``splitVal'' member indicates
 * the threshold at which nodes are split (for a tree-like data structure).
 * The ``number'' field indicates how many data items are stored and the 
 * ``size'' member shows how big each data item is.
 *
 * [EndDoc]
 *
 */

/*
 * [BeginDoc]
 *
 * [Verbatim] */

#define isAtTail(lh)    ((lh)!=0&&(lh)->current==(lh)->tail)
#define isAtHead(lh)    ((lh)!=0&&(lh)->current==(lh)->head)
#define isEmptyList(lh) ((lh)!=0&&(lh)->head->next==(lh)->tail&&\
    (lh)->tail->prev==(lh)->head)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The ``isAtTail'', ``isAtHead'' and ``isEmptyList'' items were implemented
 * as macros for performance sake.  They are TRUE if, respectively, we are at
 * the tail of the list, we are at the head of the list or the list is empty.
 *
 * [EndDoc]
 */

/*
 * [BeginDoc]
 *
 * \subsection{Sort data structures, variables and constants}
 *
 * The following are used by the sort routines to manage internal information
 * and to determine how the sort routines function:
 * [Verbatim] */

#define MIDPOINT        5      /* BUGBUG: was 4 */
#define OUTPOINT        9      /* BUGBUG: was 8 */
#define NUMNODES        15     /* BUGBUG: was 15 */
#define RESTRUCT        9      /* BUGBUG: was 8 */

#ifdef STATS
extern double hln, zln, llt;
#endif

#define NODE_LEVEL 10

typedef struct _shell_node {
  struct _shell_node *prev;
  struct _shell_node *next;
  Link *here;
  struct _shell_node *level[NODE_LEVEL];
} shellNode;

typedef struct _shell_header {
  struct _shell_node *head;   /* head and tail */
  struct _shell_node *tail;
  struct _shell_node *current;/* current pointer */
  shellError shlError;        /* shell error for this shell */
  int numNodes;               /* number of nodes */
  int nodeLevel;              /* level of nodes */
  int isUnique;               /* is this index unique? */
  int manageAllocs;           /* are we managing allocations? */
  ListHeader *lh;             /* list of data */
  int numCompares;            /* number of comparisons */
  int thresh;                 /* threshold for restructuring */
  int numRestruct;            /* number of times this shell */
                              /*   has been restructured */
  int (*compare) (void *, void *); /* compare */
} shellHeader;

typedef struct _shell_level {
  int lvl[NODE_LEVEL];
} shellLevel;

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * MIDPOINT, OUTPOINT, NUMNODES and RESTRUCT constants effect the way the sort
 * algorithm splits and maintains the node structure.  These have been set via
 * testing to be optimal. It is not recommended that you adjust these without
 * knowing what you are doing. Also, you should make changes based on actual
 * testing.
 *
 * The ``shellNode'' data item contains information about the nodes for a
 * sorted list.  An explanation of how the shell sort sets up the data
 * structures is in order at this point.  Basically, the data that is being
 * sorted in a shell is kept in a list.  This allows us to easily traverse the
 * sorted data (starting at the head and traversing forward, starting at the
 * tail and traversing backwards, or starting somewhere within the list and
 * traversing in either direction).
 *
 * The ``shellNode'' structure is used to create an index into the sorted list.
 * So, for example, there may be a shellNode item pointing to every third or
 * fourth item in the sorted list.\footnote{This is an over simplification, but
 * not one that is impossible or even improbable, especially after 
 * restructureShellNodes() has been called.}  This index is itself a linked
 * list with ``head'', ``tail'' and ``current'' pointers maintained in the
 * ``shellHeader'' structure.  The ``prev'' and ``next'' members of the
 * shellNode structure are used to link the nodes together and the ``here''
 * member is used to point to the link in the sorted list that this node points
 * too.
 *
 * The ``level'' array in the node structure is used to construct a tree that
 * speeds up searches and adds to the sorted list.  When the list of shellNode
 * items gets to a certain threshold, the level[0] value of the first shellNode
 * in the node list gets populated.  This is set to the pointer to another node
 * such that each level[0] item points to every third or so shellNode in the
 * node list.\footnote{Again, this an over simplification.}  Then, when the
 * number of level[0] items gets to a certain threshold, the level[1] item
 * of the first shellNode in the node list gets populated.  This is set to
 * the pointer to another node such that each level[1] item points to every
 * third or so level[0] item.  This continues on up the levels as items are
 * added to the sorted list.
 *
 * So, to search a shell, the code needs to look at the first shellNode in the
 * node list, find the highest level that is not NULL and use that as the root
 * of the tree to begin traversal.  This is also the same procedure that is
 * used when inserting an item into the shell.  For more details on searching
 * or inserting into a shell, see the source code that performs these functions.
 *
 * The ``shellHeader'' structure is used to manage a shell.  All shell routines
 * expect as one of their arguments a shellHeader item.  As shells are used
 * the items in the shellHeader are updated to reflect the current state of the
 * sorted list.  The ``numNodes'' member of the shellHeader gives the number of
 * shellNodes in the node list for the shell.  The ``nodeLevel'' member shows
 * the highest level in the node ``tree'' that is being used.  The ``isUnique''
 * member is TRUE if unique constraints are being maintained, FALSE otherwise.
 * The ``manageAllocs'' member if TRUE if the shell code is being expected to
 * free the links and the data items that are added to the shell.  Otherwise,
 * the end user is responsible to free these.  The ``lh'' member is the actual
 * list of sorted items.  The ``compare'' member is a pointer to a function
 * that is used to determine the order of sorted items in the list.
 *
 * The ``shellLevel'' structure is used to gather statistics about the nodes
 * for a shell that is being populated or has been populated.  It is used
 * in calls to ``nodeLevels''.
 *
 * [EndDoc]
 */

/*
 * [BeginDoc]
 *
 * \subsection{Hash functions}
 *
 * The hash functions here are provided to give users access to the fast
 * search capabilities of a hash using a relatively small pool of hash
 * buckets where each hash bucket is a sorted shell.
 * \footnote{Note: no attempt is made here to define a hash search.  Please
 * refer to other documentation on the internet for a defnition.}
 * The hash functions are
 * relatively simple with simplifying assumptions built in.  For example,
 * duplicate items are not allowed in a hash.  Also, it is assumed that the
 * user will be using the hash functions for strings solely and not other
 * data types.  If the user needs this capability with duplicates or with
 * data types other than C strings, it would be fairly trivial to roll their
 * own using what I have done here as a template.
 * 
 * The advantage of this set of routines is that it is designed to handle
 * a considerable number of hash items.  If you have fewer than 100,000 items
 * or so, to hash, it is probably better to use another hash search 
 * implementation - this is probably overkill.  Also, there is no capability
 * for storing a hash to disk.  That could be implemented with some work 
 * using functionality already designed for storing shell sorts, although
 * you would have to impose a size on the items being hashed.
 *
 * The following is the definition of a ``hashHeader''\index{hashHeader}, the
 * primary data type of a hash:
 *
 * [Verbatim] */

#define HASH_SIZE 151

typedef struct _hash_header {
  int number;                          // Number of items in the hash buckets.
  int numCompares;                     // Number of compares.
  unsigned int (*hash) (const char *); // Hash function.
  shellHeader *shls[HASH_SIZE];        // Array of shell headers.
} hashHeader;

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The ``hash'' item is a pointer to a function that will do the actual
 * hashing of the strings passed to it.  If you pass NULL to the initHash
 * function, a basic hash designed and proven to be effective for strings
 * will be used.  The ``shls'' item is an array of shells that actually
 * store the hashed items.  These are the buckets and collisions are
 * handled by storing items within the shells as sorted, searchable lists.
 *
 * [EndDoc]
 */

/*
 * @DocInclude ../lib/sort.c
 */

/*
 * Prototypes
 */

/*
 * fexists
 */
int fexists (char *fname);

/*
 * Time function
 */
int elapsed (double *sec);

/*
 * Split function
 */
char **split (char *ln, int ch);
int split_r (char **cpp, int maxnum, char *ln, int ch);

/*
 * File functions
 */
void set_fioError (fioErrorType er);
#ifndef DEBUG
int fileCreate (const char *file);
int fileOpen (const char *file);
int fileClose (int fd);
#endif
#ifdef  DEBUG
int debug_fileCreate (const char *file,const char *f,int l);
int debug_fileOpen (const char *file,const char *f,int l);
int debug_fileClose (int fd,const char *f,int l);
#define fileCreate(f)   debug_fileCreate(f,__FILE__,__LINE__)
#define fileOpen(f)     debug_fileOpen(f,__FILE__,__LINE__)
#define fileClose(f)    debug_fileClose(f,__FILE__,__LINE__)
#endif
__inline__ off_t fileSeekBegin (int fh, off_t offset);
__inline__ off_t fileSeekEnd (int fh, off_t offset);
__inline__ off_t fileSeekCurrent (int fh, off_t offset);
__inline__ off_t fileWrite (int fh, void *buf, size_t len);
__inline__ off_t fileRead (int fh, void *buf, size_t len);
__inline__ int fileFlush (int fh);
__inline__ int fileLockRegion (int fh, int cmd, int type, size_t offset, int whence,
                size_t len);
__inline__ pid_t fileLockTest (int fh, int type, size_t offset, int whence,
                size_t len);
__inline__ int fileRemove (const char *fname);
__inline__ int fileRename (const char *oldname, const char *newname);
size_t fileWriteBlock (int fh, off_t where, size_t reclen, ListHeader *lh);
ListHeader *fileReadBlock (int fh, off_t where, size_t reclen, size_t numrecs);

/*
 * List functions
 */
void set_ListError (ListHeader *lh, ListErrorType le);
ListHeader *initList (ListType lt, int (*cmp) (const void *, const void *));
int delList (ListHeader *lh);
int insertLink (ListHeader *lh, Link *lnk);
__inline__ int insertLinkHere (ListHeader *lh, Link *lnk);
Link *removeLink (ListHeader *lh);
__inline__ int nextLink (ListHeader *lh);
__inline__ int prevLink (ListHeader *lh);
__inline__ int firstLink (ListHeader *lh);
__inline__ int lastLink (ListHeader *lh);
Link *searchLink (ListHeader *lh, Link *lnk);
Link *searchList (Link *lnk, int number, Link *begin, Link *end, 
    int (*cmp)(const void *, const void *));
int clearList (ListHeader *lh);
Link *findSortedInsert (Link *lnk, int number, Link *begin, Link *end,
    int (*cmp)(const void *, const void *));
int storeList (ListHeader *list, treeStore *ts,
    int reclen, const char *fname);
int saveList (ListHeader *lh, const char *desc, int size,
    const char *file);
ListHeader *retrieveList (treeStore *ts, const char *fname,
    long magic);
ListHeader *getList (const char *file);
ListHeader *mergeSorted (ListHeader *l1, ListHeader *l2);

#ifdef  DEBUG
int DebugCheckList (ListHeader *lh);
#else
#define DebugCheckList(lh)
#endif

/*
 * Sort functions
 */
void set_shlError (shellHeader *shl, shellError er);
shellHeader *initShell (int (*compare) (void *, void *),
      int isUnique, int manageAllocs);
int delShell (shellHeader * shl, void (*delFunc) (void *));
int addShellItem (shellHeader * shl, Link * lnk);
int newShellNode (shellHeader * shl, shellNode * prevNode,
                    shellNode * nextNode, Link * here);
Link *findShellItem (shellHeader * shl, Link * lnk);
Link *queryShellItem (shellHeader * shl, Link * lnk);
int storeShell (shellHeader * shl, treeStore * ts, int reclen,
                  const char *fname);
int saveShell (shellHeader * shl, const char *desc,
               int reclen, const char *fname);
int retrieveShell (shellHeader * shl, treeStore * ts, const char *fname,
                     long magic);
shellHeader *getShell (int (*compare) (void *, void *), const char *fname);
int restructureShellNodes (shellHeader * shl);
Link *removeShellItem (shellHeader * shl, Link * lnk);
int nodeLevels (shellHeader * shl, shellLevel * lvl);
shellHeader *shlQsort (void *base, size_t numItems, size_t recSize,
                         int (*qcompare) (const void *, const void *),
                         int (*scompare) (void *, void *));
shellHeader *shlListQsort (ListHeader * lh, size_t recSize,
                             int (*compare) (const void *, const void *));
ListHeader *shl2List (shellHeader * shl);
shellHeader *list2Shell (ListHeader * lh);
void returnClean (void *p);
int shsort (void **p, size_t num, int (*cmp)(void*p1,void*p2));
int thsort (void **p, size_t num, int (*cmp)(void*p1,void*p2));

#define mergeShells(sh1,sh2)    \
        list2Shell(mergeSorted(shl2List(sh1),shl2List(sh2)))

#ifdef  DEBUG
  void debugCheckShell (shellHeader * shl);
#endif
#ifndef DEBUG
#define debugCheckShell(shl)
#endif

int bqsort (void *base_ptr, int total_elems, int size,
    int(*cmp)(const void*,const void*));

/*
 * Hash functions.
 */
hashHeader *initHash (int (*compare) (void *, void *),
    unsigned int (*hash) (const char *));
int addHashItem (hashHeader *hsh, const char *item);
char *findHashItem (hashHeader *hsh, const char *item);
int delHash (hashHeader *hsh);


#ifdef __cplusplus
}
#endif

#endif /* __SORT_H__ */
