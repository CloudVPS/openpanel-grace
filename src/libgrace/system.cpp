// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#include <grace/system.h>
#include <grace/md5.h>
#include "platform.h"
#include <unistd.h>

systemclass core;
systemclass &kernel (core);
const char *SALTSRC = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
					  "0123456789./";

// ========================================================================
// FUNCTION __grace_internal_crypt
// ========================================================================
string *__grace_internal_crypt (const char *key, const char *salt)
{
	returnclass (string) res retain;
	if ((salt[0] == '$')&&(salt[1] == '1')&&(salt[2] == '$'))
	{
		md5checksum md5;
		res = md5.md5pw (key, salt+3);
	}
	else
	{
		res = ::crypt (key, salt);
	}
	return &res;
}
