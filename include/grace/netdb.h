// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _GNETDB_H
#define _GNETDB_H 1

#include <grace/value.h>
#include <grace/ipaddress.h>

$exception (badAddressException, "Invalid address format");

/// Resolver utility class.
class netdb
{
public:
	/// Resolve a host from its hostname.
	/// \param name The hostname.
	/// \return New value object with result.
	static value *gethostbyname (const string &name);
	
	/// Quickly resolve a host from its hostname to an ip address.
	static ipaddress resolve (const string &name);
	
	/// Resolve an address to a name.
	/// \param addr The address in dotted quad notation
	/// \return New value object with result.
	static value *gethostbyaddr (const string &addr);
	
	/// Convert a libc hostentry to a value object.
	/// \return New value object with result.
	static value *converthostentry (struct hostent *entry);
};

#endif
