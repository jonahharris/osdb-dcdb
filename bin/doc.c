/*	Source File:	document.c	*/

/*
 * @DocInclude ../include/disclaim.txt
 * 
 * [BeginDoc]
 * 
 * -----------------------------------------------------------------
 * 
 * document
 * 
 * Extracts documentation from source files.  This program will
 * look through source files, the names of which will be given on
 * the command line, seeking for the documentation delimiters.
 * If it finds them, it will pull the text between them, non
 * inclusively, assuming it is documentation.
 * 
 * The documentation delimiters are "[BeginDoc]" and "[EndDoc]".
 * These delimiters are case sensitive and must include the '[' and
 * ']' characters, although you should not use '"' where you want
 * the delimiters to be counted.  In other words, if you want to use the
 * delimiters without them being interpreted as delimiters, surround
 * them in double quotes.
 *
 * All the lines that document pulls (the lines between the delimiters)
 * will be placed into a file called "manual.txt".  The ability to
 * specify the file name may be added later.  This utility allows any
 * documentation line to begin with "<whitespace>*<whitespace>", which 
 * is removed from the front of the line before the line is written to 
 * the document.  For example, the following are all stripped from the
 * beginning of a line of documentation:
 * 
 * =>	"     " just spaces
 * =>	"  *  " any combination of spaces and '*'s
 * =>	"\t*  " any combination of spaces, tabs and '*'s
 * =>	" *\t " 
 * 
 * Everything on the line after all the spaces, tabs and '*'s have been
 * stripped is sent as is to the document file, so you can use the technique
 * shown above with these examples to indent.
 * 
 * The stripping away of '*'s at the beginning of the input line is to
 * allow the user to place comments inside of C source files without
 * having to change their comment styles (look at document.c to see
 * how this is used in practice).  The same support is provided for
 * shell/Perl scripts (#), for C++ programs (//) and for Assembly
 * programs (;).  So, the following characters will get stripped from
 * the beginning of the line:  * # // ;
 * 
 * The document program returns a 0 if successful, a 1 if the program was
 * invoked unsuccessfully (bad command line) and a -1 if there was an error.
 * 
 * Document will also look for the string "@DocInclude" in the file.  If
 * it finds it, the document program will assume that the user wants to
 * enter another file in the documentation.  The "@DocInclude" directive
 * is used as follows:
 * 
 * |
 * | "@DocInclude" filename
 * |
 * 
 * When this is seen in the source, filename is opened and any
 * documentation in it (text between the document delimiters) is
 * included as if it were part of the original file.
 * 
 * =================================================================
 * 
 * document takes 2 switches on the command-line, as follows:
 * 
 * |  -o
 * |  -f filename
 * 
 * -o tells document to overwrite the output file, if one exists with
 * that name.  The default is to append to the output file.
 * 
 * -f filename tells document to save the output into "filename".
 * The default is to save the data to a file called manual.txt.
 * 
 * The document program will interpret wildcard characters on the command
 * line correctly.  So, to run it on all the ".c" files in the current
 * directory, type:
 * 
 * =>	document *.c
 * 
 * Bug reports, suggestions for enhancement, etc. should be mailed to 
 * dmay@tvi.edu
 * 
 * [EndDoc]
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>		/* for getopt() */
#ifdef	__CYGWIN__
#ifndef WIN32
#include <getopt.h>
#endif
#endif
#include <sort.h>

#define	MANUALFILE	"manual.txt"
#define	MANUALMODE	"ab"

int document (char *fname, FILE * outputFP, int txtMode, int included);

#ifdef	__linux__
#include <ctype.h>
int strlwr (char *str);
#endif

#define	EXIT(st)	{status=st;goto CleanUpAndQuit;}

int main (int argc, char *argv[])
{
  int status = 0;
  FILE *outputFP = NULL;
  int ch;
  int useDefaultManualName = TRUE;
  static char outputFile[255];
  int useDefaultMode = TRUE;
  static char modeFlags[10];

  if (argc == 1 || strcmp (argv[1], "-h") == 0) {
    printf ("\n\nUsage: document [-o] [-f manual] file [file...]\n\n");
    EXIT (1);
  }

  /*
   * Process the command-line
   */
  opterr = 0;
  while ((ch = getopt (argc, argv, "of:")) != EOF) {
    switch (ch) {
    case 'o':
      strcpy (modeFlags, "wb");
      useDefaultMode = FALSE;
      break;
    case 'f':
      strcpy (outputFile, optarg);
      useDefaultManualName = FALSE;
      break;
    case '?':
      printf ("\n\nInvalid command-line\n");
      printf ("Usage: document [-o] [-f manual] file [file...]\n\n");
      EXIT (-1);
    }
  }
  if (useDefaultManualName)
    strcpy (outputFile, MANUALFILE);
  if (useDefaultMode)
    strcpy (modeFlags, MANUALMODE);

  /*
   * Open the output file.
   */
  outputFP = fopen (outputFile, modeFlags);	/* "at" => append, text mode */
  if (outputFP == NULL) {
    printf ("\n\n***Error: Could not open %s\n", outputFile);
    EXIT (-1);
  }

  if (optind == argc) {
    printf ("\n\nNo non-switch arguments on the command-line\n");
    printf ("Usage: document [-o] [-f manual] file [file...]\n\n");
    EXIT (-1);
  }
  for (; optind < argc; optind++) {
    printf ("\n%s", argv[optind]);
    status = document (argv[optind], outputFP, TRUE, FALSE);
    if (-1 == status) {
      EXIT (-1);
    }
  }

CleanUpAndQuit:
  if (NULL != outputFP)
    fclose (outputFP);
  return status;
}
