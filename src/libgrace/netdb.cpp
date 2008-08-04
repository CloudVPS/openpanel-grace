#include <netdb.h>
#include <grace/netdb.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

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
#ifndef HAVE_GETHOSTBYNAME_R
	struct hostent *he;
	
	he = ::gethostbyname (name.str());
	if (! he)
	{
		returnclass (value) result retain;
		result = false;
		return &result;
	}
	return netdb::converthostentry (he);
#else
	struct hostent he;
	struct hostent *result = NULL;
	char addr_space[2560];
	int err;
	
	if (gethostbyname_r (name.str(), &he, addr_space, 2560, &result, &err))
	{
		returnclass (value) result retain;
		result = false;
		return &result;
	}
	return netdb::converthostentry (result);
#endif
}

// ========================================================================
// METHOD ::resolve
// ========================================================================
ipaddress netdb::resolve (const string &name)
{
#ifndef HAVE_GETHOSTBYNAME_R
	ipaddress result = 0;
	struct hostent *he;
	
	he = ::gethostbyname (name.str());
	if (! he) return result;
	if (he->h_addrtype == AF_INET) // h_addr == h_addr_list[0];
	{
		struct in_addr *sin;
		sin = ((struct in_addr *)he->h_addr_list[0]);
		result = ntohl (sin->s_addr);
	}
	
	return result;
#else
	struct hostent he;
	struct hostent *result = NULL;
	char addr_space[2560];
	int err;
	ipaddress resval = 0;
	
	if (gethostbyname_r (name.str(), &he, addr_space, 2560, &result, &err))
	{
		return resval;
	}
	if (! result) return resval;
	if (result->h_addrtype == AF_INET) 
	{
		struct in_addr *sin;
		sin = ((struct in_addr *) result->h_addr_list[0]);
		resval = ntohl (sin->s_addr);
	}
	return resval;
#endif
}

// ========================================================================
// METHOD ::gethostbyaddr
// ========================================================================
value *netdb::gethostbyaddr (const string &addr)
{
#ifndef HAVE_GETHOSTBYNAME_R
	struct hostent *he;
	struct in_addr ina;
	
	if (!inet_aton (addr.str(), &ina))
	{
		throw badAddressException();
	}
	
	he = ::gethostbyaddr ((char *) &ina, sizeof (struct in_addr),
							AF_INET);
	if (! he)
	{
		returnclass (value) result retain;
		result = false;
		return &result;
	}
	return netdb::converthostentry (he);
#else
	struct hostent he;
	struct in_addr ina;
	struct hostent_data he_data;
	
	if (!inet_aton (addr.str(), &ina))
	{
		throw badAddressException();
	}
	
	if (gethostbyaddr_r ((char *) &ina, sizeof (struct in_addr),
						 AF_INET, &he, &he_data));
	{
		returnclass (value) result retain;
		result = false;
		return &result;
	}
	
	return netdb::converthostentry (&he);
#endif
}

// ========================================================================
// METHOD ::converthostentry
// ========================================================================
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
