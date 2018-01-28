/*	Source File:	document.c	*/

#include <stdio.h>
#include <string.h>
#include <sort.h>

#ifndef	HAVE_STRLWR
#include <ctype.h>
int strlwr (char *str);
#endif

#define	EXIT(st)	{status=st;goto CleanUpAndQuit;}
#define	TABSTOP		4	/* redefine this if you have bigger tabs */

void emitVerbatimLine (FILE * fp, char *str);
void emitTextLine (FILE * fp, char *str);

int document (char *fname, FILE * outputFP, int txtMode, int included)
{
  char str[512];
  int status;
  FILE *inputFP = NULL;
  char *cp, *cp1;
  char fileName[256];

  /*
   * First inner loop, looking for "[BeginDoc]"
   */
  inputFP = fopen (fname, "rb");
  if (inputFP == NULL) {
    printf ("\n\n** Warning: Could not open %s\n", fname);
    return _OK_;		/* could just exit here, but... */
  }
  strcpy (fileName, fname);
  if (!included) {
    strlwr (fileName);
    if (txtMode) {
      fprintf (outputFP, "Source File:\t%s\n", fileName);
      fflush (outputFP);
    }
  }
  while (1 == 1) {
    memset (str, 0, 512);
    cp = fgets (str, 510, inputFP);
    if (ferror (inputFP)) {
      printf ("\n\n***Error: Couldn't read %s: terminating.\n", fileName);
      EXIT (-1);
    }
    /*
     * Look for End Of File
     */
    if (feof (inputFP))
      break;
    /*
     * Look for "@DocInclude " within the file.  If it is valid,
     * do the include.
     */
    cp = strstr (str, "@DocInclude ");
    if (cp != NULL) {
      if (*(cp - 1) == '\"')
	continue;
      cp = cp + strlen ("@DocInclude ");
      cp1 = strchr (cp, (int) '\n');
      while (!isalpha ((int) *cp1) && cp1 != NULL) {
	*cp1 = '\0';
	cp1--;
      }
      status = document (cp, outputFP, txtMode, TRUE);
      if (-1 == status)
	return -1;
    }
    /*
     * Look for "[BeginDoc]" and make sure it *is* documentation 
     * and not code
     */
    cp = strstr (str, "[BeginDoc]");
    if (cp == NULL)
      continue;
    if (*(cp - 1) == '\"')
      continue;			/* skip "[BeginDoc]" in quotes */
    /*
     * Found it; start processing documentation
     */
    /*
     * Second inner loop
     */
    while (1 == 1) {
      memset (str, 0, 512);
      cp = fgets (str, 510, inputFP);
      if (ferror (inputFP)) {
	printf ("\n\n***Error: Couldn't read %s: terminating.\n", fname);
	EXIT (-1);
      }
      /*
       * This is a problem, but the user will figure it out because her/his
       * documentation is trashed.
       */
      if (feof (inputFP))
	break;
      /*
       * Look for "[EndDoc]" condition - not ending condition if "[EndDoc]" is
       * in double quotes.
       */
      cp = strstr (str, "[EndDoc]");
      if (cp && *(cp - 1) != '\"')
	break;
      /*
       * Look again to see if we bust into verbatim mode.
       */
      cp = strstr (str, "[Verbatim]");
      if (cp != NULL && *(cp - 1) != '\"') {
	if (!txtMode) {
	  fprintf (outputFP, "\n\\small\n");
	  fprintf (outputFP, "\n\\begin{verbatim}");
	  fflush (outputFP);
	}
	while (1 == 1) {
	  memset (str, 0, 512);
	  cp = fgets (str, 510, inputFP);
	  if (ferror (inputFP)) {
	    printf ("\n\n***Error: Couldn't read %s: terminating.\n", fname);
	    EXIT (-1);
	  }
	  if (feof (inputFP))
	    break;
	  cp = strstr (str, "[EndDoc]");
	  if (cp && *(cp - 1) != '\"') {
	    if (!txtMode) {
	      fprintf (outputFP, "\\end{verbatim}\n");
	      fprintf (outputFP, "\\normalsize\n");
	      fflush (outputFP);
	    }
	    cp = fgets (str, 510, inputFP);
	    goto DoneVerbatimMode;
	  }
	  if (txtMode)
	    fprintf (outputFP, str);
	  else
	    emitVerbatimLine (outputFP, str);
	  fflush (outputFP);
	}
      }
      /*
       * We have documentation; print it to the manual and continue
       */
      cp = str;
      while ((iswhite (*cp) || *cp == '*' || *cp == '#' || *cp == ';' ||
	      (*cp == '/' && (*(cp + 1)) == '/') || (*cp == '/'
						     && (*(cp - 1)) == '/'))
	     && *cp != '\0')
	cp++;
      if (txtMode)
	fprintf (outputFP, str);
      else
	emitTextLine (outputFP, cp);
      fflush (outputFP);
    }
    DoneVerbatimMode:
    {}; /* accommodate the new iso C standard wrt labels at the end of a block. */
  }
  fclose (inputFP);
  /*
   * Delimit the file divisions
   */
  if (!included && txtMode) {
    fprintf (outputFP,
	     "\n-----------------------------------------------------------------\n\n");
    fflush (outputFP);
  }

  status = 0;			/* everything was fine if we get here */

CleanUpAndQuit:
  printf ("\n\n");
  fflush (stdout);
  return status;
}

#ifndef HAVE_STRLWR

int strlwr (char *str)
{
  while (*str) {
    *str = tolower (*str);
    str++;
  }
  return _OK_;
}

#endif

void emitVerbatimLine (FILE * fp, char *str)
{
  char *cp;
  int i;
  int pos, depth;

  cp = str;
  pos = 0;
  while (*cp != '\0') {
    if (*cp == '\t') {
      depth = TABSTOP - pos % TABSTOP;
      for (i = 0; i < depth; i++) {
	fputc (' ', fp);
	pos++;
      }
    }
    else {
      fputc (*cp, fp);
      pos++;
    }
    cp++;
  }
}

void emitTextLine (FILE * fp, char *str)
{
  char *cp;

  cp = str;
  while (*cp != '\0') {
    switch (*cp) {
    case '$':			/* math formula delimiter */
      if (*(cp - 1) != '\\') {
	fputc ('\\', fp);
	fputc (*cp, fp);
      }
      else
	fputc (*cp, fp);
      break;
    case '%':			/* comment */
      if (*(cp - 1) != '\\') {
	fputc ('\\', fp);
	fputc (*cp, fp);
      }
      else
	fputc (*cp, fp);
      break;
    case '&':			/* table alignment mark */
      if (*(cp - 1) != '\\') {
	fputc ('\\', fp);
	fputc (*cp, fp);
      }
      else
	fputc (*cp, fp);
      break;
    case '^':			/* math superscript text */
      if (*(cp - 1) != '\\') {
	fputs ("\\^{}", fp);
      }
      else
	fputc (*cp, fp);
      break;
    case '_':			/* math subscript text */
      if (*(cp - 1) != '\\') {
	fputs ("\\_{}", fp);
      }
      else
	fputc (*cp, fp);
      break;
    case '#':			/* symbol replacement definition mark */
      if (*(cp - 1) != '\\') {
	fputc ('\\', fp);
	fputc (*cp, fp);
      }
      else
	fputc (*cp, fp);
      break;
#if 0
    case '{':			/* group begin symbol */
      if (*(cp - 1) != '\\') {
	fputs ("$\\{$", fp);
      }
      else
	fputc (*cp, fp);
      break;
    case '}':			/* group end symbol */
      if (*(cp - 1) != '\\') {
	fputs ("$\\}$", fp);
      }
      else
	fputc (*cp, fp);
      break;
#endif
    case '~':			/* unbreakable space */
      if (*(cp - 1) != '\\') {
	fputs ("\\~{}", fp);
      }
      else
	fputc (*cp, fp);
      break;
    case '>':			/* math symbol */
      if (*(cp - 1) != '$') {
	fputc ('$', fp);
	fputc (*cp, fp);
	fputc ('$', fp);
      }
      else
	fputc (*cp, fp);
      break;
    case '<':			/* math symbol */
      if (*(cp - 1) != '$') {
	fputc ('$', fp);
	fputc (*cp, fp);
	fputc ('$', fp);
      }
      else
	fputc (*cp, fp);
      break;
    default:
      fputc (*cp, fp);
      break;
    }
    if (*cp == '\n')
      break;
    cp++;
  }
  return;
}
