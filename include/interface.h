/*	Header File:	interface.h	*/

#ifndef	__INTERFACE_H__
#define	__INTERFACE_H__

#undef INLINE
/*
 * 20020809
 * SWIG changed the interface on its newest version.  This was being
 * used for swig conditional compiles.
 */
#define	SWIGCODE
#include <cdb.h>

/*
 * prototypes for C command handling functions
 */

/*
 * [BeginDoc]
 * 
 * \section{DCDB Bindings}
 * 
 * DCDB was designed to be embedded into applications easily.  This design facilitates
 * using DCDB in a scripting language, like Tcl or Perl.  In fact, with the original
 * distribution of DCDB, I included hand coded Tcl/Tk bindings.  However, with SWIG,
 * it is unnecessary to do that.
 * 
 * SWIG is the Simplified Wrapper and Interface Generator (http://www.swig.org).  It
 * provides a simplified interface for creating bindings between various scripting
 * languages and C or C++ libraries.  SWIG supports many more languages than I am
 * able to fully support because I am not familiar with the scripting languages.
 * 
 * I have written bindings for Perl and Tcl/Tk.  These will be fully supported.  I
 * have begun work on Python bindings, but don't know the Python language well enough
 * to full test the work.
 * 
 * [EndDoc]
 * 
 * @DocInclude ../lib/cdbtcl.i
 *
 * @DocInclude ../lib/cdbpl.i
 */

extern char cdberror[];

char *dbgeterror (void);
int dbcreate (char *arg);
const char *dbopen (char *arg);
int dbclose (char *arg);
int dbnumrecs (char *arg);
int dbnumfields (char *arg);
int dbseq (char *arg);
int dbiseof (char *arg);
int dbisbof (char *arg);
char *dbshow (const char *arg1, const char *arg2);
int dbflen (char *arg1, char *arg2);
int dbdeclen (char *arg1, char *arg2);
int dbsetint (char *arg1, char *arg2, int arg3);
int dbsetnum (char *arg1, char *arg2, double arg3);
int dbsetlog (char *arg1, char *arg2, char arg3);
int dbsetchar (char *arg1, char *arg2, char *arg3);
int dbsetdate (char *arg1, char *arg2, char *arg3);
int dbsettime (char *arg1, char *arg2, char *arg3);
int dbadd (char *arg);
int dbretrieve (char *arg);
int dbupdate (char *arg);
int dbdel (char *arg);
int dbundel (char *arg);
int dbisdeleted (char *arg);
char *dbfldname (char *arg, int fldnum);
char *dbfldtype (char *arg, int fldnum);
int dbfldlen (char *arg, int fldnum);
int dbflddec (char *arg, int fldnum);
int dbcurrent (char *arg1, char *arg2);
int dbgo (char *arg, int recno);
int dbnext (char *arg);
int dbnextindex (char *arg);
int dbprev (char *arg);
int dbprevindex (char *arg);
int dbhead (char *arg);
int dbheadindex (char *arg);
int dbtail (char *arg);
int dbtailindex (char *arg);
int dbsearchindex (char *arg1, char *arg2);
int dbsearchexact (char *arg1, char *arg2);
int dbpack (char *arg);
int dbreindex (char *arg);
int dbexit (void);
double dbtime (void);

unsigned char *md5sum (char *fname);
unsigned char *sha1sum (char *fname);
unsigned char *rmd160sum (char *fname);

int dbnummidx (char *arg);
int dbismidx (char *arg);
char *dbmidxname (char *arg, int midxnum);
int dbmidxblksz (char *arg1, char *arg2);
int dbmidxnumfldnames (char *arg1, char *arg2);
char *dbmidxfldname (char *arg1, char *arg2, int num);
int dbisindexed (char *arg1, char *arg2);
int dbidxblksz (char *arg1, char *arg2);
char *dbshowinfo (char *arg1);

char *dbteststring (int arg1, int arg2);
char *dbtestupperstring (int arg1, int arg2);
char *dbtestmixedstring (int arg1, int arg2);
char *dbtestnumber (int arg1, int arg2);
char *dbtestnumstring (int arg1, int arg2);

char *dbencrypt (char *arg1, int arg2, char *arg3);
char *dbdecrypt (char *arg1, int arg2, char *arg3);

char *crc32sum (char *fname);

char *bcnumadd (char *arg1, char *arg2, int arg3);
char *bcnumsub (char *arg1, char *arg2, int arg3);
int bcnumcompare (char *arg1, char *arg2, int arg3);
char *bcnummultiply (char *arg1, char *arg2, int arg3);
char *bcnumdivide (char *arg1, char *arg2, int arg3);
char *bcnumraise (char *arg1, char *arg2, int arg3);

int bcnumiszero (char *arg1);
int bcnumisnearzero (char *arg1, int scale);
int bcnumisneg (char *arg1);

int bcnumuninit (void);

#endif	/* __INTERFACE_H__ */
