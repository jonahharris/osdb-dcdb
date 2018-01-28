/*	Source File:	document.c	*/

/*
 * 
 * @DocInclude ../include/disclaim.tex
 * 
 * [BeginDoc]
 *
 * \section{document}
 * 
 * Extracts documentation from source files.  This program will
 * look through source files, the names of which will be given on
 * the command line, seeking for the documentation delimiters.
 * If it finds them, it will pull the text between them, non
 * inclusively, assuming it is documentation.
 * 
 * The documentation delimiters are "[BeginDoc]" and "[EndDoc]".
 * These delimiters are case sensitive and must include the `[' and
 * `]' characters, although you should not use ``'' where you want
 * the delimiters to be counted.  In other words, if you want to use the
 * delimiters without them being interpreted as delimiters, surround
 * them in double quotes.
 *
 * All the lines that document pulls (the lines between the delimiters)
 * will be placed into a file called ``manual.tex'' unless the `-f'
 * switch is used.
 * 
 * \section{Usage}
 * 
 * This utility allows any
 * documentation line to begin with ``whitespace*whitespace'', which 
 * is removed from the front of the line before the line is written to 
 * the document.  For example, the following are all stripped from the
 * beginning of a line of documentation:
 * 
 * \begin{enumerate}
 * 
 * \item ``     '' - just spaces
 * 
 * \item ``  *  '' - any combination of spaces and `*'s
 * 
 * \item \begin{verbatim} \t*\t \end{verbatim} - any combination of spaces, 
 * tabs and `*'s
 * 
 * \end{enumerate}
 * 
 * Everything on the line after all the spaces, tabs and `*'s have been
 * stripped is sent as is to the document file, so you can use the technique
 * shown above with these examples to indent.
 * 
 * The stripping away of `*'s at the beginning of the input line is to
 * allow the user to place comments inside of C source files without
 * having to change their comment styles (look at ldoc.c to see
 * how this is used in practice).  The same support is provided for
 * shell/Perl scripts (\#), for \C++ programs (//) and for Assembly
 * programs (;).  So, the following characters will get stripped from
 * the beginning of the line:  * \# // ;
 * 
 * The document program returns a 0 if successful, a 1 if the program was
 * invoked unsuccessfully (bad command line) and a -1 if there was an error.
 * 
 * \subsection{Including other files in the current document}
 * 
 * Document will also look for the string "@DocInclude" in the file.  If
 * it finds it, the document program will assume that the user wants to
 * enter another file in the documentation.  The "@DocInclude" directive
 * is used as follows:
 * 
 * \begin{verbatim}
 * 
 * "@DocInclude filename"
 * 
 * \end{verbatim}
 * 
 * When this is seen in the source, filename is opened and any
 * documentation in it (text between the document delimiters) is
 * included as if it were part of the original file.
 * 
 * \subsection{ldoc Command line}
 * 
 * {\emph ldoc} takes the following switches on the command-line, as follows:
 * 
 * \begin{enumerate}
 * 
 * \item ``-o'' - overwrite output file, if it exists.
 * 
 * \item ``-f filename'' - save the ouput in ``filename'' instead of
 * ``manual.tex''.
 * 
 * \item ``-t title'' - insert ``title'' in the title page of the ouput.
 * 
 * \item ``-a author'' - insert ``author'' in the title page of the ouput.
 * 
 * \end{enumerate}
 * 
 * The document program will interpret wildcard characters on the command
 * line correctly.  So, to run it on all the ``.c'' files in the current
 * directory, type:
 * 
 * \begin{verbatim}
 * 
 * document *.c
 * 
 * \end{verbatim}
 * 
 * \section{Final notes}
 * 
 * Bug reports, suggestions for enhancement, etc. should be mailed to 
 * dmay@tvi.edu
 * 
 * [EndDoc]
 */

#include <stdio.h>
#include <string.h>
#ifndef	WIN32
#include <unistd.h>		/* for getopt() */
#ifdef	__CYGWIN__
#include <getopt.h>
#endif
#endif
#include <sort.h>

#define	MANUALFILE	"manual.tex"
#define	MANUALMODE	"a"

int document (char *fname, FILE * outputFP, int txtMode, int included);
void emitHeader (FILE * outputFP, int isTitle, char *title,
		 int isAuthor, char *author);
void emitTrailer (FILE * outputFP);

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
  int isTitle = FALSE;
  char title[255];
  int isAuthor = FALSE;
  char author[255];

  if (argc == 1 || strcmp (argv[1], "-h") == 0) {
    printf
      ("\n\nUsage: document [-o] [-f manual] [-t title] [-a author] file [file...]\n\n");
    printf ("\t-o => overwrite output file, if it exists\n");
    printf ("\t-f manual => write output to file <manual>\n");
    printf ("\t-t title => place <title> in the header as the title\n");
    printf ("\t-a author => place <author> in the header as the author\n");
    EXIT (1);
  }

  /*
   * Process the command-line
   */
  opterr = 0;
  while ((ch = getopt (argc, argv, "of:t:a:")) != EOF) {
    switch (ch) {
    case 'o':
      strcpy (modeFlags, "w");
      useDefaultMode = FALSE;
      break;
    case 'f':
      strcpy (outputFile, optarg);
      useDefaultManualName = FALSE;
      break;
    case 't':
      isTitle = TRUE;
      strcpy (title, optarg);
      break;
    case 'a':
      isAuthor = TRUE;
      strcpy (author, optarg);
      break;
    case '?':
      printf ("\n\nInvalid command-line\n");
      printf ("Usage: document [-o] [-f manual] file [file...]\n\n");
      printf ("\t-o => overwrite output file, if it exists\n");
      printf ("\t-f manual => write output to file <manual>\n");
      printf ("\t-t title => place <title> in the header as the title\n");
      printf ("\t-a author => place <author> in the header as the author\n");
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
    printf ("\t-o => overwrite output file, if it exists\n");
    printf ("\t-f manual => write output to file <manual>\n");
    printf ("\t-t title => place <title> in the header as the title\n");
    printf ("\t-a author => place <author> in the header as the author\n");
    EXIT (-1);
  }
  emitHeader (outputFP, isTitle, title, isAuthor, author);
  for (; optind < argc; optind++) {
    printf ("\n%s", argv[optind]);
    status = document (argv[optind], outputFP, FALSE, FALSE);
    if (-1 == status) {
      EXIT (-1);
    }
  }
  emitTrailer (outputFP);

CleanUpAndQuit:
  if (NULL != outputFP)
    fclose (outputFP);
  return status;
}

void emitHeader (FILE * outputFP, int isTitle, char *title, int isAuthor,
		 char *author)
{
  fprintf (outputFP, "\\documentclass[letterpaper,10pt]{article}\n\n");
  fprintf (outputFP,
	   "\\def\\C++{{\\rm C\\kern-.05em\\raise.3ex\\hbox{\\footnotesize ++}}}\n\n");
  if (isTitle)
    fprintf (outputFP, "\\title{%s}\n", title);
  if (isAuthor)
    fprintf (outputFP, "\\author{%s}\n", author);
  fprintf (outputFP, "\\date{\\today}\n\n");
  fprintf (outputFP, "\\pagestyle{headings}\n");
  fprintf (outputFP, "\\usepackage{latexsym}\n\n");
  fprintf (outputFP, "\\usepackage{makeidx}\n\\makeindex\n\n");
  fprintf (outputFP, "\\begin{document}\n\n");
  if (isTitle)
    fprintf (outputFP, "\\maketitle\n");
  fprintf (outputFP, "\\tableofcontents\n\\vfill \\eject\n\n");
}

void emitTrailer (FILE * outputFP)
{
  fprintf (outputFP, "\\printindex\n\n\\end{document}\n");
}
