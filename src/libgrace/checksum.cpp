#include <string.h>
#include <sys/types.h>
#include <grace/tolower.h>

const unsigned char obox[32] = {1,27,5,17,3,19,12,4,18,11,26,25,22,2,15,6,24,
								7,21,13,0,19,8,16,10,20,4,21,7,18,3,23};

// ========================================================================
// FUNCTION checksum
// ========================================================================
unsigned int checksum (const char *string)
{
	if (! string) return 0;
	register int len,pos;
	register char c;
	len = strlen (string);
	
	register unsigned int result;
	
	result = (((len&0xff)^0x88) << 24) + (((len&0xff)^0x24) << 16) +
			  (((len&0xff)^0x41) << 8) + ((len&0xff)^0x12);
	
	for (pos=0;pos<len;++pos)
	{
		c = (char) _tolower((int) string[pos]);
		if (c & 0xc0)
		{
			result ^= ((((c & 0xdf)^obox[(pos+7) & 0x1f]) & 0x3f) << obox[pos & 0x1f]);
		}
		else
		{
			result ^= (((c^obox[(pos+17) & 0x1f]) & 0x3f) <<
					   obox[(pos^c) & 0x1f]);
		}
		result ^= c;
	}
	return result;
}

const unsigned char obox64[64] = {2,47,54,7,10,37,34,15,6,43,38,9,24,41,8,
								  21,36,33,22,17,52,39,50,1,44,27,4,43,30,15,
								  12,49,48,13,14,31,42,5,26,45,0,51,38,53,16,
								  23,32,37,20,9,40,25,8,39,42,7,14,35,36,11,
								  6,55,46,3};

// ========================================================================
// FUNCTION checksum64
// ========================================================================
unsigned long long checksum64 (const char *string)
{
	register int len, pos;
	unsigned long long c;
	len = strlen (string);
	
	register unsigned long long result = 0;
	unsigned long long len8 = len & 0xff;
	
	result  = (len8 ^ 0x18) << 56;
	result |= (len8 ^ 0x24) << 48;
	result |= (len8 ^ 0x42) << 40;
	result |= (len8 ^ 0x81) << 32;
	result |= (len8 ^ 0x14) << 24;
	result |= (len8 ^ 0x41) << 16;
	result |= (len8 ^ 0x28) << 8;
	result |= (len8 ^ 0x82);
	
	for (pos=0; pos<len; ++pos)
	{
		c = (unsigned long long) _tolower ((int) string[pos]);
	}

	for (pos=0;string[pos];++pos)
	{
		c = (unsigned long long) _tolower((int) string[pos]);
		if (c & 0xc0)
		{
			result ^= ((((c & 0xdf)^obox64[(pos+7) & 0x3f]) & 0x3f) << obox64[pos & 0x3f]);
			result ^= ((((c & 0xdf)^obox64[(len-(pos+7)) & 0x3f]) & 0x3f) << obox64[(len-pos) & 0x3f]);
		}
		else
		{
			result ^= (((c^obox64[(pos+17) & 0x3f]) & 0x3f) <<
					   obox64[(pos^c) & 0x3f]);
			result ^= (((c^obox64[((len-pos)+17) & 0x3f]) & 0x3f) <<
					   obox64[((len-pos)^c) & 0x3f]);
		}
		result += (c * pos);
	}
	return result;
}

// ========================================================================
// FUNCTION djbhash64
// ========================================================================
unsigned long long djbhash64 (const char *string)
{
	register int len, pos;
	unsigned long long c;
	
	register unsigned long long result = 5381;
	for (pos=0; string[pos]; ++pos)
	{
		result = result * 33 ^ _tolower((int) string[pos]);
	}
	
	return result;
}

char _TRANS_TABLE[128] =
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63,
	 0,0,0,0,0,0,0,0,0,0,0,0,62,0,0,0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,
	 0,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,
	 31,32,33,34,35,0,0,0,0,0,0,36,37,38,39,40,41,42,43,44,45,46,47,48,
	 49,50,51,52,53,54,55,56,57,58,59,60,61,0,0,0,0,0};
	 
// ========================================================================
// FUNCTION resid
// ========================================================================
unsigned int resid (const char *str)
{
	char out[4] = {0,0,0,0};
	
	if (*str)
	{
		out[0] = _TRANS_TABLE[str[0]&127];
		if (str[1])
		{
			out[1] = _TRANS_TABLE[str[1]&127];
			if (str[2])
			{
				out[2] = _TRANS_TABLE[str[2]&127];
				if (str[3])
				{
					out[3] = _TRANS_TABLE[str[3]&127];
				}
			}
		}
	}
	return ((out[0]<<18)|(out[1]<<12)|(out[2]<<6)|out[3]);
}

// ========================================================================
// FUNCTION restochar
// ========================================================================
char restochar (int val)
{
	if (val<10) return ('0'+val);
	if (val<36) return ('A'+(val-10));
	if (val<62) return ('a'+(val-36));
	if (val==63) return (' ');
	return ('-');
}
