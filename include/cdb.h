/*	Header File:	cdb.h	*/
/*
 * To create documentation:
 * 
 * ../bin/ldoc -o -f cdb.tex -t "DCDB database engine" -a "by David F. May" cdb.h
 */

#ifndef	__DCDB_H__
#define	__DCDB_H__

#include <sort.h>

externC

/*
 * @DocInclude ../include/disclaim.tex
 * 
 * [BeginDoc]
 * 
 * \section{Introduction}
 * 
 * This is DCDB (David's C DataBase engine).  
 * It is a database engine designed around the Indexed
 * Sequential Access Method (ISAM).  A valid question at this point
 * \index{ISAM} \index{DCDB}
 * could be, ``Why another database application for Linux?''.  There
 * are numerous projects that are open source and that meet the needs
 * of various segments of Linux users.  However, there is a hole in
 * the area of database engines that are open source and that allow
 * functionality similar in scope to CodeBase (TM).  The following are
 * some things about the DCDB database engine that set it apart from other
 * database products.
 * 
 * \begin{enumerate}
 * 
 * \item DCDB is not relational.  There is no built in functionality to
 * maintain relations between table items.  If you need such functionality
 * for your applications, you may want to consider Typhoon or some other
 * such open source project.
 * 
 * \item DCDB is not an SQL database.  If you need a good SQL database, you
 * would be better served by one of the excellent open source SQL database
 * projects currently available, like MySQL or PostgreSQL.
 * 
 * \item DCDB is not an xBase database engine.  There is no attempt in DCDB
 * to support the xBase file format, which in my estimation is a limiting
 * format.  The file format of DCDB tables and indexes is totally unique
 * to DCDB, as far as I know.
 * 
 * \item DCDB is not very feature rich, although it does have features that
 * have made it useful to me in my needs for database functionality.  It is
 * fast and has been tested extensively under conditions on all the supported
 * platforms.
 * 
 * \item DCDB is open source.  Other database engines, like C-ISAM or
 * CodeBase
 * are large and feature rich.  They are documented well and provide an
 * excellent value if you can afford them.  However, they are expensive,
 * even more so if they support multiple platforms.  DCDB is designed to
 * be totally free from the ground up.
 *
 * \item DCDB is designed to be embedded into scripting languages, like TCL/Tk
 * or Perl.  That way, you can take a scripting language that you are somewhat
 * familiar with, tack on the DCDB bindings and have a simple database
 * component to the language.  This gives you the ability to manipulate tables
 * and indexes from a language you know using a fairly fast database engine.
 * 
 * \end{enumerate}
 * 
 * The goals of the DCDB project are as follows (in order of priority):
 * 
 * \begin{enumerate}
 * 
 * \item Stability.
 * 
 * Priority number one is to insure stability in DCDB.
 * Bug fixes will take priority over any feature enhancements.  Debugging
 * directives that allow us to check for memory leaks, dangling pointer
 * problems, etc. have been coded right into the system at all levels to
 * help the programmer insure stability in final applications.  The error
 * checking and reporting at all levels of the code is extensive (if not
 * exhaustive).  Also, the base functionality of the code has been tested
 * extensively.  The tests were coded and implemented at the time that the
 * various features were implemented, so that further features could be
 * guaranteed to not produce problems with current functionality.  Finally,
 * the table maintenance code is designed to store data relatively safely
 * without compromising performance.
 * 
 * \item Performance.
 * 
 * After stability, performance is of top priority.  I want a database
 * engine that is fast and streamlined.  The fat has been trimmed from as
 * much of the base functionality as possible.  DCDB is built upon very
 * little in the way of underlying code; a handful of modules make up
 * the base functionality upon which DCDB is built.
 * Various parameters (the number of
 * cached blocks in indexes, for example) are determined based on extensive
 * testing.  Also, various sorting algorithms were tested to determine which
 * would be preferable for indexing.  The indexing algorithm uses a split 
 * index file
 * (in-memory indexes to blocks of index information).  This is sort of a 
 * combination of a hash and B+tree, giving you the performance of a hash
 * with the advantage of sequential, ordered traversal.  It was implemented
 * for performance.
 * 
 * \item Well documented.
 * 
 * Open source projects tend to be difficult to use because of poor
 * documentation.  With the DCDB project, it is hoped that this trend can
 * be reversed somewhat.  I aim to carefully document in the code itself
 * the features of the database engine.  Also, I aim to provide samples
 * that are well documented and that allow the end user to get some feel for
 * how to use DCDB.  These will be fairly simple applications that are
 * instructive in scope.
 * This should minimize the time that it takes a user to
 * get up and working with DCDB.
 * 
 * \item Future goals\ldots
 * 
 * I would like to make DCDB multi-threaded at some point.  Also, I should
 * probably add a locking mechanism that is a little more advanced than file
 * locking (although this has worked well with serial programs, etc.).
 * I think the capability to open a table and index in read-only mode could
 * be useful for certain applications (like a database on a CD-ROM).
 * Finally, I need to redesign the index to provide for reasonable record
 * update capability.
 * 
 * \end{enumerate}
 * 
 * [EndDoc]
 */
#ifndef	QUICKINDEX
#include <index.h>
#endif
#ifdef	QUICKINDEX
#include <qindex.h>
#endif

#ifdef	__DJGPP__
#pragma pack(1)
#endif

/*
 * [BeginDoc]
 * 
 * \section{DCDB constants and types}
 * 
 * There are numerous constants that determine how DCDB works.  Most of
 * these are arbitrary values that can be changed by the user to enhance
 * the functionality of DCDB (like MAX_IDX, which is the maximum number
 * of indexes for a table).  Others are based on testing and should be
 * confirmed through experimentation before changed (like INDEX_BLOCK_SIZE,
 * the default index block size).  Those that were derived from testing
 * have ``tested'' in a comment field beside them.
 * 
 * [EndDoc]
 */
#define TIME_STAMP_WIDTH    26
#define TABLE_INFO_WIDTH    125
#define TABLE_NAME_WIDTH    115
#define SYNTAX_ERROR_WIDTH  512
#define MAX_FIELD_WIDTH     64
#define TABLE_FLUSH         10000
#define MAX_IDX             20
#define MAX_MIDX            4
#define MAX_FIELD           50
#define MAX_ERROR_STRING    128
#define MAX_WS_TABLES       32

#define INDEX_BLOCK_SIZE    512  /* tested */
#define TABLE_BUFFER_SIZE   100  /* tested */

#define	_NOTFOUND_			0
#define	_FOUND_				1

/*
 * [BeginDoc]
 * 
 * \subsection{Error handling}
 * 
 * Errors are communicated to the user in DCDB by way of a few global
 * variables and some error handling information in the table structure.
 * However, error handling is simplified by a few macros that decay
 * to tests of the error variables for exception conditions.  The
 * constants used in error handling in the DCDB routines are as follows:
 * 
 * [Verbatim] */

enum __dberror_type {
	DB_NOERROR,			/* no problema */
	DB_IO_DENIED,		/* I/O Error - FIO_DENIED */
	DB_IO_TOOMANY,		/* I/O Error - FIO_TOOMANY */
	DB_IO_NOFILE,		/* I/O Error - FIO_NOFILE */
	DB_IO_BADFD,		/* I/O Error - FIO_BADFD */
	DB_IO_READWRITE,	/* read or write not complete */
	DB_IO_PROHIB,		/* I/O Error - FIO_PROHIB */
	DB_IO_DEADLK,		/* I/O Error - FIO_DEADLK */
	DB_IO_NOLOCK,		/* I/O Error - FIO_NOLOCK */
	DB_NOMEMORY,		/* memory Error */
	DB_INVALIDFILE,		/* invalid cdb table or index */
	DB_LISTERROR,		/* error from the list level */
	DB_FIELD,			/* field error */
	DB_INDEX,			/* index error */
	DB_CREATEINDEX,		/* couldn't create the index */
	DB_UNSTABLE,		/* memory elements are corrupted */
	DB_UNIQUE,			/* unique index constraint violated */
	DB_TOOMANYIDX,		/* too many indexes for this table */
	DB_PARAM,			/* bad function parameter */
	DB_SHELL,			/* error from the shell level */
	DB_LOCK,			/* table locked */
	DB_PARSE,			/* error from the parser */
	DB_SYNTAX,			/* syntax error */
	DB_INPUT,			/* bad input */
	/* put new ones here */
	DB_UNSPECIFIED		/* unspecified error */
};

typedef enum __dberror_type dbErrorType;

extern dbErrorType dbError;

extern char *dbErrMsg[];	/* error messages */

#define	isTableError(t) (!(t->dbError==DB_NOERROR&&dbError==DB_NOERROR))
#define isDBError()		(dbError != DB_NOERROR)

/* [EndDoc] */

/*
 * [BeginDoc]
 * 
 * \label{ErrorHandling}
 * 
 * Errors can occur in two contexts in the DCDB routines.  There are
 * errors which occur during the creation, opening or closing of a table
 * and during use of a workspace.  These areas are considered to be
 * global in scope (not table specific) because we don't have
 * a valid table descriptor to attach the error to.  However, other
 * errors in the DCDB routines are table specific and therefore can
 * and should be attached to the table they occurred for.  In that case,
 * the table descriptor will have members that show error information.
 * 
 * If the error occurs in a global context,
 * the error will be kept in ``dbError''.  The message pertaining to the
 * error would be available in ``dbErrMsg[dbError]''.  If, however, the
 * error is table specific, the error will be kept in ``tbl->dbError''
 * and ``dbErrMsg[tbl->dbError]'' will point to a message describing the
 * error.
 * 
 * The problem here is that this is
 * not easy to remember.  Therefore global errors can be checked by using the
 * macro ``isDBError()''.  This will be TRUE if an error has occurred in a
 * global context.  A description of the error can be gotten by calling
 * the function ``dberror()''.
 * If the error is table specific, the macro
 * ``isTableError(tbl)'' will be TRUE and the user can get a description
 * of the error with a call to ``dbtblerror()''.
 * You should be aware that if an error occurs, any further calls to DCDB
 * functions will not work correctly, as all DCDB routines do consistent
 * error checks during processing.  Therefore, if an error occurs, you
 * \emph{must} call dberror() or dbtblerror(), depending on the context,
 * to clear the error condition before you can call DCDB routines to
 * close tables and indexes by way of cleanup.
 * [EndDoc]
 */

/*
 * [BeginDoc]
 * 
 * \subsection{DCDB types}
 * 
 * The basic structure of the cdb module is the dbTable structure.
 * This is the data type that facilitates the management of data
 * in the table.  The first part of this structure is a header,
 * which is saved to the table on disk directly as is.  This
 * header is of type dbHeader, which is defined in cdb.h.
 * The second member of the dbTable structure is an array of field
 * structures of type dbField.  These are kept in the table on
 * disk as well.  The rest of the fields in the dbTable structure
 * are used for managing a table in memory.
 * 
 * The format on disk of a cdb table is as follows:
 * 
 * [Verbatim]

|--------|--------|--------|------ ... |--------|---- ...
header   field 1  field 2  field 3 ... field n  records...

 * [EndDoc]
 * 
 * [BeginDoc]
 * There are no special delimiters between parts of the table.
 * The first 4 bytes of the table will be a magic number, #defined
 * as DB_MAGIC in cdb.h.
 * [Verbatim] */

/*
 * Header information.  This is the first thing stored in a 
 * data file.
 */
#define	DB_MAGIC	0xbeeecaafUL	/* be careful */

struct __db_header {
	unsigned long magic;               /* magic number - */
                                           /*   denotes one of ours */
	char timeStamp[TIME_STAMP_WIDTH+1];/* date and time */
                                           /*   of last update */
	char tableInfo[TABLE_INFO_WIDTH+1];/* information */
                                           /*   specific to the */
                                           /*   application */
	char midxInfo[MAX_MIDX][TABLE_INFO_WIDTH*2+1]; 
                                           /* multi-field index information */
	int nextSequence;                  /* next sequence number */
	int numFields;                     /* number of fields in the table */
	unsigned long numRecords;          /* num of records in the table */
                                           /*   currently */
	size_t sizeFields;                 /* size of field descriptions */
                                           /*   in the table */
	size_t sizeRecord;                 /* size of each record */
};

typedef struct __db_header dbHeader;

/* [EndDoc] */

/*
 * [BeginDoc]
 * The ``timeStamp'' member is just the result from a call to 
 * asctime().  The ``tableInfo'' member is anything the application 
 * wants it to be.  The ``midxInfo'' member is how the DCDB library
 * keeps track of multiple index information.
 * The ``nextSequence'' member is provided to allow
 * users to have a sequence for any given table.  This is stored in the
 * header so the management routines can make sure that a unique number
 * is provided to the user each time.  For example, if your application
 * requires a unique purchase order number for each document it
 * tracks, you could initialize tbl->nextSequence to 1.  Then, when a
 * document is created, you could use this to derive a purchase order number
 * (like P00000001) and then increment the next sequence.  This gives you
 * the ability to create a unique number for each purchase order and
 * makes sorting by purchase order number reasonable.  The nextSequence
 * information is stored in the table header, so it will be that same
 * value after you close the table and reopen it.
 * 
 * The ``numFields'' member will 
 * contain the number of fields in the table.  The ``numRecords'' 
 * member is the number of records the table currently contains.
 * The ``sizeFields'' member contains the size of all the field 
 * headers in the table combined.  So, the size of the dbHeader plus 
 * the sizeFields member will give the user the offset in the file 
 * where records are stored.  The ``sizeRecord'' member will contain the
 * size of each record in the table.  The DCDB module stores records 
 * in a table in constant width fields that are padded with 0's
 * (binary 0's, not '0's).  So if we move to the first record in the 
 * table and retrieve sizeRecord bytes from it, we will get all the 
 * data from the first record.  The first byte of the record is
 * a character indicator of whether the record is deleted or not.
 * 
 * [EndDoc]
 */

/*
 * [BeginDoc]
 * 
 * The dbField structure is defined as follows:
 * [Verbatim] */

typedef enum _index_type {
	ITYPE_NOINDEX,			/* no index */
	ITYPE_UNIQUECASE,		/* unique index, case */
	ITYPE_UNIQUENOCASE,		/* unique index, no case */
	ITYPE_DUPCASE,			/* dups allowed, case */
	ITYPE_DUPNOCASE,		/* dups allowed, no case */
	/* Add new ones here */
	ITYPE_LAST
}	indexType;

typedef enum _field_type {
	FTYPE_NONE,				/* no field type */
	FTYPE_CHAR,				/* character */
	FTYPE_NUMBER,			/* number */
	FTYPE_LOGICAL,			/* logical */
	FTYPE_DATE,				/* date */
	FTYPE_TIME,				/* time stamp */
	/* Add new ones here */
	FTYPE_LAST
}	fieldType;

/*
 * Field information.  One of these is stored for each field.
 */
struct __field_descriptor {
	char fieldName[MAX_FIELD_WIDTH+1];/* name of field */
	char indexName[INDEX_NAME_WIDTH+1]; /* name of */
							/*   index file for */
							/*   this field */
	fieldType ftype;		/* type of field */
	indexType indexed;		/* type of index */
	int indexBlkSize;		/* block size of the index */
	int validated;			/* TRUE => validated */
	int fieldLength;		/* total length of the field */
	int decLength;			/* length of the decimal part, */
							/* including the */
							/*  decimal (if applicable) */
};

typedef struct __field_descriptor dbField;

/* [EndDoc] */

/*
 * [BeginDoc]
 * 
 * The ``fieldName'' member is the name of the field. 
 * The ``fieldType'' member is one  of character string, 
 * floating point numerical data,  logical/boolean value
 * (can be one of ``tTyYfFnN''), date (an 8 digit date kept in
 * YYYYMMDD format), and a time stamp as returned by
 * asctime().  Note: See ``indexType'' above for the actual
 * constants used.
 * There is no support
 * for a memo field.  If the user requires storage of textual
 * information, a more portable way is to create a table to save
 * the text a line at a time.  The ``indexed'' member will indicate
 * the type of index applied to this field, if applicable.  
 * You have five possibilities with indexes.  The item may not be
 * indexed.  If it is, DCDB supports unique indexes (as well as
 * non-unique indexes) and case sensitive/insensitive indexes.  The
 * possibilities are as follows:
 * 
 * \begin{itemize}
 * 
 * \item no index (which means this field isn't indexed)
 * 
 * \item unique, case sensitive index
 * 
 * \item unique, no case index
 * 
 * \item duplicates allowed, case sensitive index, 
 * 
 * \item duplicates, case insensitive index.
 * 
 * \end{itemize}
 * 
 * Note: See ``fieldType'' above for the actual constants used.
 * 
 * There is also support for multiple field indexes.  These are
 * indexes which contain sorted information for 2 or more fields.
 * The field items are concatenated together and indexed.  The
 * information for multiple fields is kept in the table header
 * instead of field records.
 * 
 * The ``validated'' field indicates whether validation should be
 * applied to this field before data is entered to the table.
 * This is currently not supported; however, it may be.  The idea
 * is that you can insure that a field only gets certain pre-defined
 * values.  For example, if you have a field for tracking part
 * numbers in a table, you would want to insure that only certain
 * part numbers are entered in that table.  Therefore, you would
 * set up another table to validate this part number field and
 * the system wouldn't allow any but valid data to be entered.
 *  ``fieldLength'' is the total length of this
 * field (not including the nul byte).  ``decLength'' is the length
 * of the decimal part, if applicable.  For all but NUMBER fields,
 * this will be 0.
 * 
 * [EndDoc]
 */

/*
 * Query information - currently not supported.
 */

#ifdef	__DJGPP__
#define	CACHE_SIZE	200
#endif

#ifdef	__linux__
#define	CACHE_SIZE	200
#endif

#ifdef	__BORLANDC__
#ifndef	_Windows
#define	CACHE_SIZE	50
#else	/* _Windows */
#define	CACHE_SIZE	200
#endif	/* _Windows */
#endif

enum __db_condition_type {
	DB_NO_CONDITION,			/* no condition - for initialization */
	DB_EQUAL,					/* values are equal */
	DB_LESS,					/* values are less */
	DB_LESSEQUAL,				/* values are less than or equal */
	DB_GREAT,					/* values are greater than */
	DB_GREATEQUAL,				/* values are greater than or equal */
	DB_INCLAUSE,				/* in clause */
	DB_BETWEEN,					/* between clause */
};

typedef enum __db_condition_type dbCondType;

/*
 * [BeginDoc]
 * 
 * The dbTable structure is declared as follows in cdb.h:
 * [Verbatim] */

struct __db_table {
	int fd;				/* file descriptor for table */
	dbHeader *hdr;			/* header information */
	dbField **fldAry;		/* array of field descriptors */
	char **fieldNames;		/* names of the fields in the */
					/*   table */
	char *fileName;			/* name of file table is in */
	off_t offset;			/* current offset into the */
					/*   file from 0 */
	off_t crec;			/* current record */
	dbErrorType dbError;		/* error information */
	char *dbErrMsg;			/* error message for the */
					/* current error */
	int bof;			/* at beginning? */
	int eof;			/* at end? */
	char *data;			/* storage for field entry on */
					/*   retrieve */
	char **fields;			/* pointers to storage for */
					/*   entering */
					/* a field */
	int *flens;			/* lengths of each field */
	int *declens;			/* decimal length for each */
					/* field, including the decimal. */
	dbIndex *current;		/* current index */
	ListHeader *idxList;		/* index list */
	ListHeader *midxList;		/* multi-field index list */
	/* the following are currently not supported */
	dbCondType *ctype;		/* condition type for queries */
	ListHeader *entryCache;		/* cache for entries */
	ListHeader *searchCache;	/* cache for searches */
	ListHeader *queryCache;		/* cache for queries */
	char *betweenTop;		/* high value for a between */
					/*   clause */
	ListHeader *inList;		/* list for "in" clause */
};

typedef struct __db_table dbTable;	/* table type */

/* [EndDoc] */

/*
 * [BeginDoc]
 * The ``fd'' member contains the file descriptor for the open table.
 * ``hdr'' is the table header (type dbHeader) and ``fldAry'' is an 
 * array of fields (type dbField) with a NULL pointer at the end
 * of the array.  ``fieldNames'' is an array of character pointers
 * that will point to the names of the fields in ``fldAry''.
 * These should be considered read-only;
 * they point to the fieldName member of each item in ``fldAry''.  The 
 * ``fileName'' member is the name of the file on disk.  The ``offset''
 * member will contain the offset in bytes of the start of the current 
 * record and ``crec'' contains the current record number less one.
 * ``dbError'' will contain the error code in the event of an error (see
 * error handling above).
 * ``bof'' indicates the table pointer is at the beginning of
 * the file and ``eof'' indicates it is at the end.  These are used
 * in sequential access of the table; if the user goes off of the
 * beginning or end of the file, bof or eof is set, respectively.
 * 
 * The ``data'' member is storage the size of a record on disk,
 * for moving data between memory and disk.  ``fields'' is an 
 * array of character pointers with enough storage for each field.
 * ``flens'' is an array of integers that corresponds to the fields
 * array.  The size given by ``flens'' should be seen as the maximum
 * size of string to copy to the fields item, so that
 * [Verbatim]

strlen (tbl->fields[i]) <= tbl->flens[i]

 * [EndDoc] */

/*
 * [BeginDoc]
 * ``current'' is the
 * current index, if there is a current index, for the table.
 * ``idxList'' is a ListHeader for the index list if there are
 * indexes open for this table.  ``midxList'' is a list that holds the
 * multi-field indexes for the table that are currently open, if 
 * there are any.
 * 
 * [EndDoc]
 */

/* internal data structure */
typedef struct _midx_field {
	char indexName[INDEX_NAME_WIDTH+1];
	int blkSize;
	int isCase;
	int isUnique;
	char *names[MAX_IDX];
}	midxField;

/*
 * [BeginDoc]
 * 
 * Support for workspaces has been added to the DCDB library to facilitate
 * the grouping of tables together in a logical way.  The user can
 * create the tables that are all part of a single application, for example,
 * set default current indexes for each table, and add them to the workspace.
 * Then, whenever a workspace is closed, all the tables that are managed by the
 * workspace are closed.  Whenever it is opened, all the tables that are
 * grouped together by it are opened.  This is just a convenience for the
 * user.  The following structures are used to manage workspaces in the DCDB
 * library:
 * [Verbatim] */

/*
 * Workspace structures
 */
typedef struct _ws_header {
	unsigned long magic;						/* magic number */
	char wsName[TABLE_INFO_WIDTH+1];	/* WS name */
	int numWS;							/* number of tables */
}	wsHeader;
	
typedef struct _ws_field {
	int fieldNum;
	dbTable *tbl;
	char tableComment[TABLE_INFO_WIDTH+1];
	char tableName[TABLE_NAME_WIDTH+1];
	char currentIndex[INDEX_NAME_WIDTH+1];
}	wsField;

typedef struct _work_space {
	wsHeader *hdr;
	int number;
	char fileName[TABLE_NAME_WIDTH+1];
	shellHeader *fields;
	wsField *current;
}	workSpace;

/* [EndDoc] */

/*
 * [BeginDoc]
 * The ``wsHeader'' type contains the data that is stored to disk in the first
 * bytes of a workspace.  The ``magic'' member uniquely identifies this
 * as a DCDB file.  The ``wsName'' member is the name of the workspace.
 * The ``numWS'' member is the number of tables grouped together by this
 * workspace.
 * 
 * The ``wsField'' structure is used to maintain the table information
 * in a workspace.  One member of this type is stored to disk for each
 * table grouped together by the workspace.  These are also maintained
 * in memory for table management.  The ``fieldNum'' member is the number
 * of this wsField.  When tables are added to a workspace, a wsFeild is
 * created and given the next number in the sequence.  This is how they are
 * stored in memory and on disk.  The ``tbl'' member is a pointer to the
 * table descriptor.  This is only used with the field in memory.  The
 * ``tableComment'' is application dependent.  It is there to give the
 * user the opportunity to store application specific information about
 * each table.  The ``tableName'' member is the name of the table for this
 * field.  The ``currentIndex'' is the default index that is set to current
 * when the table is opened.  This is determined by the workspace routines
 * when the table is added to the workspace.
 * 
 * The ``workSpace'' structure is the actual type that the user will
 * work with.  ``hdr'' is the wsHeader for this workspace.  ``number'' is the
 * number of tables stored in this workspace (same as ``hdr->numWS'').
 * ``fileName'' is the name of the file the workspace is stored in (same
 * as ``hdr->wsName'').  ``fields'' is a sorted shell of the wsFeild types
 * for all the tables.  ``current'' is the table currently in use by the
 * workspace routines.
 * [EndDoc]
 */

/*char syntaxError[SYNTAX_ERROR_WIDTH+1];*/
extern char syntaxError[];

/* Not currently supported */
#define	PRECNO_LEN	10

#ifdef	__BORLANDC__
#ifndef	__FLAT__
#define	PRECNO_MASK	"%10ld"
#else	/* __FLAT__ */
#define	PRECNO_MASK	"%10d"
#endif	/* __FLAT__ */
#endif	/* __BORLANDC__ */

#ifdef	__DJGPP__
#define	PRECNO_MASK	"%10d"
#endif

struct __db_query {
	size_t num;
	int recno;
	int precno;
};

typedef struct __db_query dbQuery;

/*
 * Table functions
 * 
 * @DocInclude ../lib/cdbtable.c
 */
dbTable *createTable (const char *fname, dbHeader *hdr, 
							dbField *flds[]);
dbTable *openTable (const char *fname);
dbTable *openTableNoIndexes (const char *fname);
int closeTable (dbTable *tbl);
int storeTableHeader (dbTable *tbl);
int storeFieldHeader (int num, dbTable *tbl);
									 
#ifdef	DEBUG
int DebugCheckTable (dbTable *tbl);
#endif

#ifndef	DEBUG
#define	DebugCheckTable(t)
#endif

/*
 * Index functions
 * 
 * @DocInclude ../lib/cdbindex.c
 */
int createDBIndex (dbTable *tbl, char *idxName, 
						 char *fldName, int isCase, 
						 int isUnique, int blkSize);
int createMultiIndex (dbTable *tbl, char *midxName, 
							char **names, int isCase,
							int blksize);
int openDBIndexes (dbTable *tbl);
int openMultiIndexes (dbTable *tbl);
int closeDBIndexes (dbTable *tbl);
int closeMultiIndexes (dbTable *tbl);
int addDBIndexes (dbTable *tbl);
int addMultiIndexes (dbTable *tbl);
int setCurrentIndex (dbTable *tbl, char *idxName);

/*
 * Utility functions
 * 
 * @DocInclude ../lib/cdbutils.c
 */
const char *getTimeStamp (void);
const char *sortedTimeStamp (void);
void field2Record (dbTable *tbl);
void record2Field (dbTable *tbl);
const char *dberror (void);
const char *dbtblerror (dbTable *tbl);
void dbClearAllErrors (dbTable *tbl);

/*
 * Move functions
 * 
 * @DocInclude ../lib/cdbmove.c
 */
off_t gotoRecord (dbTable *tbl, off_t recno);
off_t nextRecord (dbTable *tbl);
off_t nextIndexRecord (dbTable *tbl);
off_t prevRecord (dbTable *tbl);
off_t prevIndexRecord (dbTable *tbl);
off_t headRecord (dbTable *tbl);
off_t headIndexRecord (dbTable *tbl);
off_t tailRecord (dbTable *tbl);
off_t tailIndexRecord (dbTable *tbl);
off_t searchIndexRecord (dbTable *tbl, char *fieldValue);
off_t searchExactIndexRecord (dbTable *tbl, char *fieldValue);

/*
 * Add functions
 * 
 * @DocInclude ../lib/cdbadd.c
 */
int addRecord (dbTable *tbl);
ListHeader *massAddRecords (dbTable *tbl, ListHeader *lh);

/*
 * Edit functions
 * 
 * @DocInclude ../lib/cdbedit.c
 */
off_t retrieveRecord (dbTable *tbl);
int updateRecord (dbTable *tbl);
int recordDeleted (dbTable *tbl);
int deleteRecord (dbTable *tbl);
int undeleteRecord (dbTable *tbl);
int isRecordDeleted (dbTable *tbl);

/*
 * Pack/reindex functions
 * 
 * @DocInclude ../lib/cdbpack.c
 */
dbTable *packTable (dbTable *tbl, int save);
int midxField2str (midxField *midx, char *str);
int str2midxField (midxField *midx, char *str);
int reindexTable (const char *tblname);

/*
 * UI functions
 * 
 * @DocInclude ../lib/cdbui.c
 */
dbTable *buildTable (const char *tblname,
						   const char *tblinfo, fieldType *ftype, 
						   char **names, int *flens, int *declens);
int getFieldNum (dbTable *tbl, const char *fieldName);
void timeStamp2Date (char *date, const char *timeStamp);
int setCharField (dbTable *tbl, const char *fieldName, 
						const char *value);
int setNumberField (dbTable *tbl, const char *fieldName, 
						  double value);
int setIntField (dbTable *tbl, const char *fieldName, 
					   int value);
int setDateField (dbTable *tbl, const char *fieldName,
						const char *value);
int setDateField2TimeStamp (dbTable *tbl, const char *fieldName, 
								  const char *timeStamp);
int setLogicalField (dbTable *tbl, const char *fieldName,
						   const char value);
int setTimeStampField (dbTable *tbl, const char *fieldName,
							 const char *timeStamp);
int setDefaultField (dbTable *tbl, const char *fieldName);
char *getField (dbTable *tbl, const char *fieldName);

/*
 * DF functions
 * 
 * @DocInclude ../lib/cdbdf.c
 */
int parseDBDef (const char *file);

/*
 * WS functions
 * 
 * @DocInclude ../lib/cdbws.c
 */
workSpace *wsCreate (const char *name);
int wsAddTable (workSpace *ws, dbTable *tbl, 
					  char *comment);
int wsClose (workSpace *ws);
workSpace *wsOpen (const char *name);
dbTable *wsGetTable (workSpace *ws, char *name);

/*
 * Getfile functions
 * 
 * I removed this from the distribution.
 */
extern int inCurses;

int getTableItems (dbTable *tbl, const char *putfile);

#include <bcnum.h>

/*
 * Table wrapper functions
 * 
 * I removed this from the documentation, although I will keep the file
 * with the distribution.
 * DocInclude ../include/cdbpp.h
 */

/*
 * High-level C interface bindings.
 *
 * @DocInclude ../lib/cdb.c
 */

/*
 * Tcl/Tk bindings
 * 
 * @DocInclude ../include/interface.h
 */

/*
 * CINT bindings
 *
 * @DocInclude ../extras/cint/cint_cdb.h
 */

/*
 * Container module
 * @DocInclude ../include/container.h
 */

/*
 * Tcl library
 *
 * @DocInclude ../extras/tcl/cdb.tcl
 */

endC

#endif		/* __DCDB_H__ */
