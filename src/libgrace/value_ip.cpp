// ========================================================================
// value_ip.cpp: Keyed generic data storage class
//
// (C) Copyright 2003-2006 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam 
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

// ========================================================================
// FUNCTION ip2str
// ========================================================================
string *ip2str (unsigned int ipaddr)
{
	string *s = new string;
	
	s->printf ("%i.%i.%i.%i",
			  (ipaddr & 0xff000000) >> 24,
			  (ipaddr & 0x00ff0000) >> 16,
			  (ipaddr & 0x0000ff00) >> 8,
			  (ipaddr & 0x000000ff));
			  
	return s;
}

// ========================================================================
// FUNCTION str2ip
// ========================================================================
unsigned int str2ip (const string &str)
{
	value spl;
	unsigned int ret;
	
	ret = 0;
	spl = strutil::split (str, '.');
	
	if (spl.count() != 4) return 0;
	
	unsigned int one = spl[0].uval();
	unsigned int two = spl[1].uval();
	unsigned int three = spl[2].uval();
	unsigned int four = spl[3].uval();
	
	if ( (one>255) || (two>255) || (three>255) || (four>255) )
	{
		return 0;
	}
	
	return (one << 24) | (two << 16) | (three << 8) | four;
}

// ========================================================================
// METHOD ::ipval
// --------------
// Returns the value as an unsigned 32 bit integer representing an ip-
// address in little-endian format.
// ========================================================================
unsigned int value::ipval (void) const
{
	if (itype != i_string) return this->uval();
	value t;
	t = this->sval();
	return t.ipval();
}

unsigned int value::ipval (void)
{
	if (itype == i_string)
	{
		value spl;
		spl = strutil::split (s, '.');
		if (spl.count() == 4)
		{
			t.uval = ((spl[0].ival() & 0xff) << 24) |
					 ((spl[1].ival() & 0xff) << 16) |
					 ((spl[2].ival() & 0xff) << 8) |
					 (spl[3].ival() & 0xff);
			
			return t.uval;
		}
	}
	return t.uval;
}

// ========================================================================
// METHOD ::setip
// --------------
// Set up the value as an ip address/unsigned int
// ========================================================================
value &value::setip (unsigned int addr)
{
	cleararray ();
	itype = i_ipaddr;
	if (_type == t_unset)
		_type = t_ipaddr;
	t.uval = addr;
	return *this;
}

value &value::operator= (const ipaddress &o)
{
	setip (o);
}

bool value::operator== (const ipaddress &o)
{
	return o == (*this);
}

bool value::operator!= (const ipaddress &o)
{
	return o != (*this);
}
