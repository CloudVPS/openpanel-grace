// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#include <string.h>
#include <sys/types.h>
#include <grace/tolower.h>

// =============================================================================
// FUNCTION checksum
// taken after 'DJBHash' http://www.partow.net/programming/hashfunctions/#DJBHashFunction
// An algorithm produced by Professor Daniel J. Bernstein and shown first to the 
// world on the usenet newsgroup comp.lang.c. It is one of the most efficient 
// hash functions ever published. 
// A test with an english dictionary (58K words) produced no collisions.
// =============================================================================
unsigned int checksum (const char *str)
{
    unsigned int hash = 5381;
    unsigned int i    = 0;
    
    const unsigned char* ustr = (const unsigned char*)str;

    for(i = 0; ustr[i]; i++)
    {
        hash = ((hash << 5) + hash) + (ustr[i] & 0xDF);
    }

    return hash;
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
