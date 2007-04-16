#include <grace/system.h>
#include <grace/md5.h>
#include "platform.h"
#include <unistd.h>

systemclass kernel;
const char *SALTSRC = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
					  "0123456789./";

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
