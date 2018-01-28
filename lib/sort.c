/* Source File: sort.c */

#ifdef __STDC__
#define _BSD_SOURCE
#endif

#include <sort.h>

/*
 * [BeginDoc]
 *
 * \section{Design Considerations}
 *
 * The sort module has been designed to be removed from the DCDB database
 * engine as a separate entity.  Essentially, there was interest in just using
 * the sort routines without having to shoulder the overhead of the entire
 * library.  So, the sort functions, while required by DCDB, do not require the
 * rest of DCDB to be used.  The following source files are needed to use
 * the sort routines:
 *
 * \begin{itemize}
 * \item dcdb-x.x/include/sort.h
 * \item dcdb-x.x/src/sort.c
 * \item dcdb-x.x/src/qsort.c
 * \end{itemize}
 *
 * These can be pulled out of the DCDB distribution and used separately.  I will
 * continue to maintain them so that this differentiation is possible.
 *
 * [EndDoc]
 */

/*
 * [BeginDoc]
 *
 * The following are used for error handling in the sort routines(see page
 * \pageref{ErrorHandling} for information on error handling):
 * [Verbatim] */

fioErrorType fioError = FIO_NOERROR;
#ifdef HAVE_PTHREAD_H
pthread_mutex_t fioError_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
char *fioErrMsg[] = { 
  "no file error",                      /* FIO_NOERR */
  "permission denied",                  /* FIO_DENIED */
  "too many open files",                /* FIO_TOOMANY */
  "file or directory doesn't exist",    /* FIO_NOFILE */
  "bad file descriptor",                /* FIO_BADFD */
  "operation prohibited",               /* FIO_PROHIB */
  "operation would cause a deadlock",   /* FIO_DEADLK */
  "locking failed",                     /* FIO_NOLOCK */
  "memory exhausted",                   /* FIO_NOMEMORY */
  /* Add new ones here */
  "unspecified file error",              /* FIO_UNSPECIFIED */
  NULL
};

ListErrorType ListError = LIST_NOERROR;
#ifdef HAVE_PTHREAD_H
pthread_mutex_t ListError_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
char *ListErrorString[] = { 
  "no error",                           /* LIST_NOERROR */
  "feature not implemented",            /* LIST_NOTIMPLEMENTED */
  "unknown list type",                  /* LIST_UNKNOWN */
  "memory error",                       /* LIST_NOMEM */
  "invalid list",                       /* LIST_INVALID */
  "can't delete non-empty list",        /* LIST_NOTEMPTY */
  "compare function cannot be NULL",    /* LIST_NULLCMP */
  "bad function parameter",             /* LIST_PARAM */
  "corrupted tree",                     /* LIST_TREECORRUPT */
  "can't add non-unique item to unique tree", /* LIST_UNIQUE */
  "error opening file",                 /* LIST_FILE */
  /* Add new ones here */
  "unspecified error",                  /* LIST_UNSPECIFIED */
  NULL
};

ListHeader * __head = NULL;   /* use these to manage lists */
ListHeader * __tail = NULL;
int listsInUse = FALSE;
#ifdef HAVE_PTHREAD_H
pthread_mutex_t __head_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t __tail_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t listsInUse_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

shellError shlError = SHELL_NOERR;
#ifdef HAVE_PTHREAD_H
pthread_mutex_t shlError_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
char *shlErrorStr[] = {
  "no error",                   /* SHELL_NOERR */
  "memory exhausted",           /* SHELL_NOMEM */
  "invalid function parameter", /* SHELL_PARAM */
  "list error",                 /* SHELL_LIST */
  "corrupt data detected",      /* SHELL_CURRUPT */
  "unique constraint violated", /* SHELL_UNIQUE */
  "link manager error",         /* SHELL_LINK_MGR */
	"error creating a thread",    /* SHELL_TH_CREATE */
	"error joining a thread",     /* SHELL_TH_JOIN */
  /* Add new ones here */
  "unspecified error",          /* SHELL_UNSPECIFIED */
  NULL
};

/* [EndDoc] */

/*
 * [BeginDoc]
 *
 * \section{Miscellaneous Routines}
 *
 * [EndDoc]
 */

/*
 * Just for data gathering.
 */
#ifdef STATS
double hln = 0.0; /* high level node traversal */
double zln = 0.0;
double llt = 0.0;
#endif

/*
 * Time function
 */

#ifndef __MINGW32__

/*
 * [BeginDoc]
 * \subsection{elapsed}
 * \index{elapsed}
 *
 * [Verbatim] */

int elapsed(double *sec)
/* [EndDoc] */
/*
 * [BeginDoc]
 * elapsed is a utility that uses the standard C library call
 * gettimeofday() to determine the current time (in sconds and
 * milliseconds since the epoch).  This information can be used to
 * get very accurate timings as follows:
 * [Verbatim] *

  double first, last;
  ...
  elapsed(&first);
  ...do some stuff here...
  elapsed(&last);
  printf ("Elapsed time while doing stuff: %f\n\n", last-first);

 * [EndDoc] */
/*
 * [BeginDoc]
 *
 * \emph{Note:} it should be emphasized that this function does not compile
 * if strict adherence to the ISO C standard is required.
 * [EndDoc]
 */
{
  struct timeval t;
  struct timezone tz;
  int stat;

  stat = gettimeofday(&t, &tz);
  *sec = (double)(t.tv_sec + t.tv_usec/1000000.0);
  return(stat);
}

#endif /* __MINGW32__ */

/*
 * [BeginDoc]
 * \subsection{split}
 * \index{split}
 * [Verbatim] */

static char *split_array[MAX_SPLIT_LENGTH];

char **split (char *ln, int ch)

/* [EndDoc] */
/*
 * [BeginDoc]
 * The split function is based on the TCL command with the same name.  It takes
 * as argument a line of characters given by ``ln'' and splits it by the
 * character ``ch''.  It returns an array of C strings.  The contents of ``ln''
 * are altered such that each occurence of ``ch'' is replaced by (char)0.  There
 * is currently a limit of MAX_SPLIT_LENGTH lines that can be split (this is
 * initially set to 1,000,000, but can be adjusted).  The split command is
 * \emph{NOT} reentrant and should not be used in multi-threaded applications.
 * Use split_r() instead.
 * 
 * [EndDoc]
 */
{
  static int inuse=FALSE;
  char *field, *nextfield, *cp;
  int i;

  if (inuse == TRUE)
    for (i = 0; i < MAX_SPLIT_LENGTH && split_array[i] != 0; i++)
      split_array[i] = 0;
  else
    inuse = TRUE;
  field = ln;
  cp = strchr (ln, ch);
  if (cp != 0) {
    *cp = 0;
    nextfield = cp+1;
  }
  else {
    split_array[0] = field;
    return split_array;
  }
  for (i = 0; i < MAX_SPLIT_LENGTH; i++) {
    split_array[i] = field;
    cp = strchr (nextfield, ch);
    if (cp != 0) {
      *cp = 0;
      field = nextfield;
      nextfield = cp+1;
    }
    else {
      if (nextfield[0] != '\0')
	split_array[i+1] = nextfield;
      return split_array;
    }
  }
  return split_array;  /* keep our gcc happy */
}

/*
 * [BeginDoc]
 * \subsection{split_r}
 * \index{split_r}
 * [Verbatim] */

int split_r (char **cpp, int maxnum, char *ln, int ch)

/* [EndDoc] */
/*
 * [BeginDoc]
 * The split_r function is a reentrant safe version of the split function.
 * In addtion to the ``ln'' and ``ch'' arguments (see the split() function
 * above), it requires a ``cpp'' argument, which is a preallocated array
 * of character pointers, and a ``maxnum'' argument, which is the number of
 * items in cpp and the maximum number of items that will be placed in
 * the cpp array.  The return value is the count of items that split_r()
 * processed.
 * 
 * [EndDoc]
 */
{
  char *field, *nextfield, *cp;
  int i;
  int counter = 1;

  field = ln;
  cp = strchr (ln, ch);
  if (cp != 0) {
    *cp = '\0';
    nextfield = cp+1;
  }
  else {
    cpp[0] = field;
    cpp[1] = 0;
    return 1;
  }

  for (i = 0; i < maxnum; i++, counter++) {
    cpp[i] = field;
    cpp[i+1] = 0; // precautionary.
    cp = strchr (nextfield, ch);
    if (cp != 0) {
      *cp = '\0';
      field = nextfield;
      nextfield = cp+1;
    }
    else {
      if (nextfield[0] != '\0') {
        cpp[i+1] = nextfield;
        cpp[i+2] = 0;
        counter++;
      }
      return counter;
    }
  }
  // Keep our gcc happy.
  return counter;
}

/*
 * File functions
 */

/*
 * [BeginDoc]
 *
 * \section{File System Functions}
 *
 * These are the file access functions.  They are designed to
 * provide a portable API to low-level C file access functions. You may need
 * to change these in order to port DCDB to a system that is not yet supported.
 * 
 * [EndDoc]
 */

/*
 * [BeginDoc]
 *
 * \subsection{set_fioError}
 * \index{set_fioError}
 *
 * [Verbatim] */

void set_fioError (fioErrorType er)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The ``set_fioError'' function set's the global error condition for the file
 * functions in a thread-safe way, using mutexes to lock the alteration of this
 * variable.  This is a step toward making the file functions thread safe.
 *
 * The ``er'' parameter is the value that the global ``fioError'' variable
 * should be set to.
 *
 * [EndDoc]
 */
{
#ifdef HAVE_PTHREAD_H
  pthread_mutex_lock (&fioError_mutex);
#endif
  fioError = er;
#ifdef HAVE_PTHREAD_H
  pthread_mutex_unlock (&fioError_mutex);
#endif
}

/*
 * [BeginDoc]
 *
 * \subsection{fexists}
 * \index{fexists}
 *
 * [Verbatim] */

int fexists (char *fname)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * ``fexists'' returns TRUE if the file given by ``fname'' exists and FALSE
 * if it doesn't exist.  
 *
 * [EndDoc]
 */
{
  struct stat sbuf;
  int status;

  status = stat (fname, &sbuf);
  if (status == 0)
    return TRUE;
  else {
    if (errno != 0)
      errno = 0;
    return FALSE;
  }
}

#ifndef	DEBUG
/*
 * [BeginDoc]
 * \subsection{fileCreate}
 * \index{fileCreate}
 *
 * [Verbatim] */

int fileCreate (const char *file)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The fileCreate() function creates the file given by ``fname''.  If
 * the file already exists, it is truncated (=> irreparable loss of
 * data - be careful).
 *
 * fileCreate() creates the file in text mode with read/write
 * permissions enabled by default.  Notice that if you want a binary
 * mode interface (which you should use if working in Windows),
 * you will have to create the file using fileCreate(),
 * close it and then reopen it using fileOpen().
 *
 * fileCreate() returns _ERROR_ (-1) on error and
 * the handle of the file if it is successful.  The handle returned
 * by fileCreate() will be used in successive fsystem function calls.
 * If an error occurs, fioErrMsg[fioError] contains a string that
 * describes the error.
 *
 * Note: \emph{if DEBUG is defined when this is compiled the function
 * ``debug_fileCreate() is called instead of fileCreate().  The debug
 * version prints a statement indicating the file was created/opened.
 * Otherwise, it is identical to fileCreate()}
 *
 * [EndDoc]
 */
{
  int fd;
  fd = creat (file, FILE_PERMISSION);
  if (fd < 0) {
    if (EACCES == errno)
      set_fioError(FIO_DENIED);
    
    else if (EMFILE == errno)
      set_fioError(FIO_TOOMANY);
    
    else if (ENOENT == errno)
      set_fioError(FIO_NOFILE);
    
    else
      set_fioError(FIO_UNSPECIFIED);
    return _ERROR_;
  }
  return fd;
}


#endif	/* !DEBUG */

#ifdef	DEBUG
int debug_fileCreate (const char *file, const char *f, int l) 
{
  int fd;
  fd = creat (file, FILE_PERMISSION);
  if (fd < 0) {
    if (EACCES == errno)
      set_fioError(FIO_DENIED);
    
    else if (EMFILE == errno)
      set_fioError(FIO_TOOMANY);
    
    else if (ENOENT == errno)
      set_fioError(FIO_NOFILE);
    
    else
      set_fioError(FIO_UNSPECIFIED);
    return _ERROR_;
  }
  printf ("\n%s created/opened from %s, line %d\n", file, f, l);
  return fd;
}

#endif	/* DEBUG */

#ifndef	DEBUG
/*
 * [BeginDoc]
 * \subsection{fileOpen}
 * \index{fileOpen}
 *
 * [Verbatim] */

__inline__ int fileOpen (const char *file)

/* [EndDoc] */
/*
 * [BeginDoc]
 * 
 * fileOpen() opens the file given by ``fname'' in binary mode with
 * read/write enabled.  If the file doesn't exist or cannot be opened
 * in read/write mode, _ERROR_ (-1) is returned.  Otherwise, the handle
 * of the newly opened file is returned.  This handle is used in
 * subsequent function calls to other functions in the fsystem
 * module wherever a file handle is expected.
 * If an error occurs, fioErrMsg[fioError] contains a string that
 * describes the error.
 *
 * Note: \emph{the debug version of this function is called if DEBUG is
 * defined.}
 *
 * [EndDoc]
 */
{
  int fd;
  fd = open (file, FILE_MODE, FILE_PERMISSION);
  if (fd < 0) {
    if (EACCES == errno)
      set_fioError(FIO_DENIED);
    
    else if (EMFILE == errno || ENFILE == errno)
      set_fioError(FIO_TOOMANY);
    
    else if (ENOENT == errno)
      set_fioError(FIO_NOFILE);
    
    else
      set_fioError(FIO_UNSPECIFIED);
    return _ERROR_;
  }
  return fd;
}
#endif	/* !DEBUG */
  
#ifdef	DEBUG
int debug_fileOpen (const char *file, const char *f, int l) 
{
  int fd;
  fd = open (file, FILE_MODE, FILE_PERMISSION);
  if (fd < 0) {
    if (EACCES == errno)
      set_fioError(FIO_DENIED);
    
    else if (EMFILE == errno)
      set_fioError(FIO_TOOMANY);
    
    else if (ENOENT == errno)
      set_fioError(FIO_NOFILE);
    
    else
      set_fioError(FIO_UNSPECIFIED);
    return _ERROR_;
  }
  printf ("\nFile %s opened from %s, line %d, fd = %d\n", file, f, l, fd);
  return fd;
}
#endif	/* DEBUG */


#ifndef	DEBUG
/*
 * [BeginDoc]
 * \subsection{fileClose}
 * \index{fileClose}
 *
 * [Verbatim] */

__inline__ int fileClose (int fd)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * fileClose() closes the file whose handle is given by ``fh''.  Any
 * buffers associated with the file are flushed to disk and the
 * file is closed.  Use of the file handle after the file is closed
 * is \emph{undefined} (to quote Eeyore of Disney fame, ``'Feel something
 * bad is about to happen'').
 * If fileClose() was successful, _OK_ is returned.  Otherwise,
 * _ERROR_ is returned.
 * If an error occurs, fioErrMsg[fioError] contains a string that
 * describes the error.
 *
 * Note: \emph{the debug version of this function is called if DEBUG is
 * defined.}
 *
 * [EndDoc]
 */
{
  int status;
  status = close (fd);
  if (status < 0) {
    if (EBADF == errno)
      set_fioError(FIO_BADFD);
    
    else
      set_fioError(FIO_UNSPECIFIED);
    return _ERROR_;
  }
  return _OK_;
}
#endif /* !DEBUG */
  
#ifdef DEBUG
int debug_fileClose (int fd, const char *f, int l) 
{
  int status;
  printf ("\nFile (fd) %d closed from %s, line %d\n", fd, f, l);
  status = close (fd);
  if (status < 0) {
    if (EBADF == errno)
      set_fioError(FIO_BADFD);
    
    else
      set_fioError(FIO_UNSPECIFIED);
    return _ERROR_;
  }
  return _OK_;
}
#endif /* DEBUG */

/*
 * [BeginDoc]
 * \subsection{fileSeekBegin}
 * \index{fileSeekBegin}
 *
 * [Verbatim] */

__inline__ off_t fileSeekBegin (int fh, off_t offset)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * fileSeekBegin() seeks ``offset'' bytes from the beginning of the
 * file given by file handle ``fh''.  It returns the new offset the
 * file pointer resides into the file.  This should always be
 * checked against the offset parameter passed to the fileSeekBegin()
 * function to insure that the expected offset was attained before
 * the user begins writing to or reading from the file.
 * If an error occurs, fioErrMsg[fioError] contains a string that
 * describes the error.
 *
 * [EndDoc]
 */
{
  off_t status;
  status = lseek (fh, offset, FILE_BEGINNING);
  if (status == -1L) {
    if (EBADF == errno)
      set_fioError(FIO_BADFD);
    
    else
      set_fioError(FIO_UNSPECIFIED);
  }
  return status;
}

/*
 * [BeginDoc]
 * \subsection{fileSeekEnd}
 * \index{fileSeekEnd}
 *
 * [Verbatim] */

__inline__ off_t fileSeekEnd (int fh, off_t offset)

/* [EndDoc] */
/*
 * [BeginDoc]
 * fileSeekEnd() is identical to fileSeekBegin except that is
 * attempts to place the file pointer ``offset'' bytes from the \emph{end}
 * of the file ``fh''.
 * If an error occurs, fioErrMsg[fioError] contains a string that
 * describes the error.
 *
 * [EndDoc]
 */
{
  off_t status;
  status = lseek (fh, offset, FILE_END);
  if (status == -1L) {
    if (EBADF == errno)
      set_fioError(FIO_BADFD);
    
    else
      set_fioError(FIO_UNSPECIFIED);
  }
  return status;
}

/*
 * [BeginDoc]
 * \subsection{fileSeekCurrent}
 * \index{fileSeekCurrent}
 *
 * [Verbatim] */

__inline__ off_t fileSeekCurrent (int fh, off_t offset)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * fileSeekCurrent() is the same as fileSeekBegin() and fileSeekEnd()
 * except that it attempts to place the file pointer ``offset'' bytes
 * from the \emph{current} position in the file ``fh''.
 * If an error occurs, fioErrMsg[fioError] contains a string that
 * describes the error.
 *
 * [EndDoc]
 */
{
  off_t status;
  status = lseek (fh, offset, FILE_CURRENT);
  if (status == -1L) {
    if (EBADF == errno)
      set_fioError(FIO_BADFD);
    
    else
      set_fioError(FIO_UNSPECIFIED);
  }
  return status;
}

/*
 * [BeginDoc]
 * \subsection{fileWrite}
 * \index{fileWrite}
 *
 * [Verbatim] */

__inline__ off_t fileWrite (int fh, void *buf, size_t len)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * fileWrite() attempts to write ``len'' bytes from the buffer ``buf'' to the
 * file pointed to by the file handle ``fh''.  If the disk is full,
 * fileWrite() will return 0.  If there was an error, fileWrite()
 * will return _ERROR_.
 * If an error occurs, fioErrMsg[fioError] contains a string that
 * describes the error.
 *
 * Even if fileWrite() returns a value other than -1, this value should always
 * be checked with the value of the len parameter passed in to insure
 * that the expected number of bytes were written.  \emph{It is an error
 * condition if len bytes weren't written after a call to fileWrite().}
 *
 * [EndDoc]
 */
{
  off_t status;
  status = (off_t) write (fh, buf, len);
  if (status < 0) {
    if (EACCES == errno)
      set_fioError(FIO_DENIED);
    if (EBADF == errno)
      set_fioError(FIO_BADFD);
    
    else
      set_fioError(FIO_UNSPECIFIED);
  }
  return status;
}

/*
 * [BeginDoc]
 * \subsection{fileRead}
 * \index{fileRead}
 *
 * [Verbatim] */

__inline__ off_t fileRead (int fh, void *buf, size_t len)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * fileRead() attempts to read ``len'' bytes into the buffer pointed to
 * by ``buf'' from the file with file handle ``fh''.  buf must contain
 * \emph{at least} len bytes of storage, or the result is undefined
 * (i.e., ``Houston, we have a problem.'').
 *
 * fileRead() will return 0 if the file pointer is at EOF.  It will
 * return _ERROR_ on an error condition.  Otherwise, it returns the
 * number of bytes actually read.  This return value should always
 * be checked against the value passed in the len parameter to insure
 * that the number of bytes read was what the user requested.
 * \emph{It is not necessarily an error if the value returned is not
 * equal to len -- it is application specific.}
 * If an error occurs, fioErrMsg[fioError] contains a string that
 * describes the error.
 *
 * [EndDoc]
 */
{
  off_t status;
  status = (off_t) read (fh, buf, len);
  if (status < 0) {
    if (EACCES == errno)
      set_fioError(FIO_DENIED);
    if (EBADF == errno)
      set_fioError(FIO_BADFD);
    
    else
      set_fioError(FIO_UNSPECIFIED);
  }
  return status;
}

/*
 * [BeginDoc]
 * \subsection{fileFlush}
 * \index{fileFlush}
 *
 * [Verbatim] */

__inline__ int fileFlush (int fh)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * fileFlush() flushes the any buffers associated with the file given
 * by file handle ``fh''.  A call to fileFlush() should be made after
 * write operations that must be flushed to disk immediately.
 * _OK_ is returned if fileFlush() was successful, _ERROR_ is returned 
 * otherwise.  An error condition is usually a result of a bad value for
 * fh.
 * If an error occurs, fioErrMsg[fioError] contains a string that
 * describes the error.
 *
 * fileFlush() works by creating a duplicate file handling and closing
 * that, thus forcing a system flush.  How effective the flush is is
 * OS dependent.  The way the system uses caching will determine whether
 * the file is actually flushed or not.
 *
 * [EndDoc]
 */
{
  int status;
  status = close (dup (fh));
  if (status < 0) {
    if (EBADF == errno)
      set_fioError(FIO_BADFD);

    else
      set_fioError(FIO_UNSPECIFIED);
    return _ERROR_;
  }
  return _OK_;
}

#ifndef __MINGW32__

/*
 * [BeginDoc]
 * \subsection{fileLockRegion}
 * \index{fileLockRegion}
 *
 * [Verbatim] */

__inline__ int fileLockRegion (int fh, int cmd, int type, size_t offset,
    int whence, size_t len)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * fileLockRegion() locks a region of the open file ``fh''.
 * The ``cmd'' is one of F_GETLK, F_SETLK or F_SETLKW (see fcntl(2)).
 * The ``type'' is one of F_RDLCK
 * (for a read lock), F_WRLCK (for a write lock) or F_UNLCK (to unlock the
 * region).  The ``offset'' argument is the offset into the file, relative to
 * the ``whence'' argument.  Whence can be one of SEEK_SET (FILE_BEGINNING),
 * SEEK_CUR (FILE_CURRENT) or SEEK_END (FILE_END).  Finally, ``len'' is the
 * length of data to lock.  If len is 0, the region to the end of the file is
 * locked.  _OK_ is returned if fileLockRegion() is successful, _ERROR_
 * otherwise.
 * If an error occurs, fioErrMsg[fioError] contains a string that
 * describes the error.
 *
 * [EndDoc]
 */
{
  struct flock lock;
  int status;
  lock.l_type = type;
  lock.l_start = offset;
  lock.l_whence = whence;
  lock.l_len = len;
  status = fcntl (fh, cmd, &lock);
  if (status == _ERROR_) {
    if (EACCES == errno)
      set_fioError(FIO_DENIED);

    else if (EMFILE == errno)
      set_fioError(FIO_TOOMANY);

    else if (ENOENT == errno)
      set_fioError(FIO_NOFILE);

    else if (EBADF == errno)
      set_fioError(FIO_BADFD);

    else if (EAGAIN == errno)
      set_fioError(FIO_PROHIB);

    else if (EDEADLK == errno)
      set_fioError(FIO_DEADLK);

    else if (ENOLCK == errno)
      set_fioError(FIO_NOLOCK);

    else
      set_fioError(FIO_UNSPECIFIED);
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{fileLockTest}
 * \index{fileLockTest}
 *
 * [Verbatim] */

__inline__ pid_t fileLockTest (int fh, int type, size_t offset,
    int whence, size_t len)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * fileLockTest() determines whether a segment of a file is locked or not.  The
 * ``fh'' is the handle of an open file.  The ``type'' is one of F_RDLCK or
 * F_WRLCK.  The ``whence'' is one of SEEK_SET, SEEK_CUR or SEEK_END.  And
 * finally, ``len'' is the length of the segment to test.  If len is 0, the
 * segment extends to the end of the file.  fileLockTest() returns FALSE if the
 * segment is not locked, or the PID of the process that has it locked.
 * If an error occurs, fioErrMsg[fioError] contains a string that
 * describes the error.
 *
 * [EndDoc]
 */
{
  struct flock lock;
  int status;
  lock.l_type = type;
  lock.l_start = offset;
  lock.l_whence = whence;
  lock.l_len = len;
  status = fcntl (fh, F_GETLK, &lock);
  if (status == _ERROR_) {
    if (EACCES == errno)
      set_fioError(FIO_DENIED);

    else if (EMFILE == errno)
      set_fioError(FIO_TOOMANY);

    else if (ENOENT == errno)
      set_fioError(FIO_NOFILE);

    else if (EBADF == errno)
      set_fioError(FIO_BADFD);

    else if (EAGAIN == errno)
      set_fioError(FIO_PROHIB);

    else if (EDEADLK == errno)
      set_fioError(FIO_DEADLK);

    else if (ENOLCK == errno)
      set_fioError(FIO_NOLOCK);

    else
      set_fioError(FIO_UNSPECIFIED);
    return _ERROR_;
  }
  if (lock.l_type == F_UNLCK)
    return FALSE;
  return (lock.l_pid);
}

#endif /* __MINGW32__ */

/*
 * [BeginDoc]
 * \subsection{fileRemove}
 * \index{fileRemove}
 * [Verbatim] */

__inline__ int fileRemove (const char *fname)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * fileRemove() deletes the file given by fname from the file system.
 * If it is successful, it returns _OK_; otherwise, _ERROR_.
 * If an error occurs, fioErrMsg[fioError] contains a string that
 * describes the error.
 *
 * [EndDoc]
 */
{
  int status;
  status = remove (fname);
  if (status < 0) {
    if (EACCES == errno)
      set_fioError(FIO_DENIED);

    else if (ENOENT == errno)
      set_fioError(FIO_NOFILE);

    else
      set_fioError(FIO_UNSPECIFIED);
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{fileRename}
 * \index{fileRename}
 * [Verbatim] */

__inline__ int fileRename (const char *oldname, const char *newname)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * fileRename() attempts to rename the file given by oldname to
 * newname.  It returns _OK_ on success, _ERROR_ otherwise.
 * If an error occurs, fioErrMsg[fioError] contains a string that
 * describes the error.
 *
 * [EndDoc]
 */

{
  int status;
  status = rename (oldname, newname);
  if (status < 0) {
    if (EACCES == errno)
      set_fioError(FIO_DENIED);

    else if (ENOENT == errno)
      set_fioError(FIO_NOFILE);

    else
      set_fioError(FIO_UNSPECIFIED);
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{fileWriteBlock}
 * \index{fileWriteBlock}
 * [Verbatim] */

size_t fileWriteBlock (int fh, off_t where, size_t reclen,
    ListHeader * lh)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * fileWriteBlock() attempts to write a block of data to the file
 * given by file handle ``fh''.  If successful, the write is done
 * where bytes from the beginning of the file.  The data is assumed
 * to be stored in the list ``lh'' (see the documentation for the list
 * module for more detail).  Each element of the data stored in
 * each link of the list is ``reclen'' bytes.
 *
 * fileWriteBlock() allocates enough memory to store the entire
 * block of data.  If the allocation fails, it returns _ERROR_.
 * If fileWriteBlock() cannot write ``where'' bytes from the beginning
 * of the file, it returns _ERROR_.  The file is flushed after the
 * write.
 * If an error occurs, fioErrMsg[fioError] contains a string that
 * describes the error.
 *
 * If DEBUG is #defined, the data is read back after the write and
 * compared to the data written.  If there are any differences,
 * _ERROR_ is returned.  It is also an error if the total number
 * of bytes in the linked list could not be written.
 *
 * If the write is successful, fileWriteBlock() will clean up the
 * dynamic allocation and return the number of bytes written.  This
 * should be checked with the number of bytes expected to insure
 * that the write was successful.
 *
 * [EndDoc]
 */

{
  char *pdata;
  unsigned int i;
  Link * lnk;
  off_t rtn;
  size_t totlen;
  dbg (char *pcheck;)

  totlen = reclen * lh->number;
  if (_ERROR_ == fileSeekBegin (fh, where))
    return (size_t) _ERROR_;    /* fioError set */
  pdata = (char *) malloc (totlen);
  Assert (0 != pdata);
  if (NULL == pdata) {
    set_fioError(FIO_NOMEMORY);
    return (size_t) _ERROR_;
  }
  memset (pdata, 0, totlen);
  lnk = lh->head->next;
  for (i = 0; i < lh->number; i++) {
    memcpy ((void *) (pdata + (i * reclen)), lnk->data, reclen);
    lnk = lnk->next;
  }
    /*
     * write it
     */
    rtn = fileWrite (fh, (void *) pdata, totlen);
  if (_ERROR_ == rtn) {
    DebugMemSet (pdata, totlen);
    free (pdata);
    return _ERROR_;             /* fioError set */
  }

  fileFlush (fh);             /* ignore errors (if any) */

#ifdef  DEBUG
    /*
     * check the written data
     */
    pcheck = malloc (totlen);
  Assert (0 != pcheck);
  if (0 == pcheck)
    return (size_t) _ERROR_;
  memset (pcheck, 0, totlen);
  rtn = fileSeekBegin (fh, where);
  Assert (_ERROR_ != rtn);
  if (_ERROR_ == i) {
    DebugMemSet (pdata, totlen);
    free (pdata);
    DebugMemSet (pcheck, totlen);
    free (pcheck);
    return (size_t) _ERROR_;
  }
  rtn = fileRead (fh, (void *) pcheck, totlen);
  if (_ERROR_ == rtn) {
    DebugMemSet (pdata, totlen);
    free (pdata);
    DebugMemSet (pcheck, totlen);
    free (pcheck);
    return _ERROR_;             /* fioError set */
  }

  i = memcmp (pcheck, pdata, totlen);
  Assert (0 == i);
  if (0 != i)
    return (size_t) _ERROR_;
  DebugMemSet (pcheck, totlen);
  free (pcheck);

#endif  /* DEBUG */
  DebugMemSet (pdata, totlen);
  free (pdata);
  return (size_t) rtn;
}

/*
 * [BeginDoc]
 * \subsection{fileReadBlock}
 * \index{fileReadBlock}
 * [Verbatim] */

ListHeader * fileReadBlock (int fh, off_t where, size_t reclen,
    size_t numrecs)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * fileReadBlock() attempts to read ``numrecs'' records of size ``reclen''
 * beginning ``where'' bytes from the beginning of the file given by
 * ``fh''.  If it is successful, it will build a linked list (see the
 * list module documentation for more information on how to use a
 * list) and return a pointer to it.  If unsuccessful,
 * fileReadBlock() returns (ListHeader *)0.
 *
 * [EndDoc]
 */

{
  char *pdata;
  ListHeader * lh;
  Link * lnk;
  unsigned int i;
  int j, num;
  size_t totlen;
  off_t brd;
  int status;
  void *vp;
  if (fileSeekBegin (fh, where) == _ERROR_)
    return 0;
  totlen = reclen * numrecs;
  pdata = (char *) malloc (totlen);
  if (NULL == pdata) {
    set_fioError(FIO_NOMEMORY);
    return 0;
  }
  memset (pdata, 0, totlen);
  brd = fileRead (fh, pdata, totlen);
  Assert (brd == totlen);
  if (brd != (off_t)totlen || -1 == brd) {
    DebugMemSet (pdata, totlen);
    free (pdata);
    return 0;
  }
  lh = initList (UNSORTED, 0);
  Assert (0 != lh);
  if (0 == lh) {
    set_fioError(FIO_UNSPECIFIED);
    DebugMemSet (pdata, totlen);
    free (pdata);
    return 0;
  }
  for (i = 0; i < numrecs; i++) {
    vp = malloc (reclen);
    if (0 == vp) {
      set_fioError(FIO_NOMEMORY);
      num = lh->number;
      for (j = 0; j < num; j++) {
        firstLink (lh);
        lnk = removeLink (lh);
        DebugMemSet (lnk->data, reclen);
        free (lnk->data);
        DebugMemSet (lnk, sizeof (Link));
        free (lnk);
      }
      free (pdata);
      return 0;
    }
    memset (vp, 0, reclen);
    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      set_fioError(FIO_NOMEMORY);
      DebugMemSet (vp, reclen);
      free (vp);
      num = lh->number;
      for (j = 0; j < num; j++) {
        firstLink (lh);
        lnk = removeLink (lh);
        DebugMemSet (lnk->data, reclen);
        free (lnk->data);
        DebugMemSet (lnk, sizeof (Link));
        free (lnk);
      }
      free (pdata);
      return 0;
    }
    memset (lnk, 0, sizeof (Link));
    memcpy (vp, (void *) (pdata + (i * reclen)), reclen);
    lnk->data = vp;

      /*
       * Insure the data gets put in the list in the same order
       * it came from the file.
       */
    lastLink (lh);
    status = insertLink (lh, lnk);
    if (_ERROR_ == status) {
      set_fioError(FIO_UNSPECIFIED);
      DebugMemSet (lnk, sizeof (Link));
      free (lnk);
      DebugMemSet (vp, reclen);
      free (vp);
      num = lh->number;
      for (j = 0; j < num; j++) {
        firstLink (lh);
        lnk = removeLink (lh);
        DebugMemSet (lnk->data, reclen);
        free (lnk->data);
        DebugMemSet (lnk, sizeof (Link));
        free (lnk);
      }
      free (pdata);
      return 0;
    }
  }
  DebugMemSet (pdata, totlen);
  free (pdata);
  return lh;
}

/*
 * List functions
 */
/*
 * [BeginDoc]
 *
 * \section{List Routines}
 *
 * This is the linked list implementation.
 * \index{linked list}
 * What's developed here is basically a container module
 * \index{linked list!container}
 * in which the nodes held by the lists are allocated and freed by
 * the user but the list module keeps data by the pointers passed
 * into it.  These routines never touch the data stored in the
 * links.  Only user defined functions touch that data.
 *
 * Following are the types declared in list.h:
 *
 * \subsection{ListType}
 * \index{ListType}
 *
 * This is an enumerated type (enum) which specifies the type of the list.
 * They are detailed below:
 *
 * \begin{enumerate}
 *
 * \item UNSORTED
 * \index{UNSORTED}
 *
 * This is a typical linked-list in which the nodes are stored in no
 * particular order.  The insertion will take place wherever the
 * current pointer is.  If you create a new UNSORTED list and start
 * inserting links without moving the current pointer, the links will
 * end up in the reverse order of insertion.  If you want them to be
 * inserted in order of entry, call lastLink() before each insertion.
 *
 * \item SORTED
 * \index{SORTED}
 *
 * It is recommended that you don't use this list type.  The sorting is slow
 * and it provides nothing over the shell sort.
 *
 * \item SLOWSORTED
 * \index{SLOWSORTED}
 *
 * As with the SORTED list, don't use this type of list.
 *
 * \item STACKED
 * \index{STACKED}
 *
 * A stack is a LIFO data structure - last in, first out.  It can be
 * \index{LIFO}
 * thought of as a stack of plates, where the next plate to be used is
 * the last one on the pile.  Insertions and retrievals are limited to
 * the beginning of the list.
 *
 * \item QUEUED
 * A queue is a FIFO data structure - first in, first out.  With a
 * \index{FIFO}
 * queue, you get a circular structure in which the first item entered
 * is the first item to come out.  Print jobs, for example, are typically
 * kept in a queue.
 *
 * \end{enumerate}
 *
 * All
 * \index{list!doubly-linked only}
 * lists in the module are doubly-linked lists (no singly-linked lists)
 * and are stored in the same types of containers.  Access to items in the
 * list, how the items are stored and retrieved is dependent on the type.
 *
 * [EndDoc]
 */

/*
 * [BeginDoc]
 * \subsection{set_ListError}
 * \index{set_ListError}
 *
 * [Verbatim] */

void set_ListError(ListHeader *lh, ListErrorType le)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The ``set_ListError'' sets the global ``ListError'' variable in a 
 * thread\-safe way.  If the ``lh'' parameter is not NULL, the local
 * error variable (lh->ListError) is set to the value of the parameter
 * ``le''.  If ``lh'' is NULL, the global ``ListError'' parameter is set
 * to the value of ``le'' in a thread\-safe way.
 *
 * [EndDoc]
 */
{
  if (lh == 0) {
#ifdef HAVE_PTHREAD_H
    pthread_mutex_lock (&ListError_mutex);
#endif
    ListError = le;
#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock (&ListError_mutex);
#endif
  }
  else
    lh->ListError = le;
}


/*
 * [BeginDoc]
 * \subsection{initList}
 * \index{initList}
 * [Verbatim]*/

ListHeader * initList (ListType lt,
                          int (*cmp) (const void *, const void *))

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * This is the starting function to using the list module.  A user
 * will typically call this function first to begin list processing.
 * Returned will be a memory address pointing to an allocated object
 * initialized to represent a list of the type indicated.  The ``lt''
 * parameter is one of the list types described above, and the
 * ``cmp'' parameter is the pointer to a function defined by the user.
 * If this function fails, it will return NULL.
 * \index{compare function}
 *
 * For example, suppose a user is going to store a structure of the
 * following type in a list:
 *
 *
 * [Verbatim]

struct mytype {
  char first[25];
  char last[50];
  float age;
};

 * [EndDoc] */

/*
 * [BeginDoc]
 *
 *
 * A valid compare function for this type of object where the user
 * wants to sort by last name would be as follows:
 *
 * [Verbatim]

int MyCompare (const void *d1, const void *d2)
{
        return strcmp (((struct mytype *)d1)->last,
                                   ((struct mytype *)d2)->last);
}

 * [EndDoc] */

/*
 * [BeginDoc]
 *
 *
 * Notice that the pointers passed into the compare function, while
 * void, are pointers to objects of the types that are being stored
 * in the list.  However you want to compare the objects is totally
 * up to you.  The compare function, however, should have a return
 * value such that:
 *
 * \begin{enumerate}
 * \item if d1  < d2, the return value  < 0
 * \item if d1 == d2, the return value == 0
 * \item if d1  > d2, the return value  > 0
 * \end{enumerate}
 *
 * [EndDoc]
 */

{
  ListHeader * lh, *plh;
  lh = malloc (sizeof (ListHeader));
  if (NULL == lh) {
    set_ListError(0, LIST_NOMEM);
    return NULL;
  }
  memset (lh, 0, sizeof (ListHeader));
  lh->head = malloc (sizeof (Link));
  if (NULL == lh->head) {
    set_ListError(0, LIST_NOMEM);
    free (lh);
    return NULL;
  }
  memset (lh->head, 0, sizeof (Link));
  lh->tail = malloc (sizeof (Link));
  if (NULL == lh->head) {
    set_ListError(0, LIST_NOMEM);
    free (lh->head);
    free (lh);
    return NULL;
  }
  memset (lh->tail, 0, sizeof (Link));

  /*
   * Establish this ListHeader in the list of ListHeaders
   */
#ifndef HAVE_PTHREAD_H
  if (listsInUse == FALSE) {
    listsInUse = TRUE;
    __head = malloc (sizeof (ListHeader));
    if (__head == NULL) {
      set_ListError(0, LIST_NOMEM);
      free (lh->head);
      free (lh->tail);
      free (lh);
      return NULL;
    }
    __tail = malloc (sizeof (ListHeader));
    if (__tail == NULL) {
      set_ListError(0, LIST_NOMEM);
      free (lh->head);
      free (lh->tail);
      free (lh);
      free (__head);
      return NULL;
    }
    memset (__head, 0, sizeof (ListHeader));
    memset (__tail, 0, sizeof (ListHeader));
    __head->__n = lh;
    __tail->__p = lh;
    lh->__n = __tail;
    lh->__p = __head;
  }

  else {
    plh = __tail->__p;

      /*
       * insert it here
       */
      plh->__n = lh;
    __tail->__p = lh;
    lh->__n = __tail;
    lh->__p = plh;
  }
#else
  if (listsInUse == FALSE) {
    pthread_mutex_lock (&listsInUse_mutex);
    listsInUse = TRUE;
    pthread_mutex_unlock (&listsInUse_mutex);
    pthread_mutex_lock (&__head_mutex);
    __head = malloc (sizeof (ListHeader));
    if (__head == NULL) {
      set_ListError(0, LIST_NOMEM);
      free (lh->head);
      free (lh->tail);
      free (lh);
      return NULL;
    }
    memset (__head, 0, sizeof (ListHeader));
    __head->__n = lh;
    lh->__p = __head;
    pthread_mutex_unlock (&__head_mutex);
    pthread_mutex_lock (&__tail_mutex);
    __tail = malloc (sizeof (ListHeader));
    if (__tail == NULL) {
      set_ListError(0, LIST_NOMEM);
      free (lh->head);
      free (lh->tail);
      free (lh);
      free (__head);
      return NULL;
    }
    memset (__tail, 0, sizeof (ListHeader));
    __tail->__p = lh;
    lh->__n = __tail;
    pthread_mutex_unlock (&__tail_mutex);
  }

  else {
    plh = __tail->__p;

      /*
       * insert it here
       */
    plh->__n = lh;
    pthread_mutex_lock (&__tail_mutex);
    __tail->__p = lh;
    pthread_mutex_unlock (&__tail_mutex);
    lh->__n = __tail;
    lh->__p = plh;
  }
#endif

  /*
   * Initialize List Header
   */
  lh->head->next = lh->tail;
  lh->head->prev = lh->head->data = NULL;
  lh->tail->prev = lh->head;
  lh->tail->next = lh->tail->data = NULL;
  lh->current = lh->head;
  lh->type = lt;
  lh->number = 0;
  lh->compare = cmp;
  return lh;
}


/*
 * [BeginDoc]
 * \subsection{delList}
 * \index{delList}
 * [Verbatim] */

int delList (ListHeader * lh)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * This function deletes a list pointed to by lh.  It is an error to
 * try to delete a list that is not empty.  It is the user's
 * responsibility to empty the list by calls to removeLink() until
 * there are no more links before calling delList().  delList()
 * returns _OK_ on success, _ERROR_ if it failed.
 *
 * [EndDoc]
 */
{
  ListHeader * plh;

  /*
   * Check the list contents.
   */
  if (lh->head->next != lh->tail || lh->tail->prev != lh->head) {
    set_ListError(0, LIST_NOTEMPTY);
    return _ERROR_;
  }

  /*
   * Make sure the list is one of ours.  If so, unlink it.
   */
#ifdef  DEBUG
  plh = __head;
  while (1 == 1) {
    if (plh->__n == lh) {
      lh->__n->__p = plh;
      plh->__n = lh->__n;
      lh->__n = lh->__p = NULL;
      break;
    }
    plh = plh->__n;
    if (plh == __tail) {
      set_ListError(0, LIST_INVALID);
      return _ERROR_;
    }
  }

#endif  /*  */
#ifndef DEBUG
  plh = lh->__p;
  lh->__n->__p = plh;
  plh->__n = lh->__n;
  lh->__n = lh->__p = NULL;

#endif  /*  */

  /*
   * Free head and tail if the list manager is empty
   */
  if (__head->__n == __tail && __tail->__p == __head) {
#ifdef HAVE_PTHREAD_H
    pthread_mutex_lock (&__head_mutex);
#endif
    DebugMemSet (__head, sizeof (ListHeader));
    free (__head);
    __head = NULL;
#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock (&__head_mutex);
#endif
#ifdef HAVE_PTHREAD_H
    pthread_mutex_lock (&__tail_mutex);
#endif
    DebugMemSet (__tail, sizeof (ListHeader));
    free (__tail);
    __tail = NULL;
#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock (&__tail_mutex);
#endif
#ifdef HAVE_PTHREAD_H
    pthread_mutex_lock (&listsInUse_mutex);
#endif
    listsInUse = FALSE;
#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock (&listsInUse_mutex);
#endif
  }

  /*
   * free up the pointers
   */
  DebugMemSet (lh->head, sizeof (Link));
  free (lh->head);
  DebugMemSet (lh->tail, sizeof (Link));
  free (lh->tail);
  DebugMemSet (lh, sizeof (ListHeader));
  free (lh);
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{insertLink}
 * \index{insertLink}
 * [Verbatim] */

int insertLink (ListHeader * lh, Link * lnk)

/* [EndDoc] */

/*
 * [BeginDoc]
 *
 * This function inserts a link ``lnk'' into the list ``lh''.
 * Typically, the user
 * will allocate an object of type Link (using malloc or calloc),
 * set the data member to point to an object of a user specified
 * type, and call insertLink () to link it into a list.  The code
 * piece that follows gives an example assuming an object of
 * struct mytype as shown above:
 * [Verbatim]

        struct mytype *mt;
        Link *lnk;
        .
        .
        .
        mt = malloc (sizeof (struct mytype));
        if (0 == mt)    {
                ...Handle the error...
        }
        memset (mt, 0, sizeof (struct mytype));
        check_pointer (mt);             ...see mymalloc for details...
        lnk = malloc (sizeof (Link));
        if (0 == lnk)   {
                ...Handle the error...
        }
        lnk->data = mt;
        status = insertLink (lh, lnk);

        ...where lh is assumed to have been created with a call to...
        ...initList()...

        if (status == _ERROR_)  {
                ...let the user know what the error was by using...
                ...ListErrorString[ListError] in a string output function
                free (lnk);
                free (mt);
        }

 * [EndDoc] */
/*
 * [BeginDoc]
 *
 *
 * This will insert lnk into lh.  How the insertion takes place
 * will depend on the type of list lh is.  For example,
 * in a sorted list, lnk will be inserted such that lnk->data
 * is in sorted order based on the (*lh->compare) function.
 * insertLink () returns _OK_ on success, _ERROR_ on error.
 *
 * [EndDoc]
 */

{
  Link * link;
  int icompare;
  switch (lh->type) {
  case UNSORTED:

    /*
     * Insert it wherever we're at
     */
    return insertLinkHere (lh, lnk);
  case SORTED:
    Assert (NULL != lh->compare);
    if (NULL == lh->compare)
      return _ERROR_;
    link = lh->head->next;
    if (link == lh->tail) {
      lh->current = lh->head;
      return insertLinkHere (lh, lnk);
    }
    if (lh->number <= 16) {
      while (1 == 1) {
        icompare = (*lh->compare) (lnk->data, link->data);
        if (icompare < 0) {
          link = link->prev;
          lh->current = link;
          break;
        }
        link = link->next;
        if (link == lh->tail) {
          link = link->prev;
          lh->current = link;
          break;
        }
      }
    }

    else {
      link = lh->tail->prev;

        /*
         * It's bigger than or equal to anything else;
         * insert it at the end.
         */
        if ((*lh->compare) (lnk->data, link->data) >= 0) {
        lh->current = link;
        return insertLinkHere (lh, lnk);
      }
      link =
        findSortedInsert (lnk, lh->number, lh->head->next, lh->tail->prev,
                          lh->compare);
      Assert (link != lh->tail);
    }
    lh->current = link;

    /* Assert this is a valid link */
    return insertLinkHere (lh, lnk);
  case SLOWSORTED:
    Assert (NULL != lh->compare);
    if (NULL == lh->compare)
      return _ERROR_;
    link = lh->head->next;
    if (link == lh->tail) {
      lh->current = lh->head;
      return insertLinkHere (lh, lnk);
    }
    icompare = (*lh->compare) (lnk->data, lh->head->next->data);
    if (icompare < 0) {
      lh->current = lh->head;
      return insertLinkHere (lh, lnk);
    }
    icompare = (*lh->compare) (lnk->data, lh->tail->prev->data);
    if (icompare > 0) {
      lh->current = lh->tail->prev;
      return insertLinkHere (lh, lnk);
    }
    while (link != lh->tail) {
      icompare = (*lh->compare) (lnk->data, link->data);
      if (icompare < 0) {
        lh->current = link->prev;
        return insertLinkHere (lh, lnk);
      }
      link = link->next;
    }

    /*
     * Should never get here, but...
     */
    lh->current = lh->tail->prev;
    return insertLinkHere (lh, lnk);
  case STACKED:

      /*
       * Insert it at the end
       */
      if (_OK_ != lastLink (lh))
      return _ERROR_;
    return insertLinkHere (lh, lnk);
  case QUEUED:

    /*
     * Insert at the end
     */
    if (_OK_ != lastLink (lh))
      return _ERROR_;
    return insertLinkHere (lh, lnk);
  default:
    set_ListError(lh, LIST_UNKNOWN);
    return _ERROR_;
  }
}


Link * findSortedInsert (Link * lnk, int number, Link * begin, Link * end,
    int (*cmp) (const void *, const void *))
{
  Link * link;
  int i;

  number = 1;
  i = (*cmp) (lnk->data, end->data);
  if (i == 0)
    return end;
  i = (*cmp) (lnk->data, begin->data);
  if (i == 0)
    return begin;
  link = begin;
  if (number <= LIST_THRESHOLD) {
    for (link = begin; link != end->next; link = link->next) {
      i = (*cmp) (lnk->data, link->data);
      if (0 == i) {
        while (0 == i && link != end->next) {
          link = link->next;
          i = (*cmp) (lnk->data, link->data);
        }
        return link->prev;
      }
      if (i < 0)
        return link->prev;
    }
    return end;
  }
  link = begin;
  for (i = number / 2; i; i--)
    link = link->next;
  i = (*cmp) (lnk->data, link->data);
  if (0 == i) {
    while (1 == 1) {

        /*
         * skip all the links that are equal.
         */
        link = link->next;
      if (link == end)
        return link;
      if ((*cmp) (lnk->data, link->data) != 0)
        return link->prev;
    }
  }
  if (i < 0)
    return findSortedInsert (lnk, number / 2, begin, link, cmp);
  return findSortedInsert (lnk, number - number / 2, link, end, cmp);
}

/*
 * [BeginDoc]
 * \subsection{insertLinkHere}
 * \index{insertLinkHere}
 * [Verbatim] */

__inline__ int insertLinkHere (ListHeader * lh, Link * lnk)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * insertLinkHere() is the same as insertLink() except it inserts
 * the link ``lnk'' directly after lh->current in ``lh'',
 * regardless of the type
 * of list being maintained.  If lh->current == lh->tail, it is
 * moved back one level in the list before the insertion is done.
 * insertLinkHere() returns _OK_ on success, _ERROR_ on failure.
 * lh->current will be the newly inserted link after this function
 * returns.
 *
 * [EndDoc]
 */

{
  Link * link;
  Assert (NULL != lh->current);
  link = lh->current;
  if (link == lh->tail)
    link = link->prev;

    /*
     * link it in
     */
  lnk->prev = link;
  lnk->next = link->next;
  link->next->prev = lnk;
  link->next = lnk;
  lh->number++;
  lh->current = lnk;
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{removeLink}
 * \index{removeLink}
 * [Verbatim] */

Link *removeLink (ListHeader * lh)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * removeLink() removes the link pointed to by lh->current from
 * the list ``lh''.  It is simply unlinked from the list and a pointer
 * to it is returned.  Nothing is changed (or freed) in the link.
 * It is the user's responsibility to free the memory being used
 * by the link and the link->data members.  removeLink() returns
 * NULL if it is unsuccessful, a pointer to the link if it succeeds.
 *
 * Note:  the type of list lh points to will have some bearing on
 * where the data is removed from.  If it is a stack, for example,
 * a call to removeLink() is comparable to a call to pop() in other
 * systems (=> gives you the last item pushed onto the stack).
 * If the list is a queue, the link will be removed from the front.
 *
 * [EndDoc]
 */

{
  Link * lnk;
  if (lh->head->next == lh->tail)
    return NULL;
  if (lh->current == lh->head || lh->current == lh->tail)
    lh->current = lh->head->next;       /* first link by default */
  if (lh->current->prev == lh->head && lh->current->next == lh->tail) {
    lnk = lh->current;
    lh->current = lh->head;
    lh->head->next = lh->tail;
    lh->tail->prev = lh->head;
    lh->number--;
    return lnk;
  }
  switch (lh->type) {
  case SORTED:
  case SLOWSORTED:
  case UNSORTED:
    lnk = lh->current;
    lh->current = lnk->next;
    break;
  case QUEUED:
    lnk = lh->head->next;
    lh->current = lnk->next;
    break;
  case STACKED:
    lnk = lh->tail->prev;
    lh->current = lnk->prev;
    break;
  default:
    set_ListError(lh, LIST_UNKNOWN);
    return NULL;
  }

    /*
     * Now, unlink it.
     */
  lnk->prev->next = lnk->next;
  lnk->next->prev = lnk->prev;
  lnk->next = lnk->prev = NULL;
  lh->number--;
  return lnk;
}

/*
 * [BeginDoc]
 * \subsection{nextLink}
 * \index{nextLink}
 * [Verbatim] */

__inline__ int nextLink (ListHeader * lh)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * This moves the lh->current pointer in ``lh''
 * to the next node in the list.
 * if lh->current == lh->tail, this call does nothing.  Currently,
 * there is no diagnostic importance in the return value.
 *
 * [EndDoc]
 */

{
  lh->current = lh->current->next;
  if (lh->current == lh->tail)
    lh->current = lh->tail->prev;
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{prevLink}
 * \index{prevLink}
 * [Verbatim] */

__inline__ int prevLink (ListHeader * lh)

/* [EndDoc] */

/*
 * [BeginDoc]
 *
 * This moves the lh->current pointer in ``lh''
 * to the previous node in the list.
 * if lh->current == lh->head, this call does nothing.  Currently,
 * there is no diagnostic importance in the return value.
 *
 * [EndDoc]
 */

{
  lh->current = lh->current->prev;
  if (lh->current == lh->head)
    lh->current = lh->head->next;
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{firstLink}
 * \index{firstLink}
 * [Verbatim] */

__inline__ int firstLink (ListHeader * lh)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * This moves the lh->current pointer in ``lh''
 * to the first node in the list.
 * Currently, there is no diagnostic importance in the return value.
 * If there are no nodes in the list, lh->current will point to
 * lh->tail.
 *
 * [EndDoc]
 */

{
  lh->current = lh->head->next;
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{lastLink}
 * \index{lastLink}
 * [Verbatim] */

__inline__ int lastLink (ListHeader * lh)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * This moves the lh->current pointer in ``lh''
 * to the last node in the list.
 * Currently, there is no diagnostic importance in the return value.
 * If there are no nodes in the list, lh->current will point to
 * lh->head.
 *
 * [EndDoc]
 */

{
  lh->current = lh->tail->prev;
  return _OK_;
}

/*
 * Don't document these 2 functions -- I don't want anyone using them.  Their
 * performance is rather embarrassing.
 */
#if 0
Link *searchLink (ListHeader * lh, Link * lnk)
{
  if (!(SORTED == lh->type || SLOWSORTED == lh->type)) {
    set_ListError(lh, LIST_INVALID);
    return NULL;
  }
  return searchList (lnk, lh->number, lh->head->next, lh->tail->prev,
                      lh->compare);
}

Link *searchList (Link * lnk, int number, Link * begin, Link * end,
    int (*cmp) (const void *, const void *))
{
  Link * link;
  int i;
  i = (*cmp) (lnk->data, begin->data);
  if (0 == i)
    return begin;
  i = (*cmp) (lnk->data, end->data);
  if (0 == i)
    return end;
  number = 1;
  link = begin;
  if (number <= LIST_THRESHOLD) {
    for (link = begin; link != end->next; link = link->next)
      if (0 == (*cmp) (lnk->data, link->data))
        return link;
    return NULL;
  }
  link = begin;
  for (i = number / 2, link = begin; i; i--)
    link = link->next;
  i = (*cmp) (lnk->data, link->data);
  if (0 == i)
    return link;
  if (i < 0)
    return searchList (lnk, number / 2, begin, link, cmp);
  return searchList (lnk, number - number / 2, link, end, cmp);
}
#endif

/*
 * [BeginDoc]
 * \subsection{clearList}
 * \index{clearList}
 * [Verbatim] */

int clearList (ListHeader * lh)

/* [EndDoc] */

/*
 * [BeginDoc]
 *
 * clearList() clears the data from the list ``lh''.  If lh is empty,
 * clearList() does nothing.  Otherwise, each link is removed from
 * the list and the data member of the link and the link itself are
 * freed.  This function always returns _OK_, so the return value
 * means nothing.
 *
 * [EndDoc]
 */

{
  Link * lnk;
  if (lh == 0)
    return _ERROR_;
  if (isEmptyList (lh))
    return _OK_;
  firstLink (lh);
  lnk = removeLink (lh);
  while (NULL != lnk) {
    free (lnk->data);
    free (lnk);
    lnk = removeLink (lh);
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{storeList}
 * \index{storeList}
 * [Verbatim] */

int storeList (ListHeader * list, treeStore * ts, int reclen,
    const char *fname)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * Store a list to disk.  The ``list'' parameter is a pointer to an
 * allocated list header.  The list should contain allocated data
 * values of size ``reclen''.  The ``ts'' parameter is a pointer to
 * a treeStore structure.  The structure should be previously
 * allocated by the caller and the ``thisMagic'', ``timeStamp'' and
 * ``description'' members should be set before the call.  The fname
 * parameter is a string that gives the name of the data file that
 * the data is to be stored in.
 *
 * storeList() will return _OK_ if it is successful, _ERROR_ if it
 * couldn't store the data.
 *
 * [EndDoc]
 */

{
  int fd;
  off_t offset = 0L;
  long status = 0;
  off_t num;
  ts->number = list->number;
  ts->size = reclen;
  fd = fileCreate (fname);
  Assert (fd != _ERROR_);
  dbg2 (if (fd == _ERROR_)
         printf ("\nFile create error: %s\n", fioErrMsg[fioError]););
  if (_ERROR_ == fd) {
    set_ListError(list, LIST_FILE);
    return _ERROR_;
  }
  fileClose (fd);
  fd = fileOpen (fname);
  Assert (fd != _ERROR_);
  if (_ERROR_ == fd) {
    set_ListError(list, LIST_FILE);
    return _ERROR_;
  }

    /*
     * Store the treeStore structure
     */
    status = fileWrite (fd, ts, sizeof (treeStore));
  Assert (status == sizeof (treeStore));
  if (status != sizeof (treeStore)) {
    fileClose (fd);
    set_ListError(list, LIST_FILE);
    return _ERROR_;
  }
  offset += status;
  num = fileWriteBlock (fd, offset, reclen, list);
  Assert (num == (size_t) (reclen * list->number));
  if ((size_t)num != (reclen * list->number)) {
    fileClose (fd);
    set_ListError(list, LIST_FILE);
    return _ERROR_;
  }
  fileClose (fd);
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{saveList}
 * \index{saveList}
 * [Verbatim] */

int saveList (ListHeader * lh, const char *desc, int size, const char *file)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * saveList() saves the list ``lh'' to a file called ``file''.  A description
 * given by ``desc'' is stamped into the header block stored to the file.  If
 * desc is NULL, it is ignored.  The ``size'' parameter is the size in bytes
 * of each item (lnk->data) stored in the list.
 *
 * saveList() is simply a wrapper over the function storeList().  It is more
 * user friendly in that it doesn't require the user to allocate a ``treeStore''
 * for the save.  The diagnostic information is the same as for storeList().
 * [EndDoc]
 */

{
  time_t now;
  treeStore * ts;
  char tstamp[27];
  char *cp;
  int status;
  ts = malloc (sizeof (treeStore));
  if (0 == ts) {
    set_ListError(lh, LIST_NOMEM);
    return _ERROR_;
  }
  memset (ts, 0, sizeof (treeStore));
  check_pointer (ts);
  ts->thisMagic = DEFAULT_MAGIC;
  if (desc != 0)
    strncpy (ts->description, desc, 127);
  time (&now);
  strcpy (tstamp, asctime (localtime (&now)));
  cp = strchr (tstamp, '\n');
  if (cp != 0)
    *cp = '\0';
  strcpy (ts->timeStamp, tstamp);
  status = storeList (lh, ts, size, file);
  free (ts);
  return status;
}

/*
 * [BeginDoc]
 * \subsection{retrieveList}
 * \index{retrieveList}
 * [Verbatim] */

ListHeader *retrieveList (treeStore * ts, const char *fname, long magic)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * Retrieve the list from disk.  The ``ts'' parameter is a pointer
 * to a ``treeStore'' structure that should have been allocated by
 * the caller.  The ``fname'' parameter is the name of the file to
 * open and retrieve data from.  The ``magic'' member is the magic
 * number that was stored in the data file.  If this is incorrect,
 * retrieveList() will not open the data file.
 *
 * retrieveList() returns a pointer to an allocated list if
 * successful, NULL (or 0) if it fails.
 *
 * [EndDoc]
 */

{
  int fd;
  long status;
  ListHeader * lh = 0;
  fd = fileOpen (fname);
  if (_ERROR_ == fd) {
    set_ListError(0, LIST_FILE);
    return 0;
  }
  status = fileRead (fd, ts, sizeof (treeStore));
  if (status != sizeof (treeStore)) {
    fileClose (fd);
    set_ListError(0, LIST_FILE);
    return 0;
  }
  if (ts->thisMagic != (unsigned long) magic) {
    fileClose (fd);
    set_ListError(0, LIST_FILE);
    return 0;
  }
  lh = fileReadBlock (fd, sizeof (treeStore), ts->size, ts->number);
  if (0 == lh) {
    fileClose (fd);
    set_ListError(0, LIST_FILE);
    return 0;
  }
  fileClose (fd);
  return lh;
}

/*
 * [BeginDoc]
 * \subsection{getList}
 * \index{getList}
 * [Verbatim] */

ListHeader *getList (const char *file)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * getList() is a user friendly version of retrieveList() in that it doesn't
 * require the user to allocate a ``treeStore'' item.  getList() retrieves
 * the list in the file called ``file'' into a list and returns a pointer
 * to the list if successful.  The diagnostics for getList() are the
 * same as those for retrieveList().
 * [EndDoc]
 */

{
  ListHeader * lh;
  treeStore * ts;
  ts = malloc (sizeof (treeStore));
  if (0 == ts) {
    set_ListError(0, LIST_NOMEM);
    return 0;
  }
  memset (ts, 0, sizeof (treeStore));
  check_pointer (ts);
  lh = retrieveList (ts, file, DEFAULT_MAGIC);
  free (ts);
  return lh;
}

/*
 * [BeginDoc]
 * \subsection{mergeSorted}
 * \index{mergeSorted}
 * [Verbatim] */

ListHeader *mergeSorted (ListHeader * l1, ListHeader * l2)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * mergeSorted() merges two sorted lists into a single list and returns
 * a pointer to the resultant listHeader.  The lists passed in are
 * destroyed completely and the resulting list contains all the items
 * that were contained in both.  The compare function from ``l1'' is
 * used in the sort.  It is the caller's responsibility to insure that
 * the items in both lists are the same type and that both lists are
 * sorted.  The results will be undefined if not.
 * On error, mergeSorted() returns NULL.
 * [EndDoc]
 */

{
  ListHeader * lh;
  Link * lnk;
  Link * l1next, *l2next;
  int status;
  int l1tail = FALSE, l2tail = FALSE;
  if (l1->head->next == l1->tail) {
    if (l2->head->next == l2->tail) {
      set_ListError(0, LIST_PARAM);
      return 0;
    }

    else {
      delList (l1);
      return l2;
    }
  }

  else {
    if (l2->head->next == l2->tail) {
      delList (l2);
      return l1;
    }
  }
  lh = initList (UNSORTED, 0);
  if (0 == lh) {

      /* error set */
      return 0;
  }
  l1next = l1->head->next;
  l2next = l2->head->next;
  while (TRUE) {
    status = l1->compare (l1next->data, l2next->data);
    if (status <= 0) {
      lnk = l1next;
      l1next = l1next->next;
    }

    else {
      lnk = l2next;
      l2next = l2next->next;
    }
    lh->current = lh->tail->prev;
    insertLinkHere (lh, lnk);
    if (l1next == l1->tail) {
      l1tail = TRUE;
      break;
    }
    if (l2next == l2->tail) {
      l2tail = TRUE;
      break;
    }
  }
  if (l1next) {
    while (l1next != l1->tail) {
      lnk = l1next;
      l1next = l1next->next;
      lh->current = lh->tail->prev;
      insertLinkHere (lh, lnk);
    }
  }
  if (l2next) {
    while (l2next != l2->tail) {
      lnk = l2next;
      l2next = l2next->next;
      lh->current = lh->tail->prev;
      insertLinkHere (lh, lnk);
    }
  }

  lh->type = SLOWSORTED;
  lh->compare = l1->compare;
  l1->head->next = l1->tail;
  l1->tail->prev = l1->head;
  l1->number = 0;
  l2->head->next = l2->tail;
  l2->tail->prev = l2->head;
  l2->number = 0;
  delList (l1);
  delList (l2);
  return lh;
}

#ifdef  DEBUG

/*
 * [BeginDoc]
 * \subsection{DebugCheckList}
 * \index{DebugCheckList}
 * [Verbatim] */

int DebugCheckList (ListHeader * lh)

/* [EndDoc] */

/*
 * [BeginDoc]
 *
 * DebugCheckList() is defined if DEBUG is \#defined.  Otherwise, it
 * decays to an empty macro call.  It will slow down production code
 * fairly extensively, so it should only be used when debugging.
 * It does a fairly thorough check on the given list, checking the
 * validity of the list (is it a list being managed in the list
 * module), the validity of all the pointers in the list (using the
 * mymalloc module) including the list management pointers.
 *
 * [EndDoc]
 */

{
  ListHeader * plh;
  Link * lnk;
  int i;
  for (plh = __head->__n; plh != __tail; plh = plh->__n) {
    check_pointer (plh);
    check_pointer (plh->head);
    check_pointer (plh->tail);
  }
  for (plh = __head->__n; plh != __tail; plh = plh->__n) {
    if (plh == lh) {
      i = 1;
      lnk = lh->head->next;
      while (1 == 1) {
        if (lnk == lh->tail) {
          i--;
          if (i == lh->number)
            return _OK_;
          return _ERROR_;
        }

          /*check_pointer (lnk); */
          check_pointer (lnk->data);
        lnk = lnk->next;
        i++;
      }
    }
  }
  return _ERROR_;
}
#endif  /* DEBUG */

/*
 * [BeginDoc]
 *
 * \section{Shell Sort Routines}
 *
 * This is the shell sort module.  It uses the shell sort algorithm to
 * sort items.  The important data structures include a shellHeader
 * structure and a shellNode structure.  Essentially, the sorted data
 * \index{shellNode}
 * will be stored in a sorted linked list.  Indexed into the linked
 * list will be nodes that reference every few items in the linked
 * list (the number varies because of the algorithm for determining
 * where to put the nodes).  Each node will contain links for making a
 * linked list of nodes.  They will also contain higher level node pointers
 * that index into the levels right below them.  These higher level pointers
 * are maintained in an index and their number is determined by the constant
 * NODE_LEVEL.
 *
 * [EndDoc]
 */

/*
 * [BeginDoc]
 *
 * \subsection{set_shlError}
 * \index{set_shlError}
 *
 * [Verbatim] */

void set_shlError(shellHeader *shl, shellError er)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The ``set_shlError'' function is used to set the global ``shlError'' in
 * a thread\-safe way.  If the ``shl'' parameter is not NULL, ``set_shlError''
 * sets the local ``shlError'' variable (shl->shlError) to the value of
 * ``er''.  If ``shl'' is NULL, then the global ``shlError'' variable is
 * set to the value of ``er'' in a thread\-safe way.
 *
 * [EndDoc]
 */
{
  if (shl == 0) {
#ifdef HAVE_PTHREAD_H
    pthread_mutex_lock (&shlError_mutex);
#endif
    shlError = er;
#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock (&shlError_mutex);
#endif
  }
  else
    shl->shlError = er;
}

/*
 * [BeginDoc]
 * \subsection{initShell}
 * \index{initShell}
 * [Verbatim] */

shellHeader *initShell (int (*compare) (void *, void *), 
    int isUnique, int manageAllocs)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * initShell() is the starting point for using a shell sort.
 * It returns a shellHeader* if successful
 * and NULL if not.  It initializes the variables and prepares a
 * shell for entry of items.  ``compare'' is a pointer to a compare
 * function that will be used to determine sorted order for the items
 * added to the shell.\footnote{``compare'' \emph{cannot} be NULL.}
 * ``isUnique'' will determine whether a
 * constraint of uniqueness is forced on the data stored in the
 * shell.  If ``isUnique'' is true, any attempt to add an item that
 * is equal\footnote{Equality is determined by the compare function}
 * to an item that is already in the list will result in an
 * error.  If ``manageAllocs'' is TRUE, the shell will treat everything
 * that is added to it as if it is blocks that are allocated on the heap.
 * Otherwise, the shell will allow the calling application to manage those.
 * If you are going to be adding items that are pointed to by Links that are
 * not allocated on the heap, for example, set manageAllocs to FALSE.
 *
 * initShell() returns NULL (0) if an error occurs or a valid
 * pointer to a shellHeader (which can be used in other shell module
 * function calls) otherwise.  The string given by shlErrorStr[shlError]
 * will contain a description of the error that occurred.
 *
 * [EndDoc]
 */
{
  shellHeader *shl;

  if (isUnique != TRUE && isUnique != FALSE) {
    set_shlError(0, SHELL_PARAM);
    return 0;
  }
  if (compare == 0) {
    set_shlError(0, SHELL_PARAM);
    return 0;
  }
  shl = malloc (sizeof (shellHeader));
  if (0 == shl) {
    set_shlError(0, SHELL_NOMEM);
    return 0;
  }
  memset (shl, 0, sizeof (shellHeader));
  check_pointer (shl);
  shl->head = malloc (sizeof (shellNode));
  if (0 == shl->head) {
    free (shl);
    set_shlError(0, SHELL_NOMEM);
    return 0;
  }
  memset (shl->head, 0, sizeof (shellNode));
  check_pointer (shl->head);
  shl->tail = malloc (sizeof (shellNode));
  if (0 == shl->tail) {
    free (shl->head);
    free (shl);
    set_shlError(0, SHELL_NOMEM);
    return 0;
  }
  memset (shl->tail, 0, sizeof (shellNode));
  check_pointer (shl->tail);
  shl->head->prev = 0;
  shl->head->next = shl->tail;
  shl->tail->prev = shl->head;
  shl->tail->next = 0;
  shl->current = shl->head;
  shl->isUnique = isUnique;
  shl->manageAllocs = manageAllocs;
  shl->compare = compare;
  shl->lh = initList (UNSORTED, 0);
  if (0 == shl->lh) {
    free (shl->tail);
    free (shl->head);
    free (shl);
    set_shlError(0, SHELL_LIST);
    return 0;
  }
  shl->numNodes = 0;
  return shl;
}

/*
 * [BeginDoc]
 * \subsection{delShell}
 * \index{delShell}
 * [Verbatim] */

int delShell (shellHeader * shl, void (*delFunc) (void *))

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The delShell() function allows the user to delete the shell given
 * by ``shl''.  ``delFun'' is made available in case the items contained
 * by your shell are not simple allocations.  For example, if the items
 * in the shell are bucket items in a hash lookup implementation, each
 * bucket will typically contain the header item (the bucket structure) and
 * a list of items referenced by the bucket.  Destroying those data items
 * will involve more that a simple call to free().  Hence, the user would
 * write a delete function for the  bucket type and pass that to the delShell()
 * function to destroy the hash.  If the items stored in the shell are
 * singly allocated items (i.e.\ can be destroyed by a call to free()),
 * you can pass NULL as the second argument to delShell().
 *
 * There are no error diagnostics produced by the delShell() function.
 *
 * [EndDoc]
 */
{
  shellNode *node, *nextNode;
  Link *lnk;

  if (shl->head->next != shl->tail) {
    node = shl->head->next;
    nextNode = node->next;
    while (TRUE) {
      check_pointer (node);
      free (node);
      if (nextNode == shl->tail)
        break;
      node = nextNode;
      nextNode = node->next;
    }
  }
  if (!shl->manageAllocs)
    delFunc = returnClean;
  check_pointer (shl->head);
  free (shl->head);
  check_pointer (shl->tail);
  free (shl->tail);
  lnk = removeLink (shl->lh);
  while (lnk != 0) {
    if (delFunc)
      delFunc (lnk->data);
    else {
      check_pointer (lnk->data);
      free (lnk->data);
    }
    if (shl->manageAllocs)
      free (lnk);
    lnk = removeLink (shl->lh);
  }
  delList (shl->lh);
  check_pointer (shl);
  free (shl);
  return _OK_;
}

#define RSTRUCT_THRESH 10
#define OUTPOINT_THRESH (6*(NODE_LEVEL*OUTPOINT))

//static int thresh;

/*
 * [BeginDoc]
 * \subsection{addShellItem}
 * \index{addShellItem}
 * [Verbatim] */

int addShellItem (shellHeader * shl, Link * lnk)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * addShellItem() adds an item to the shell.  The item is sorted
 * according to the compare function.  Minimum parameter checking is
 * done to insure that parameters are passed correctly.  addShellItem()
 * returns _OK_ if everything is OK and _ERROR_ if an error
 * occurs.  If an error occurs, shlErrorStr[shl->shlError] will contain
 * a string that describes the error, where ``shl'' is the name of your
 * shell.  The following is a sample snippet of code:
 *
 * [Verbatim]

  shellHeader *shl;
  int status;
  ...
  shl = initShell (myCompare, TRUE, TRUE);
  ...
  status = addShellItem (shl, lnk);
  if (status == _ERROR_) {
    printf ("\n\n***Error: addShellItem: %s\n", shlErrorStr[shl->shlError]);
    ...
  }

 * [EndDoc] */
/*
 * [BeginDoc]
 *
 * In prior implementations of the shell sort, the global variable shlError
 * would be used, as in shlErrorStr[shlError], rather than the error
 * variable in the shellHeader itself (shl->shlError).  Using the global
 * variable caused problems with thread safety and was eliminated.
 *
 * [EndDoc]
 */
{
  register int status;
  register Link *link;
  Link *midpoint = 0;
  int linkCount;
  register shellNode *node;
  shellNode *midnode = 0;
  shellNode *oldLevel[NODE_LEVEL];
  register int nodeCount = 0;
  int i;
#ifdef STATS
  double t1, t2;
#endif

  /*
   * Stopgap restructure of shell nodes based on previous call(s).
   * This keeps the shell tree balanced.
   */
  if (shl->numCompares > OUTPOINT_THRESH) {
    ++shl->thresh;
    if (shl->thresh > RSTRUCT_THRESH) {
      shl->thresh = 0;
      status = restructureShellNodes (shl);
      if (status == _ERROR_)
        return _ERROR_;
    }
  }
  shl->numCompares = 0; /* reset the counter */
  dbg (
  if (shl->manageAllocs) {
    check_pointer (lnk);
    check_pointer (lnk->data);
  }
      );
  /* Case 1: no items in the node list */
  check_pointer (shl);
  if (shl->head->next == shl->tail) {
    /* no items in the list - just add it after the head */
    if (shl->lh->head->next == shl->lh->tail) {
      shl->lh->current = shl->lh->head;
      insertLinkHere (shl->lh, lnk);
      return _OK_;
    }
    dbg (
    if (shl->manageAllocs)
      check_pointer (shl->lh->head->next->data);
        );
    /*
     * There are items in the list - find the place and add them.
     * If we pass the threshold, add a node to the node list.
     */
    status = shl->compare (lnk->data, shl->lh->head->next->data);
    shl->numCompares++;
    if (status < 0) {
      shl->lh->current = shl->lh->head;
      insertLinkHere (shl->lh, lnk);
      return _OK_;
    }
    if (status == 0) {
      if (shl->isUnique) {
        set_shlError(shl, SHELL_UNIQUE);
        return _ERROR_;
      }
      link = shl->lh->head->next;
      dbg (
      if (shl->manageAllocs)
        check_pointer (link->next->data);
          );
      if (link->next->data != 0) {
        status = shl->compare (lnk->data, link->next->data);
        shl->numCompares++;
      }
      while (status == 0 && link->next != shl->lh->tail) {
        link = link->next;
        check_pointer (link->next->data);
        status = shl->compare (lnk->data, link->next->data);
        shl->numCompares++;
      }
      shl->lh->current = link;
      insertLinkHere (shl->lh, lnk);
      return _OK_;
    }
    dbg (
    if (shl->manageAllocs)
      check_pointer (shl->lh->tail->prev->data);
        );
    status = shl->compare (lnk->data, shl->lh->tail->prev->data);
    shl->numCompares++;
    if (status >= 0) {
      shl->lh->current = shl->lh->tail->prev;
      insertLinkHere (shl->lh, lnk);
      return _OK_;
    }
    link = shl->lh->head->next;
    linkCount = 1;
    while (link != shl->lh->tail) {
      if (linkCount == MIDPOINT) {
        midpoint = link;
      }
      if (linkCount == OUTPOINT) {
        status = newShellNode (shl, shl->head, shl->tail, midpoint);
        if (status == _ERROR_)
          return _ERROR_;
      }
      dbg (
      if (shl->manageAllocs)
        check_pointer (link->data);
          );
      status = shl->compare (lnk->data, link->data);
      shl->numCompares++;
      if (status < 0) {
        shl->lh->current = link->prev;
        insertLinkHere (shl->lh, lnk);
        return _OK_;
      }
      link = link->next;
      linkCount++;
    }
    set_shlError(shl, SHELL_CORRUPT);
    return _ERROR_;
  }

  /*
   * There are indexed items in the node list -
   * check endpoints of list first.  If not,
   * start with the highest level first and work down.
   */
  dbg (
  if (shl->manageAllocs)
    check_pointer (shl->lh->head->next->data);
      );
  status = shl->compare (lnk->data, shl->lh->head->next->data);
  shl->numCompares++;
  /* it's less than anything already there - add it to the begging */
  if (status < 0) {
    shl->current = shl->head->next;
    shl->lh->current = shl->lh->head;
    insertLinkHere (shl->lh, lnk);
    return _OK_;
  }
  /*
   * it's equal to the first item there...add it right after
   * the ones already there
   */
  if (status == 0 && shl->isUnique == TRUE) {
    /* DOH! */
    set_shlError(shl, SHELL_UNIQUE);
    return _ERROR_;
  }
  if (status == 0) {
    shl->current = shl->head->next;
    link = shl->lh->head->next;
    dbg (
    if (shl->manageAllocs)
      check_pointer (link->next->data);
        );
    shl->lh->current = link;
    insertLinkHere (shl->lh, lnk);
    return _OK_;
  }
  dbg (
  if (shl->manageAllocs)
    check_pointer (shl->lh->tail->prev->data);
      );
  /*
   * It's not equal to the first one...is it equal to or greater than
   * the last one?
   */
  status = shl->compare (lnk->data, shl->lh->tail->prev->data);
  shl->numCompares++;
  if (status == 0 && shl->isUnique == TRUE) {
    set_shlError(shl, SHELL_UNIQUE);
    return _ERROR_;
  }
  if (status >= 0) {
    shl->current = shl->tail->prev;
    shl->lh->current = shl->lh->tail->prev;
    insertLinkHere (shl->lh, lnk);
    return _OK_;
  }
#ifdef STATS
  elapsed (&t1);
#endif
  /*
   * None of the above...let's find a home for it.
   */
  node = shl->head->next;
  while (node->level[NODE_LEVEL-1] != shl->tail) {
    dbg (
    if (shl->manageAllocs)
      check_pointer (node->level[NODE_LEVEL-1]->here->data);
        );
    status = shl->compare (lnk->data, node->level[NODE_LEVEL-1]->here->data);
    shl->numCompares++;
    if (status == 0 && shl->isUnique == TRUE) {
      set_shlError(shl, SHELL_UNIQUE);
      return _ERROR_;
    }
    if (status <= 0)
      break;
    node = node->level[NODE_LEVEL-1];
  }
  for (i = NODE_LEVEL-2; i >= 0; i--) {
    nodeCount = 0;
    oldLevel[i+1] = node;
    while (node->level[i] != shl->tail) {
      dbg (
      if (shl->manageAllocs)
        check_pointer (node->level[i]->here->data);
          );
      status = shl->compare (lnk->data, node->level[i]->here->data);
      shl->numCompares++;
      if (status == 0 && shl->isUnique == TRUE) {
        set_shlError(shl, SHELL_UNIQUE);
        return _ERROR_;
      }
      if (status <= 0)
        break;
      node = node->level[i];
      dbg (
	if(node == node->level[i]) {
	  printf ("node == node->level[%d]\n", i);
	  printf ("node->level[i]->here->data = %s\n",
	    node->level[i]->here->data);
	});
      if (node == node->level[i]) {
	set_shlError(shl, SHELL_CORRUPT);
	return _ERROR_;
      }
      nodeCount++;
      if (nodeCount == MIDPOINT)
        midnode = node;
      if (nodeCount == OUTPOINT) {
        midnode->level[i+1] = oldLevel[i+1]->level[i+1];
        oldLevel[i+1]->level[i+1] = midnode;
      }
    }
  }
#ifdef STATS
  elapsed (&t2);
  hln = t2-t1;
  elapsed (&t1);
#endif
  nodeCount = 0;
  oldLevel[0] = node;
  while (node != shl->tail) {
    dbg (
    if (shl->manageAllocs)
      check_pointer (node->here->data);
        );
    status = shl->compare (lnk->data, node->here->data);
    shl->numCompares++;
    if (status == 0 && shl->isUnique == TRUE) {
      set_shlError(shl, SHELL_UNIQUE);
      return _ERROR_;
    }
    if (status <= 0) {
      goto insertNode;
    }
    dbg (
      if (node == node->next)
        printf ("\nnode == node->next\n");
    );
    if (node == node->next) {
      set_shlError(shl, SHELL_CORRUPT);
      return _ERROR_;
    }
    node = node->next;
    nodeCount++;
    if (nodeCount == MIDPOINT)
      midnode = node;
    if (nodeCount == OUTPOINT) {
      midnode->level[0] = oldLevel[0]->level[0];
      oldLevel[0]->level[0] = midnode;
    }
  }

insertNode:

#ifdef STATS
  elapsed (&t2);
  zln=t2-t1;
  elapsed (&t1);
#endif

  if (node->prev == shl->head)
    link = shl->lh->head->next;
  else
    link = node->prev->here;
  linkCount = 0;
  while (link != shl->lh->tail) {
    if (linkCount == MIDPOINT) {
      midpoint = link;
    }
    if (linkCount == OUTPOINT) {
      linkCount= 0;
      status = newShellNode (shl, node->prev, node, midpoint);
      if (status == _ERROR_)
        return _ERROR_;
    }
    dbg (
    if (shl->manageAllocs)
      check_pointer (link->data);
        );
    status = shl->compare (lnk->data, link->data);
    shl->numCompares++;
    if (status == 0 && shl->isUnique == TRUE) {
      set_shlError(shl, SHELL_UNIQUE);
      return _ERROR_;
    }
    if (status <= 0) {
      shl->current = node->prev;
      shl->lh->current = link->prev;
      insertLinkHere (shl->lh, lnk);
#ifdef STATS
      elapsed (&t2);
      llt=t2-t1;
#endif
      return _OK_;
    }
    dbg (
      if (link == link->next)
        printf ("\nlink == link->next\n");
    );
    if (link == link->next) {
      set_shlError(shl, SHELL_CORRUPT);
      return _ERROR_;
    }
    link = link->next;
    linkCount++;
  }
  set_shlError(shl, SHELL_CORRUPT);
  return _ERROR_;
}

int newShellNode (shellHeader *shl, shellNode *prevNode,
    shellNode *nextNode, Link *here)
{
  shellNode *newNode;
  int i;

  newNode = malloc (sizeof (shellNode));
  if (newNode == 0) {
    set_shlError(shl, SHELL_NOMEM);
    return _ERROR_;
  }
  /*memset (newNode, 0, sizeof (shellNode));*/
  if (shl->manageAllocs) {
    check_pointer (newNode);
  }
  newNode->prev = prevNode;
  newNode->next = nextNode;
  prevNode->next = newNode;
  nextNode->prev = newNode;
  newNode->here = here;
  for (i = 0; i < NODE_LEVEL; i++)
    newNode->level[i] = shl->tail;
  shl->current = newNode;
  shl->numNodes++;
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{removeShellItem}
 * \index{removeShellItem}
 * [Verbatim] */

Link *removeShellItem (shellHeader * shl, Link * lnk)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * removeShellItem () removes the item given by ``lnk'' from the shell
 * ``shl''.  ``lnk'' must be an actual link that is being managed in
 * the shell (i.e.\ it cannot simply be an allocated link with lnk->data
 * pointing to something that is like an item in the shell).  If an error
 * occurs, removeShellItem() returns NULL (0).  It returns a valid pointer
 * to the link lnk on success.  It is the caller's responsibility to
 * destroy memory used to store lnk and lnk->data.
 *
 * It should be understood by the caller that deletions from a shell are
 * inherently slow (much slower than adding items to it).  If speed is of the
 * utmost, the user should rethink the application to deem whether deletions are
 * really necessary.
 *
 * If an error occurs, shlErrorStr[shl->shlError] contains a string that
 * describes the error, where ``shl'' is the name of your shell.
 *
 * [EndDoc]
 */
{
  shellNode *node, *nextNode;
  Link *link, *found;
  int status;

  shl->numCompares = 0;
  if (shl->isUnique) {
    found = findShellItem (shl, lnk);
    if (0 == found) {
      set_shlError(shl, SHELL_CORRUPT);
      return 0;
    }
    if (lnk == shl->current->here) {
      shl->lh->current = lnk;
      link = removeLink (shl->lh);
      if (link == 0) {
	set_ListError(0, shl->lh->ListError);
        set_shlError(shl, SHELL_LIST);
        return 0;
      }
      // This sucks but is necessary.  You don't know here if you are
      // deleting a link that is pointed to by a node.  If so and you don't
      // do this, you will have corruption.  This is what slows down a
      // remove to the point where it is almost useless in practice.
      restructureShellNodes (shl);
      return link;
    }
    else {
      shl->lh->current = lnk;
      link = removeLink (shl->lh);
      if (link == 0) {
	set_ListError(0, shl->lh->ListError);
        set_shlError(shl, SHELL_LIST);
        return 0;
      }
      // Ditto here.
      restructureShellNodes (shl);
      return link;
    }
  }
  /*
   * Not a unique shell - have to jump through some hoops.
   */
  found = queryShellItem (shl, lnk);
  if (found == 0) {
    set_shlError(shl, SHELL_CORRUPT);
    return 0;
  }
  node = shl->current;
  if (node->here == lnk) {
    /*
     * We are on a node
     */
    shl->lh->current = lnk;
    link = removeLink (shl->lh);
    if (link == 0) {
      set_ListError(0, shl->lh->ListError);
      set_shlError(shl, SHELL_LIST);
      return 0;
    }
    // Ditto here.
    status = restructureShellNodes (shl);
    return link;
  }
  nextNode = node->next;
  if (nextNode->here != 0) {
    status = shl->compare (lnk->data, nextNode->here->data);
    shl->numCompares++;
    while (status == 0) {
      if (nextNode->here == lnk) {
        shl->lh->current = lnk;
        link = removeLink (shl->lh);
        if (0 == link) {
	  set_ListError(0, shl->lh->ListError);
          set_shlError(shl, SHELL_LIST);
          return 0;
        }
	// Ditto here.
        status = restructureShellNodes (shl);
        return link;
      }
      nextNode = nextNode->next;
      if (nextNode == shl->tail || nextNode->here == 0)
        break;
      else {
        status = shl->compare (lnk->data, nextNode->here->data);
        shl->numCompares++;
      }
    }
  }
  /*
   * It looks like the link is not on a node; so, remove it.
   */
  shl->lh->current = lnk;
  link = removeLink (shl->lh);
  if (0 == link) {
    set_shlError(shl, SHELL_CORRUPT);
    return 0;
  }
  return link;
}

/*
 * [BeginDoc]
 * \subsection{findShellItem}
 * \index{findShellItem}
 * [Verbatim] */

Link *findShellItem (shellHeader * shl, Link * lnk)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * findShellItem() finds the first item identical to the item pointed
 * to by ``lnk'' in the shell given by ``shl''.  If there is no item
 * in the shell identical to lnk->data, findShellItem returns NULL (0);
 * otherwise it returns a
 * pointer to the Link that points to the item.  Upon return of a NULL
 * value, the value of shl->shlError should be checked to make sure that
 * an error didn't occur.  If an error \emph{did not} occur, shl->shlError
 * will be SHELL_NOERR.  Any other non-zero value indicates an error
 * condition.
 *
 * [EndDoc]
 */
{
  int status = 0;
  register Link *link;
  register shellNode *node;
  int i;

  if (shl->numCompares > OUTPOINT_THRESH) {
    ++shl->thresh;
    if (shl->thresh > RSTRUCT_THRESH) {
      shl->thresh = 0;
      status = restructureShellNodes (shl);
      if (status == _ERROR_)
        return 0;
    }
  }
  shl->numCompares = 0;
  if (shl->head->next == shl->tail) {
    link = shl->lh->head->next;
    while (link != shl->lh->tail) {
      status = shl->compare (lnk->data, link->data);
      shl->numCompares++;
      if (status == 0) {
        shl->lh->current = link;
        return link;
      }
      link = link->next;
    }
    return 0;
  }
  status = shl->compare (lnk->data, shl->lh->head->next->data);
  shl->numCompares++;
  if (status == 0) {
    shl->current = shl->head->next;
    return shl->lh->head->next;
  }
  if (status < 0)
    return 0;
  status = shl->compare (lnk->data, shl->lh->tail->prev->data);
  shl->numCompares++;
  if (status == 0) {
    link = shl->lh->tail->prev;
    while (link != shl->lh->head) {
      status = shl->compare (lnk->data, link->data);
      shl->numCompares++;
      if (status != 0)
        break;
      link = link->prev;
    }
    shl->current = shl->tail->prev;
    return link->next;
  }
  if (status > 0)
    return 0;
  node = shl->head->next;
  for (i = NODE_LEVEL-1; i >= 0; i--) {
    while (node->level[i] != shl->tail) {
      status = shl->compare (lnk->data, node->level[i]->here->data);
      shl->numCompares++;
      if (status <= 0)
        break;
      node = node->level[i];
    }
  }
  while (node->next != shl->tail) {
    status = shl->compare (lnk->data, node->next->here->data);
    shl->numCompares++;
    if (status <= 0) {
      link = node->here;
      if (node == shl->head->next) {
        link = shl->lh->head->next;
      }
      while (link != node->next->here) {
        status = shl->compare (lnk->data, link->data);
        shl->numCompares++;
        if (status == 0) {
          shl->current = node;
          shl->lh->current = link;
          return link;
        }
	      if (status < 0)
	        return 0;
        link = link->next;
      }
    }
    node = node->next;
  }
  if (node->next == shl->tail) {
    link = node->here;
    while (link != shl->lh->tail) {
      status = shl->compare (lnk->data, link->data);
      shl->numCompares++;
      if (status == 0) {
        shl->current = shl->tail->prev;
        shl->lh->current = link;
        return link;
      }
      if (status < 0)
	      return 0;
      link = link->next;
    }
  }
  return 0;
}

/*
 * [BeginDoc]
 * \subsection{queryShellItem}
 * \index{queryShellItem}
 * [Verbatim] */

Link *queryShellItem (shellHeader *shl, Link *lnk)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * queryShellItem() works the same as findShellItem() except in the
 * case where it does not find an exact match for the item.  In that
 * case, it returns the item in the sorted list before the point where
 * the item would be logically added.  The only time a return value of
 * NULL (0) is returned is in the case of an error, in which case
 * shl->shlError will contain the error.
 *
 * [EndDoc]
 */
{
  int status;
  Link *link;
  shellNode *node;
  int i;

  if (shl->numCompares > OUTPOINT_THRESH) {
    ++shl->thresh;
    if (shl->thresh > RSTRUCT_THRESH) {
      shl->thresh = 0;
      status = restructureShellNodes (shl);
      if (status == _ERROR_)
        return 0;
    }
  }
  shl->numCompares = 0;
  if (shl->head->next == shl->tail) {
    if (shl->lh->head->next == shl->lh->tail) {
      set_shlError(shl, SHELL_PARAM);
      return 0;
    }
    link = shl->lh->head->next;
    while (link != shl->lh->tail) {
      status = shl->compare (lnk->data, link->data);
      shl->numCompares++;
      if (status == 0) {
        shl->lh->current = link;
        return link;
      }
      if (status < 0) {
        if (link->prev != shl->lh->head)
          return link->prev;
        else
          return link;
      }
      link = link->next;
    }
    return shl->lh->tail->prev;
  }
  status = shl->compare (lnk->data, shl->lh->head->next->data);
  shl->numCompares++;
  if (status == 0) {
    shl->current = shl->head->next;
    return shl->lh->head->next;
  }
  if (status < 0)
    return shl->lh->head->next;
  status = shl->compare (lnk->data, shl->lh->tail->prev->data);
  shl->numCompares++;
  if (status == 0) {
    if (shl->isUnique)
      return shl->lh->tail->prev;
    link = shl->lh->tail->prev;
    while (link != shl->lh->head) {
      status = shl->compare (lnk->data, link->data);
      shl->numCompares++;
      if (status != 0)
        break;
      link = link->prev;
    }
    shl->current = shl->tail->prev;
    if (link->next != shl->lh->tail)
      link = link->next;
    return link;
  }
  if (status > 0) {
    shl->current = shl->tail->prev;
    return shl->lh->tail->prev;
  }
  node = shl->head->next;
  for (i = NODE_LEVEL-1; i >= 0; i--) {
    while (node->level[i] != shl->tail) {
      status = shl->compare (lnk->data, node->level[i]->here->data);
      shl->numCompares++;
      if (status <= 0)
        break;
      node = node->level[i];
    }
  }
  while (node->next != shl->tail) {
    status = shl->compare (lnk->data, node->next->here->data);
    shl->numCompares++;
    if (status <= 0) {
      link = node->here;
      if (node == shl->head->next) {
        link = shl->lh->head->next;
      }
      while (link != node->next->here) {
        status = shl->compare (lnk->data, link->data);
        shl->numCompares++;
        if (status == 0) {
          shl->current = node;
          shl->lh->current = link;
          return link;
        }
        if (status < 0) {
          shl->current = node;
          if (link->prev != shl->lh->head) {
            shl->lh->current = link->prev;
            return link->prev;
          }
          else {
            shl->lh->current = link;
            return link;
          }
        }
        link = link->next;
      }
    }
    node = node->next;
  }
  if (node->next == shl->tail) {
    link = node->here;
    while (link != shl->lh->tail) {
      status = shl->compare (lnk->data, link->data);
      shl->numCompares++;
      if (status == 0) {
        shl->current = shl->tail->prev;
        shl->lh->current = link;
        return link;
      }
      if (status < 0) {
        shl->current = node;
        if (link->prev != shl->lh->head) {
          shl->lh->current = link->prev;
          return link->prev;
        }
        else {
          shl->lh->current = link;
          return link;
        }
      }
      link = link->next;
    }
  }
  shl->current = shl->tail->prev;
  return shl->lh->tail->prev;
}

/*
 * [BeginDoc]
 * \subsection{storeShell}
 * \index{storeShell}
 * [Verbatim] */

int storeShell (shellHeader *shl, treeStore *ts, int reclen, const char *fname)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The storeShell() function stores the contents of the shell ``shl''
 * to the file given by ``fname''.  The items are assumed to be of
 * length ``reclen''.  For documentation on how the ``ts'' data item is
 * used and how it should be populated, see the discussion of
 * the storeList() function in the list documentation.  storeList()
 * returns _OK_ on success, _ERROR_ when an error occurs.  If an error
 * occurs, shlErrorStr[shl->shlError] will contain a description of the
 * error.
 *
 * [EndDoc]
 */
{
  int status;

  ts->isUnique = shl->isUnique;
  status = storeList (shl->lh, ts, reclen, fname);
  Assert (status != _ERROR_);
  if (_ERROR_ == status) {
    set_ListError(0, shl->lh->ListError);
    set_ListError(shl->lh, LIST_NOERROR);
    set_shlError(shl, SHELL_LIST);
    return _ERROR_;
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{saveShell}
 * [Verbatim] */

int saveShell (shellHeader *shl, const char *desc, int reclen,
    const char *fname)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * \index{saveShell}
 *
 * saveShell() stores the shell given by ``shl'' to the file given by ``fname''.
 * saveShell() differs from storeShell() in that the user doesn't have to
 * allocate or populate a ``treeStore'' data parameter.
 * ``desc'' is a descriptive string that is stored in the 
 * header of the file and ``reclen'' is the length
 * of each item that is stored.  saveShell() returns _OK_ if it is successful,
 * _ERROR_ if an error occurs.  If an error occurs,
 * shlErrorStr[shlError] will contain a string describing the error.
 *
 * [EndDoc]
 */
{
  time_t now;
  treeStore *ts;
  char tstamp[27];
  char *cp;
  int status;

  ts = malloc (sizeof (treeStore));
  if (0 == ts) {
    set_shlError(shl, SHELL_NOMEM);
    return _ERROR_;
  }
  memset (ts, 0, sizeof (treeStore));
  check_pointer (ts);

  ts->thisMagic = DEFAULT_MAGIC;
  if (desc)
    strncpy (ts->description, desc, 127);
  time (&now);
  strcpy (tstamp, asctime (localtime (&now)));
  cp = strchr (tstamp, '\n');
  if (cp != 0)
    *cp = '\0';
  strcpy (ts->timeStamp, tstamp);
  status = storeShell (shl, ts, reclen, fname);
  free (ts);
  return status;
}

/*
 * [BeginDoc]
 * \subsection{retrieveShell}
 * \index{retrieveShell}
 * [Verbatim] */

int retrieveShell (shellHeader *shl, treeStore *ts,
    const char *fname, long magic)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * retrieveShell() retrieves the data stored in the file given by
 * ``fname'' into the shell given by ``shl''.
 * ``shl'' should have been allocated
 * and the compare function set when this function is called.  Also,
 * ``ts'' should be allocated and prepared to be used by the retrieveList()
 * function.  ``magic'' is the magic number used to store the contents of
 * the shell to disk.  retrieveShell() returns _OK_ if successful, _ERROR_
 * otherwise.  If an error occurs, shlErrorStr[shl->shlError] will contain
 * a string describing the error.
 *
 * [EndDoc]
 */
{
  int status;
  int counter, ctr_lvl[NODE_LEVEL];
  int thisNumber = RESTRUCT;
  Link *lnk;
  shellNode *node, *oldLNode;
  int i;

  for (i = 0; i < NODE_LEVEL; i++)
    ctr_lvl[i] = 0;
  shl->lh = retrieveList (ts, fname, magic);
  if (0 == shl->lh) {
    // global ListError is already set.
    set_shlError(shl, SHELL_LIST);
    return _ERROR_;
  }
  shl->head = malloc (sizeof (shellNode));
  if (0 == shl->head) {
    set_shlError(shl, SHELL_NOMEM);
    return _ERROR_;
  }
  memset (shl->head, 0, sizeof (shellNode));
  check_pointer (shl->head);
  shl->tail = malloc (sizeof (shellNode));
  if (0 == shl->tail) {
    free (shl->head);
    set_shlError(shl, SHELL_NOMEM);
    return _ERROR_;
  }
  memset (shl->tail, 0, sizeof (shellNode));
  check_pointer (shl->tail);
  shl->current = shl->head;
  shl->isUnique = ts->isUnique;
  shl->head->next = shl->tail;
  shl->tail->prev = shl->head;
  shl->head->prev = 0;
  shl->tail->next = 0;

  if (shl->lh->number < (size_t)thisNumber) {
    shl->head->prev = 0;
    shl->head->next = shl->tail;
    shl->tail->prev = shl->head;
    shl->tail->next = 0;
    return _OK_;
  }
  else {
    /*
     * Set up index pointers to links.
     */
    lnk = shl->lh->head->next;
    status = newShellNode (shl, shl->head, shl->tail, lnk);
    if (_ERROR_ == status) {
      free (shl->head);
      shl->head = 0;
      free (shl->tail);
      shl->tail = 0;
      return _ERROR_;
    }
    while (lnk != shl->lh->tail) {
      for (counter = 0; counter < thisNumber && lnk != shl->lh->tail;
           counter++, lnk = lnk->next);
      if (lnk == shl->lh->tail)
        break;
      status = newShellNode (shl, shl->tail->prev, shl->tail, lnk);
      if (_ERROR_ == status)
        return _ERROR_;
    }
    if (shl->numNodes > thisNumber) {
      node = shl->head->next;
      oldLNode = node;
      while (node != shl->tail) {
        for (counter = 0; counter < thisNumber && node != shl->tail;
             counter++, node = node->next);
        if (node == shl->tail)
          break;
        oldLNode->level[0] = node;
        oldLNode = node;
        ctr_lvl[0]++;
      }
    }
    for (i = 0; i < NODE_LEVEL-1; i++) {
      if (ctr_lvl[i] > thisNumber) {
        node = shl->head->next;
        oldLNode = node;
        while (node != shl->tail) {
          for (counter=0; counter < thisNumber && node != shl->tail;
              counter++, node = node->level[i]);
          if (node == shl->tail)
            break;
          oldLNode->level[i+1] = node;
          oldLNode = node;
          ctr_lvl[i+1]++;
        }
      }
    }
  }
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{getShell}
 * \index{getShell}
 * [Verbatim] */

shellHeader *getShell (int (*compare) (void *, void *), const char *fname)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The getShell() function is similar to retrieveShell() except that the user
 * doesn't have to allocate a treeStore data structure for the function to
 * use.  ``compare'' is a pointer to a function that is used to compare the
 * data items and ``fname'' is the name of the file to open.  getShell() returns
 * NULL (0) if an error occurs, a valid pointer to a shellHeader if it succeeds.
 * If an error occurs, shlErrorString[shlError] will contain a string that
 * describes the error.
 *
 * [EndDoc]
 */
{
  shellHeader *shl;
  treeStore *ts;
  int status;

  shl = malloc (sizeof (shellHeader));
  if (0 == shl) {
    set_shlError(0, SHELL_NOMEM);
    return 0;
  }
  memset (shl, 0, sizeof (shellHeader));
  check_pointer (shl);
  shl->compare = compare;

  ts = malloc (sizeof (treeStore));
  if (0 == ts) {
    free (shl);
    set_shlError(0, SHELL_NOMEM);
    return 0;
  }
  check_pointer (ts);

  status = retrieveShell (shl, ts, fname, DEFAULT_MAGIC);
  free (ts);
  if (status == _ERROR_) {
    delShell (shl, 0);
    return 0;
  }
  return shl;
}

/*
 * [BeginDoc]
 * \subsection{restructureShellNodes}
 * \index{restructureShellNodes}
 *
 * [Verbatim] */

int restructureShellNodes (shellHeader *shl)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The ``restructureShellNodes'' function restructures the shell nodes in the
 * shell given by ``shl''.  This function should rarely need to be called by
 * applications, but is provided in case there is a need.  Basically, when a
 * shell sort is built from adding somewhat random data, the indexes that
 * allow fast traversal and adding can get a little out of sorts.  For example,
 * the ``distance'' between one node and another node in the index could be
 * very great (thousands of links).  So, if you search for an item that happens
 * to be between one of these nodes, you could do thousands of comparisons
 * before you arrive at the item you want to find.  Needless to say, this
 * ``badness'' effects the performance of the shell.  The
 * ``restructureShellNodes'' function restructures the shell nodes in a shell
 * to eliminate this problem.  There is an algorithm that is used by
 * addShellItem that forces a restructure when a certain threshold of compares
 * is reached for a single add or search.  This should be sufficient in
 * practice to keep the shell fairly clean.
 *
 * If an error occurs, _ERROR_ is returned and shlErrorStr[shl->shlError]
 * contains a description of the error.
 *
 * [EndDoc]
 */
{
  shellNode *node, *nextNode, *oldLNode;
  Link *lnk;
  int thisNumber = RESTRUCT;
  int status;
  int i;
  int counter, ctr_lvl[NODE_LEVEL];

  for (i = 0; i < NODE_LEVEL; i++)
    ctr_lvl[i] = 0;
  /* step 1 - remove shell nodes (if exist) */
  if (shl->head->next != shl->tail) {
    node = shl->head->next;
    nextNode = node->next;
    while (TRUE) {
      free (node);
      if (nextNode == shl->tail)
        break;
      node = nextNode;
      nextNode = node->next;
    }
  }
  shl->head->prev = 0;
  shl->head->next = shl->tail;
  shl->tail->prev = shl->head;
  shl->tail->next = 0;
  shl->current = shl->head;
  shl->numNodes = 0;

  /* step 2 - rebuild shell nodes (if necessary) */
  if (shl->lh->number < (size_t)thisNumber)
    return _OK_;
  lnk = shl->lh->head->next;
  status = newShellNode (shl, shl->head, shl->tail, lnk);
  if (_ERROR_ == status) {
    return _ERROR_;
  }
  while (lnk != shl->lh->tail) {
    for (counter = 0; counter < thisNumber && lnk != shl->lh->tail;
         counter++, lnk = lnk->next);
    if (lnk == shl->lh->tail)
      break;
    status = newShellNode (shl, shl->tail->prev, shl->tail, lnk);
    if (_ERROR_ == status) {
      return _ERROR_;
    }
  }
  /* step 3 - rebuild indexes */
  /* traverse the nodes first, building the first high-order index */
  if (shl->numNodes > thisNumber) {
    node = shl->head->next;
    oldLNode = node;
    while (node != shl->tail) {
      for (counter = 0; counter < thisNumber && node != shl->tail;
           counter++, node = node->next);
      if (node == shl->tail)
        break;
      oldLNode->level[0] = node;
      oldLNode = node;
      ctr_lvl[0]++;
    }
  }
  /* Then, traverse subsequent high-order index levels */
  for (i = 0; i < NODE_LEVEL-1; i++) {
    if (ctr_lvl[i] > thisNumber) {
      node = shl->head->next;
      oldLNode = node;
      while (node != shl->tail) {
        for (counter = 0; counter < thisNumber && node != shl->tail;
            counter++, node = node->level[i]);
        if (node == shl->tail)
          break;
        oldLNode->level[i+1] = node;
        oldLNode = node;
        ctr_lvl[i+1]++;
      }
    }
  }
  shl->numRestruct ++;
  return _OK_;
}

/*
 * [BeginDoc]
 * \subsection{nodeLevels}
 * \index{nodeLevels}
 * [Verbatim] */

int nodeLevels (shellHeader *shl, shellLevel *lvl)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * nodeLevels() returns statistics regarding how many of the indexed levels in
 * a shell ``shl'' is being used.  Each node in a shell contains multi-level
 * indexes (stored in an array).
 * The information about how many of each of these levels is being
 * used by a shell is calculated and returned in the shellLevel variable ``lvl''
.
 * There is no diagnostic importance to the return value of the nodeLevels()
 * function.
 *
 * [EndDoc]
 */
{
  int counter = 0;
  shellNode *node;
  int i;

  for (i = NODE_LEVEL-1, counter = 0; i >= 0; i--) {
    node = shl->head->next;
    if (node->level[i] == shl->tail)
      lvl->lvl[i] = 0;
    else {
      while (node->level[i] != shl->tail) {
        node = node->level[i];
        counter++;
      }
      lvl->lvl[i] = counter;
    }
  }
  return _OK_;
}

void returnClean (void *p)
{
  if (p)
    return;
}

/*
 * [BeginDoc]
 * \subsection{shlQsort}
 * \index{shlQsort}
 * [Verbatim] */

shellHeader *shlQsort (void *base, size_t numItems, size_t recSize,
          int (*qcompare) (const void *, const void *),
          int (*scompare) (void *, void *))

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * shlQsort() uses a fast quick sort to sort the data passed into it.  ``base''
 * is a contiguous array of ``numItems'' of size ``recSize''.  The function
 * ``qcompare'' is used to compare the values for sorting.  The ``scompare''
 * function is the compare function that is used by the shell.   These can be
 * the same function, although you will have to use a typecast on one of them
 * because of the ``const void *'' verses the ``void *'' parameters.
 *
 * It should be noted that the first four parameters to this function are the
 * same that you would pass into the ANSI C qsort function.  So, if you 
 * already are set up for a qsort call, as follows:
 * [Verbatim]

  qsort ((void*)array, numit, sizit, mycompare);

 * [EndDoc] */
/*
 * [BeginDoc]
 *
 * From this you can make a simple change and call shlQsort() instead, as
 * follows:
 * [Verbatim]

  shellHeader *shl;
  shl = shlQsort((void*)array, numit, sizit, mycompare, 
    (int(*)(void*,void*)) mycompare);
  if (shl == 0) {
    ... handle the error ...
  }
  ... use the shell ...

 * [EndDoc] */
/*
 * [BeginDoc]
 *
 * You might ask, ``Why bother?''.  The answer lies in the reason that I
 * designed the shell sort in the first place.  I needed a sort that I could
 * search into and that I could populate, save to disk, retrieve from disk,
 * add records to and delete records from, save to disk, etc.  A qsort'ed
 * list doesn't give itself to these applications.  However, a fast qsort is
 * slightly faster than a shell sort for a large number of items.  So, I can
 * have the benefits of the shell sort with the initial sorting speed of the
 * quick sort by using this function.
 *
 * If an error occurs, shlQsort returns NULL and shlErrorStr[shlError]
 * contains a string that describes the error.
 *
 * [EndDoc]
 */
{
  unsigned int i;
  Link *lnk;
  shellHeader *shl;
  int status;
  void **cpp;

  bqsort (base, numItems, recSize, qcompare);

  shl = malloc (sizeof (shellHeader));
  if (0 == shl) {
    set_shlError(0, SHELL_NOMEM);
    return 0;
  }
  memset (shl, 0, sizeof (shellHeader));
  check_pointer (shl);
  shl->compare = scompare;

  shl->lh = initList (UNSORTED, 0);
  if (0 == shl->lh) {
    // global ListError set.
    set_shlError(0, SHELL_LIST);
    return 0;
  }
  shl->head = malloc (sizeof (shellNode));
  if (0 == shl->head) {
    set_shlError(0, SHELL_NOMEM);
    return 0;
  }
  memset (shl->head, 0, sizeof (shellNode));
  shl->tail = malloc (sizeof (shellNode));
  if (0 == shl->tail) {
    free (shl->head);
    set_shlError(0, SHELL_NOMEM);
    return 0;
  }
  memset (shl->tail, 0, sizeof (shellNode));
  shl->current = shl->head;
  shl->isUnique = FALSE;
  shl->head->next = shl->tail;
  shl->tail->prev = shl->head;
  shl->head->prev = 0;
  shl->tail->next = 0;

  cpp = (void **) base;
  for (i = 0; i < numItems; i++) {
    if (*(cpp + i) == 0)
      break;
    lnk = malloc (sizeof (Link));
    if (lnk == 0) {
      set_shlError(0, SHELL_NOMEM);
      free (shl);
      return 0;
    }
    lnk->data = (*(cpp + i));
    shl->lh->current = shl->lh->tail->prev;
    insertLinkHere (shl->lh, lnk);
  }
  status = restructureShellNodes (shl);
  if (_ERROR_ == status) {
    delShell (shl, returnClean);
    /* error is set */
    return 0;
  }
  return shl;
}

/*
 * [BeginDoc]
 * \subsection{shlListQsort}
 * \index{shlListQsort}
 * [Verbatim] */

shellHeader *shlListQsort (ListHeader *lh, size_t recSize,
    int (*compare) (const void *, const void *))

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * shlListQsort() uses a fast quick sort to sort the items that are in the
 * list given by ``lh''.  ``recSize'' is the size of the records and ``compare''
 * is the compare function used in the sort (it also becomes the compare
 * function used in the populated shell).  shlListQsort returns NULL if it
 * fails and a pointer to a populated shell if it succeeds.  If there is an
 * error, shlErrorStr[shlError] contains a string that describes the error.
 *
 * [EndDoc]
 */
{
  char *sortArray = 0;
  Link *lnk;
  shellHeader *shl;
  int status;
  register int counter;

  sortArray = malloc (recSize * lh->number + 1);
  if (0 == sortArray) {
    set_shlError(0, SHELL_NOMEM);
    return 0;
  }
  check_pointer (sortArray);

  /*
   * populate sortArray
   */
  lnk = lh->head->next;
  counter = 0;
  while (lnk != lh->tail) {
    if ((size_t)counter >= lh->number) {
      set_shlError(0, SHELL_CORRUPT);
      free (sortArray);
      return 0;
    }
    memmove (sortArray + counter * recSize, lnk->data, recSize);
    counter++;
    lnk = lnk->next;
  }
  bqsort ((void *) sortArray, lh->number, recSize, compare);
  /*
   * Now, put the sorted data back into the list and build the shell.
   */
  lnk = lh->head->next;
  counter = 0;
  while (lnk != lh->tail) {
    if ((size_t)counter >= lh->number) {
      set_shlError(0, SHELL_CORRUPT);
      free (sortArray);
      return 0;
    }
    memmove (lnk->data, sortArray + counter * recSize, recSize);
    counter++;
    lnk = lnk->next;
  }

  shl = malloc (sizeof (shellHeader));
  if (0 == shl) {
    set_shlError(0, SHELL_NOMEM);
    free (sortArray);
    return 0;
  }
  memset (shl, 0, sizeof (shellHeader));
  check_pointer (shl);
  shl->compare = (int (*)(void *, void *)) compare;

  shl->lh = lh;
  shl->head = malloc (sizeof (shellNode));
  if (0 == shl->head) {
    set_shlError(0, SHELL_NOMEM);
    free (sortArray);
    return 0;
  }
  memset (shl->head, 0, sizeof (shellNode));
  shl->tail = malloc (sizeof (shellNode));
  if (0 == shl->tail) {
    free (shl->head);
    free (sortArray);
    set_shlError(0, SHELL_NOMEM);
    return 0;
  }
  memset (shl->tail, 0, sizeof (shellNode));
  shl->current = shl->head;
  shl->isUnique = FALSE;
  shl->head->next = shl->tail;
  shl->tail->prev = shl->head;
  shl->head->prev = 0;
  shl->tail->next = 0;

  status = restructureShellNodes (shl);
  if (_ERROR_ == status) {
    /* error is set */
    free (sortArray);
    delShell (shl, 0);
    return 0;
  }
  free (sortArray);
  return shl;
}

/*
 * [BeginDoc]
 * \subsection{shl2List}
 * \index{shl2List}
 * [Verbatim] */

ListHeader *shl2List (shellHeader *shl)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * shl2List destroys the shell pointed to by ``shl'' and just returns the
 * list that is maintained within the shell.  The type of the list is set
 * to SLOWSORTED and the compare function is set to the shell's compare
 * function.  The only condition that would make shl2List() fail is bad
 * parameters.  Otherwise,
 * it is just dismantling an existing shell and returning the list that is
 * part of that, which is pretty no-brainerish.
 * 
 * [EndDoc]
 */
{
  ListHeader *lh;
  shellNode *node, *nextNode;

  check_pointer (shl);
  if (shl == 0 || shl->lh == 0) {
    set_shlError(0, SHELL_PARAM);
    // Don't even touch the shl; major corruption.
    return 0;
  }
  lh = shl->lh;
  check_pointer (lh);
  shl->lh = 0;
  lh->type = SLOWSORTED;
  lh->compare = (int (*)(const void *, const void *)) shl->compare;

  /*
   * Now, just dismantle the shell and return the list.
   */
  if (shl->head->next != shl->tail) {
    node = shl->head->next;
    nextNode = node->next;
    while (TRUE) {
      check_pointer (node);
      free (node);
      if (nextNode == shl->tail)
        break;
      node = nextNode;
      nextNode = node->next;
    }
  }
  check_pointer (shl->head);
  free (shl->head);
  check_pointer (shl->tail);
  free (shl->tail);
  check_pointer (shl);
  free (shl);
  return lh;
}

/*
 * [BeginDoc]
 * \subsection{list2Shell}
 * \index{list2Shell}
 * [Verbatim] */

shellHeader *list2Shell (ListHeader *lh)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * list2Shell() takes a list pointed to by ``lh'' and builds a shell around
 * it.  Dire things will happen if the data in lh is \emph{not} sorted.  On
 * error, list2Shell() returns NULL and shlErrorStr[shlError] contains
 * a string that describes the error.
 *
 * [EndDoc]
 */
{
  shellHeader *shl;
  int status;

  shl = malloc (sizeof (shellHeader));
  if (0 == shl) {
    set_shlError(0, SHELL_NOMEM);
    return 0;
  }
  memset (shl, 0, sizeof (shellHeader));
  check_pointer (shl);
  shl->compare = (int (*)(void *, void *)) lh->compare;

  shl->lh = lh;
  shl->head = malloc (sizeof (shellNode));
  if (0 == shl->head) {
    set_shlError(0, SHELL_NOMEM);
    return 0;
  }
  memset (shl->head, 0, sizeof (shellNode));
  shl->tail = malloc (sizeof (shellNode));
  if (0 == shl->tail) {
    free (shl->head);
    set_shlError(0, SHELL_NOMEM);
    return 0;
  }
  memset (shl->tail, 0, sizeof (shellNode));
  shl->current = shl->head;
  shl->isUnique = FALSE;
  shl->head->next = shl->tail;
  shl->tail->prev = shl->head;
  shl->head->prev = 0;
  shl->tail->next = 0;

  status = restructureShellNodes (shl);
  if (_ERROR_ == status) {
    /* error is set */
    delShell (shl, 0);
    return 0;
  }
  return shl;
}

/*#define HASH_INC 32897 * 128*2^8+128+1 - if you use hsh4 */
/*#define HASH_INC 8421504 * if you use hsh8 */

/*#define HASH_INC 128*8+128 */
#define HASH_INC ((256<<16)+(256<<8)+256)

/*
 * [BeginDoc]
 * \subsection{shsort}
 * \index{shsort}
 * [Verbatim] */

int shsort (void **p, size_t num, int ((*cmp)(void*p1,void*p2)))

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * shsort() is a function that takes an array of C strings (``p'') and uses
 * a shell sort to sort them.  The ``num'' parameter is the number of items
 * to sort and ``cmp'' is the function to do the sort by.  This is test code
 * in which I was pitting the shell sort up against the system qsort() and
 * a fast qsort() included in this distribution as bqsort().\footnote{See
 * the qosrt.c file in the lib directory.}  For a performance comparison,
 * see the stuff in the lib/sort directory of the distribution.
 *
 * There is a characteristic of the shell sort provided here that is
 * different than qsort().  Essentially, shell performs better if the data
 * is already partially sorted.  To facilitate that, I use a quick hashing
 * scheme that results in the items passed in being sorted by the first 3
 * characters.\footnote{The implication of this is that this routine will not
 * work correctly if numerical or blob data is being sorted.}  Under these
 * circumstances, the shell sort is typically faster than the recursive
 * ANSI C qsort() provided with GNU C although not as fast as the highly
 * optimized bqsort().  This function is not thread safe and should not be
 * used in multi-threaded applications.
 *
 * [EndDoc]
 */
{
  int status;
  register int i;
  Link *plnk;
  register Link *lnk;
  shellHeader *shl;
  int sort_min = 0, sort_max = 0;
  register int this_val = 0;
  static int hashUsed = FALSE;
  static Link *hash[HASH_INC];

  if (hashUsed) {
    for (i = 0; i < HASH_INC; i++) {
      hash[i] = 0;
    }
  }
  else {
    hashUsed = TRUE;
  }
  plnk = malloc (num*sizeof (Link));
  if (0 == plnk) {
    set_shlError(0, SHELL_NOMEM);
    return _ERROR_;
  }

  shl = initShell (cmp, FALSE, FALSE);
  if (0 == shl) {
    return _ERROR_;
  }
  /*
   * First, hash the elements into a rough ascii order
   */
  for (i = 0; i < (int)num; i++) {
    plnk[i].data = p[i];
    this_val = (((unsigned char**)p)[i][0]<<16)+
      (((unsigned char**)p)[i][1]<<8)+
      (((unsigned char**)p)[i][2]);
    if (sort_min == 0)
      sort_min = this_val;
    if (sort_max == 0)
      sort_max = this_val;
    if (sort_min > this_val)
      sort_min = this_val;
    if (sort_max < this_val)
      sort_max = this_val;
    if (hash[this_val] == 0) {
      hash[this_val] = &plnk[i];
      plnk[i].next = 0;
    }
    else {
      plnk[i].next = hash[this_val];
      hash[this_val] = &plnk[i];
    }
  }
  /*
   * Next, add the hashed items to the shell.
   */
  for (i = sort_min; i <= sort_max; i++) {
    if (hash[i] != 0) {
      while (hash[i] != 0) {
        lnk = hash[i];
        hash[i] = hash[i]->next;
        status = addShellItem (shl, lnk);
        if (status == _ERROR_) {
          set_shlError (0, shl->shlError);
          delShell (shl, 0);
          return _ERROR_;
        }
      }
    }
  }
  /*
   * Finally, put the sorted items back into the array and clean up.
   */
  for (i = 0, lnk = shl->lh->head->next; 
      lnk != shl->lh->tail; 
      i++, lnk=lnk->next)
    p[i] = lnk->data;
  delShell(shl, 0);
  free (plnk);
  return _OK_;
}

#if 0
#define THSORT_NUM_THREAD 4
#define MEGEXTRA 1000000

typedef struct _thsort_thread_data {
	shellHeader *shl;
	int threadnum;
	int number;
	int status;
	char **cpp;
	Link **lpp;
} thsortThreadData;

void *thsortPopulateShell (void *tt)
{
	int i;
	thsortThreadData *t;
	int status;

	t = (thsortThreadData *)tt;
	for (i = 0; i < t->number; i++) {
		if (t->lpp[i] == 0)
			break;
		if (t->lpp[i]->data == 0)
			break;
		status = addShellItem (t->shl, t->lpp[i]);
		if (status == _ERROR_) {
  		if (t->shl->shlError == SHELL_UNIQUE) {
  			set_shlError (t->shl, SHELL_NOERR);
  			continue;
  		}
		  t->status = _ERROR_;
		  return (void*)0;
		}
	}
	pthread_exit ((void*)0);
	return (void*)0;
}

/*
 * [BeginDoc]
 * \subsection{shsort}
 * \index{shsort}
 * [Verbatim] */

int thsort (void **p, size_t num, int ((*cmp)(void*p1,void*p2)))

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * thsort() is a function that takes an array of C strings (``p'') and uses
 * a group of shells to sort them.  The ``num'' parameter is the number
 * of items to sort and ``cmp'' is the function to do the sort by.  This
 * is code written in the ever reaching attempt to sho performance comparable
 * to qsort, perportedly the fastest sort algorithm available.  This code
 * compares favorably with a non-recursive fast qsort() included in this
 * distribution as bqsort().
 *
 * I use two techniques to get performance on par with qsort.  First, the
 * data is presorted using a fast hash algorithm that sorts the strings
 * by the first three characters.\footnote{Note that, as with the shsort
 * function, the implication of this is that you can't use this function to
 * sort integers or other non-string data.}  Secondly, I thread the actual
 * sorts across multiple instances of shell sort and then use a merge sort to
 * put it all back together.
 *
 * [EndDoc]
 */
{
	thsortThreadData *tt[THSORT_NUM_THREAD];
	pthread_t th[THSORT_NUM_THREAD];
	pthread_attr_t attr;
  int status;
  int i, j, k, l;
  int itemcount;
  int count[THSORT_NUM_THREAD];
  ListHeader *lhs[THSORT_NUM_THREAD];
  ListHeader *final_lh, *tmp_lh;
  Link *lnk;
  Link *plnk;
	Link **hash;
  register unsigned int this_val = 0;
  int sort_min = 0, sort_max = 0;

	hash = malloc (HASH_INC*sizeof (Link*));
	if (hash == 0) {
		set_shlError (0, SHELL_NOMEM);
		return _ERROR_;
	}
	memset (hash, 0, HASH_INC*sizeof (Link*));
  plnk = malloc (num*sizeof (Link));
  if (0 == plnk) {
    set_shlError(0, SHELL_NOMEM);
    return _ERROR_;
  }
  /*
   * First, hash the elements into a rough ascii order
   */
  for (i = 0; i < (int)num; i++) {
    plnk[i].data = p[i];
    this_val = (((unsigned char**)p)[i][0]<<16)+
      (((unsigned char**)p)[i][1]<<8)+
      (((unsigned char**)p)[i][2]);
    if (sort_min == 0)
      sort_min = this_val;
    if (sort_max == 0)
      sort_max = this_val;
    if ((unsigned)sort_min > this_val)
      sort_min = this_val;
    if ((unsigned)sort_max < this_val)
      sort_max = this_val;
    if (hash[this_val] == 0) {
      hash[this_val] = &plnk[i];
      plnk[i].next = 0;
    }
    else {
      plnk[i].next = hash[this_val];
      hash[this_val] = &plnk[i];
    }
  }
	// Call it insurance.
	if (num % THSORT_NUM_THREAD == 0) {
		itemcount = num/THSORT_NUM_THREAD;
	}
	else {
		itemcount = num/THSORT_NUM_THREAD+1;
	}
	for (i = 0; i < THSORT_NUM_THREAD; i++) {
		tt[i] = malloc (sizeof (thsortThreadData));
		if (0 == tt[i]) {
			set_shlError (0, SHELL_NOMEM);
			free (plnk);
			free (hash);
			return _ERROR_;
		}
		memset (tt[i], 0, sizeof (thsortThreadData));
		tt[i]->threadnum = i;
		tt[i]->number = itemcount;
		tt[i]->shl = initShell (cmp, FALSE, FALSE);
		if (tt[i]->shl == 0) {
			// error already set.
			free (plnk);
			free (tt[i]);
			free (hash);
			return _ERROR_;
		}
		tt[i]->cpp = malloc (itemcount * sizeof (char*));
		if (tt[i]->cpp == 0) {
			set_shlError (0, SHELL_NOMEM);
			free (plnk);
			free (hash);
			delShell (tt[i]->shl, 0);
			free (tt[i]);
			return _ERROR_;
		}
		memset (tt[i]->cpp, 0, itemcount*sizeof (char*));
		tt[i]->lpp = malloc (itemcount * sizeof (Link*));
		if (tt[i]->lpp == 0) {
			set_shlError (0, SHELL_NOMEM);
			free (plnk);
			free (hash);
			delShell (tt[i]->shl, 0);
			free (tt[i]->cpp);
			free (tt[i]);
			return _ERROR_;
		}
		memset (tt[i]->lpp, 0, itemcount*sizeof (Link*));
	}
	// i (sort_min-> <=sort_max), j (0-> <THSORT_NUM_THREAD),
	// k (0-> <itemcount), l (0->num)
  i = sort_min; k = 0; l = 0;
  while (i <= sort_max && (unsigned)l < num) {
    for (j = 0; j < THSORT_NUM_THREAD && k < itemcount; j++) {
		  if (hash[i] == 0) {
				while (hash[i] == 0) {
			    i++;
			    if (i > sort_max)
				    goto thsortDoneLoop; /* Bug out */
				}
			}
		  lnk = hash[i];
			hash[i] = hash[i]->next;
			tt[j]->cpp[k] = (char *)p[l++];
			lnk->data = tt[j]->cpp[k];
			tt[j]->lpp[k] = lnk;
		}
		if (k++ >= itemcount)
			goto thsortDoneLoop;
   }

thsortDoneLoop:

	// OK, sort the data in threads.
	pthread_attr_init (&attr);
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE);
	for (i = 0; i < THSORT_NUM_THREAD; i++) {
		status = pthread_create (&th[i], &attr, thsortPopulateShell, tt[i]);
		if (status) {
			set_shlError (0, SHELL_TH_CREATE);
			free (plnk);
			for (j = 0; j < THSORT_NUM_THREAD; j++) {
			  delShell (tt[j]->shl, 0);
		    free (tt[j]);
			}
			free (hash);
			return _ERROR_;
		}
	}
	pthread_attr_destroy (&attr);
	// Connect to the threads and wait for them to finish.
	for (i = 0; i < THSORT_NUM_THREAD; i++) {
		status = pthread_join (th[i], (void**)&count[i]);
		if (status || tt[i]->status) {
			set_shlError (0, SHELL_TH_JOIN);
			free (plnk);
			for (j = 0; j < THSORT_NUM_THREAD; j++) {
			  delShell (tt[j]->shl, 0);
		    free (tt[j]);
			}
			free (hash);
			return _ERROR_;
		}
	}
	for (i = 0; i < THSORT_NUM_THREAD; i++) {
		printf ("Shell sort %d has %d items\n", i, tt[i]->shl->lh->number);
		lhs[i] = shl2List (tt[i]->shl);
	}
	//
	// Merge the sorted lists.
	//
	if (THSORT_NUM_THREAD > 2) {
		final_lh = mergeSorted (lhs[0], lhs[1]);
		if (final_lh == 0) {
			set_shlError (0, SHELL_LIST);
			free (plnk);
			for (j = 0; j < THSORT_NUM_THREAD; j++) {
			  delShell (tt[j]->shl, 0);
		    free (tt[j]);
			}
			free (hash);
			return _ERROR_;
		}
		for (i = 2; i < THSORT_NUM_THREAD; i++) {
			tmp_lh = final_lh;
			final_lh = mergeSorted (tmp_lh, lhs[i]);
			if (final_lh == 0) {
				set_shlError (0, SHELL_LIST);
			  free (plnk);
			  for (j = 0; j < THSORT_NUM_THREAD; j++) {
			    delShell (tt[j]->shl, 0);
		      free (tt[j]);
			  }
				free (hash);
				return _ERROR_;
			}
		}
	}
	else {
		final_lh = mergeSorted (lhs[0], lhs[1]);
		if (final_lh == 0) {
			set_shlError (0, SHELL_LIST);
			free (plnk);
			free (hash);
			for (j = 0; j < THSORT_NUM_THREAD; j++) {
			  delShell (tt[j]->shl, 0);
		    free (tt[j]);
			}
			return _ERROR_;
		}
	}
  /*
   * Finally, put the sorted items back into the array and clean up.
   */
  for (i = 0, lnk = final_lh->head->next; 
      lnk != final_lh->tail; 
      i++, lnk=lnk->next)
    p[i] = lnk->data;
  free (plnk);
	free (hash);
	for (j = 0; j < THSORT_NUM_THREAD; j++) {
	  /*delShell (tt[j]->shl, 0);*/
    free (tt[j]);
	}
  return _OK_;
}
#endif

/*
 * This is a hash algorithm from Aho's book on compiler design.
 * It is purported to be good with strings.
 */
static unsigned int _dcdb_hash_function(const char *key)
{
  unsigned int val = 0;

  while (*key != '\0') {
    unsigned int tmp;
    val = (val << 4) + (*key);
    tmp = (val & 0xf0000000);
    if (tmp) {
      val = val ^ (tmp >> 24);
      val = val ^ tmp;
    }
    key++;
  }
  return val % HASH_SIZE;
}

/*
 * [BeginDoc]
 * \subsection{initHash}
 * \index{initHash}
 *
 * [Verbatim] */

hashHeader *initHash (
    int (*compare) (void *, void *),
    unsigned int (*hash) (const char *) )
/* [EndDoc] */
/*
 * The ``initHash'' function initializes a search hash and returns a
 * pointer to a ``hashHeader'' if successful and NULL if it fails or there
 * is an error.  The ``compare'' function is used to sort the items that
 * are added to the shells that make up the buckets on the hash search.
 * The ``compare'' function should be defined as follows:
 * [Verbatim]

int thisCompare (void *p1, void *p2)
{
  return (strcmp ((char*)p1, (char*)p2));
}

 * [EndDoc]
 * [BeginDoc]
 * 
 * The ``hash'' function is used to get a hash of the data items passed
 * in on adds.  The requirements for a good hash function are beyond the
 * scope of this document.  If ``hash'' is 0 (NULL), then a hash function
 * is used that is known to be good with strings and is used for compiler
 * implementations.
 *
 * [EndDoc]
 */
{
  int i, j;
  hashHeader *hsh;

  if (compare == 0) {
    set_shlError(0, SHELL_PARAM);
    return 0;
  }
  // Set reasonable defaults.
  hsh = malloc (sizeof (hashHeader));
  if (0 == hsh) {
    set_shlError(0, SHELL_NOMEM);
    return 0;
  }
  memset (hsh, 0, sizeof (hashHeader));
  if (hash == 0)
    hsh->hash = _dcdb_hash_function;
  else
    hsh->hash = hash;
  for (i = 0; i < HASH_SIZE; i++) {
    hsh->shls[i] = initShell (compare, TRUE, TRUE);
    if (hsh->shls[i] == 0) {
      for (j = 0; j < i; j++) {
	delShell (hsh->shls[j], 0);
	free (hsh);
      }
      // error condition is already set.
      return 0;
    }
  }
  return hsh;
}

/*
 * [BeginDoc]
 * \subsection{addHashItem}
 * \index{addHashItem}
 *
 * [Verbatim] */

int addHashItem (hashHeader *hsh, const char *item)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * ``addHashItem'' adds the item given by ``item'' to the hash given by ``hsh''.
 * A memory buffer is allocated on the heap and ``item'' is copied into it
 * before it is added.
 * [EndDoc]
 */
{
  int status;
  unsigned int hash_offset;
  Link *lnk;
  char *data;

  if (hsh == 0 || item == 0 || item[0] == '\0') {
    set_shlError(0, SHELL_PARAM);
    return _ERROR_;
  }
  hash_offset = hsh->hash (item);
  lnk = malloc (sizeof (Link));
  if (0 == lnk) {
    set_shlError(0, SHELL_NOMEM);
    return _ERROR_;
  }
  data = malloc (strlen (item)+1);
  if (0 == data) {
    set_shlError(0, SHELL_NOMEM);
    free (lnk);
    return _ERROR_;
  }
  strcpy (data, item);
  lnk->data = data;
  hsh->numCompares = 0;
  status = addShellItem (hsh->shls[hash_offset], lnk);
  if (status != _ERROR_)
    hsh->number++;
  hsh->numCompares = hsh->shls[hash_offset]->numCompares;
  if (hsh->shls[hash_offset]->shlError != SHELL_NOERR) {
    set_shlError(0, hsh->shls[hash_offset]->shlError);
    set_shlError(hsh->shls[hash_offset], SHELL_NOERR);
  }
  return status;
}

/*
 * [BeginDoc]
 * \subsection{findHashItem}
 * \index{findHashItem}
 *
 * [Verbatim] */

char *findHashItem (hashHeader *hsh, const char *item)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The ``findHashItem'' function searches for the string given by ``item'' in
 * the hash given by ``hsh''.  It returns NULL if nothing is found and on
 * error.  You should check the value of shlError on return to insure that an
 * error didn't occur.
 *
 * [EndDoc]
 */
{
  Link *found;
  Link *lnk;
  register char *data;
  unsigned int hash_offset;

  if (hsh == 0 || item == 0 || item[0] == '\0') {
    set_shlError(0, SHELL_PARAM);
    return 0;
  }
  hash_offset = hsh->hash (item);
  lnk = malloc (sizeof (Link));
  if (0 == lnk) {
    set_shlError(0, SHELL_NOMEM);
    return 0;
  }
  data = malloc (strlen (item)+1);
  if (0 == data) {
    set_shlError(0, SHELL_NOMEM);
    free (lnk);
    return 0;
  }
  strcpy (data, item);
  check_pointer (data);
  lnk->data = data;
  hsh->numCompares = 0;
  found = findShellItem (hsh->shls[hash_offset], lnk);
  hsh->numCompares = hsh->shls[hash_offset]->numCompares;
  free (data);
  free (lnk);
  if (found != 0)
    return found->data;
  else {
    set_shlError(0, hsh->shls[hash_offset]->shlError);
    set_shlError(hsh->shls[hash_offset], SHELL_NOERR);
  }
  return 0;
}

/*
 * [BeginDoc]
 * \subsection{delHash}
 * \index{delHash}
 *
 * [Verbatim] */

int delHash (hashHeader *hsh)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * The ``delHash'' function deletes the hash given by ``hsh''.  There is no
 * error diagnostics provided with this function and it always returns _OK_.
 *
 * [EndDoc]
 */
{
  int i;
  if (hsh == 0) {
    set_shlError(0, SHELL_PARAM);
    return _ERROR_;
  }
  for (i = 0; i < HASH_SIZE; i++) {
    delShell (hsh->shls[i], 0);
    hsh->shls[i] = 0;
  }
  free (hsh);
  return _OK_;
}


#ifdef  DEBUG

static int debugCheckShellCompare (void *p1, void *p2)
{
  return (strcmp ((char*)p1, (char*)p2));
}

/*
 * Note: this may cause testing to be much slower.  For example, say you have
 * a problem that occurs after each add.  You may want to to include a call
 * to this after each add.  However, if you have lots of items you are adding,
 * this will slow down the adds to a crawl.  But, hey, debugging code is
 * supposed to be exhaustive, not fast :o)
 */
void
debugCheckShell (shellHeader * shl)
{
  shellNode *node;
  Link *lnk;
  int status;
  int i;
  char *nodep;
  Link *link;
  shellHeader *dbgshl;

  dbgshl = initShell (debugCheckShellCompare, TRUE, TRUE);
  if (dbgshl == 0) {
    printf ("\n\n***Error: error initializing shell in debugCheckShell\n");
    set_shlError(0, SHELL_UNSPECIFIED);
    return;
  }
  if (!check_pointer (shl)) {
    delShell (dbgshl, 0);
    set_shlError(0, SHELL_UNSPECIFIED);
    return;
  }
  if (!check_pointer (shl->head)) {
    delShell (dbgshl, 0);
    set_shlError(0, SHELL_UNSPECIFIED);
    return;
  }
  if (!check_pointer (shl->tail)) {
    delShell (dbgshl, 0);
    set_shlError(0, SHELL_UNSPECIFIED);
    return;
  }
  Assert (shl->head->prev == 0);
  Assert (shl->tail->next == 0);
  node = shl->head->next;
  while (node != shl->tail) {
    nodep = malloc (12);
    if (0 == nodep) {
      printf ("\n\n***Error: error allocating nodep in debugCheckShell\n");
      delShell (dbgshl, 0);
      set_shlError(0, SHELL_UNSPECIFIED);
      return;
    }
    snprintf (nodep, 10,"%x", node);
    link = malloc (sizeof (Link));
    if (0 == link) {
      printf ("\n\n***Error: error allocating Link in debugCheckShell\n");
      delShell (dbgshl, 0);
      set_shlError(0, SHELL_UNSPECIFIED);
      return;
    }
    link->data = nodep;
    status = addShellItem (dbgshl, link);
    if (status == _ERROR_) {
      if (shlError == SHELL_UNIQUE) {
	printf ("\n\n***Error: node %s is a duplicate\n", nodep);
	delShell (dbgshl, 0);
	set_shlError(0, SHELL_UNSPECIFIED);
	return;
      }
    }
    if (!check_pointer (node->next)) {
      delShell (dbgshl, 0);
      set_shlError(0, SHELL_UNSPECIFIED);
      return;
    }
    if (!check_pointer (node->prev)) {
      delShell (dbgshl, 0);
      set_shlError(0, SHELL_UNSPECIFIED);
      return;
    }
    for (i = 0; i < NODE_LEVEL-1; i++) {
      if (!check_pointer (node->level[i])) {
      delShell (dbgshl, 0);
	set_shlError(0, SHELL_UNSPECIFIED);
	return;
      }
    }
    /*node = node->next;*/
    node = node->level[0];
  }
  if (shl->lh->head->next == shl->lh->tail ||
      shl->lh->head->next->next == shl->lh->tail) {
    /* don't bother if there are 0 or 1 item */
    delShell (dbgshl, 0);
    return;
  }
  lnk = shl->lh->head->next->next;
  while (lnk != shl->lh->tail) {
    if (shl->manageAllocs && !check_pointer (lnk->prev->data)) {
      set_shlError(0, SHELL_UNSPECIFIED);
      delShell (dbgshl, 0);
      return;
    }
    if (shl->manageAllocs && !check_pointer (lnk->data)) {
      set_shlError(0, SHELL_UNSPECIFIED);
      delShell (dbgshl, 0);
      return;
    }
    status = shl->compare (lnk->prev->data, lnk->data);
    if (status > 0) {
      /*
       * NOTE: this will give goofy results if the data is not C strings.
       */
      printf ("\n***Error: \"%s\" is before \"%s\" in the shell\n",
	  (char*)lnk->prev->data, (char*)lnk->data);
      delShell (dbgshl, 0);
      set_shlError(0, SHELL_UNSPECIFIED);
      return;
    }
    lnk = lnk->next;
  }
  delShell (dbgshl, 0);
  return;
}
#endif /* DEBUG */

/*
 * [BeginDoc]
 *
 * \section{Debug Heap Functions}
 *
 * \index{heap} \index{memory management} \index{debugging}
 * One of the most difficult things about programming in C or C++ is the
 * necessity to use pointers to do anything of significance.  The following are
 * some functions that were designed to make this task easier.  When the sort
 * code is compiled with DEBUG defined, the functions provided in the standard
 * C library for managing memory on the heap are wrapped with functions that
 * do some checking to help insure that things are done properly.  This helps
 * you find difficult bugs like dangling pointers and buffer overflows.  These
 * functions are not an end all, but they are extremely helpful if used
 * properly.
 *
 * [EndDoc]
 */
/*
 * Debug functions
 */
#define __NEW__


#define TRACE_C_PLUS_PLUS_ALLOCATION  1    /* 1 to trace new/delete calls */

/*
 * Remove the mymalloc #defines so we can call the base functions.
 */
#undef malloc
#undef calloc
#undef realloc
#undef free
#undef strdup
#undef print_block_list

#ifdef  DEBUG

#ifndef EXIT_ON_ERROR      /* if not set by compiler option */
#define EXIT_ON_ERROR  0   /* 1 if want to exit on error */
  /* 0 if try to keep going */
#endif  /*  */

#ifndef MARK_INACTIVE
#define MARK_INACTIVE  0   /* 0 if don't want nodes to be retained */
  /* 1 if garbage from list nodes is ok */
#endif  /*  */

#ifndef EXIT_MALLOC_NULL
#define EXIT_MALLOC_NULL  1   /* 1 if exit when allocation fails */
  /* usually desirable since the checking */
  /* becomes hopeless if malloc in the  */
  /* add_to_list function returns NULL */
#endif  /*  */

#ifndef WARN_MALLOC_NULL
#define WARN_MALLOC_NULL  1   /* 1 if error for allocation failure */
#endif  /*  */

#ifndef EXIT_NEW_NULL
#define EXIT_NEW_NULL  1   /* 1 if exit when allocation fails */
  /* usually desirable since the checking */
  /* becomes hopeless if malloc in the  */
  /* add_to_list function returns NULL */
#endif  /*  */

#ifndef WARN_NEW_NULL
#define WARN_NEW_NULL  1   /* 1 if error for allocation failure */
#endif  /*  */

/*------------------------------------------------------------------*/

#define FALSE 0                            /* A pseudo-Boolean type, bool */
#define TRUE  1
#define bool int
/*------------------------------------------------------------------*/
void print_block_list (void);
bool check_valid (void *address, char *file, int line, int type);
bool check_address (void *address, char *file, int line);
static void add_to_list (void *p, int size, char *file, int line, int type);
static void remove_block (void *address, char *file, int line, int type);
static void alloc_error (char *mesg, char *file, int line, int type);

/*------------------------------------------------------------------*/

  /* Possible values of 'type' field below */
  enum
{ MALLOC, CALLOC, REALLOC, FREE, USER_CALL, STRDUP, NEW, DELETE };
static char *type_string[] =
  { "malloc", "calloc", "realloc", "free", "user-call", "strdup", "new",
"delete"
};


#define FILE_LEN  255
  struct node
{
  char file[FILE_LEN + 1];    /* Filename of allocation call */
   int line;                  /* Line number of allocation call */
   int size;                  /* Size of the allocated block */
   void *address;             /* Address of the real allocated block */
   void *memory;              /* Address of the memory to return to caller */
   unsigned long *header;     /* Points to the header */
   unsigned long *trailer;    /* Points to the footer */
   int type;                  /* Block from malloc, calloc or realloc? */
   bool active;               /* Flag if block currently active, or if */

    /*                has already been freed                 */
  struct node *next;            /* Linked list pointer */
 };
static struct node *list = NULL;      /* Head of linked list of header blocks */

/*------------------------------------------------------------------*/
/* MY_MALLOC:  Front end to the malloc() library function                       */
/*------------------------------------------------------------------*/
void *
mymalloc (int size, char *file, int line)
{
  void *p;
  p = malloc (size + SIZE_DEBUG);   /* call the real malloc */
  DebugMemSet (p, size + SIZE_DEBUG);
  if (p != NULL)
    add_to_list (p, size, file, line, MALLOC);

  else {

#if WARN_MALLOC_NULL
      alloc_error ("malloc returned NULL", file, line, MALLOC);

#endif  /*  */
#if EXIT_MALLOC_NULL
      exit (EXIT_FAILURE);

#endif  /*  */
  }
  return (void *) ((char *) p + SIZE_DEBUG_HEADER);
} 

/*------------------------------------------------------------------*/
/* myinsidemalloc - allows the use of malloc with correct tracing       */
/*------------------------------------------------------------------*/
void *
myinsidemalloc (int size, char *file, int line, int type)
{
  void *p;
  p = malloc (size + SIZE_DEBUG);   /* call the real malloc */
  DebugMemSet (p, size + SIZE_DEBUG);
  if (p != NULL)
    add_to_list (p, size, file, line, type);

  else {

#if WARN_MALLOC_NULL
      alloc_error ("malloc returned NULL", file, line, type);

#endif  /*  */
#if EXIT_MALLOC_NULL
      exit (EXIT_FAILURE);

#endif  /*  */
  }
  return (void *) ((char *) p + SIZE_DEBUG_HEADER);
} 

/*------------------------------------------------------------------*/
/* MY_CALLOC:  Front end to the calloc() library function                       */
/*------------------------------------------------------------------*/
void *
mycalloc (int size, int num, char *file, int line)
{
  void *p;
  p = malloc (size * num + SIZE_DEBUG);     /* call the real calloc */
  DebugMemSet (p, size * num + SIZE_DEBUG);
  if (p != NULL) {
    add_to_list (p, size * num, file, line, CALLOC);
    memset ((void *) ((char *) p + SIZE_DEBUG_HEADER), 0, num * size);
  }

  else {

#if WARN_MALLOC_NULL
      alloc_error ("calloc returned NULL", file, line, CALLOC);

#endif  /*  */
#if EXIT_MALLOC_NULL
      exit (EXIT_FAILURE);

#endif  /*  */
  }
  return ((void *) ((char *) p + SIZE_DEBUG_HEADER));
} 

/*------------------------------------------------------------------*/
/* MY_FREE:    Front end to the free() library function                         */
/*------------------------------------------------------------------*/
  void
myfree (void *p, char *file, int line)
{
  if (p == NULL) {
    alloc_error ("free(NULL) is strange usage (although legal)", file, line,
                  FREE);
    return;                   /* do nothing - free(NULL) has no effect */
  }
  if (!check_valid (p, file, line, FREE))
    return;
  remove_block (p, file, line, FREE);
  free ((void *) ((char *) p - SIZE_DEBUG_HEADER));   /* Call the real free */
} void
myinternalfree (void *p, char *file, int line)
{
  if (p == NULL) {
    alloc_error ("free(NULL) is strange usage (although legal)", file, line,
                  DELETE);
    return;                   /* do nothing - free(NULL) has no effect */
  }
  if (!check_valid (p, file, line, DELETE))
    return;
  remove_block (p, file, line, DELETE);
  free ((void *) ((char *) p - SIZE_DEBUG_HEADER));   /* Call the real free */
} 

/*------------------------------------------------------------------*/
/* MY_REALLOC:  Front end to the realloc() library function             */
/*------------------------------------------------------------------*/
void *
myrealloc (void *p, int newsize, char *file, int line)
{
  void *n;
  if (p == NULL) {
    alloc_error ("realloc(NULL,...) is strange usage (although legal)", file,
                  line, REALLOC);
  }

  else {                        /* pointer is not NULL */
    if (!check_valid (p, file, line, REALLOC))
      return NULL;            /* return NULL for bad reallocation */
  }
  remove_block (p, file, line, REALLOC);
  myfree (p, NULL, 0);
  n = myinsidemalloc (newsize, file, line, REALLOC);
  if (n == NULL) {

#if WARN_MALLOC_NULL
      alloc_error ("realloc returned NULL", file, line, REALLOC);

#endif  /*  */
#if EXIT_MALLOC_NULL
      exit (EXIT_FAILURE);

#endif  /*  */
  }
  return n;
}

/*------------------------------------------------------------------*/
/* MY_STRDUP:  Front end to the strdup() non-ANSI library function      */
/*------------------------------------------------------------------*/

#if STRDUP_EXISTS
char *
mystrdup (char *s, char *file, int line)
{
  char *p;
  p = (char *) myinsidemalloc (strlen (s) + 1, file, line, STRDUP);
  if (p != NULL)
    strcpy (p, s);
  return p;
}


#endif  /* STRDUP_EXISTS */

/*------------------------------------------------------------------*/
/*       ADD_TO_LIST:    Add new header node info onto linked list              */
/*------------------------------------------------------------------*/
  static void
add_to_list (void *p, int size, char *file, int line, int type)
{
  struct node *n;
  n = (struct node *) malloc (sizeof (struct node));        /* Make new header no
de */
  if (n == NULL) {
    alloc_error
      ("(Internal) malloc returned NULL; unable to track allocation", file,
       line, type);
    return;
  }
  n->address = (void *) ((char *) p + SIZE_DEBUG_HEADER);
  n->memory = p;
  n->size = size;
  n->header = (unsigned long *) p;
  n->trailer = (unsigned long *) ((char *) p + size + SIZE_DEBUG_HEADER);
  *n->header = DBGMEM_HEADER;
  *n->trailer = DBGMEM_TRAILER;

    /* Need to be careful that __FILE__ is not too long */
    strncpy (n->file, file, FILE_LEN);
  n->file[FILE_LEN] = '\0';   /* ensure terminating zero present */
  n->line = line;
  n->type = type;
  n->active = TRUE;           /* Has not been freed yet */
  n->next = list;             /* Add new header node to front of list */
  list = n;
} 
/*------------------------------------------------------------------*/
/*       REMOVE_BLOCK: Remove header node for a block, or mark inactive */
/*------------------------------------------------------------------*/
  static void
remove_block (void *address, char *file, int line, int type)
{
  struct node *prev, *temp;
  for (prev = NULL, temp = list; temp != NULL && temp->address != address;
         prev = temp, temp = temp->next)
     {
    }                           /* empty loop */
  if (temp == NULL) {

      /* should always find the block on list */
      alloc_error ("Internal block list corrupted, address not found", file,
                   line, type);
    return;
  }
  
#if MARK_INACTIVE
    temp->active = FALSE;       /* Dont really free the block, */

    /*    just mark block as freed */
#else   /*  */
    if (prev == NULL)           /* Delete block from the linked list */
    list = temp->next;                /* Delete from front of linked list */

  else
    prev->next = temp->next;  /* Delete from middle or end */
  free (temp);

#endif  /*  */
}


/*------------------------------------------------------------------*/
/*       ERROR:    Print out error message, filename, line number               */
/*------------------------------------------------------------------*/
  static void
alloc_error (char *mesg, char *file, int line, int type)
{
  fprintf (stdout, "ALLOCATION ERROR: %s\n", mesg);
  fprintf (stdout, "                                  ");
  if (type != USER_CALL)
    fprintf (stdout, "Call to %s: ", type_string[type]);
  fprintf (stdout, "File '%s',  Line %d\n", file, line);
  
#if EXIT_ON_ERROR
    print_block_list ();
  exit (EXIT_FAILURE);

#endif  /*  */
}
/*------------------------------------------------------------------*/
/*       PRINT_BLOCK_LIST:        Print current status of the heap
        */
/*------------------------------------------------------------------*/

/*
 * [BeginDoc]
 *
 * \subsection{print_block_list}
 * \index{print_block_list}
 * [Verbatim] */

void print_block_list (void)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * This function will print the current status of the heap based on
 * allocations and frees.  It will tell you where in your source files
 * a memory allocation occurred and which standard function (malloc, etc.)
 * was used to allocate the memory.  This can be used at the end of
 * the program, for example, to determine that all allocations are freed
 * properly.  This helps you trace down memory leaks.
 *
 * Used at any other point in the program, print_block_list() will list
 * all the allocations currently outstanding.  If there are no blocks
 * being used on the heap, print_block_list() will print
 * "No blocks on heap".
 *
 * The main limitation of this function is that it will print the
 * information to stdout using fprintf().  This will be a problem
 * (potentially) under windowing environments and may need to be refined
 * to accommodate other printing schemes.
 *
 * [EndDoc]
 */
 {
  struct node *temp;
  int count = 0;
  fprintf (stdout, "\n              CURRENT HEAP STATUS \n");
  for (temp = list; temp != NULL; temp = temp->next) {
    if (temp->active) {
      count++;
      fprintf (stdout, "%s: Address %p, Size %d, File: %s, Line %d\n",
                type_string[temp->type], temp->address, temp->size,
                temp->file, temp->line);
    }
  }
  if (count == 0)
    fprintf (stdout, "No blocks on heap\n");
  fprintf (stdout, "\n");
}

/*------------------------------------------------------------------*/
/*       CHECK_VALID:   Check an address to be freed is actually valid  */
/*------------------------------------------------------------------*/

/*
 * [BeginDoc]
 *
 * \subsection{check_pointer}
 * \index{check_pointer}
 *
 * check_pointer (void *ptr)
 *
 * check_pointer() Checks that a pointer is valid
 * and that no illegal actions were
 * taken with it (overwrite of boundaries, etc.).  Returns TRUE (1)
 * if the pointer is valid, FALSE (0) otherwise.  This is actually
 * a wrapper (macro) around the function check_valid ().  The prototype
 * for check_valid() is as follows:
 * [Verbatim] */

int check_valid (void *address, char *file, int line, int type)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * check_pointer () sets file and line to the file and line,
 * respectively of the user call; type gets set to 4 (USER_CALL).
 *
 * The following are the things that are checked for the address:
 * 
 * \begin{itemize}
 * \item is it in our managed list of addresses?
 * \item were the boundaries of the memory block overwritten during
 *   execution?
 * \item is it a block that has already been freed?
 * \item invalid combinations of new/free or m(c,re)alloc/delete.
 * \end{itemize}
 * 
 * [EndDoc]
 */

{
  struct node *temp;
  bool found_inactive = FALSE;
  for (temp = list; temp != NULL; temp = temp->next) {
    if (temp->address == address) {
      if (temp->active) {     /* found a valid address */

          /* but still check for mismatches */
          if (type == FREE && temp->type == NEW)
          alloc_error ("Block free'd was allocated using new", file, line,
                        type);

        else
        if (type == DELETE
               && (temp->type == MALLOC || temp->type == CALLOC
                     || temp->type == REALLOC))
          alloc_error
            ("Block delete'd was allocated using malloc/calloc/realloc", file,
             line, type);
        
          /*
           * Check the header, trailer values
           */
          if (*temp->header != DBGMEM_HEADER
              || *temp->trailer != DBGMEM_TRAILER)
          alloc_error ("Block overwritten during program execution", file,
                        line, type);
        return TRUE;          /* it is a valid block */
      }

      else                      /* not an active block */
        found_inactive = TRUE;
    }
  }
  if (found_inactive) {     /* Found block but already freed */
    if (type == USER_CALL)
      alloc_error ("Pointer to block already freed", file, line, type);

    else
      alloc_error ("Memory block freed twice", file, line, type);
  }

  else {                        /* Not valid (not on list of blocks) */
    if (type == USER_CALL)
      alloc_error ("Pointer not an allocated block", file, line, type);

    else
    if (type == DELETE)
      alloc_error ("Block delete'd is not allocated", file, line, type);

    else
      alloc_error ("Block free'd is not allocated", file, line, type);
  }
  return FALSE;               /* Not a legal block */
}
/*------------------------------------------------------------------*/
/*       CHECK_ADDRESS:   Check an address is IN one of the blocks              */
/*------------------------------------------------------------------*/

/*
 * [BeginDoc]
 *
 * \subsection{check_reference}
 * \index{check_reference}
 *
 * check_reference (void *address);
 *
 * check_reference() checks the memory block pointed to by address to
 * make sure it points to a valid area in memory.  The reference is
 * valid if it is within the list managed by the mymalloc routing;
 * otherwise, it is flagged as an invalid address.  check_reference()
 * returns TRUE if the block is OK, FALSE otherwise.  check_reference()
 * is a macro that calls a function called check_address().  Following
 * is the prototype for check_address().
 * [Verbatim] */

int check_address (void *address, char *file, int line)

/* [EndDoc] */
/*
 * [BeginDoc]
 *
 * check_reference () sets file and line to the file and line of the
 * source from which it is called.
 *
 * [EndDoc]
 */
{
  struct node *temp;
  for (temp = list; temp != NULL; temp = temp->next) {
    if (temp->active && address >= temp->address
         &&(char *) address < (char *) temp->address + temp->size)
      return TRUE;            /* Address is in this block */
  }
  
    /* Address is not valid */
    alloc_error ("Pointer not inside block", file, line, USER_CALL);
  return FALSE;
}

#endif /* DEBUG */


