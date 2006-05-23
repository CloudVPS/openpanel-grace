#ifndef _GNETDB_H
#define _GNETDB_H 1

#include <grace/value.h>

/// NetDB-related exceptions.
enum netdbException {
	EX_NETDB_BAD_ADDRESS = 0xf005831a /// Badly formed address exception.
};

/// Resolver utility class.
class netdb
{
public:
	/// Exceptions.
	enum exception {
		exBadAddress /// Badly formed address exception.
	} netdbExceptions;
	
	/// Resolve a host from its hostname.
	/// \param name The hostname.
	/// \return New value object with result.
	static value *gethostbyname (const string &name);
	
	/// Resolve an address to a name.
	/// \param addr The address in dotted quad notation
	/// \return New value object with result.
	static value *gethostbyaddr (const string &addr);
	
	/// Convert a libc hostentry to a value object.
	/// \return New value object with result.
	static value *converthostentry (struct hostent *entry);
};

#endif
