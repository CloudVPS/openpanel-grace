// ========================================================================
// md5.cpp: MD5 checksum class
//
// (C) Copyright 2006 Pim van Riezen <pi@madscience.nl>
//                    Madscience Labs, Rotterdam
//
// Based loosely on the MD5 implementation by L. Peter Deutsch of
// Aladdin Enterprises.
// ========================================================================

#include <grace/md5.h>

md5checksum::md5checksum (void)
{
	init();
}

md5checksum::~md5checksum (void)
{
}

void md5checksum::init (void)
{
	count[0] = count[1] = 0;
	abcd[0] = 0x67452301;
	abcd[1] = 0xefcdab89;
	abcd[2] = 0x98badcfe;
	abcd[3] = 0x10325476;
}

void md5checksum::append (const string &in)
{
	return append ( (md5_byte_t *) in.cval(), in.strlen());
}

void md5checksum::append (const md5_byte_t *dt, int nbytes)
{
	// Set up some counters.
	const md5_byte_t *p = dt;
	int left = nbytes;
	int offset = (count[0] >> 3) & 63;
	md5_word_t nbits = (md5_word_t)(left << 3);

	// Update message length counters.
	count[1] += nbytes >> 29;
	count[0] += nbits;
	if (count[0] < nbits) count[1]++;
	
	// If there's a partial block left in the buffer, process this
	// first.
	if (offset)
	{
		int copy = (offset + nbytes > 64 ? 64-offset : nbytes);
		
		memcpy (buf+offset, p, copy);
		if (offset + copy < 64) return;
		
		p += copy;
		left -= copy;
		process(buf);
	}
	
	// Process any full 64 bit blocks we can from the original
	for (; left >= 64; p += 64, left -= 64)
	{
		process (p);
	}
	
	// Store the remainder in the buffer for the next round.
	if (left) memcpy (buf, p, left);
}

// Lots of magic MD5 smoke
#define T1 0xd76aa478
#define T2 0xe8c7b756
#define T3 0x242070db
#define T4 0xc1bdceee
#define T5 0xf57c0faf
#define T6 0x4787c62a
#define T7 0xa8304613
#define T8 0xfd469501
#define T9 0x698098d8
#define T10 0x8b44f7af
#define T11 0xffff5bb1
#define T12 0x895cd7be
#define T13 0x6b901122
#define T14 0xfd987193
#define T15 0xa679438e
#define T16 0x49b40821
#define T17 0xf61e2562
#define T18 0xc040b340
#define T19 0x265e5a51
#define T20 0xe9b6c7aa
#define T21 0xd62f105d
#define T22 0x02441453
#define T23 0xd8a1e681
#define T24 0xe7d3fbc8
#define T25 0x21e1cde6
#define T26 0xc33707d6
#define T27 0xf4d50d87
#define T28 0x455a14ed
#define T29 0xa9e3e905
#define T30 0xfcefa3f8
#define T31 0x676f02d9
#define T32 0x8d2a4c8a
#define T33 0xfffa3942
#define T34 0x8771f681
#define T35 0x6d9d6122
#define T36 0xfde5380c
#define T37 0xa4beea44
#define T38 0x4bdecfa9
#define T39 0xf6bb4b60
#define T40 0xbebfbc70
#define T41 0x289b7ec6
#define T42 0xeaa127fa
#define T43 0xd4ef3085
#define T44 0x04881d05
#define T45 0xd9d4d039
#define T46 0xe6db99e5
#define T47 0x1fa27cf8
#define T48 0xc4ac5665
#define T49 0xf4292244
#define T50 0x432aff97
#define T51 0xab9423a7
#define T52 0xfc93a039
#define T53 0x655b59c3
#define T54 0x8f0ccc92
#define T55 0xffeff47d
#define T56 0x85845dd1
#define T57 0x6fa87e4f
#define T58 0xfe2ce6e0
#define T59 0xa3014314
#define T60 0x4e0811a1
#define T61 0xf7537e82
#define T62 0xbd3af235
#define T63 0x2ad7d2bb
#define T64 0xeb86d391

void md5checksum::process (const md5_byte_t *data)
{
	md5_word_t a = abcd[0];
	md5_word_t b = abcd[1];
	md5_word_t c = abcd[2];
	md5_word_t d = abcd[3];
	md5_word_t t;
	
#ifdef ARCH_LITTLE_ENDIAN

	md5_word_t xbuf[16];
	const md5_word_t *X;
	
	if (!((data - (const md5_byte_t *)0) & 3))
	{
		X = (const md5_word_t *) data;
	}
	else
	{
		memcpy (xbuf, data, 64);
		X = xbuf;
	}

#else

	md5_word_t X[16];
	const md5_byte_t *xp = data;
	int i;
	
	for (i=0; i<16; ++i, xp += 4)
	{
		X[i] = xp[0] | (xp[1] << 8) | (xp[2] << 16) | (xp[3] << 24);
	}

#endif

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

	// Round 1

#define F(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define SET(a, b, c, d, k, s, Ti)\
		t = a + F(b,c,d) + X[k] + Ti;\
		a = ROTATE_LEFT(t, s) + b

	SET (a, b, c, d, 0, 7, T1);
    SET (d, a, b, c,  1, 12,  T2);
    SET (c, d, a, b,  2, 17,  T3);
    SET (b, c, d, a,  3, 22,  T4);
    SET (a, b, c, d,  4,  7,  T5);
    SET (d, a, b, c,  5, 12,  T6);
    SET (c, d, a, b,  6, 17,  T7);
    SET (b, c, d, a,  7, 22,  T8);
    SET (a, b, c, d,  8,  7,  T9);
    SET (d, a, b, c,  9, 12, T10);
    SET (c, d, a, b, 10, 17, T11);
    SET (b, c, d, a, 11, 22, T12);
    SET (a, b, c, d, 12,  7, T13);
    SET (d, a, b, c, 13, 12, T14);
    SET (c, d, a, b, 14, 17, T15);
    SET (b, c, d, a, 15, 22, T16);

#undef SET

	// Round 2

#define G(x, y, z) (((x) & (z)) | ((y) & ~(z)))
#define SET(a, b, c, d, k, s, Ti)\
		t = a + G(b,c,d) + X[k] + Ti;\
		a = ROTATE_LEFT(t, s) + b

    SET (a, b, c, d,  1,  5, T17);
    SET (d, a, b, c,  6,  9, T18);
    SET (c, d, a, b, 11, 14, T19);
    SET (b, c, d, a,  0, 20, T20);
    SET (a, b, c, d,  5,  5, T21);
    SET (d, a, b, c, 10,  9, T22);
    SET (c, d, a, b, 15, 14, T23);
    SET (b, c, d, a,  4, 20, T24);
    SET (a, b, c, d,  9,  5, T25);
    SET (d, a, b, c, 14,  9, T26);
    SET (c, d, a, b,  3, 14, T27);
    SET (b, c, d, a,  8, 20, T28);
    SET (a, b, c, d, 13,  5, T29);
    SET (d, a, b, c,  2,  9, T30);
    SET (c, d, a, b,  7, 14, T31);
    SET (b, c, d, a, 12, 20, T32);

#undef SET

	// Round 3

#define H(x, y, z) ((x) ^ (y) ^ (z))
#define SET(a, b, c, d, k, s, Ti)\
		t = a + H(b,c,d) + X[k] + Ti;\
		a = ROTATE_LEFT(t, s) + b

    SET (a, b, c, d,  5,  4, T33);
    SET (d, a, b, c,  8, 11, T34);
    SET (c, d, a, b, 11, 16, T35);
    SET (b, c, d, a, 14, 23, T36);
    SET (a, b, c, d,  1,  4, T37);
    SET (d, a, b, c,  4, 11, T38);
    SET (c, d, a, b,  7, 16, T39);
    SET (b, c, d, a, 10, 23, T40);
    SET (a, b, c, d, 13,  4, T41);
    SET (d, a, b, c,  0, 11, T42);
    SET (c, d, a, b,  3, 16, T43);
    SET (b, c, d, a,  6, 23, T44);
    SET (a, b, c, d,  9,  4, T45);
    SET (d, a, b, c, 12, 11, T46);
    SET (c, d, a, b, 15, 16, T47);
    SET (b, c, d, a,  2, 23, T48);

#undef SET

	// Round 4
	
#define I(x, y, z) ((y) ^ ((x) | ~(z)))
#define SET(a, b, c, d, k, s, Ti)\
		t = a + I(b,c,d) + X[k] + Ti;\
		a = ROTATE_LEFT(t, s) + b

    SET (a, b, c, d,  0,  6, T49);
    SET (d, a, b, c,  7, 10, T50);
    SET (c, d, a, b, 14, 15, T51);
    SET (b, c, d, a,  5, 21, T52);
    SET (a, b, c, d, 12,  6, T53);
    SET (d, a, b, c,  3, 10, T54);
    SET (c, d, a, b, 10, 15, T55);
    SET (b, c, d, a,  1, 21, T56);
    SET (a, b, c, d,  8,  6, T57);
    SET (d, a, b, c, 15, 10, T58);
    SET (c, d, a, b,  6, 15, T59);
    SET (b, c, d, a, 13, 21, T60);
    SET (a, b, c, d,  4,  6, T61);
    SET (d, a, b, c, 11, 10, T62);
    SET (c, d, a, b,  2, 15, T63);
    SET (b, c, d, a,  9, 21, T64);

#undef SET

	abcd[0] += a;
	abcd[1] += b;
	abcd[2] += c;
	abcd[3] += d;
}

void md5checksum::finish (md5_byte_t digest[16])
{
    static const md5_byte_t pad[64] = {
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    md5_byte_t data[8];
    int i;

	for (i=0; i<8; ++i)
	{
		data[i] = (md5_byte_t) (count[i>>2] >> ((i&3) << 3));
	}
	
	append (pad, ((55 - (count[0] >> 3)) & 63) + 1);
	append (data, 8);
	
	for (i=0; i<16; ++i)
	{
		digest[i] = (md5_byte_t) (abcd[i>>2] >> ((i&3) << 3));
	}
}

string *md5checksum::checksum (void)
{
	returnclass (string) res retain;
	
	md5_byte_t digest[16];
	finish (digest);
	
	res.strcpy ((const char *) digest, 16);
	return &res;
}

string *md5checksum::base64 (void)
{
	string tmp = checksum();
	return tmp.encode64();
}

string *md5checksum::hex (void)
{
	returnclass (string) res retain;
	
	md5_byte_t digest[16];
	finish (digest);
	
	for (int i=0; i<16; ++i)
	{
		res.printf ("%02x", digest[i]);
	}
	
	return &res;
}

void md5checksum::addencode (string &into, unsigned int v, int n)
{
	static char itoa64[] = "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
						   "abcdefghijklmnopqrstuvwxyz";
	
	while (--n >= 0)
	{
		into.strcat (itoa64[v&0x3f]);
		v >>= 6;
	}
}

string *md5checksum::pw (void)
{
	returnclass (string) res retain;
	
	md5_byte_t digest[16];
	finish (digest);
	
	unsigned int l;
	
	addencode (res, (digest[ 0]<<16) | (digest[ 6]<<8) | digest[12], 4);
	addencode (res, (digest[ 1]<<16) | (digest[ 7]<<8) | digest[13], 4);
	addencode (res, (digest[ 2]<<16) | (digest[ 8]<<8) | digest[14], 4);
	addencode (res, (digest[ 3]<<16) | (digest[ 9]<<8) | digest[15], 4);
	addencode (res, (digest[ 4]<<16) | (digest[10]<<8) | digest[ 5], 4);
	addencode (res, digest[11], 2);
	
	return &res;
}

string *md5checksum::md5pw (const char *pw, const char *salt)
{
	returnclass (string) passwd retain;
	
	md5checksum ctx1;
	unsigned long l;
	int sl, pl;
	unsigned int i;
	string final;
	static const char *sp, *ep;
	static const char *magic = "$1$";
	string tstr;
	
	sp = salt;
	
	if (! strncmp (sp, magic, 3)) sp += 3;
	
	for (ep=sp; *ep && *ep != '$' && ep < (sp+8); ep++) continue;
	
	sl = ep - sp;
	
	append (pw);
	append (magic);
	
	tstr.strcpy (sp, sl);
	append (tstr);
	
	ctx1.append (pw);
	ctx1.append (tstr);
	ctx1.append (pw);
	
	final = ctx1.checksum();
	
	for (pl = strlen(pw); pl>0; pl -= 16)
	{
		tstr.crop();
		tstr.strcpy (final, (pl>16 ? 16 : pl));
		append (tstr);
	}
	
	for (i = strlen(pw); i; i>>=1)
	{
		tstr.crop();
		if (i&1)
		{
			tstr.strcat (final[0]);
			append (tstr);
		}
		else
		{
			tstr.strcat (pw[0]);
			append (tstr);
		}
	}
	
	passwd = magic;
	passwd.strcat (sp, sl);
	passwd.strcat ('$');
	
	final = checksum();
	
	for (i=0; i<1000; ++i)
	{
		ctx1.init();
		
		if (i&1) ctx1.append (pw);
		else ctx1.append (final);
		
		if (i%3)
		{
			tstr = sp;
			tstr.crop (sl);
			ctx1.append (tstr);
		}
		
		if (i%7) append (pw);
		{
			ctx1.append (pw);
		}
		
		if (i&1) ctx1.append (final);
		else ctx1.append (pw);
		
		final = ctx1.checksum();
	}
	
	tstr = ctx1.pw();
	passwd.strcat (tstr);
	
	return &passwd;
}
