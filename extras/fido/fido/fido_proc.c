/* Source File: fido_proc.c */

/*
 * It is clear that the fido client is not up to snuff in terms of performance.
 * I am going to need to redo parts of it in C to get legitimate performance.
 * This program is an attempt to do that.  It will open fido dir and use that
 * to generate a list of files to process.  Then, it will open fido.excl,
 * generate a list of those and exclude them from the original list.  The final
 * output will be a list of files/directories that can be processed.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sort.h>

#define SIZE_FSPEC 256
#define MAXDATA 10000000

#ifdef HASH_SEARCH
#ifndef HASH_SIZE
#define HASH_SIZE 127
#endif

/*
 * This is a hash algorythm from Aho's book on compiler design.
 * It is purported to be good with strings.
 */
static __inline__ unsigned int hash(const char *key) {
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
#endif

static __inline__ int fidoCompare (void *p1, void *p2)
{
  return (strcmp ((char *) p1, (char *) p2));
}

int main (int argc, char *argv[])
{
  int status;
  static shellHeader *fido_dir;
#ifndef HASH_SEARCH
  static shellHeader *fido_excl;
#else
  // fido_excl is an array of shellHeader's.
  static shellHeader *fido_excl[HASH_SIZE];
  unsigned int hash_offset;
  int counter;
#endif
  FILE *dfp;
  char *cp, *data;
  char **cpp;
  Link *lnk, *found;
  struct stat statbuf;
  int i;
  double t1, t2;
  int numitems;

  if (argc != 5) {
    fprintf (stderr, "\n\n***Error: wrong command-line.\n"
	"Usage: fido_proc numitems file1 file2 file3\n"
  "Where numitems is the number of items to process,\n"
	"  file1 is the list of files to process,\n"
	"  file2 is the list of files to exclude,\n"
	"  and file3 is the file to put the result in.\n\n");
    return _ERROR_;
  }
  numitems = atoi (argv[1]);
  if (numitems < 0 || numitems > MAXDATA) {
    printf ("\n\n***Error: invalid numitems value: %d\n", numitems);
    return _ERROR_;
  }
  fprintf (stderr, "\nProcessing %s and %s\n", argv[2], argv[3]);
  dfp = fopen (argv[3], "r");
  if (0 == dfp) {
    fprintf (stderr, "\n***Error: Could not open %s", argv[3]);
    perror(" ");
    return _ERROR_;
  }
  stat (argv[3], &statbuf);
  cp = malloc ((size_t) statbuf.st_size+1);
  if (0 == cp) {
    printf ("\n\n***Error: critical memory error allocating file buffer\n");
    return _ERROR_;
  }
  fread (cp, (size_t)statbuf.st_size, 1, dfp);
  check_pointer (cp);
  fclose (dfp);
  cpp = malloc ((numitems+1)*sizeof (char*));
  if (0 == cpp) {
    printf ("\n\n***Error: couldn't allocate cpp\n");
    free (cp);
    free (cpp);
    return _ERROR_;
  }
  memset (cpp, 0, (numitems+1)*sizeof(char*));
  check_pointer (cpp);
  status = split_r (cpp, numitems, cp, '\n');
  check_pointer (cpp);
  if (status > numitems) {
    printf ("\n\n***Warning: the number of lines in %s is greater than %d\n",
        argv[3], numitems);
    printf ("\tWe will only process %d items\n", numitems);
  }
#ifndef HASH_SEARCH
  fido_excl = initShell (fidoCompare, TRUE, TRUE);
  if (fido_excl == 0) {
    printf ("\n\n***Error: initializing shell, %s\n", shlErrorStr[shlError]);
    return _ERROR_;
  }
#else
  for (i = 0; i < HASH_SIZE; i++) {
    fido_excl[i] = initShell (fidoCompare, TRUE, TRUE);
    if (fido_excl[i] == 0) {
      fprintf (stderr,"\n\n***Error: initializing shell, %s\n", shlErrorStr[shlError]);
      return _ERROR_;
    }
  }
  counter = 0;
#endif
  elapsed (&t1);
  for (i = 0; cpp[i]; i++) {
    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      fprintf (stderr,"\n\n***Error: memory error allocating Link\n");
      free (cp);
      free (cpp);
      return _ERROR_;
    }
    data = malloc (SIZE_FSPEC+1);
    if (0 == data) {
      fprintf (stderr,"\n\n***Error: memory error allocating Link\n");
      free (cp);
      free (cpp);
      free (lnk);
      return _ERROR_;
    }
    if (strlen (cpp[i]) >= 256)
      fprintf (stderr,"\n\n***Warning: strlen %s is >= 256: truncating\n", cpp[i]);
    snprintf (data, SIZE_FSPEC, "%s", cpp[i]);
    lnk->data = data;
#ifndef HASH_SEARCH
    status = addShellItem (fido_excl, lnk);
#else
    hash_offset = hash ( (const char *)lnk->data );
    status = addShellItem (fido_excl[hash_offset], lnk);
#endif
    if (status == _ERROR_) {
#ifndef HASH_SEARCH
      if (fido_excl->shlError == SHELL_UNIQUE) {
        set_shlError(fido_excl, SHELL_NOERR);
        continue;
      }
      if (shlError == SHELL_LIST) {
        fprintf (stderr,"\n\n***Error: List Error, item %d, %s\n",
            i, ListErrorString[fido_excl->lh->ListError]);
        delShell (fido_excl, 0);
        return _ERROR_;
      }
      else {
        fprintf (stderr,"\n\n***Error: item %d, %s entering \"%s\"\n",
          i, shlErrorStr[fido_excl->shlError], cpp[i]);
	      delShell (fido_excl, 0);
	      return _ERROR_;
      }
#else
      if (fido_excl[hash_offset]->shlError == SHELL_UNIQUE) {
        set_shlError(fido_excl[hash_offset], SHELL_NOERR);
        continue;
      }
      if (fido_excl[hash_offset]->shlError == SHELL_LIST) {
        fprintf (stderr,"\n\n***Error: List Error, item %d, %s\n",
            i, ListErrorString[fido_excl[hash_offset]->lh->ListError]);
	      goto cleanupHashTable;
      }
      else {
        fprintf (stderr,"\n\n***Error: item %d, %s entering \"%s\"\n",
          i, shlErrorStr[fido_excl[hash_offset]->shlError], cpp[i]);
	      goto cleanupHashTable;
	      return _ERROR_;
      }
#endif
    }
#ifdef HASH_SEARCH
    counter++;
#endif
    if (shlError == SHELL_UNSPECIFIED)
      return _ERROR_;
  }
  elapsed (&t2);
  free (cp);
#ifndef HASH_SEARCH
  fprintf (stderr,"\nAdded %d items to the fido exclude list in %f seconds\n",
      fido_excl->lh->number, t2-t1);
  status = saveShell (fido_excl, "Temporary save", SIZE_FSPEC, "fido_excl_shl.dat");
  if (status == _ERROR_) {
    if (shlError == SHELL_LIST)
      fprintf (stderr,"\n***Error: Storing fido excludes, %s\n",
	  ListErrorString[ListError]);
    else
      fprintf (stderr,"\n***Error: storing fido excludes, %s\n",
	  shlErrorStr[shlError]);
    delShell (fido_excl, 0);
    return _ERROR_;
  }
  delShell (fido_excl, 0);
#else
  fprintf (stderr,"\nAdded %d items to the fido exclude list in %f seconds\n",
      counter, t2-t1);
#endif

  /*
   * OK, read in the fido dir data.
   */
  dfp = fopen (argv[2], "r");
  if (0 == dfp) {
    fprintf (stderr, "\n***Error: Could not open %s", argv[2]);
    perror(" ");
    return _ERROR_;
  }
  stat (argv[2], &statbuf);
  cp = malloc ((size_t) statbuf.st_size+1);
  if (0 == cp) {
    fprintf (stderr,"\n\n***Error: critical memory error allocating file buffer\n");
    return _ERROR_;
  }
  fread (cp, (size_t)statbuf.st_size, 1, dfp);
  check_pointer (cp);
  memset (cpp, 0, (numitems+1)*sizeof(char*));
  check_pointer (cpp);
  status = split_r (cpp, numitems, cp, '\n');
  fclose (dfp);
  if (status > numitems) {
    printf ("\n\n***Warning: the number of lines in %s is greater than %d\n",
        argv[2], numitems);
    printf ("\tWe will only process %d items\n", numitems);
  }
  fido_dir = initShell (fidoCompare, TRUE, TRUE);
  if (fido_excl == 0) {
    fprintf (stderr,"\n\n***Error: initializing shell, %s\n", shlErrorStr[shlError]);
    return _ERROR_;
  }
  elapsed (&t1);
  for (i = 0; cpp[i]; i++) {
    int len;
    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      fprintf (stderr,"\n\n***Error: memory error allocating Link\n");
      free (cpp);
      free (cp);
      return _ERROR_;
    }
    len = strlen (cpp[i]);
    if (len >= 256) {
      fprintf (stderr,"\n\n***Warning: strlen %s is >= 256: truncating\n", cpp[i]);
      len = 256;
    }
    data = malloc (len+1);
    if (0 == data) {
      fprintf (stderr,"\n\n***Error: memory error allocating Link\n");
      free (cpp);
      free (cp);
      free (lnk);
      return _ERROR_;
    }
    snprintf (data, len+1, "%s", cpp[i]);
    lnk->data = data;
    status = addShellItem (fido_dir, lnk);
    if (status == _ERROR_) {
      if (fido_dir->shlError == SHELL_UNIQUE) {
	// don't worry about it.
        set_shlError(fido_dir, SHELL_NOERR);
        continue;
      }
      if (fido_dir->shlError == SHELL_LIST) {
        fprintf (stderr,"\n\n***Error: List Error, item %d, %s\n",
            i, ListErrorString[fido_dir->lh->ListError]);
        delShell (fido_dir, 0);
#ifdef HASH_SEARCH
	goto cleanupHashTable;
#endif
        return _ERROR_;
      }
      else {
        fprintf (stderr,"\n\n***Error: item %d, %s entering \"%s\"\n",
      i, shlErrorStr[fido_dir->shlError], cpp[i]);
	  delShell (fido_dir, 0);
#ifdef HASH_SEARCH
	  goto cleanupHashTable;
#endif
	  return _ERROR_;
      }
    }
    if (shlError == SHELL_UNSPECIFIED)
      return _ERROR_;
  }
  elapsed (&t2);
  fprintf (stderr,"\nAdded %d items to the fido directory list in %f seconds\n",
      fido_dir->lh->number, t2-t1);

  free (cp);
  free (cpp);
#ifndef HASH_SEARCH
  fido_excl = getShell (fidoCompare, "fido_excl_shl.dat");
  if (fido_excl == 0) {
    if (shlError == SHELL_LIST)
      fprintf (stderr,"\n***Error: retrieving fido excludes shell, %s\n",
	  ListErrorString[ListError]);
    else
      fprintf (stderr,"\n***Error: retrieving fido excludes shell, %s\n",
	  shlErrorStr[shlError]);
    return _ERROR_;
  }
#else
  counter = 0;
#endif
  elapsed (&t1);
  dfp = fopen (argv[4], "w");
  if (dfp == 0) {
    fprintf (stderr,"\n\n***Error: Could not open output file, %s", argv[4]);
    perror(" ");
    return _ERROR_;
  }
  lnk = fido_dir->lh->head->next;
  while (lnk != fido_dir->lh->tail) {
#ifndef HASH_SEARCH
    found = findShellItem (fido_excl, lnk);
#else
    hash_offset = hash ( (const char*)lnk->data );
    found = findShellItem (fido_excl[hash_offset], lnk);
#endif
    if (0 == found) {
      fprintf (dfp, "%s\n", (char*)lnk->data);
#ifdef HASH_SEARCH
      counter++;
#endif
    }
    lnk = lnk->next;
  }
  elapsed (&t2);
#ifdef HASH_SEARCH
  fprintf (stderr,"\nAdded %d items to the fido directory list in %f seconds\n",
      counter, t2-t1);
#endif
  delShell (fido_dir, 0);
#ifndef HASH_SEARCH
  delShell (fido_excl, 0);
#else
cleanupHashTable:
  for (i = 0; i < HASH_SIZE; i++) {
    if (fido_excl[i] != 0)
      delShell (fido_excl[i], 0);
  }
#endif
  return 0;
}
