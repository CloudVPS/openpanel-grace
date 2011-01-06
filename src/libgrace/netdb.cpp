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
	returnclass (value) res retain;
	struct addrinfo *ainf;
	
	res["name"] = name;
	
	if (getaddrinfo (name.cval(), NULL, NULL, &ainf))
	{
		return &res;
	}
	
	struct addrinfo *crsr = ainf;
	for (;crsr;crsr = crsr->ai_next)
	{
		if (crsr->ai_family != AF_INET && crsr->ai_family != AF_INET6) continue;
		if (crsr->ai_socktype && (crsr->ai_socktype != SOCK_STREAM)) continue;
		
		if (crsr->ai_addr)
		{
			if (crsr->ai_addr->sa_family == AF_INET)
			{
				struct sockaddr_in *sa_in = (struct sockaddr_in *) crsr->ai_addr;
				res["address"].newval() = ipaddress(sa_in->sin_addr);
			}
			else if (crsr->ai_addr->sa_family == AF_INET6)
			{
				struct sockaddr_in6 *sa_in = (struct sockaddr_in6 *) crsr->ai_addr;
				res["address"].newval() =  ipaddress(sa_in->sin6_addr);
			}
		}
	}
	
	freeaddrinfo (ainf);
	if (! res.exists ("address"))
	{
		ipaddress i (name);
		if (i) res["address"].newval() = i;
	}
	return &res;
}

// ========================================================================
// METHOD ::resolve
// ========================================================================
ipaddress netdb::resolve (const string &name, bool v4, bool v6)
{
	struct addrinfo *ainf;
	
	if (getaddrinfo (name.cval(), NULL, NULL, &ainf))
	{
		return 0;
	}
	
	struct addrinfo *crsr = ainf;
	for (;crsr;crsr = crsr->ai_next)
	{
		if (crsr->ai_family == AF_INET && !v4) continue;
		if (crsr->ai_family == AF_INET6 && !v6) continue;
		if (crsr->ai_family != AF_INET && crsr->ai_family != AF_INET6) continue;
		if (crsr->ai_socktype && (crsr->ai_socktype != SOCK_STREAM)) continue;
		
		if (crsr->ai_addr)
		{
			if (crsr->ai_addr->sa_family == AF_INET)
			{
				struct sockaddr_in *sa_in = (struct sockaddr_in *) crsr->ai_addr;
				ipaddress result = ipaddress(sa_in->sin_addr);
				freeaddrinfo (ainf); 
				return result;
			}
			else if (crsr->ai_addr->sa_family == AF_INET6)
			{
				struct sockaddr_in6 *sa_in = (struct sockaddr_in6 *) crsr->ai_addr;
				ipaddress result = ipaddress( sa_in->sin6_addr );
				freeaddrinfo (ainf); 
				return result;
			}
		}
	}
	
	freeaddrinfo (ainf);

	ipaddress li (name);
	if (li) return li;
	return 0;
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
	struct hostent *res = NULL;
	struct in_addr ina;
	char he_data[2560];
	int retval = 0;
	
	if (!inet_aton (addr.str(), &ina))
	{
		throw badAddressException();
	}
	
	if (gethostbyaddr_r ((char *) &ina, sizeof (struct in_addr),
						 AF_INET, &he, he_data, 2560, &res, &retval));
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
		for (int i=0; he->h_addr_list[i]; ++i)
		{
			struct in_addr *sin = ((struct in_addr *)he->h_addr_list[i]);
			res["address"].newval() = ipaddress( *sin ) ;
		}
	}
	else if (he->h_addrtype == AF_INET6) // h_addr == h_addr_list[0];
	{
		for (int i=0; he->h_addr_list[i]; ++i)
		{
			struct in6_addr *sin = ((struct in6_addr *)he->h_addr_list[i]);
			res["address"].newval() = ipaddress( *sin ) ;
		}
	}
	
	return &res;
}
