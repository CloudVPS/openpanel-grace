#include <grace/version.h>

namespace grace
{
	const char *MAGICSTR = "@VERSION_MAGIC@";
	const char *VERSION = "%BUILD_SHORTVERSION%";
	const char *VERSION_FULL = "%BUILD_VERSION%";
	const char *VERSION_DETAILS = "%BUILD_DATE% %BUILD_USER%@%BUILD_HOST%";
};
