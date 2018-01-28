/*  Header File:    getopt.h    */

/*
**  Header file for the getopt.c function.
*/

extern int moptind;
extern char *moptarg;
extern int mopterr;
extern char moptinvch;

#ifdef  __cplusplus
extern "C" {
#endif

int	mgetopt(int argc, char *argv[], char *optionS);

#ifdef  __cplusplus
}
#endif
