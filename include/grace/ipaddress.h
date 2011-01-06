// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _IPADDRESS_H
#define _IPADDRESS_H 1

#include "str.h"

/// Representation of an IPv6 address.
class ipaddress
{
public:
	static bool			str2ip( const char*, unsigned char* );
	static bool			str2ip( const char* c, ipaddress& i ) { return str2ip(c, i.addr); }
	static string*		ip2str( const unsigned char* c );	
	static string*		ip2str( const ipaddress& c ) { return ip2str (c.addr); }	

public:
						/// Default constructor. Sets the value
						/// to ::
						ipaddress (void)
						{
							memset( addr, 0, sizeof(addr) );
						}
	
						/// Copy-constructor.
						ipaddress (const ipaddress &o)
						{
						 	memcpy(addr, o.addr, sizeof(addr));
						}
						
						/// Constructor from a string. 
						ipaddress (const char *o);

						/// Constructor from system structs
						ipaddress (const struct in_addr&);
						ipaddress (const struct in6_addr&);
						
						ipaddress (const string& str);

						/// Constructor from a value.
						/// Just calls ipval().
						ipaddress (const value &o);

						 /// Boring destructor.
						~ipaddress (void)
						 {
						 }

						 /// Assignment operator.
						 /// \param o The original.
	ipaddress			&operator= (const ipaddress &o)
						 {
						 	memcpy(addr, o.addr, sizeof(addr));
						 	return *this;
						 }
						 
	ipaddress			&operator= (const value &v);
						 
	ipaddress			&operator= (const char *c);
	ipaddress			&operator= (const string &);


						 operator bool () const
						 {
						 	for (int i=0; i<sizeof(addr); i++)
						 	{
						 		if( addr[i] ) return true;
						 	}
						 	return false;
						 }						 

						 operator bool ()
						 {
						 	for (int i=0; i<sizeof(addr); i++)
						 	{
						 		if( addr[i] ) return true;
						 	}
						 	return false;
						 }						 
						 
						 /// Cast-o-matic operator to unsigned int.
						 operator const struct in_addr& (void) const;
						 operator const struct in6_addr& (void) const;

	bool				 operator== (const ipaddress &o) const
						 {
						 	return memcmp(addr,o.addr,sizeof(addr)) == 0;
						 }
	
	bool				 operator!= (const ipaddress &o) const
						 {
						 	return !( this->operator==(o) );
						 }

	bool				 operator== (const value &o) const;
						 
	bool				 operator!= (const value &o) const
						 {
						 	return !( this->operator==(o) );
						 }
						 
						 
	bool				 isv4() const
						 {
						 	return 
						 		addr[ 0] == 0x00 && addr[ 1] == 0x00 && 
						 		addr[ 2] == 0x00 && addr[ 3] == 0x00 && 
						 		addr[ 4] == 0x00 && addr[ 5] == 0x00 && 
						 		addr[ 6] == 0x00 && addr[ 7] == 0x00 && 
						 		addr[ 8] == 0x00 && addr[ 9] == 0x00 && 
						 		addr[10] == 0xFF && addr[11] == 0xFF; 
						 }						 
protected:
	unsigned char	 	 addr[16]; // The IPv6 address in network order
};

#endif
