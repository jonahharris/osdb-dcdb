/* Source File: beatuptree.c */

/*
 * Put the shell code through the ringer.
 */

#include <sort.h>

#ifdef __CINT__
#include "sort.c"
#include "qsort.c"
#endif /* __CINT__ */

#ifndef __CINT__
#define HASH_STR(x)  (((x)[0]<<15)+((x)[1]<<10)+((x)[2]<<5)+(x)[3])
__inline__ int testCompare (void *p1, void *p2)
{
  unsigned int v1 = HASH_STR((char*)p1),
               v2 = HASH_STR((char*)p2);
  if (v1 < v2)
    return -1;
  else if (v1 > v2)
    return 1;
  else
    return (strcmp ((char *)p1, (char *)p2));
}

__inline__ void printDump (void *data)
{
	printf ("%s\n", (char *)data);
}
#else
int testCompare (void *p1, void *p2)
{
  return (strcmp ((char *)p1, (char *)p2));
}

void printDump (void *data)
{
	printf ("%s\n", (char *)data);
}
#endif

#define	DATASIZE	40
#define MAXDATA		1000000

char data[512];
int main (int argc, char *argv[])
{
	FILE *fp;
	char *ln, *cp;
	Link *lnk = 0, *found = 0;
	int status;
	double t1, t2;
	int isOK = TRUE;
	shellHeader *shl;
	int counter;
	shellLevel lvl;
	int i;
	static int compares[MAXDATA];
	int maxCompares;
	double avgCompares;
#ifdef	OLD_STORE_METHOD
	treeStore *ts;
	time_t now;
#endif
#ifdef STATS
	double high_level_nodes = 0.0;
	double zero_level_nodes = 0.0;
	double list_level_nodes = 0.0;
#endif

	if (argc != 4)  {
		printf ("\nUsage: %s <file> <file.srt> <file.rev.srt>\n", argv[0]);
		printf ("\twhere <file> is the name of the file to read\n");
		printf ("\t  and <file.srt> is a sorted version of <file>\n");
		printf ("\t  and <file.rev.srt> is in reverse sorted order.\n\n");
		return 1;
	}

	Assert (argv[1] != NULL && argv[1][0] != '\0');
	Assert (argv[2] != NULL && argv[2][0] != '\0');
	fp = fopen (argv[1], "r");
	if (fp == NULL) {
		printf ("\n***Error: couldn't open %s\n", argv[1]);
		return -1;
	}

	/*data = malloc (512);
	if ( 0 == data )	{
		printf ("\n***Error: fatal memory error\n");
		return -1;
	}*/
	/* memset (data, 0, 512); */
	/*check_pointer (data);*/

	/*
	 * Populate the shell
	 */
	shl = initShell (testCompare, FALSE, TRUE);
#ifndef __CINT__
	elapsed (&t1);
#else
	t1 = dbtime ();
#endif

	counter = 0;
	while (!feof (fp) && counter < MAXDATA)    {
		counter++;
		/* memset (data, 0, 512); */
		cp = fgets (data, 80, fp);
		if (0 == cp)	{
			break;
		}
		if (data[0] == '\0')
		  break;
		cp = strchr (data, '\n');
		if (0 != cp)
		  *cp = '\0';
		/* skip blank lines */
		if (strlen (data) == 0)
		  continue;
		ln = malloc (DATASIZE);
		if (0 == ln)    {
			printf ("\n***Error: memory error\n");
			return -1;
		}
		/* memset (ln, 0, DATASIZE); */
		strncpy (ln, data, DATASIZE);
		ln[DATASIZE-1] = '\0';
		check_pointer (ln);

		lnk = malloc (sizeof (Link));
		if (0 == lnk)   {
			printf ("\n***Error: memory error\n");
			return -1;
		}
		/* memset (lnk, 0, sizeof (Link));*/ 
	   check_pointer (lnk);
		lnk->data = ln;

		status = addShellItem (shl, lnk);
#ifdef STATS
		high_level_nodes += hln;
		zero_level_nodes += zln;
		list_level_nodes += llt;
#endif
/*#ifdef MASS_DUPS
		if (counter % 350 == 0)
			restructureShellNodes (shl);
#endif*/
		/* BUGBUG */
		/*printf ("%04d:%s\n", counter, (char *)lnk->data);*/
		/* BUGBUG */
		compares[counter-1] = shl->numCompares;
		if (status == _ERROR_)	{
			if (shlError == SHELL_LIST)	{
				printf ("\n\n***Error: List Error, item %d, %s\n",
					counter, ListErrorString[ListError]);
				delShell (shl, 0);
				return _ERROR_;
			}
			else	{
				printf ("\n\n***Error: item %d, %s entering \"%s\"\n", 
						counter, shlErrorStr[shlError], ln);
				delShell (shl, 0);
				return _ERROR_;
			}
		}
		debugCheckShell (shl);
		if (shlError == SHELL_UNSPECIFIED)
			return _ERROR_;

	}
	printf ("\n\nDone\n");
#ifdef STATS
	printf ("High level node traversal: %12.10lf\n", high_level_nodes);
	printf ("Zero level node traversal: %12.10lf\n", zero_level_nodes);
	printf ("List level traversal: %12.10lf\n\n", list_level_nodes);
#endif
#ifndef __CINT__
	elapsed (&t2);
#else
	t2 = dbtime();
#endif
	fclose (fp);
	nodeLevels (shl, &lvl);


	printf ("Add time: %f\n", t2-t1);

	printf ("\nSucceeded...%d lines stored\n", shl->lh->number);
	printf ("Number of nodes = %d\n", shl->numNodes);
	printf ("Node levels: ");
	for (i = 0; i < NODE_LEVEL; i++)
	  printf ("%d %d, ", i, lvl.lvl[i]);
	printf ("\n");

	maxCompares = 0;
	avgCompares = 0.0;
	for (i = 0; i < counter; i++) {
		if (compares[i] > maxCompares)
			maxCompares = compares[i];
		avgCompares += compares[i];
	}
	avgCompares /= counter;
	printf ("\n\nCompares during adds: max = %d, avg = %f\n",
	    maxCompares, avgCompares);

#ifdef	VERBOSE

	printf ("\nOpening test file and comparing with shell contents\n");
#endif

	/*
	 * Open test file and search for each item in the shell
	 */
	fp = fopen (argv[1], "r");
	if (0 == fp)	{
		printf ("\n\n***Error: Couldn't open %s\n", argv[1]);
		return _ERROR_;
	}

	lnk = malloc (sizeof (Link));
	if (0 == lnk)	{
		printf ("\n\n***Error: fatal memory error\n");
		return _ERROR_;
	}
	/*memset (lnk, 0, sizeof (Link));*/
	check_pointer (lnk);

	ln = malloc (DATASIZE);
	if (0 == ln) {
	  printf ("\n\n***Error: fatal memory error\n");
	  return _ERROR_;
	}
	restructureShellNodes (shl);
#ifndef __CINT__
	elapsed (&t1);
#else
	t1 = dbtime();
#endif
	counter = 0;
	while (!feof (fp))	{
		/*memset (data, 0, 512);*/
		counter++;
		cp = fgets (data, 80, fp);
		if (0 == cp)	{
			break;
		}
		if (data[0] == '\0')
		  break;
		cp = strchr (data, '\n');
		if (0 != cp)
		  *cp = '\0';
		if (strlen (data) == 0)
		  continue;
		strcpy (ln, data);
		lnk->data = ln;
		/* BUGBUG */
		/*printf ("\nCalling findShellItem\n");*/
		/* BUGBUG */
		found = findShellItem (shl, lnk);
		if (0 == found)	{
			printf ("Didn't find \"%s\"\n", (char *)ln);
			isOK = FALSE;
		}
		compares[counter-1] = shl->numCompares;
		found = queryShellItem (shl, lnk);
		if (0 == found)	{
			printf ("Didn't find (in query) \"%s\"\n", (char *)data);
			isOK = FALSE;
		}
	}
#ifndef __CINT__
	elapsed (&t2);
#else
	t2 = dbtime();
#endif

	if (!isOK)	{
		printf ("\n***Problem with shell data, original filling\n");
		return -1;
	}

	free (lnk);
	free (ln);
	fclose (fp);


	printf ("Search Time for 2 searches: %f\n", t2-t1);

	maxCompares = 0;
	avgCompares = 0.0;
	for (i = 0; i < counter; i++) {
		if (compares[i] > maxCompares)
			maxCompares = compares[i];
		avgCompares += compares[i];
	}
	avgCompares /= counter;
	printf ("\nCompares during find: max = %d, avg = %f\n",
	    maxCompares, avgCompares);

#ifdef	VERBOSE
	printf ("Doing sorted and reverse sorted compares\n");
#endif

	/*
	 * Do the ordered compare
	 */
	fp = fopen (argv[2], "r");
	if (0 == fp)	{
		printf ("\n\n***Error: Couldn't open %s\n", argv[2]);
		return _ERROR_;
	}

#ifndef __CINT__
	elapsed (&t1);
#else
	t1 = dbtime();
#endif
	found = shl->lh->head->next;
	while (!feof (fp) && found != 0)	{
		/*memset (data, 0, 512);*/
		cp = fgets (data, 80, fp);
		if (0 == cp)	{
			break;
		}
		if (data[0] == '\0')
		  break;
		cp = strchr (data, '\n');
		if (0 != cp)
		  *cp = '\0';
		if (strlen (data) == 0)
		  continue;
		if (strcmp (data, (char *)found->data))	{
			printf ("Bad compare: \"%s\" <-> \"%s\"\n",
					(char *)data, (char *)found->data);
			isOK = FALSE;
		}
		found = found->next;
	}

	if (!isOK)	{
		printf ("\n***Problem with shell data, original filling\n");
		return -1;
	}

	fclose (fp);

	/*
	 * Do the reverse ordered compare
	 */
	fp = fopen (argv[3], "r");
	if (0 == fp)	{
		printf ("\n\n***Error: Couldn't open %s\n", argv[3]);
		return _ERROR_;
	}

	found = shl->lh->tail->prev;
	while (!feof (fp) && found != 0)	{
		/*memset (data, 0, 512);*/
		cp = fgets (data, 80, fp);
		if (0 == cp)	{
			break;
		}
		if (data[0] == '\0')
		  break;
		cp = strchr (data, '\n');
		if (0 != cp)
		  *cp = '\0';
		if (strlen (data) == 0)
		  continue;
		if (strcmp (data, (char *)found->data))	{
			printf ("Bad compare: \"%s\" <-> \"%s\"\n",
					(char *)data, (char *)found->data);
			isOK = FALSE;
		}
		found = found->prev;
	}
#ifndef __CINT__
	elapsed (&t2);
#else
	t2 = dbtime();
#endif

	if (!isOK)	{
		printf ("\n***Problem with shell data, original filling\n");
		return -1;
	}

	fclose (fp);

#ifdef	VERBOSE
	printf ("Compare Time: %f\n", t2-t1);

	printf ("Restructuring shell nodes\n\n");
#endif

	/*
	 * Now, do a restructureShellNodes() and run through the
	 * searches, etc. again.
	 */
#ifndef __CINT__
	elapsed (&t1);
#else
	t1 = dbtime ();
#endif
	status = restructureShellNodes (shl);
#ifndef __CINT__
	elapsed (&t2);
#else
	t1 = dbtime ();
#endif
	if (_ERROR_ == status)	{
		printf ("\n\n***Error: restructuring nodes, %s\n",
				shlErrorStr[shlError]);
		delShell (shl, 0);
		return _ERROR_;
	}
#ifdef	VERBOSE
	nodeLevels (shl, &lvl);
	printf ("Number of nodes after restruct = %d\n", shl->numNodes);
	printf ("Node levels: ");
	for (i = 0; i < NODE_LEVEL; i++)
	  printf ("%d %d, ", i, lvl.lvl[i]);
	printf ("\n");
	printf ("Time to restructure, %f\n", t2-t1);
	printf ("Number of nodes = %d\n", shl->numNodes);
#endif

	fp = fopen (argv[1], "r");
	if (0 == fp)	{
		printf ("\n\n***Error: Couldn't open %s\n", argv[1]);
		return _ERROR_;
	}

	lnk = malloc (sizeof (Link));
	if (0 == lnk)	{
		printf ("\n\n***Error: fatal memory error\n");
		return _ERROR_;
	}
	/*memset (lnk, 0, sizeof (Link));*/
	check_pointer (lnk);

	ln = malloc (DATASIZE);
	if (0 == ln) {
	  printf ("\n\n***Error: critical memory error\n");
	  return _ERROR_;
	}
	counter = 0;
	restructureShellNodes (shl);
#ifndef __CINT__
	elapsed (&t1);
#else
	t1 = dbtime ();
#endif
	while (!feof (fp))	{
		/*memset (data, 0, 512);*/
		cp = fgets (data, 80, fp);
		if (0 == cp)	{
			break;
		}
		if (data[0] == '\0')
		  break;
		cp = strchr (data, '\n');
		if (0 != cp)
		  *cp = '\0';
		if (strlen (data) == 0)
		  continue;
		counter++;
		strcpy (ln, data);
		lnk->data = ln;
		found = findShellItem (shl, lnk);
		if (0 == found)	{
			printf ("After restruct: didn't find \"%s\"\n", (char *)data);
			isOK = FALSE;
		}
		found = queryShellItem (shl, lnk);
		if (0 == found)	{
			printf ("After restruct: didn't find (in query) \"%s\"\n", (char *)data);
			isOK = FALSE;
		}
		compares[counter-1] = shl->numCompares;
	}
#ifndef __CINT__
	elapsed (&t2);
#else
	t1 = dbtime ();
#endif

	free (ln);
	if (!isOK)	{
		printf ("\n***Problem with shell data after restructure\n");
		return -1;
	}

	maxCompares = 0;
	avgCompares = 0.0;
	for (i = 0; i < counter; i++) {
		if (compares[i] > maxCompares)
			maxCompares = compares[i];
		avgCompares += compares[i];
	}
	avgCompares /= counter;
	printf ("\nCompares during queries: max = %d, avg = %f\n",
	    maxCompares, avgCompares);

	free (lnk);
	fclose (fp);


#ifdef	VERBOSE
	printf ("After restruct:search Time: %f\n", t2-t1);

	printf ("Doing sorted and reverse sorted compares\n");
#endif

	/*
	 * Do the ordered compare
	 */
	fp = fopen (argv[2], "r");
	if (0 == fp)	{
		printf ("\n\n***Error: Couldn't open %s\n", argv[2]);
		return _ERROR_;
	}

#ifndef __CINT__
	elapsed (&t1);
#else
	t1 = dbtime ();
#endif
	found = shl->lh->head->next;
	while (!feof (fp) && found != 0)	{
		/*memset (data, 0, 512);*/
		cp = fgets (data, 80, fp);
		if (0 == cp)	{
			break;
		}
		if (data[0] == '\0')
		  break;
		cp = strchr (data, '\n');
		if (0 != cp)
		  *cp = '\0';
		if (strlen (data) == 0)
		  continue;
		if (strcmp (data, (char *)found->data))	{
			printf ("Bad compare: \"%s\" <-> \"%s\"\n",
					(char *)data, (char *)found->data);
			isOK = FALSE;
		}
		found = found->next;
	}

	if (!isOK)	{
		printf ("\n***Problem with shell data, original filling\n");
		return -1;
	}

	fclose (fp);

	/*
	 * Do the reverse ordered compare
	 */
	fp = fopen (argv[3], "r");
	if (0 == fp)	{
		printf ("\n\n***Error: Couldn't open %s\n", argv[3]);
		return _ERROR_;
	}

	found = shl->lh->tail->prev;
	while (!feof (fp) && found != 0)	{
		/*memset (data, 0, 512);*/
		cp = fgets (data, 80, fp);
		if (0 == cp)	{
			break;
		}
		if (data[0] == '\0')
		  break;
		cp = strchr (data, '\n');
		if (0 != cp)
		  *cp = '\0';
		if (strlen (data) == 0)
		  continue;
		if (strcmp (data, (char *)found->data))	{
			printf ("Bad compare: \"%s\" <-> \"%s\"\n",
					(char *)data, (char *)found->data);
			isOK = FALSE;
		}
		found = found->prev;
	}
#ifndef __CINT__
	elapsed (&t2);
#else
	t1 = dbtime ();
#endif

	if (!isOK)	{
		printf ("\n***Problem with shell data, original filling\n");
		return -1;
	}

	fclose (fp);

#ifdef	VERBOSE
	printf ("After restruct:Compare Time: %f\n", t2-t1);

	printf ("\nSaving tree to file beatupshell.dat\n");
#endif

#ifndef	OLD_STORE_METHOD
#ifndef __CINT__
	elapsed (&t1);
#else
	t1 = dbtime ();
#endif
	status = saveShell (shl, "Test shell storage functionality",
						DATASIZE, "beatupshell.dat");
	if (_ERROR_ == status)	{
		if (shlError == SHELL_LIST)
		  printf ("\n***Error: storing data, %s\n", 
				  ListErrorString[ListError]);
		else
		  printf ("\n***Error: storing data, %s\n",
				  shlErrorStr[shlError]);
		return (_ERROR_);
	}
#ifndef __CINT__
	elapsed (&t2);
#else
	t1 = dbtime ();
#endif

#ifdef	VERBOSE
	printf ("Store Time: %f\n", t2-t1);

	printf ("\nDeleting and initializing the Shell again\n");
#endif

	delShell (shl, NULL);

#ifndef __CINT__
	elapsed (&t1);
#else
	t1 = dbtime ();
#endif
	shl = getShell (testCompare, "beatupshell.dat");
	if (shl == 0)	{
		if (shlError == SHELL_LIST)
		  printf ("\n***Error retrieving shell, %s\n", 
				  ListErrorString[ListError]);
		else
		  printf ("\n***Error retrieving shell, %s\n",
				  shlErrorStr[shlError]);
		return _ERROR_;
	}
#ifndef __CINT__
	elapsed (&t2);
#else
	t1 = dbtime ();
#endif

#endif	/* OLD_STORE_METHOD */

#ifdef	OLD_STORE_METHOD
#ifndef __CINT__
	elapsed (&t1);
#else
	t1 = dbtime ();
#endif
	ts = malloc (sizeof (treeStore));
	if (0 == ts)	{
		printf ("\n***Error: memory error allocating tree store\n");
		return -1;
	}
	/*memset (ts, 0, sizeof (treeStore));*/
	check_pointer (ts);

	/*
	 * Fill in the treeStore variables we have to now.
	 */
	ts->thisMagic = 0xdeadbeefUL;
	time (&now);
    strcpy (ts->timeStamp, asctime (localtime (&now)));
	strcpy (ts->description, "File to test shell storage functionality");
	/*
	 * The rest of the ts fields are filled out by storeShell().
	 */
	status = storeShell (shl, ts, DATASIZE, "beatupshell.dat");
	if (_ERROR_ == status)	{
		if (shlError == SHELL_LIST)
		  printf ("\n***Error: storing data, %s\n", 
				  ListErrorString[ListError]);
		else
		  printf ("\n***Error: storing data, %s\n",
				  shlErrorStr[shlError]);
		return (_ERROR_);
	}
#ifndef __CINT__
	elapsed (&t2);
#else
	t1 = dbtime ();
#endif

#ifdef	VERBOSE
	printf ("Store Time: %f\n", t2-t1);

	printf ("\nDeleting and initializing the Shell again\n");
#endif

	delShell (shl, NULL);
	/*memset (ts, 0, sizeof (treeStore));*/
	check_pointer (ts);
	shl = malloc (sizeof (shellHeader));
	if (0 == shl)	{
		printf ("\n***Error initializing shell again, no memory\n");
		return (-1);
	}
	memset (shl, 0, sizeof (shellHeader));
	shl->compare = testCompare;

#ifndef __CINT__
	elapsed (&t1);
#else
	t1 = dbtime ();
#endif
	status = retrieveShell (shl, ts, "beatupshell.dat", 0xdeadbeefUL);
	if (status == _ERROR_)	{
		if (shlError == SHELL_LIST)
		  printf ("\n***Error retrieving shell, %s\n", 
				  ListErrorString[ListError]);
		else
		  printf ("\n***Error retrieving shell, %s\n",
				  shlErrorStr[shlError]);
		return _ERROR_;
	}
#ifndef __CINT__
	elapsed (&t2);
#else
	t1 = dbtime ();
#endif
#endif	/* OLD_STORE_METHOD */

	nodeLevels (shl, &lvl);

	printf ("\nTime to re-populate the shell, %f\n", t2-t1);

#ifdef	VERBOSE

	printf ("\nRetrieval of data into shell successful");
	printf ("\n%d items stored in shell", shl->lh->number);
	printf ("\nNumber of nodes = %d\n", shl->numNodes);

#endif

/*	printf ("Node levels: 2 %d, 3 %d, 4 %d, 5 %d 6 %d\n",
			lvl.lvltwo, lvl.lvlthree, lvl.lvlfour, lvl.lvlfive,
			lvl.lvlsix);*/
	printf ("Node levels: ");
	for (i = 0; i < NODE_LEVEL; i++)
	  printf ("%d %d, ", i, lvl.lvl[i]);
	printf ("\n");

#ifdef	VERBOSE

	printf ("\nOpening test file and comparing with tree contents\n");

#endif

#ifdef	OLD_STORE_METHOD
	free (ts);
#endif

	fp = fopen (argv[2], "r");
	if (0 == fp)	{
		printf ("\n\n***Error: Couldn't open %s\n", argv[2]);
		return _ERROR_;
	}

	lnk = malloc (sizeof (Link));
	if (0 == lnk)	{
		printf ("\n\n***Error: memory error allocating link\n");
		return _ERROR_;
	}
	ln = malloc (DATASIZE);
	if (0 == ln) {
	  printf ("\n\n***Error: critical memory error\n");
	  return _ERROR_;
	}
#ifndef __CINT__
	elapsed (&t1);
#else
	t1 = dbtime ();
#endif
	memset (data, 0, 512);
	counter = 0;
	while (!feof (fp))	{
		cp = fgets (data, 80, fp);
		if (0 == cp)	{
			break;
		}
		cp = strchr (data, '\n');
		if (0 != cp)
		  *cp = '\0';
		if (data[0] == '\0')
		  continue;
		counter++;
		strcpy (ln, data);
		lnk->data = ln;
		found = findShellItem (shl, lnk);
		if (0 == found)	{
			printf ("Didn't find \"%s\"\n", (char *)data);
			isOK = FALSE;
		}
		found = queryShellItem (shl, lnk);
		if (0 == found)	{
			printf ("Didn't find (in query) \"%s\"\n", (char *)data);
			isOK = FALSE;
		}
		compares[counter-1] = shl->numCompares;
	}
#ifndef __CINT__
	elapsed (&t2);
#else
	t1 = dbtime ();
#endif
	if (!isOK)	{
		printf ("\n***Problem with tree data, retieval from data disk\n");
		return -1;
	}
	fclose (fp);
	free (lnk);

	printf ("Search Time: %f\n", t2-t1);

	maxCompares = 0;
	avgCompares = 0.0;
	for (i = 0; i < counter; i++) {
		if (compares[i] > maxCompares)
			maxCompares = compares[i];
		avgCompares += compares[i];
	}
	avgCompares /= counter;
	printf ("\nCompares during find: max = %d, avg = %f\n",
	    maxCompares, avgCompares);

#ifdef	VERBOSE
	printf ("Doing sorted and reverse sorted compares\n");
#endif

	/*
	 * Do the ordered compare
	 */
	fp = fopen (argv[2], "r");
	if (0 == fp)	{
		printf ("\n\n***Error: Couldn't open %s\n", argv[2]);
		return _ERROR_;
	}

#ifndef __CINT__
	elapsed (&t1);
#else
	t1 = dbtime ();
#endif
	found = shl->lh->head->next;
	while (!feof (fp) && found != shl->lh->tail)	{
		/*memset (data, 0, 512);*/
		cp = fgets (data, 80, fp);
		if (0 == cp)	{
			break;
		}
		if (data[0] == '\0')
		  break;
		cp = strchr (data, '\n');
		if (0 != cp)
		  *cp = '\0';
		/* skip blank lines */
		if (strlen (data) == 0)
		  continue;
		if (strcmp (data, (char *)found->data))	{
			printf ("Bad compare: \"%s\" <-> \"%s\"\n",
					(char *)data, (char *)found->data);
			isOK = FALSE;
		}
		found = found->next;
	}

	if (!isOK)	{
		printf ("\n***Problem with tree data, retrieval from disk\n");
		return -1;
	}

	fclose (fp);

	/*
	 * Do the reverse ordered compare
	 */
	fp = fopen (argv[3], "r");
	if (0 == fp)	{
		printf ("\n\n***Error: Couldn't open %s\n", argv[3]);
		return _ERROR_;
	}

	found = shl->lh->tail->prev;
	while (!feof (fp) && found != shl->lh->head)	{
		/*memset (data, 0, 512);*/
		cp = fgets (data, 80, fp);
		if (0 == cp)	{
			break;
		}
		if (data[0] == '\0')
		  break;
		cp = strchr (data, '\n');
		if (0 != cp)
		  *cp = '\0';
		/* skip blank lines */
		if (strlen (data) == 0)
		  continue;
		if (strcmp (data, (char *)found->data))	{
			printf ("Bad compare: \"%s\" <-> \"%s\"\n",
					(char *)data, (char *)found->data);
			isOK = FALSE;
		}
		found = found->prev;
	}
#ifndef __CINT__
	elapsed (&t2);
#else
	t1 = dbtime ();
#endif
	free (ln);

#ifdef	VERBOSE
	printf ("Search Time: %f\n", t2-t1);

	printf ("Done!\nCleaning up.\n\n");
#endif

	if (!isOK)	{
		printf ("\n***Problem with tree data, retrieval from disk\n");
		return -1;
	}

	fclose (fp);

	/*free (data);*/
/*	free (lnk);*/

	delShell (shl, NULL);
	/*print_block_list ();*/

	return 0;
/*#endif	*/ /* if 0 */
}
