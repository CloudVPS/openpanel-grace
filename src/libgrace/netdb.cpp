#include <netdb.h>
#include <grace/netdb.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#ifndef gethostbyname_r
 #define gethostbyname_r gethostbyname
 #define gethostbyaddr_r gethostbyaddr
#endif

// =======================================================================
// METHOD gethostbyname
// --------------------
// Looks up a host by its hostname and returns a value structure with
// the following contents:
//
// <grace.netdb.host>
//   <host.name><hostname>foo.bar</hostname></host.name>
//   <host.address>
//     <address>10.1.1.5</address>
//     <address>10.1.1.6</address>
//   </host.address>
// </grace.netdb.host>
//
// Resolving a simple hostname to an arbitrary matching address is as
// simple as this:
//
// {
//     value res;
//     string ipaddr;
//     res = netdb::gethostbyname ("foo.bar");
//     ipaddr = res["address"];
// }
//
// The construction of the value class makes this return the first entry
// of the address array.
// =======================================================================
value *netdb::gethostbyname (const string &name)
{
	struct hostent *he;
	
	he = ::gethostbyname_r (name.str());
	if (! he)
	{
		returnclass (value) result retain;
		result = false;
		return &result;
	}
	return netdb::converthostentry (he);
}

value *netdb::gethostbyaddr (const string &addr)
{
	struct hostent *he;
	struct in_addr ina;
	
	if (!inet_aton (addr.str(), &ina))
	{
		throw (badAddressException());
	}
	
	he = ::gethostbyaddr_r ((char *) &ina, sizeof (struct in_addr),
							AF_INET);
	if (! he)
	{
		returnclass (value) result retain;
		result = false;
		return &result;
	}
	return netdb::converthostentry (he);
}

value *netdb::converthostentry (struct hostent *he)
{
	returnclass (value) res retain;
	
	if (he->h_name)
	{
		res["name"].newval() = he->h_name;
	}
	else
	{
		res["name"].newval() = "";
	}
	
	if (he->h_aliases)
	{
		for (int i=0; he->h_aliases[i]; ++i)
		{
			res["name"].newval() = he->h_aliases[i];
		}
	}
	
	if (he->h_addrtype == AF_INET) // h_addr == h_addr_list[0];
	{
		struct in_addr *sin;
		for (int i=0; he->h_addr_list[i]; ++i)
		{
			sin = ((struct in_addr *)he->h_addr_list[i]);
			res["address"].newval().setip (sin->s_addr);
		}
	}
	
	return &res;
}
