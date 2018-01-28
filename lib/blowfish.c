/*	Source File:	blowfish.c	*/

/*
 * Uses openssl/blowfish to encrypt data.  Provides interface functions that
 * are used in dcdb.
 */
#include "blowfish/blowfish.h"

static char return_string[MAX_STRING*2+1];
static BF_KEY key;

/*
 * Encrypt len bytes of buf using password as the password.
 * len must be a multiple of 8 and buf and password should not be NULL.
 */
char *blowfishEncryptString (char *buf, int len, char *password)
{
	int pwlen = 0;
	int i;
	char *bufptr, *outptr;
	unsigned char outstr[MAX_STRING+1];

	if (buf == 0)
		return 0;
	if (len % 8)
		return 0;
	if (len > MAX_STRING)	{
		return 0;
	}
	if (password != 0)
		pwlen = strlen (password);
	if (password == 0)	{
		if (key.P[0] == 0)
			return 0;
	}
	else
		BF_set_key (&key, pwlen, password);
	i = len;
	bufptr = buf;
	outptr = outstr;
	memset (outstr, 0, MAX_STRING+1);
	while (i > 0)	{
		BF_ecb_encrypt (bufptr, outptr, &key, BF_ENCRYPT);
		bufptr += 8;
		outptr += 8;
		i -= 8;
	}
	memset (return_string, 0, MAX_STRING*2+1);
	for (i = 0; i < len; i++)
		sprintf (&return_string[i*2], "%02x", outstr[i]);
	return return_string;
}

/*
 * decrypt len bytes of data in buf.  len must be the original length
 * of the data that was encrypted with blowfishEncryptString(), not
 * the actual length of buf.  Also, if password == NULL, it checks to insure
 * that key has been initialized correctly.  If so, it uses the last password
 * entered to decrypt with.
 */
char *blowfishDecryptString (char *buf, int len, char *password)
{
	int pwlen = 0, i;
	unsigned char outstr[MAX_STRING+1];
	char *bufptr, *outptr;
	unsigned char val;

	if (len % 8)
		return 0;
	if (len > MAX_STRING)
		return 0;
	if (password != 0)
		pwlen = strlen (password);
	if (password == 0)	{
		if (key.P[0] == 0)
			return 0;
	}
	else
		BF_set_key (&key, pwlen, password);
	/*
	 * Note: this assumes the encrypted string is lower case.
	 */
	for (i = 0; i < len; i++)	{
		if (buf[2*i] >= '0' && buf[2*i] <= '9')
			val = (buf[2*i]-'0')*16;
		else
			val = (buf[2*i]-'a'+10)*16;
		if (buf[2*i+1] >= '0' && buf[2*i+1] <= '9')
			val += buf[2*i+1]-'0';
		else
			val += buf[2*i+1]-'a'+10;
		outstr[i] = val;
	}
	i = len;
	bufptr = outstr;
	outptr = return_string;
	memset (return_string, 0, MAX_STRING*2+1);
	while (i > 0)	{
		BF_ecb_encrypt (bufptr, outptr, &key, BF_DECRYPT);
		bufptr += 8;
		outptr += 8;
		i -= 8;
	}
	return return_string;
}

#ifdef  TEST_BLOWFISH

#define	MAX_PASSWD	56
#define TEST_STRING     128

char data_stuff[] =     "Four score and seven years ago, our forefathers...\n"
                        "Daisy dotes and dosey dotes...\n"
                        "To be or not to be?  That is the question.\n";
int main (void)
{
        char password[MAX_PASSWD] = "to be or not to be";
        unsigned char data[TEST_STRING+1];
	unsigned char data_copy[TEST_STRING+1];
        int len;
        char *cp;

        memset (data, 0, TEST_STRING+1);
        strcpy (data, data_stuff);
        printf ("String before encryption: \n%s, len = %d\n\n", data, strlen (data));
	cp = blowfishEncryptString (data, 128, password);
	printf ("String after encryption: \n%s, len = %d\n\n", cp, strlen (cp));
	memset (data_copy, 0, TEST_STRING+1);
	strcpy (data_copy, cp);
	cp = blowfishDecryptString (data_copy, 128, password);
	printf ("String after decryption: \n%s, len = %d\n\n", cp, strlen (cp));
	return 0;
}

#endif	/* TEST_BLOWFISH */
