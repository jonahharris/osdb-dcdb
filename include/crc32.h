#ifndef __CRC32_H__
#define __CRC32_H__

/*
 * Compute a 32-bit sum.  Borrowed from linux-2.4.18.
 * /usr/src/linux/fs/jffs2/crc32.[c,h].
 *
 * The author made this code available to anyone for any use.  See
 * cdb-1.1/src/crc32.c for the original copyright.
 */

extern const unsigned crc32_table[256];

/* Return a 32-bit CRC of the contents of the buffer. */

static inline unsigned 
crc32(unsigned val, const void *ss, int len)
{
	const unsigned char *s = (const unsigned char *)ss;
        while (--len >= 0)
                val = crc32_table[(val ^ *s++) & 0xff] ^ (val >> 8);
        return val;
}

extern unsigned crc32Sum (char *file);

#endif
