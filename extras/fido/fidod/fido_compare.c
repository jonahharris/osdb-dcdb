/* Source File: fido_compare.c */

/*
 * Compare 2 fido databases.  Uses the low-level DCDB database engine for the
 * comparison.  Does exactly the same thing as is done by fidod for adds,
 * changes, and deletes.  Should be called as follows:
 *
 * fido_compare -o /etc/trpwr/fido/tmp /tmp/tmp_lum01.db /etc/trpwr/fido/lum01.db
 * where /etc/trpwr/fido/tmp is where the reports should go, /tmp/tmp_lum01.db
 * is the database snapshot of lum01 (the new database) and
 * /etc/trpwr/fido/lum01.db is the prior snapshot of lum01 (the old database).
 * 
 * This must be compiled with cdb.c as I use the interface functions provided
 * there for access to the DCDB functions.
 *
 */

#include <errno.h>
#include <unistd.h>  /* for getopt() */
#include <libgen.h> /* for dirname() and basename() */
#include <sort.h>
#include <interface.h>

static char output_directory[1024];
static char tstamp[20];

static char
  dname_newdb[1024], /* directory component, newdb */
  dname_olddb[1024], /* directory component, olddb */
  bname_newdb[1024], /* base name, newdb */
  bname_olddb[1024]; /* base name, olddb */

int fido_adds (char *newdb, char *olddb);
int fido_deletes (char *newdb, char *olddb);
int fido_changes (char *newdb, char *olddb);

shellHeader *excludes = 0;
int isExcludes = FALSE;

shellHeader *xtimes = 0;
int isXtimes = FALSE;

int isQuiet = FALSE;

extern int errno;
extern char cdberror[];

int isXtime (char *new_time)
{
  Link *lnk;
  ListHeader *lh;
  char *cp;

  if (!isXtimes || xtimes == 0 || xtimes->lh->number == 0)
    return FALSE;
  lh = xtimes->lh;
  for (lnk = lh->head->next; lnk != lh->tail; lnk = lnk->next) {
    cp = strstr (new_time, (char *)lnk->data);
    if (cp != 0)
      return TRUE;
  }
  return FALSE;
}

int fido_adds (char *newdb, char *olddb)
{
  int status;
  char new_spec[1024];
  char new_mtime[1024];
  char tmp[1024];
  char file[1024];
  char *cp;
  int is_adds = 0;
  FILE *fp;
  Link *lnk = 0, *found;

  dbheadindex (newdb);
  status = chdir (output_directory);
  if (status == _ERROR_) {
    perror ("\n***Error: chdir");
    return _ERROR_;
  }
  strcpy (tmp, bname_olddb);
  /*
  cp = strchr (tmp, '.');
  if (cp != 0)
    *cp = '\0';
  */
  tmp[strlen(tmp)-3] = '\0';
  strcpy (file, tmp);
  strcat (file, "_adds.");
  strcat (file, tstamp);
  fp = fopen (file, "w");
  if (fp == 0) {
    printf ("\n***Error: could not open %s\n", file);
    return _ERROR_;
  }
  strcpy (tmp, bname_olddb);
  /*
  cp = strchr (tmp, '.');
  if (cp != 0)
    *cp = '\0';
  */
  tmp[strlen(tmp)-3] = '\0';
  fprintf (fp, "Adds on %s: %s\n\n", tmp, tstamp);

  if (isExcludes) {
    lnk = malloc (sizeof (Link));
    if (lnk == 0) {
      printf ("\n\n***Error: fatal error allocating memory in fido_adds()\n");
      return _ERROR_;
    }
  }
  while (TRUE) {
    if (dbiseof (newdb) == 1)
      break;
    status = dbretrieve(newdb);
    if (status == 0) {
      /* skip deleted records */
      dbnextindex (newdb);
      continue;
    }
    cp = dbshow (newdb, "fspec");
    if (cp != 0)
      strcpy (new_spec, cp);
    else {
      printf ("\n\n***Error: dbshow(%s, fspec): %s\n",
	  newdb, cdberror);
      return _ERROR_;
    }
    if (strlen (new_spec) == 0)
      continue;
    cp = dbshow (newdb, "mtime");
    if (cp != 0)
      strcpy (new_mtime, cp);
    else {
      printf ("\n\n***Error: dbshow(%s: %s\n",
	  newdb, cdberror);
      return _ERROR_;
    }
    status = dbsearchexact (olddb, new_spec);
    if (status == _ERROR_) {
      printf ("\n***Error: dbsearchexact(%s, %s): %s\n",
	  olddb, new_spec, cdberror);
      return _ERROR_;
    }
    if (status == 0) {
      if (isExcludes) {
	lnk->data = &new_spec;
	found = findShellItem (excludes, lnk);
	if (found == 0) {
	  is_adds++;
	  fprintf (fp, "%s : %s\n", new_mtime, new_spec);
	}
      }
      else {
        is_adds++;
        fprintf (fp, "%s : %s\n", new_mtime, new_spec);
      }
    }
    dbnextindex (newdb);
  }
  if (!is_adds) {
    fclose (fp);
    strcpy (tmp, bname_olddb);
    tmp[strlen(tmp)-3] = '\0';
    strcpy (file, output_directory);
    strcat (file, tmp);
    strcat (file, "_adds.");
    strcat (file, tstamp);
    /* attempt to remove it. */
    remove (file);
  } 
  else {
    strcpy (tmp, bname_olddb);
    tmp[strlen(tmp)-3] = '\0';
    strcpy (file, output_directory);
    strcat (file, tmp);
    strcat (file, "_adds.");
    strcat (file, tstamp);
    printf ("%d adds in %s\n\n", is_adds, file);
    fprintf (fp,"\n\nDone: %d adds\n\n", is_adds);
    fclose (fp);
  }
  if (isExcludes)
    free (lnk);

  return _OK_;
}

int fido_deletes (char *newdb, char *olddb)
{
  int status;
  char del_spec[1024];
  /*char del_mtime[1024];*/
  char tmp[1024];
  char file[1024];
  char *cp;
  int is_deletes = 0;
  FILE *fp;
  Link *lnk = 0, *found;

  dbheadindex (olddb);
  status = chdir (output_directory);
  if (status == _ERROR_) {
    perror ("\n***Error: chdir");
    return _ERROR_;
  }
  strcpy (tmp, bname_olddb);
  tmp[strlen (tmp) - 3] = '\0';
  strcpy (file, output_directory);
  strcat (file, tmp);
  strcat (file, "_deletes.");
  strcat (file, tstamp);
  fp = fopen (file, "w");
  if (fp == 0) {
    printf ("\n***Error: could not open %s\n", file);
    return _ERROR_;
  }
  strcpy (tmp, bname_olddb);
  tmp[strlen (tmp) - 3] = '\0';
  fprintf (fp,"Deletes on %s: %s\n\n", tmp, tstamp);

  if (isExcludes) {
    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      printf ("\n\n***Error: fatal memory error in fido_deletes()\n");
      return _ERROR_;
    }
  }
  while (TRUE) {
    if (dbiseof(olddb) == 1)
      break;
    status = dbretrieve (olddb);
    if (status == 0) {
      dbnextindex (olddb);
      continue;
    }
    cp = dbshow (olddb, "fspec");
    if (cp != 0)
      strcpy (del_spec, cp);
    else {
      printf ("\n***Error: dbshow (%s, fspec): %s\n",
	  olddb, cdberror);
      return _ERROR_;
    }
    status = dbsearchexact (newdb, del_spec);
    if (status == 0) {
      if (isExcludes) {
	lnk->data = &del_spec;
	found = findShellItem (excludes, lnk);
	if (0 == found) {
	  is_deletes++;
	  fprintf (fp, "%s\n", del_spec);
	}
      }
      else {
        is_deletes++;
        fprintf (fp,"%s\n", del_spec);
      }
    }
    dbnextindex (olddb);
  }
  if (! is_deletes) {
    fclose (fp);
    strcpy (tmp, bname_olddb);
    tmp[strlen(tmp)-3] = '\0';
    strcpy (file, output_directory);
    strcat (file, tmp);
    strcat (file, "_deletes.");
    strcat (file, tstamp);
    remove (file);
  }
  else {
    strcpy (tmp, bname_olddb);
    tmp[strlen(tmp)-3] = '\0';
    strcpy (file, output_directory);
    strcat (file, tmp);
    strcat (file, "_deletes.");
    strcat (file, tstamp);
    printf ("%d deletes in %s\n\n", is_deletes, file);
    fprintf (fp, "\nDone: %d deletes\n\n", is_deletes);
    fclose (fp);
  }
  if (isExcludes)
    free (lnk);

  return _OK_;
}

int fido_changes (char *newdb, char *olddb)
{
  int status;
  char old_spec[1024];
  char tmp[1024];
  char file[1024];
  static char
    old_fspec[1024],
    old_uid[11], new_uid[11],
    old_gid[11], new_gid[11],
    old_size[13], new_size[13],
    old_mode[11], new_mode[11],
    old_time[20], new_time[20],
    old_md5[33], new_md5[33],
    old_sha1[41], new_sha1[41],
    old_ripe[41], new_ripe[41];
  int user_chg, grp_chg, size_chg, mode_chg, mtime_chg,
    md5_chg, sha1_chg, ripe_chg;
  char *cp;
  int is_changes = 0;
  int print_changes;
  FILE *fp;
  Link *lnk = 0, *found;

  dbheadindex (olddb);
  status = chdir (output_directory);
  if (status == _ERROR_) {
    perror ("\n***Error: chdir");
    return _ERROR_;
  }
  strcpy (tmp, bname_olddb);
  /*
  cp = strchr (tmp, '.');
  if (cp != 0)
    *cp = '\0';
  */
  tmp[strlen(tmp)-3] = '\0';
  strcpy (file, output_directory);
  strcat (file, tmp);
  strcat (file, "_changes.");
  strcat (file, tstamp);
  fp = fopen (file, "w");
  if (fp == 0) {
    printf ("\n***Error: could not open %s\n", file);
    return _ERROR_;
  }
  strcpy (tmp, bname_olddb);
  /*
  cp = strchr (tmp, '.');
  if (cp != 0)
    *cp = '\0';
  */
  tmp[strlen(tmp)-3] = '\0';
  fprintf (fp,"Changes on %s: %s\n\n", tmp, tstamp);

  if (isExcludes) {
    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      printf ("\n\n***Error: fatal memory error in fido_changes()\n");
      return _ERROR_;
    }
  }
  while (TRUE) {
    print_changes = 0;
    if (dbiseof(olddb) == 1)
      break;
    status = dbretrieve (olddb);
    if (status == 0) {
      dbnextindex (olddb);
      continue;
    }
    cp = dbshow (olddb, "fspec");
    if (cp != 0)
      strcpy (old_spec, cp);
    else {
      printf ("\n***Error: dbshow (%s, fspec): %s\n",
	  olddb, cdberror);
      return _ERROR_;
    }
    status = dbsearchexact (newdb, old_spec);
    if (status == 0) {
      dbnextindex (olddb);
      continue;
    }
    status = dbretrieve (newdb);
    if (status == 0) {
      dbnextindex (olddb);
      continue;
    }
    user_chg = 0;
    cp = dbshow (olddb, "uid");
    if (cp != 0)
      strcpy (old_uid, cp);
    else {
      printf ("\n***Error: dbshow (%s, uid): %s\n", olddb, cdberror);
      goto errorExit;
    }
    cp = dbshow (newdb, "uid");
    if (cp != 0)
      strcpy (new_uid, cp);
    else {
      printf ("\n***Error: dbshow (%s, uid): %s\n", newdb, cdberror);
      goto errorExit;
    }
    if (strcmp (old_uid, new_uid)) {
      user_chg = 1;
      print_changes++;
    }
    grp_chg = 0;
    cp = dbshow (olddb, "gid");
    if (cp != 0)
      strcpy (old_gid, cp);
    else {
      printf ("\n***Error: dbshow (%s, gid): %s\n", olddb, cdberror);
      goto errorExit;
    }
    cp = dbshow (newdb, "gid");
    if (cp != 0)
      strcpy (new_gid, cp);
    else {
      printf ("\n***Error: dbshow (%s, gid): %s\n", newdb, cdberror);
      goto errorExit;
    }
    if (strcmp (old_gid, new_gid)) {
      grp_chg = 1;
      print_changes++;
    }
    size_chg = 0;
    cp = dbshow (olddb, "size");
    if (cp != 0)
      strcpy (old_size, cp);
    else {
      printf ("\n***Error: dbshow (%s, size): %s\n", olddb, cdberror);
      goto errorExit;
    }
    cp = dbshow (newdb, "size");
    if (cp != 0)
      strcpy (new_size, cp);
    else {
      printf ("\n***Error: dbshow (%s, size): %s\n", newdb, cdberror);
      goto errorExit;
    }
    if (strcmp (old_size, new_size)) {
      size_chg = 1;
      print_changes++;
    }
    mode_chg = 0;
    cp = dbshow (olddb, "mode");
    if (cp != 0)
      strcpy (old_mode, cp);
    else {
      printf ("\n***Error: dbshow (%s, mode): %s\n", olddb, cdberror);
      goto errorExit;
    }
    cp = dbshow (newdb, "mode");
    if (cp != 0)
      strcpy (new_mode, cp);
    else {
      printf ("\n***Error: dbshow (%s, mode): %s\n", newdb, cdberror);
      goto errorExit;
    }
    if (strcmp (old_mode, new_mode)) {
      mode_chg = 1;
      print_changes++;
    }
    mtime_chg = 0;
    cp = dbshow (olddb, "mtime");
    if (cp != 0)
      strcpy (old_time, cp);
    else {
      printf ("\n***Error: dbshow (%s, mtime): %s\n", olddb, cdberror);
      goto errorExit;
    }
    cp = dbshow (newdb, "mtime");
    if (cp != 0)
      strcpy (new_time, cp);
    else {
      printf ("\n***Error: dbshow (%s, mtime): %s\n", newdb, cdberror);
      goto errorExit;
    }
    if (strcmp (old_time, new_time)) {
      mtime_chg = 1;
      /* 
       * don't set change flag based solely on time because some stupid
       * software (like backup software) uses this for incremental backups.
       */
    }
    md5_chg = 0;
    cp = dbshow (olddb, "md5");
    if (cp != 0)
      strcpy (old_md5, cp);
    else {
      printf ("\n***Error: dbshow (%s, md5): %s\n", olddb, cdberror);
      goto errorExit;
    }
    cp = dbshow (newdb, "md5");
    if (cp != 0)
      strcpy (new_md5, cp);
    else {
      printf ("\n***Error: dbshow (%s, md5): %s\n", newdb, cdberror);
      goto errorExit;
    }
    if (strcmp (old_md5, new_md5)) {
      md5_chg = 1;
      print_changes++;
    }
    sha1_chg = 0;
    cp = dbshow (olddb, "sha1");
    if (cp != 0)
      strcpy (old_sha1, cp);
    else {
      printf ("\n***Error: dbshow (%s, sha1): %s\n", olddb, cdberror);
      goto errorExit;
    }
    cp = dbshow (newdb, "sha1");
    if (cp != 0)
      strcpy (new_sha1, cp);
    else {
      printf ("\n***Error: dbshow (%s, sha1): %s\n", newdb, cdberror);
      goto errorExit;
    }
    if (strcmp (old_sha1, new_sha1)) {
      sha1_chg = 1;
      print_changes++;
    }
    ripe_chg = 0;
    cp = dbshow (olddb, "ripe");
    if (cp != 0)
      strcpy (old_ripe, cp);
    else {
      printf ("\n***Error: dbshow (%s, ripe): %s\n", olddb, cdberror);
      goto errorExit;
    }
    cp = dbshow (newdb, "ripe");
    if (cp != 0)
      strcpy (new_ripe, cp);
    else {
      printf ("\n***Error: dbshow (%s, ripe): %s\n", newdb, cdberror);
      goto errorExit;
    }
    if (strcmp (old_ripe, new_ripe)) {
      ripe_chg = 1;
      print_changes++;
    }

    if (print_changes) {
      if (isExcludes) {
	cp = dbshow (olddb, "fspec");
	if (cp != 0)
	  strcpy (old_fspec, cp);
	else {
	  printf ("\n\n***Error: dbshow (%s, fspec): %s\n", olddb, cdberror);
	  return _ERROR_;
	}
	lnk->data = &old_fspec;
	found = findShellItem (excludes, lnk);
	if (found != 0) {
	  dbnextindex (olddb);
	  continue;
	}
      }
      if (isXtimes) {
        if (isXtime (new_time) == TRUE) {
          dbnextindex (olddb);
          continue;
        }
      }
      is_changes++;
      cp = dbshow (olddb, "fspec");
      if (cp != 0)
	strcpy (old_fspec, cp);
      else {
	printf ("\n\n***Error: dbshow (%s, fspec): %s\n", olddb, cdberror);
	return _ERROR_;
      }
      if (!isQuiet) {
        fprintf (fp, "%s (%d changed parameters)\n", old_fspec,
	    (mtime_chg)?print_changes+1:print_changes);
        fprintf (fp, " Changes [ ");
        if (user_chg)
	  fprintf (fp, "user ");
        if (grp_chg)
	  fprintf (fp, "grp ");
        if (size_chg)
	  fprintf (fp, "size ");
        if (mode_chg)
	  fprintf (fp, "mode ");
        if (mtime_chg)
	  fprintf (fp, "time ");
        if (md5_chg)
	  fprintf (fp, "md5 ");
        if (sha1_chg)
	  fprintf (fp, "sha1 ");
        if (ripe_chg)
	  fprintf (fp, "ripe ");
        fprintf (fp, "]\n");
        fprintf (fp, "   old uid: %20s  new uid: %20s\n", old_uid, new_uid);
        fprintf (fp, "   old gid: %20s  new gid: %20s\n", old_gid, new_gid);
        fprintf (fp, "  old size: %20s new size: %20s\n", old_size, new_size);
        fprintf (fp, "  old mode: %20s new mode: %20s\n", old_mode, new_mode);
        fprintf (fp, "  old time: %20s new time: %20s\n\n", old_time, new_time);
        fprintf (fp, "  old md5sum =  %s\n", old_md5);
        fprintf (fp, "  new md5sum =  %s\n\n", new_md5);
        fprintf (fp, "  old sha1sum = %s\n", old_sha1);
        fprintf (fp, "  new sha1sum = %s\n\n", new_sha1);
        fprintf (fp, "  old ripesum = %s\n", old_ripe);
        fprintf (fp, "  new ripesum = %s\n\n\n", new_ripe);
      }
      else { /* isQuiet == TRUE */
	fprintf (fp, "%s : %s\n", new_time, old_spec);
      }
    }
    dbnextindex (olddb);
  }
  if (! is_changes) {
    fclose (fp);
    strcpy (tmp, bname_olddb);
    tmp[strlen(tmp)-3] = '\0';
    strcpy (file, output_directory);
    strcat (file, tmp);
    strcat (file, "_changes.");
    strcat (file, tstamp);
    remove (file);
  }
  else {
    strcpy (tmp, bname_olddb);
    tmp[strlen(tmp)-3] = '\0';
    strcpy (file, output_directory);
    strcat (file, tmp);
    strcat (file, "_changes.");
    strcat (file, tstamp);
    printf ("%d changes in %s\n", is_changes, file);
    fprintf (fp, "\nDone: %d changes\n\n", is_changes);
    fclose (fp);
  }
  if (isExcludes)
    free (lnk);

  return _OK_;

errorExit:
  fclose (fp);
  return _ERROR_;
}

static int excludes_compare (void *p1, void *p2)
{
  return (strcmp ((char*)p1, (char*)p2));
}

#define EXC_PARAM    1 /* bad parameter */
#define EXC_FILE     2 /* could not open file or file doesn't exist */
#define EXC_SHELL    3 /* shell error */
#define EXC_NOMEM    4 /* dynamic memory error */

int populate_excludes_list (char *file)
{
  FILE *fp;
  char line[2048]; /* arbitrarily large */
  char *cp;
  Link *lnk;
  char *data;
  int status;

  excludes = 0;
  if (file == 0 || *file == 0)
    return EXC_PARAM;
  if (!fexists (file))
    return EXC_FILE;
  fp = fopen (file, "r");
  if (0 == fp) {
    return EXC_FILE;
  }
  excludes = initShell (excludes_compare, FALSE, TRUE);
  if (0 == excludes)
    return EXC_SHELL;
  while (TRUE) {
    if (fgets (line, 2040, fp) == 0)
      break;
    if (feof (fp))
      break;
    if (line[0] == '\n')
      continue;
    cp = strchr (line, '\n');
    if (0 != cp)
      *cp = '\0';
    if (strlen (line) == 0)
      continue;
    data = malloc (strlen (line)+1);
    if (0 == data) {
      delShell (excludes, 0);
      return EXC_NOMEM;
    }
    strcpy (data, line);
    check_pointer (data);
    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      delShell (excludes, 0);
      free (data);
      return EXC_NOMEM;
    }
    lnk->data = data;
    check_pointer (lnk);
    status = addShellItem (excludes, lnk);
    if (status == _ERROR_) {
      delShell (excludes, 0);
      free (lnk);
      free (data);
      return EXC_SHELL;
    }
  }
  if (excludes && excludes->lh->number > 0)
    isExcludes = TRUE;
  fclose (fp);
  return _OK_;
}

int populate_xtimes_list (char *file)
{
  FILE *fp;
  char line[2048]; /* arbitrarily large */
  char *cp;
  Link *lnk;
  char *data;
  int status;

  xtimes = 0;
  if (file == 0 || *file == 0)
    return EXC_PARAM;
  if (!fexists (file))
    return EXC_FILE;
  fp = fopen (file, "r");
  if (0 == fp) {
    return EXC_FILE;
  }
  xtimes = initShell (excludes_compare, FALSE, TRUE);
  if (0 == xtimes)
    return EXC_SHELL;
  while (TRUE) {
    if (fgets (line, 2040, fp) == 0)
      break;
    if (feof (fp))
      break;
    if (line[0] == '\n')
      continue;
    cp = strchr (line, '\n');
    if (0 != cp)
      *cp = '\0';
    if (strlen (line) == 0)
      continue;
    data = malloc (strlen (line)+1);
    if (0 == data) {
      delShell (xtimes, 0);
      return EXC_NOMEM;
    }
    strcpy (data, line);
    check_pointer (data);
    lnk = malloc (sizeof (Link));
    if (0 == lnk) {
      delShell (xtimes, 0);
      free (data);
      return EXC_NOMEM;
    }
    lnk->data = data;
    check_pointer (lnk);
    status = addShellItem (xtimes, lnk);
    if (status == _ERROR_) {
      delShell (xtimes, 0);
      free (lnk);
      free (data);
      return EXC_SHELL;
    }
  }
  if (xtimes && xtimes->lh->number > 0)
    isXtimes = TRUE;
  fclose (fp);
  return _OK_;
}

void usage (char *arg0)
{
  printf ("\nUsage:\n\n");
  printf ("%s [-h] [-q] [-o out_dir] [-e excl_file] [-x xtime] olddb newdb\n", arg0);
  printf ("\n%s compares the two databases given on the command line\n",
      arg0);
  printf ("and generates a report of adds, deletes and changes.\n");
  printf (
      "\n"
      "Command line:"
      "\n\n"
      "-h\tDisplay this information."
      "\n\n"
      "-q\tOnly include the name of the file and the new time\n"
      "\tin the change report."
      "\n\n"
      "-o\tSpecify the output directory.  By default, /tmp is\n"
      "\tused without the -o switch."
      "\n\n"
      "-e\tSpecify an excludes file.  This is a file that lists\n"
      "\tall the names of files to exclude from the change report.\n"
      "\n\n"
      "-x\tSpecify an xtimes file.  This is a file that lists\n"
      "\ttimes that should be excluded from the change report.\n"
      "\tSo, for example, if you do an upgrade and all changes\n"
      "\tfor the upgrade occur at 2004/11/19 from 1:??PM-2:??PM,\n"
      "\tyou could create an xtimes file with the following in\n"
      "\tit to exclude those changes:\n\n"
      "\t2004/11/19-13\n"
      "\t2004/11/19-14\n\n"
      "olddb\tthis is the old fido snapshot database - you must\n"
      "\tinclude the full path."
      "\n\n"
      "newdb\tthis is the new fido snapshot database - you must\n"
      "\tinclude the full path."
      "\n\n"
      "Examples:"
      "\n\n"
      );
  printf (
      "%s -o /etc/trpwr/fido/tmp -e /tmp/excludes_file /etc/trpwr/fido/host.db /tmp/tmp_host.db\n\n",
      arg0
      );
  printf (
      "Process the fido map for the host called 'host'.  Exclude\n"
      "all files that are listed in the file '/tmp/excludes_file'.\n"
      "The reports will be printed to the directory /etc/trpwr/fido/tmp"
      "\n\n"
      );
  printf (
      "%s -o /tmp -q -o /etc/trpwr/fido/host.db /tmp/tmp_host.db\n\n", arg0
      );
  printf (
      "Process the fido map for the host called 'host'.  Generate\n"
      "the change file so that all that's listed in the change\n"
      "report is the name of the file and the new time.  The reports\n"
      "will be printed to the directory /etc/trpwr/fido/tmp"
      "\n\n"
      );
  printf (
      "%s /etc/trpwr/fido/host.db /tmp/tmp_host.db\n\n", arg0
      );
  printf (
      "Process the fido map for the host called 'host'.  The reports\n"
      "will be printed in the directory /tmp."
      "\n\n"
      "Notes:"
      "\n\n"
      "You must use the full path for the database names.  The first\n"
      "database must be the older version of the snapshot generated\n"
      "by fido and the second must be the recent version.  If\n"
      "you reverse this, the reports will be useless."
      "\n\n"
      );
}

int main (int argc, char *argv[])
{
  int status = 0;
  int default_output_directory = TRUE;
  char *cp;
  int ch;
  static char pwd[1024];
  char newdb[1024], olddb[1024];

  if (argc == 1) {
    usage(argv[0]);
    return _ERROR_;
  }
  opterr = 0;
  while (( ch = getopt (argc, argv, "hqo:e:x:")) != EOF) {
    switch (ch) {
      case 'o':
	strncpy (output_directory, optarg, 1020);
	default_output_directory = FALSE;
	break;
      case 'q':
	isQuiet = TRUE;
	break;
      case 'e':
	status = populate_excludes_list (optarg);
	if (status != _OK_) {
	  switch (status) {
	    case EXC_PARAM:
	      printf ("\n\n***Error: bad function parameter");
	      printf (" to populate_excludes_list(): %s\n", optarg);
	      return _ERROR_;
	    case EXC_FILE:
	      printf ("\n\n***Error: could not open %s or %s does not exist\n",
		  optarg, optarg);
	      return _ERROR_;
	    case EXC_SHELL:
	      printf ("\n\n***Error: shell error creating excludes shell: %s\n",
		  shlErrorStr[shlError]);
	      return _ERROR_;
	    case EXC_NOMEM:
	      printf ("\n\n***Error: fatal memory error\n");
	      return _ERROR_;
	    default:
	      /* shouldn't get here, but let's keep gcc happy, eh */
	      return _ERROR_;
	  }
	}
	break;
      case 'x':
	status = populate_xtimes_list (optarg);
	if (status != _OK_) {
	  switch (status) {
	    case EXC_PARAM:
	      printf ("\n\n***Error: bad function parameter");
	      printf (" to populate_xtimes_list(): %s\n", optarg);
	      return _ERROR_;
	    case EXC_FILE:
	      printf ("\n\n***Error: could not open %s or %s does not exist\n",
		  optarg, optarg);
	      return _ERROR_;
	    case EXC_SHELL:
	      printf ("\n\n***Error: shell error creating xtimes shell: %s\n",
		  shlErrorStr[shlError]);
	      return _ERROR_;
	    case EXC_NOMEM:
	      printf ("\n\n***Error: fatal memory error\n");
	      return _ERROR_;
	    default:
	      /* shouldn't get here, but let's keep gcc happy, eh */
	      return _ERROR_;
	  }
	}
	break;
      case 'h':
	usage(argv[0]);
	return _OK_;
    }
  }
  if (optind+2 != argc) {
    printf ("\n\n***Error: no non-switch arguments on the command line\n\n");
    usage (argv[0]);
    return _ERROR_;
  }
  if (default_output_directory == TRUE) {
    strcpy (output_directory, "/tmp/");
  }
  else
    strcat (output_directory, "/");
  cp = (char *)sortedTimeStamp();
  strcpy (tstamp, cp);
  tstamp[8] = '\0';

  /*
   * OK, let's open the db files and begin.
   */
  if (!fexists (argv[optind])) {
    printf ("\n***Error: \"%s\" does not exist\n", argv[optind]);
    return _ERROR_;
  }
  if (!fexists (argv[optind+1])) {
    printf ("\n***Error: \"%s\" does not exist\n", argv[optind+1]);
    return _ERROR_;
  }
  /* first, let's populate the dirname and basename arrays */
  errno = 0;
  strcpy (pwd, argv[optind]);
  cp = dirname (pwd);
  if (errno == _ERROR_) {
    perror ("\n***Error: dirname");
    return _ERROR_;
  }
  strcpy (dname_olddb, cp);
  strcpy (pwd, argv[optind]);
  cp = basename (pwd);
  if (errno == _ERROR_) {
    perror ("\n***Error: basename");
    return _ERROR_;
  }
  strcpy (bname_olddb, cp);
  strcpy (pwd, argv[optind+1]);
  cp = dirname (pwd);
  if (errno == _ERROR_) {
    perror ("\n***Error: dirname");
    return _ERROR_;
  }
  strcpy (dname_newdb, pwd);
  strcpy (pwd, argv[optind+1]);
  cp = basename (pwd);
  if (errno == _ERROR_) {
    perror ("\n***Error: basename");
    return _ERROR_;
  }
  strcpy (bname_newdb, cp);

  /* now, set pwd to cwd */
  getcwd (pwd, 1020);
  if (errno == _ERROR_) {
    perror ("\n****Error: getcwd");
    return _ERROR_;
  }
  status = chdir (dname_newdb);
  if (status == _ERROR_) {
    perror ("\n***Error: chdir");
    return _ERROR_;
  }
  cp = (char *)dbopen (bname_newdb);
  if (cp != 0)
    strcpy (newdb, cp);
  else {
    printf ("\n***Error: could not open \"%s\": %s\n", bname_newdb,
	cdberror);
    return _ERROR_;
  }
  status = chdir (dname_olddb);
  if (status == _ERROR_) {
    perror ("\n***Error: chdir");
    dbclose (newdb);
    return _ERROR_;
  }
  cp = (char *)dbopen (bname_olddb);
  if (cp != 0)
    strcpy (olddb, cp);
  else {
    printf ("\n***Error: could not open \"%s\": %s\n", bname_olddb,
	cdberror);
    dbclose (newdb);
    return _ERROR_;
  }

  printf ("Doing adds...\n");
  status = fido_adds (newdb, olddb);
  if (status == _ERROR_)
    goto errorExit;
  printf ("Doing deletes...\n");
  status = fido_deletes (newdb, olddb);
  if (status == _ERROR_)
    goto errorExit;
  printf ("Doing changes...\n");
  status = fido_changes (newdb, olddb);
  if (status == _ERROR_)
    goto errorExit;

  /*
   * Close the tables and hit the road.
   */
  status = chdir (dname_newdb);
  if (status == _ERROR_) {
    /* try to close them anyway*/
    perror ("\n***Error: chdir");
    goto errorExit;
  }
  dbclose (newdb);
  status = chdir (dname_olddb);
  if (status == _ERROR_) {
    /* try to close it anyway */
    chdir (dname_olddb);
    dbclose (olddb);
    perror ("\n***Error: chdir");
    return _ERROR_;
  }
  dbclose (olddb);
  chdir (pwd);
  if (isExcludes)
    delShell (excludes, 0);
  if (isXtimes)
    delShell (xtimes, 0);

  return _OK_;

errorExit:
  chdir (dname_newdb);
  dbclose (newdb);
  chdir (dname_olddb);
  dbclose (olddb);
  return _ERROR_;
}
