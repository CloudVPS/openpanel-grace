// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

// ========================================================================
// value_ip.cpp: Keyed generic data storage class
// ========================================================================

//      012345678 TAB-O-METER should measure 4
//      ^	^


#include <grace/value.h>
#include <grace/file.h>
#include <grace/stack.h>
#include <grace/strutil.h>
#include <grace/filesystem.h>
#include <grace/ipaddress.h>

#include <stdio.h>
#include <string.h>
#include <netinet/in.h>

// ========================================================================
// METHOD ::ipval
// --------------
// ========================================================================
ipaddress value::ipval (void) const
{
	if (arraysz) return array[0]->ipval();

	
	if (_type == t_ipaddr)
	{
		return ipaddress( (const in6_addr*)t.ipval );
    }
    else if (_type == t_string )
	{
		ipaddress result;
		const string& ip_str = sval();
		ipaddress::str2ip( ip_str, result );
		return result;
	}	
	
	return ipaddress();
}

value &value::operator= (const ipaddress &o)
{
	cleararray ();
	_itype = i_ipaddr;
	_type = t_ipaddr;
	
	memcpy( t.ipval, &(const in6_addr&)o, 16 );

	return *this;
}

bool value::operator== (const ipaddress &o)
{
	return o == this->ipval();
}

bool value::operator!= (const ipaddress &o)
{
	return o != this->ipval();
}
