// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

//      012345678 TAB-O-METER should measure 4
//      ^	^

#include <grace/ipaddress.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

// ==========================================================================
// METHOD ipaddress::str2ip
// ==========================================================================
bool ipaddress::str2ip (const char* ipstr, unsigned char* out_result)
{
	if (strchr( ipstr, ':') == NULL )
	{
		// no ':' found, this can't be an IPv6 address
		char bytes[4];
		
		if (inet_pton(AF_INET, ipstr, bytes) == 1)
		{
			memset(out_result,0,10);
	
			out_result[10] = 0xFF;
			out_result[11] = 0xFF;
			// bytes are in network order.
			out_result[12] = bytes[0];
			out_result[13] = bytes[1];
			out_result[14] = bytes[2];
			out_result[15] = bytes[3];
			
			return true;
		}
	}
	else if (inet_pton(AF_INET6, ipstr, out_result) == 1) // success!
	{
		return true;
	}
	
	return false;
}

// ==========================================================================
// METHOD ipaddress::ip2str
// ==========================================================================
bool ipaddress::ip2str (const unsigned char* c, string& into)
{
	ipaddress* ipaddr = (ipaddress*)(void*)c;
	
	if( ipaddr->isv4() )
	{
		into.printf ("%i.%i.%i.%i", 
			c[12], c[13], c[14], c[15] );
	}
	else
	{
		char addr_str[INET6_ADDRSTRLEN];
		if( inet_ntop( AF_INET6, c, addr_str, sizeof(addr_str) ) != NULL )
		{
			into.strcat( addr_str );
		}
		else
		{
		    return false;
        }
		
	}
	return true;
}


// ==========================================================================
// CONSTRUCTOR ipaddress
// ==========================================================================
ipaddress::ipaddress (const value &o)
{
	*this = o.ipval ();
}

/// Constructor from a value.
/// Just calls ipval().
ipaddress::ipaddress (const string &s)
{
	str2ip( s, addr );
}

/// Constructor from a value.
/// Just calls ipval().
ipaddress::ipaddress (const char* s)
{
	if(s)
		str2ip( s, addr );
	else
		memset( addr, 0, sizeof(addr) );
}


/// Constructor from system structs
ipaddress::ipaddress (const struct in_addr& address)
{
    *this=address;
	
}

ipaddress::ipaddress (const struct in6_addr& address)
{
    *this=address;
}


// ==========================================================================
// METHOD ipaddress::operator=
// ==========================================================================
ipaddress& ipaddress::operator= (const struct in_addr& address)
{
    memset(addr,0,10);
	
	addr[10] = 0xFF;
	addr[11] = 0xFF;
	
	
	const unsigned char* bytes = (const unsigned char*)&(address.s_addr);
	// bytes are in network order.
	addr[12] = bytes[0];
	addr[13] = bytes[1];
	addr[14] = bytes[2];
	addr[15] = bytes[3];
	return *this;
}

ipaddress& ipaddress::operator= (const struct in6_addr& address)
{
	memcpy( addr, address.s6_addr, 16 );
	return *this;
}

ipaddress &ipaddress::operator= (const string &s)
{
	if(s)
		str2ip( s, addr );
	else
		memset( addr, 0, sizeof(addr) );

	return *this;
}

ipaddress &ipaddress::operator= (const char *s)
{
	if(s)
		str2ip( s, addr );
	else
		memset( addr, 0, sizeof(addr) );

	return *this;
}

ipaddress& ipaddress::operator= (const value &v)
{
    *this = v.ipval();
    return *this;
}

// ==========================================================================
// METHOD ipaddress::operator const struct in_addr
// ==========================================================================
ipaddress::operator const struct in_addr (void) const
{
	in_addr result;
	memcpy( &result, addr+12, sizeof(result) );
	return result;
}

// ==========================================================================
// METHOD ipaddress::operator const struct in6_addr
// ==========================================================================
ipaddress::operator const struct in6_addr (void) const
{
	in6_addr result;
	memcpy( &result, addr, sizeof(result) );
	return result;
}


// ==========================================================================
// METHOD ipaddress::operator==
// ==========================================================================
bool ipaddress::operator== (const value &o) const
{
    return o.ipval() == *this;
}


// ==========================================================================
// METHOD ipaddress::operator&=
// ==========================================================================
ipaddress& ipaddress::operator&=(const ipaddress& other)
{
    for( size_t i=0; i<sizeof(addr); i+= sizeof(unsigned int) )
    {
        unsigned int* a = (unsigned int*)&addr[i];
        const unsigned int* b = (const unsigned int*)&other.addr[i];
        *a &= *b;
    }
    
    return *this;
}

// ==========================================================================
// METHOD ipaddress::operator!=
// ==========================================================================
ipaddress& ipaddress::operator|=(const ipaddress& other)
{
    for( size_t i=0; i<sizeof(addr); i+= sizeof(unsigned int) )
    {
        unsigned int* a = (unsigned int*)&addr[i];
        const unsigned int* b = (const unsigned int*)&other.addr[i];
        *a |= *b;
    }
    return *this;
}


// ==========================================================================
// METHOD ipaddress::toblob
// ==========================================================================
string *ipaddress::toblob (void) const
{
	returnclass (string) res retain;
	if (isv4())
	{
		res.strcpy ((const char *) addr+12, 4);
	}
	else
	{
		res.strcpy ((const char *) addr, 16);
	}
	return &res;
}

// ==========================================================================
// METHOD ipaddress::fromblob
// ==========================================================================
void ipaddress::fromblob (const string &blob)
{
	if (blob.strlen() == 4)
	{
		memset(addr,0,10);
		addr[10] = 0xFF;
		addr[11] = 0xFF;
		memcpy (addr+12, blob.str(), 4);
	}
	else if (blob.strlen() == 16)
	{
		memcpy (addr, blob.str(), 16);
	}
	else
	{
		memset (addr, 0, sizeof(addr));
	}
}
	

