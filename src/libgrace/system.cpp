#include <grace/system.h>
#include "platform.h"
#include <unistd.h>

systemclass kernel;
const char *SALTSRC = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
					  "0123456789./";

char *__grace_internal_crypt (const char *key, const char *salt)
{
	return ::crypt (key, salt);
}
