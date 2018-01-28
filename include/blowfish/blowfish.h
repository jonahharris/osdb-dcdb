/*	Header File:	blowfish.h	*/

/*
 * Use the openssl/blowfish to encrypt/decrypt.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <openssl/blowfish.h>

#define	MAX_STRING	2048

char *blowfishEncryptString (char *buf, int len, char *password);
char *blowfishDecryptString (char *buf, int len, char *password);

