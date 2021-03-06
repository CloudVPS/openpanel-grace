// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

// ========================================================================
// value.cpp: Keyed generic data storage class
// ========================================================================

//      012345678 TAB-O-METER should measure 4
//      ^	^

#define _VALUE_CPP 1

#include "platform.h"
#include <grace/value.h>
#include <grace/file.h>
#include <grace/stack.h>
#include <grace/strutil.h>
#include <grace/filesystem.h>
#include <grace/valuable.h>
#include <grace/ipaddress.h>

#include <stdio.h>
#include <string.h>

extern threadref_t getref (void);

value *$ (const statstring &id, const value &v)
{
	returnclass (value) res retain;
	res[id] = v;
	return &res;
}

value *$merge (const value &v)
{
	returnclass (value) res retain;
	res = v;
	return &res;
}

value *$ (const value &v)
{
	returnclass (value) res retain;
	res.newval() = v;
	return &res;
}

value *$attr (const statstring &id, const value &v)
{
	returnclass (value) res retain;
	res(id) = v;
	return &res;
}

value *$type (const statstring &t)
{
	returnclass (value) res retain;
	res.type (t);
	return &res;
}

value *$val (const value &v)
{
	returnclass (value) res retain;
	res = v;
	return &res;
}

// ========================================================================
// CONSTRUCTOR
// -----------
// This constructor initializes an empty value object
// ========================================================================
value::value (void)
{
	init (true);
}

// ========================================================================
// CONSTRUCTOR (char)
// -----------
// This constructor initializes a value object with string data.
// ========================================================================
value::value (const char *orig)
{
	init (true);
	(*this) = orig;
}

// ========================================================================
// CONSTRUCTOR (double)
// --------------------
// This constructor initializes a value object with a float value.
// ========================================================================
value::value (double orig)
{
	init (true);
	(*this) = orig;
}

// ========================================================================
// CONSTRUCTOR (key)
// -----------------
// This constructor initializes a value object with an integer key
// ========================================================================
value::value (creatorlabel l, unsigned int k)
{
	_type = t_unset;
	_itype = i_unset;
	t.lval = 0;
	key = k;
	lower = higher = NULL;
	array = NULL;
	arraysz = 0;
	arrayalloc = 0;
	ucount = 0;
	attrib = NULL;
	threadref = getref();
}

// ========================================================================
// CONSTRUCTOR (c-string)
// ----------------------
// This constructor initializes value object with a c-string key
// ========================================================================
value::value (creatorlabel l,const char *k)
{
	_type = t_unset;
	t.lval = 0;
	key = checksum (k);
	_name = k;
	lower = higher = NULL;
	array = NULL;
	ucount = 0;
	arraysz = 0;
	arrayalloc = 0;
	attrib = NULL;
	threadref = getref();
}

// ========================================================================
// CONSTRUCTOR (c-string + uint key)
// ---------------------------------
// This constructor initializes value object with a c-string key and a
// defined numeric key.
// ========================================================================
value::value (creatorlabel l, const char *k, unsigned int ki)
{
	_type = t_unset;
	_itype = i_unset;
	
	t.lval = 0;
	key = ki;
	_name = k;
	lower = higher = NULL;
	array = NULL;
	ucount = 0;
	arraysz = 0;
	arrayalloc = 0;
	attrib = NULL;
	threadref = getref();
}

// ========================================================================
// CONSTRUCTOR (string)
// --------------------
// This constructor initializes value object with a string key
// ========================================================================
value::value (creatorlabel l, const string &k)
{
	_type = t_unset;
	_itype = i_unset;
	t.lval = 0;
	
	// Calculate the hash value
	key = checksum (k.str());
	
	// Copy the _name element to te C string (a string object would be bloat here)
	_name = k;
		
	// Linking is done from the parent
	lower = higher = NULL;
	array = NULL;
	arraysz = 0;
	arrayalloc = 0;
	ucount = 0;
	attrib = NULL;
	threadref = getref();
}

// ========================================================================
// CONSTRUCTOR (ipaddress)
// ========================================================================
value::value (const ipaddress &o)
{
	init ();
	operator=(o);
}

// ========================================================================
// CONSTRUCTOR (timestamp)
// ========================================================================
value::value (const timestamp &o)
{
	init ();
	settime (o);
}

// ========================================================================
// COPY CONSTRUCTOR
// ----------------
// Initializes a value as a copy of another value
// ========================================================================
value::value (value &v)
{
	// First do a default null-initialization
	init ();
	
	// Copy the name
	if (v._name)
	{
		_name = v._name;
	}
	
	key = v.key;

	// Copy the value's data, depending on type

	switch (v._itype)
	{
		case i_unsigned:
		case i_date:
		case i_int:
		case i_bool:
		case i_ipaddr:
			_itype = v._itype;
			t = v.t;
			break;
		
		case i_string:
			_itype = v._itype;
			s = v.s;
			break;
		
		case i_long:
		case i_ulong:
		case i_currency:
			_itype = v._itype;
			t.ulval = v.t.ulval;
			break;
			
		case i_double:
			_itype = i_double;
			t.dval = v.t.dval;
			break;
			
		default:
			_itype = i_unset;
			break;
	}
	
	_type = v.type();
	
	if (v.attrib)
	{
		//if (attrib) delete attrib;
		attrib = new value;
		(*attrib) = (*(v.attrib));
	}
	
	// Iterate through the children and make copies as our own
	
	for (unsigned int i=0; i<v.arraysz; ++i)
	{
		if (v.array[i])
		{
			if (v.array[i]->_name)
				*(findchild (v.array[i]->key, v.array[i]->_name.str())) = *(v.array[i]);
			else
				newval() = *(v.array[i]);
		}
	}
}

value::value (const value &v)
{
	_type = t_unset;
	_itype = i_unset;
	
	t.lval = 0;
	key = 0;
	lower = higher = NULL;
	array = NULL;
	arraysz = 0;
	arrayalloc = 0;
	ucount = 0;
	attrib = NULL;
	threadref = getref();
	
	(*this) = v;
}

// ========================================================================
// COPY CONSTRUCTOR
// ----------------
// Initializes a value as a copy of another value, destroying the
// original.
// ========================================================================
value::value (value *v)
{
	init (true);
	(*this) = v;
}

// ========================================================================
// COPY CONSTRUCTOR
// ========================================================================
value::value (const valuable &v)
{
	// First do a default null-initialization
	init ();
	v.tovalue (*this);
}

// ========================================================================
// COPY CONSTRUCTOR
// ========================================================================
value::value (valuable *v)
{
	init ();
	v->tovalue (*this);
	delete v;
}

// ========================================================================
// COPY CONSTRUCTOR
// ========================================================================
value::value (const string &str)
{
	init ();
	s = str;
	_itype = i_string;
	_type= t_string;
}

// ========================================================================
// COPY CONSTRUCTOR
// ========================================================================
value::value (const statstring &str)
{
	init ();
	s = str;
	_itype = i_string;
	_type = t_string;
}

// ========================================================================
// COPY CONSTRUCTOR
// ========================================================================
value::value (string *str)
{
	init ();
	s = str;
	_itype = i_string;
	_type = t_string;
}

// ========================================================================
// COPY CONSTRUCTOR
// ========================================================================
value::value (int orig)
{
	init ();
	t.ival = orig;
	_itype = i_int;
	_type = t_int;
}

value::value (unsigned int orig)
{
	init ();
	t.uval = orig;
	_itype = i_unsigned;
	_type = t_unsigned;
}

value::value (long long orig)
{
	init ();
	t.lval = orig;
	_itype = i_long;
	_type = t_long;
}

value::value (unsigned long long orig)
{
	init ();
	t.ulval = orig;
	_itype = i_ulong;
	_type = t_ulong;
}

value::value (bool b)
{
	init ();
	t.ival = b ? 1 : 0;
	_itype = i_bool;
	_type = t_bool;
}

// ========================================================================
// DESTRUCTOR
// ----------
// Releases claimed memory
// ========================================================================
value::~value (void)
{
	if (arraysz) // Infanticide
	{
		for (unsigned int i=0; i<arraysz; ++i)
		{
			if (array[i]) delete array[i];
			array[i] = NULL;
		}
		::free (array);
		array = NULL;
		arraysz = 0;
		arrayalloc = 0;
		ucount = 0;
	}
	
	if (attrib) delete attrib;
	// In death, we do not need a name.
}

// ========================================================================
// METHOD ::operator=
// ========================================================================
value &value::operator= (valuable &other)
{
	cleararray ();
	other.tovalue (*this);
	return *this;
}

value &value::operator= (valuable *other)
{
	cleararray ();
	other->tovalue (*this);
	delete other;
	return *this;
}

value &value::settime (const timestamp &o)
{
	cleararray ();
	_itype = i_date;
	t.uval = o.unixtime();
	return *this;
}

value &value::operator= (const timestamp &o)
{
	cleararray ();
	_type = t_date;
	return settime (o);
}

value &value::operator= (int i)
{
	cleararray ();
	t.ival = i;
	_itype = i_int;
	if (_type == t_unset || isbuiltin (_type)) _type  = t_int;
	return *this;
}

value &value::operator= (unsigned int i)
{
	cleararray ();
	t.uval = i;
	_itype = i_unsigned;
	if (_type == t_unset || isbuiltin (_type)) _type = t_unsigned;
	return *this;
}

value &value::operator= (double d)
{
	cleararray ();
	t.dval = d;
	_itype = i_double;
	if (_type == t_unset || isbuiltin (_type)) _type = t_double;
	return *this;
}

value &value::operator= (bool bval)
{
	cleararray ();
	t.ival = bval ? 1 : 0;
	_itype = i_bool;
	if (_type == t_unset || isbuiltin (_type)) _type = t_bool;
	return *this;
}

value &value::operator= (long long dval)
{
	cleararray ();
	t.lval = dval;
	_itype = i_long;
	if (_type == t_unset || isbuiltin (_type)) _type = t_long;
	return *this;
}

value &value::operator= (const currency &c)
{
	assign (c);
	return *this;
}

value &value::operator= (currency &c)
{
	assign (c);
	return *this;
}

value &value::operator= (unsigned long long val)
{
	cleararray ();
	t.ulval = val;
	_itype = i_ulong;
	if (_type == t_unset || isbuiltin (_type)) _type = t_ulong;
	return *this;
}

value &value::operator= (const char *str)
{
	cleararray ();
	s = str;
	_itype = i_string;
	if (_type == t_unset || isbuiltin (_type)) _type = t_string;
	return *this;
}

value &value::operator= (const unsigned char *str)
{
	cleararray ();
	s = str;
	_itype = i_string;
	if (_type == t_unset || isbuiltin (_type)) _type = t_string;
	return *this;
}

value &value::operator= (const string &str)
{
	cleararray ();
	s = str;
	_itype = i_string;
	if (_type == t_unset || isbuiltin (_type)) _type = t_string;
	return *this;
}

value &value::operator= (string *str)
{
	cleararray();
	s = str;
	_itype = i_string;
	if (_type == t_unset || isbuiltin (_type)) _type = t_string;
	return *this;
}

value &value::operator= (const statstring &str)
{
	cleararray ();
	s = str.sval();
	_itype = i_string;
	if (_type == t_unset || isbuiltin (_type)) _type = t_string;
	return *this;
}

value &value::operator= (statstring *str)
{
	cleararray ();
	s = str->sval();
	_itype = i_string;
	if (_type == t_unset || isbuiltin (_type)) _type= t_string;
	delete str;
	return *this;
}

// ========================================================================
// METHOD ::operator=
// ------------------
// Copy another value into this one
// ========================================================================
value &value::operator= (const value &v)
{
	if (this == &v) return *this;
	clear();
	ucount = 0;	

	// Prefer to keep the original object's type.
	
	_type = v.type();
	_itype = v._itype;
	
	switch (v._itype)
	{
		case i_string: s.strclone (v.s); break;
		case i_ipaddr:
		case i_bool:
		case i_unsigned:
		case i_date:
		case i_currency:
		case i_ulong:
		case i_double:
		case i_long:
		case i_int: t = v.t; break;
	}
	
	// Now clone the children if needed
	
	if (attrib)
	{
		delete attrib;
		attrib = NULL;
	}
	
	if (v.count()) // has children
	{
		for (int i=0; i<v.count(); ++i)
		{
			if (v[i]._name)
			{
				(*this)[v[i]._name] = v[i];
			}
			else
			{
				newval() = v[i];
			}
		}
		if (arraysz != (unsigned int) v.count())
		{
			throw valueCountMismmatchException();
		}
	}
	if (v.attrib)
	{
		attrib = new value;
		(*attrib) = ((const value &) *(v.attrib));
	}
	return *this;
}

// ========================================================================
// METHOD ::operator=
// ------------------
// Copy another value into this one
// ========================================================================
value &value::operator= (value *v)
{
	if (v && v->threadref != threadref)
	{
		(*this) = (*v);
		delete v;
		return *this;
	}
	
	clear();
	
	// Prefer to keep the original type
	
	if (v != NULL)
	{
		switch (v->_itype)
		{
			case i_ipaddr: t = v->t; break;
			case i_string: s = v->s; break;
			case i_bool:
			case i_unsigned:
			case i_int:
			case i_date:
			case i_double: t.dval = v->t.dval; break;
			case i_long:
			case i_ulong: t.ulval = v->t.ulval; break;
		}
		_itype = v->_itype;
		_type = v->type();
	}
	else
	{
		_type = t_unset;
	}
	
	// FIXME: Shouldn't this be handled by clear()?
	
	if (arraysz>0)
	{
		for (unsigned int i=0; i<arraysz; ++i)
		{
			if (array[i]) delete array[i];
		}
		::free (array);
		array = NULL;
		arrayalloc = 0;
		arraysz = 0;
		ucount = 0;
	}
	if (attrib)
	{
		delete attrib;
		attrib = NULL;
	}
	ucount = 0;
	
	// Nick the children, we're gonna shoot the original anyway
	
	if ((v != NULL) && (v->arraysz)) // has children
	{
		array = v->array;
		arraysz = v->arraysz;
		arrayalloc = v->arrayalloc;
		ucount = v->ucount;
		v->arraysz = 0;
		v->ucount = 0;
		v->arrayalloc = 0;
		v->array = NULL;
	}
	
	if ((v != NULL) && (v->attrib))
	{
		attrib = v->attrib;
		v->attrib = NULL;
	}
	
	// Get rid of the original
	
	delete v;
	
	return *this;
}

// ========================================================================
// METHOD ::sval (dynamic versions)
// ------------------------------------------
// Returns the value as a string reference.
// ========================================================================
const string &value::sval (void) const
{
	// If we're an array, return the string cast of our first child	
	if (arraysz) return array[0]->sval();
	
	// Ok we're a boring primary type. If we're not a string, use
	// our embedded string object to store the converted results. This
	// will not affect the 'true' type. Note that this
	// call technically violates its const-contract by manipulating the 
	// 's' member to contain a string-representation of the value, if
	// the _itype is not i_string.
	//
	// This string representation is an internal affair of the class and
	// its side-effects  do not harm the principal constness, the
	// code 'owning' a value-object and passing it as const to another
	// function does not find the object in a functionally altered state.
    string &S = (string &) s;

	switch (_itype)
	{
		case i_string:
			return s;

		case i_ipaddr:
		    S.crop(); 
		    ipaddress::ip2str( t.ipval, S );
		    return s;

		case i_bool:
			S = t.ival ? "true" : "false";
			return s;
			
		case i_int:
			S.crop(); S.printf ("%i", t.ival);
			return s;
			
		case i_unsigned:
			S.crop(); S.printf ("%u", t.uval);
			return s;
			
		case i_date:
			S = __make_timestr (t.uval);
			return s;
		
		case i_double:
			S.crop(); S.printf ("%f", t.dval);
			return s;

		case i_currency:
			S.crop();
			printcurrency (S, t.lval);
			return s;

		case i_long:
			S.crop(); S.printf ("%L", t.lval);
			return s;
			
		case i_ulong:
			S.crop(); S.printf ("%U", t.ulval);
			return s;
	}	
		
	// Unknown datatype, treat as an empty string
	
	S = "";
	return s;
}

// ========================================================================
// METHOD ::cval
// -------------
// Returns the value as a c-string reference
// ========================================================================
const char *value::cval (void) const
{
	if (arraysz) return array[0]->sval().str();
	return sval().str();
}


// ========================================================================
// METHOD ::ival
// -------------
// Returns the value as an integer
// ========================================================================
int value::ival (void) const
{
	if (arraysz) return ((*this)[0].ival());
	switch (_itype)
	{
		case i_int:
		case i_bool:
		case i_date:
		case i_unsigned:
			return t.ival;

		case i_string:
			return ::atoi (s.str());

		case i_double:
			return (int) t.dval;
			
		case i_currency:
			return ((t.lval/1000) & 0x00000000ffffffff);
			
		case i_long:
		case i_ulong:
			return (t.lval & 0x00000000ffffffff);
	}
	return 0;
}

// ========================================================================
// METHOD ::uval
// ========================================================================
unsigned int value::uval (void) const
{
	if (arraysz) return ((*this)[0].uval());
	switch (_itype)
	{
		case i_int:
		case i_bool:
		case i_date:
		case i_unsigned:
			return t.uval;
		    
		case i_string:
			return ::strtoul (s.str(),NULL,10);

		case i_double:
			return (unsigned int) t.dval;
		
		case i_currency:
			return ((t.ulval/1000) & 0x00000000ffffffff);
			
		case i_long:
		case i_ulong:
			return (unsigned int) (t.ulval & 0x00000000ffffffff);
	}
	return 0;
}

// ========================================================================
// METHOD ::dval
// -------------
// Returns the value as a double precision floating point number
// ========================================================================
double value::dval (void) const
{
	if (arraysz) return ((*this)[0].dval());
	switch (_itype)
	{
		case i_double: return t.dval;
		case i_string: return ::atof (s.str());
		case i_int: return (double) t.ival;
		case i_unsigned: return (double) t.uval;
		case i_bool: return t.ival ? 1.0 : 0.0;
		case i_long: return (double) t.lval;
		case i_ulong: return (double) t.ulval;
		case i_currency: return ((double) t.lval) / 1000.0;
	}
	return 0.0;
}

// ========================================================================
// METHOD ::lval
// ========================================================================
long long value::lval (void) const
{
	if (arraysz) return ((*this)[0].lval());
	switch (_itype)
	{
		case i_long:
		case i_ulong: return t.lval;
		case i_double: return (long long) t.dval;
		case i_int:
			if (t.ival >= 0) return (long long) t.ival;
			return ((long long) t.ival) | 0xffffffff00000000LL;
		case i_date:
		case i_bool:
		case i_unsigned: return (long long) t.uval;
		case i_string: return strtoll (s.str(),NULL,10);
		case i_currency: return t.lval/1000;
	}
	return 0;
};

// ========================================================================
// METHOD ::ulval
// ========================================================================
unsigned long long value::ulval (void) const
{
	if (arraysz) return ((*this)[0].ulval());
	switch (_itype)
	{
		case i_long:
		case i_ulong: return t.ulval;
		case i_double: return (unsigned long long) t.dval;
		case i_int:
			if (t.ival >= 0) return (long long) t.ival;
			return ((long long) t.ival) | 0xffffffff00000000LL;
		case i_date:
		case i_bool:
		case i_unsigned: return (unsigned long long) t.uval;
		case i_string: return strtoull (s.str(),NULL,10);
		case i_currency:
			if (t.lval<0) return (unsigned long long) (-t.lval);
			return t.ulval;
	}
	return 0;
};

// ========================================================================
// METHOD ::bval
// ========================================================================
bool value::bval (void) const
{
	if (arraysz) return true;
	if ((_itype == i_bool) || (_itype == i_int) || (_itype == i_unsigned))
		return (bool) t.ival;
	if (_itype == i_double)
		return (bool) t.dval;
	if (_itype == i_ipaddr)
		return (bool)ipval();
	if (_itype == i_string)
		return s.strcasecmp ("true") ? false : true;
	if ((_itype == i_long) || (_itype == i_ulong) || (_itype == i_currency))
		return (bool) t.lval;
	return false;
}

// ========================================================================
// METHOD ::exists
// ----------------
// Locate a sub-value inside the array by its key value
// ========================================================================
bool value::exists (const char *key) const
{
	if (havechild (checksum(key),key)) return true;
	return false;
}

bool value::exists (unsigned int ki, const char *key) const
{
	if (havechild (ki, key)) return true;
	return false;
}

bool value::exists (const string &key) const
{
	if (havechild (checksum(key.str()), key.str())) return true;
	return false;
}

bool value::exists (const value &key) const
{
	return exists (key.sval());
}

bool value::exists (const statstring &key) const
{
	if (! key) return false;
	if (havechild (key.key(), key.str())) return true;
	return false;
}

// ========================================================================
// METHOD ::strlen
// ========================================================================
int value::strlen (void) const
{
	return sval().strlen();
}

// ========================================================================
// METHOD ::isarray
// ========================================================================
bool value::isarray (void) const
{
	return arraysz;
}

// ========================================================================
// METHOD ::isdict
// ========================================================================
bool value::isdict (void) const
{
	return arraysz && (! ucount);
}

// ========================================================================
// METHOD ::ismixed
// ========================================================================
bool value::ismixed (void) const
{
	return arraysz && (arraysz != ucount);
}

// ========================================================================
// METHOD ::findchild
// ------------------
// Locate a sub-value inside the array by its key value
// ========================================================================
value *value::findchild (const char *key) const
{
		// Calculate the checksum key
		unsigned int ki = checksum (key);
		return findchild (ki,key);
}
value *value::findchild (const char *key)
{
		// Calculate the checksum key
		unsigned int ki = checksum (key);
		return findchild (ki,key);
}

value *value::findchild (unsigned int ki, const char *key)
{	
	// Have children already been assigned to this value?
	if (arraysz > ucount) // yes
	{
		// Start searching from the first array index
		value *crsr = array[ucount];
		
		// If the node does not exist yet, this pointer will point at
		// the "lower" or "higher" pointer of the last end of the branch
		value *insert_at = NULL;
		
		while (crsr)
		{
			// The new key is lower than this node
			if (ki < crsr->key)
			{
				// There are lower nodes
				if (crsr->lower)
					crsr = crsr->lower;
				else
				{
					// No lower nodes, a node with the key does not exist
					// yet.
					insert_at = crsr;
					crsr = NULL;
				}
			}
			else
			{
				// Perhaps this is the node we're looking for?
				if ((crsr->key == ki) && ( (!key) || (::strcasecmp (key, crsr->_name.str()) == 0)) )
				{
					return crsr;
				}
				
				// Seems not, are there higher nodes?
				if (crsr->higher)
					crsr = crsr->higher;
				else
				{
					// Insert after this node then.
					insert_at = crsr;
					crsr = NULL;
				}
			}
		}

		// If control reaches this point, no node was found so we haves
		// to create a new one.

		++arraysz;
		alloc (arraysz);
		array[arraysz-1] = key ? new value (valueWithKey,key,ki) : 
						   new value (valueWithKey,ki);
		
		// And link it in the tree.
		if (insert_at) 
		{
			if (insert_at->key > ki)
				insert_at->lower = array[arraysz-1];
			else
				insert_at->higher = array[arraysz-1];
		}
	}
	else
	{
		if (! arraysz)
		{
			arraysz = 1;
			ucount = 0;
			alloc (arraysz);
			array[0] = key ? new value (valueWithKey,key,ki) : 
					   new value (valueWithKey,ki);
		}
		else
		{
			++arraysz;
			alloc (arraysz);
			array[arraysz-1] = key ? new value (valueWithKey,key,ki) : 
							   new value (valueWithKey,ki);
		}
	}
	return (array[arraysz-1]);
}

value *value::findchild (unsigned int ki, const char *key) const
{	
	// Have children already been assigned to this value?
	if (arraysz > ucount) // yes
	{
		// Start searching from the first array index
		value *crsr = array[ucount];
		
		// If the node does not exist yet, this pointer will point at
		// the "lower" or "higher" pointer of the last end of the branch
		value *insert_at = NULL;
		
		while (crsr)
		{
			// The new key is lower than this node
			if (ki < crsr->key)
			{
				// There are lower nodes
				if (crsr->lower)
					crsr = crsr->lower;
				else
				{
					// No lower nodes, a node with the key does not exist
					// yet.
					insert_at = crsr;
					crsr = NULL;
				}
			}
			else
			{
				// Perhaps this is the node we're looking for?
				if ((crsr->key == ki) && ( (!key) || (::strcasecmp (key, crsr->_name.str()) == 0)) )
				{
					return crsr;
				}
				
				// Seems not, are there higher nodes?
				if (crsr->higher)
					crsr = crsr->higher;
				else
				{
					// Insert after this node then.
					insert_at = crsr;
					crsr = NULL;
				}
			}
		}
		return NULL;
	}
	else
	{
		return NULL;
	}
	return (array[arraysz-1]);
}

// ========================================================================
// METHOD ::havechild
// ------------------
// Locate a sub-value inside the array by its key value, never create
// a node on demand.
// ========================================================================
value *value::havechild (unsigned int ki, const char *key) const
{	
	// Have children already been assigned to this value?
	if (arraysz > ucount) // yes
	{
		// Start searching from the first array index
		value *crsr = array[ucount];
		
		while (crsr)
		{
			// The new key is lower than this node
			if (ki < crsr->key)
			{
				// There are lower nodes
				if (crsr->lower)
					crsr = crsr->lower;
				else
				{
					// No lower nodes, a node with the key does not exist
					// yet.
					return NULL;
				}
			}
			else
			{
				// Perhaps this is the node we're looking for?
				if ((crsr->key == ki) && 
					( (!key) || (::strcasecmp (key, crsr->_name.str()) == 0)) )
				{
					return crsr;
				}
				
				// Seems not, are there higher nodes?
				if (crsr->higher)
					crsr = crsr->higher;
				else
				{
					// Insert after this node then.
					return NULL;
				}
			}
		}
	}
	return NULL;
}

// ========================================================================
// METHOD ::rmindex
// ----------------
// Delete a node at a specific index position.
// ========================================================================
void value::rmindex (int index)
{
	rmval (31337, NULL, index);
}

// ========================================================================
// METHOD ::rmval
// --------------
// Remove a member value by key, name or index.
// ========================================================================
void value::rmval (unsigned int ki, const char *key, int pindex)
{
	// Check for children, if there are none we might as well
	// leave. Sort of like Michael Jackson.
	
	int index = pindex;
	
	#define KEYMATCH(obj) ((obj->key == ki) && ( (!key) || (::strcasecmp (key, obj->_name.str()) == 0)))
	
	if ((index<0) && (ki == 31337) && (key == NULL))
	{
		index += arraysz;
		if (index<0) return;
	}
	
	unsigned int uindex = index;
	
	if (arraysz) // yes
	{
		bool rearrange = false;
		
		// Skip through the array to find it
		for (unsigned int i = (unsigned int) (index<0 ? 0:index); i<arraysz; ++i)
		{
			if ((i==uindex)||(KEYMATCH(array[i])))
			{
				value *crsr= array[i];
				if ((i+1) < arraysz)
				{
					::memmove (array+i, array+i+1,
							   (arraysz - (i+1)) * sizeof (value *));
				}
				delete crsr;
				--arraysz;
				array[arraysz] = NULL;
				if (! arraysz)
				{
					::free (array);
					array = NULL;
					arrayalloc = 0;
				}
				if (i<ucount) --ucount;
				rearrange = true;
				i = arraysz;
			}
		}
		if (rearrange) relinktree ();
	}
}


void value::relinktree (void)
{
	unsigned int i;
	for (i=0; i<arraysz; ++i)
	{
		array[i]->lower = array[i]->higher = NULL;
	}
	
	for (i=ucount+1; i<arraysz; ++i)
	{
		value *nobj = array[i];
		value *crsr = array[ucount];
		
		while (crsr)
		{
			if (nobj->key < crsr->key)
			{
				if (crsr->lower)
					crsr = crsr->lower;
				else
				{
					crsr->lower = nobj;
					crsr = NULL;
					break;
				}
			}
			else
			{
				if (crsr->higher)
					crsr = crsr->higher;
				else
				{
					crsr->higher = nobj;
					crsr = NULL;
					break;
				}
			}
		}
	}
}

void value::rmval (unsigned int ki)
{
	rmval (ki, NULL);
}

void value::rmval (const statstring &kstr)
{
	rmval (kstr.key(), kstr.str(), -1);
}

void value::rmval (const char *key)
{
	rmval (checksum (key), key);
}

void value::rmval (const value &v)
{
	rmval (v.cval());
}

// ========================================================================
// METHOD ::last
// -------------
// Return array tail.
// ========================================================================
value &value::last (void)
{
	return (*this)[-1];
}

// ========================================================================
// INDEX OPERATOR (value)
// ----------------------
// Locate a sub-value inside the array by its key value
// ========================================================================
const value	&value::operator[] (int i) const
{
	if ((i<0) && ((arraysz+i) >=0)) return *(array[arraysz+i]);
	value *v = getposition (i);
	if (!v) return emptyvalue;
	return *v;
}
value &value::operator[] (int i)
{
	static value emptyvalue;

	if (_type == t_unset)
		_type = t_array;

	if ((i<0) && ((arraysz+i) >=0)) return *(array[arraysz+i]);
	value *v = getposition ((unsigned int) i);
	if (!v) return emptyvalue;
	return *v;
}

					 /// Valuebuilder: set explicit value.
					 /// \param v The explicit value. Attributes
					 /// are skipped unless if the original is
					 /// of an array type.
					 /// \param v Original value.

// ========================================================================
// METHOD ::$val
// ---------------
value* value::$val (const value &v)
{
	switch (v._itype)
	{
		case i_unset: break;
		case i_bool: (*this) = v.bval(); break;
		case i_long: (*this) = v.lval();
		case i_unsigned: (*this) = v.uval();
		case i_ulong: (*this) = v.ulval();
		case i_int: (*this) = v.ival(); break;
		case i_double: (*this) = v.dval(); break;
		case i_ipaddr: (*this) = v.ipval(); break;
		case i_string:
		case i_date:
		case i_currency:
			(*this) = v.sval();
			break;
		default:
			(*this) = v;
			break;
	}
	return this;
}
					 
					 
// ========================================================================
// METHOD ::newval
// ---------------
// A convenient way to expand an array with a new subvalue with no
// key.
// ========================================================================
value &value::newval (dtenum typ)
{
	if (arraysz)
	{
		++arraysz;
		alloc (arraysz);

		if (arraysz>(ucount+1))
		{
			::memmove (array+ucount+1, array+ucount,
					   (arraysz - (ucount+1)) * sizeof (value *));
		}
		array[ucount] = new value;
		array[ucount]->_type = typ;
		++ucount;
	}
	else
	{
		arraysz = ucount = 1;
		alloc (1);
		array[0] = new value;
		array[0]->_type = typ;
	}
	return *(array[ucount-1]);
}

// ========================================================================
// METHOD ::insertval
// ========================================================================
value &value::insertval (unsigned int atpos, dtenum typ)
{
	if (ucount > atpos)
	{
		++arraysz;
		alloc (arraysz);
		::memmove (array+atpos+1, array+atpos,
				   (arraysz - (atpos+1)) * sizeof (value *));
		array[atpos] = new value;
		array[atpos]->_type = typ;
		ucount++;
	}
	
	return (*this)[atpos];
}

// ========================================================================
// METHOD ::getposition
// --------------------
// Get a numbered sub value out of the array.
// ========================================================================
value *value::getposition (unsigned int idx) const
{
	if (idx >= arraysz) return NULL;
	return array[idx];
}

value *value::getposition (unsigned int idx)
{
	if (idx >= arraysz)
	{
		alloc (idx+1);
		if (arraysz == ucount)
		{
			ucount = idx+1;
		}
		for (;arraysz<idx;++arraysz)
		{
			array[arraysz] = new value;
		}
		array[arraysz++] = new value;
	}
	if (array[idx] == NULL)
		array[idx] = new value;
	return array[idx];
}

// ========================================================================
// METHOD ::name
// -------------
// Returns the string value of the value's key, if it is set, or an empty
// string if it was unset.
// ========================================================================
const char *value::name (void) const
{
	return _name ? _name.str() : "";
}

// ========================================================================
// METHOD ::label
// ========================================================================
const statstring &value::label (void) const
{
	return _name;
}

// ========================================================================
// METHOD ::id
// ========================================================================
const statstring &value::id (void) const
{
	return _name;
}

const char ___SPC[] = "                                                                                                                                ";
const char *_VALUE_INDENT_TABS="\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

// ========================================================================
// METHOD ::filter
// ========================================================================
value *value::filter (const statstring &label, const string &what) const
{
	returnclass (value) res retain;
	
	for (unsigned int i=0; i<arraysz; ++i)
	{
		if ( (*array[i])[label].sval().globcmp (what) )
		{
			if (array[i]->_name)
			{
				res[array[i]->_name] = (*(array[i]));
			}
			else
			{
				res.newval(array[i]->type()) = (*(array[i]));
			}
		}
	}
	return &res;
}

// ========================================================================
// METHOD ::clear
// ========================================================================
void value::clear (void)
{
	if (arraysz)
	{
		for (unsigned int i=0; i<arraysz; ++i)
		{
			delete array[i];
		}
	}
	if (array)
	{
		::free (array);
		array = NULL;
	}
	arraysz = 0;
	arrayalloc = 0;
	ucount = 0;
	_type = t_unset;
	_itype = i_unset;
	s.crop (0);
	if (attrib)
	{
		delete attrib;
		attrib = NULL;
	}
}

void value::cleararray (void)
{
	if (arraysz)
	{
		for (unsigned int i=0; i<arraysz; ++i)
		{
			delete array[i];
		}
	}
	if (array)
	{
		::free (array);
		array = NULL;
	}
	arraysz = 0;
	arrayalloc = 0;
	ucount = 0;
}

// ========================================================================
// METHOD ::alloc
// ========================================================================
void value::alloc (unsigned int count)
{
	if (count & 0x80000000) throw valueArraySizeException();
	unsigned int wanted = 4;
	while (wanted < count)
	{
		if (wanted < 16384) wanted = wanted << 1;
		else wanted = wanted + 16384;
	}
	
	if (wanted > arrayalloc)
	{
		if (arrayalloc)
		{
			array = (value **) realloc (array, wanted * sizeof (value *));
		}
		else
		{
			array = (value **) malloc (wanted * sizeof (value *));
		}
		while (arrayalloc < wanted) array[arrayalloc++] = NULL;
	}
}

// ========================================================================
// METHOD ::cutleft
// ========================================================================
value *value::cutleft (int pcnt)
{
	returnclass (value) res retain;

	if (! pcnt) return &res;
	if (! arraysz) return &res;
	
	int cnt = pcnt;
	unsigned int i;
	
	if (cnt<0) cnt += arraysz;
	if (cnt<0) return &res;
	
	unsigned int ucnt = cnt;
	if (ucnt>arraysz) ucnt = arraysz;
	
	for (i=0; i<ucnt; ++i)
	{
		if (array[0]->id())
		{
			res[array[0]->id()] = *(array[0]);
		}
		else
		{
			res.newval() = *(array[0]);
		}
		rmindex (0);
	}
	return &res;
}

// ========================================================================
// METHOD ::copyleft
// ========================================================================
value *value::copyleft (int pcnt) const
{
	returnclass (value) res retain;

	if (! pcnt) return &res;
	if (! arraysz) return &res;
	
	int cnt = pcnt;
	unsigned int i;
	
	if (cnt<0) cnt += arraysz;
	if (cnt<0) return &res;

	unsigned int ucnt = cnt;
	if (ucnt>arraysz) ucnt = arraysz;
	
	for (i=0; i<ucnt; ++i)
	{
		if (array[i]->id())
		{
			res[array[i]->id()] = *(array[i]);
		}
		else
		{
			res.newval() = *(array[i]);
		}
	}
	return &res;
}

// ========================================================================
// METHOD ::cutright
// ========================================================================
value *value::cutright (int pcnt)
{
	returnclass (value) res retain;
	if (! pcnt) return &res;
	if (! arraysz) return &res;
	
	int cnt = pcnt;
	unsigned int i;
	
	if (cnt<0) cnt += arraysz;
	if (cnt<0) return &res;
	
	unsigned int ucnt = cnt;
	if (ucnt>arraysz) ucnt = arraysz;
	
	for (i=0; i<ucnt; ++i)
	{
		if (array[arraysz-1]->id())
		{
			res[array[arraysz-1]->id()] = *(array[arraysz-1]);
		}
		else
		{
			res.newval() = *(array[arraysz-1]);
		}
		rmindex (-1);
	}
	return &res;
}

// ========================================================================
// METHOD ::copyright
// ========================================================================
value *value::copyright (int pcnt) const
{
	returnclass (value) res retain;

	if (! pcnt) return &res;
	if (! arraysz) return &res;
	
	int cnt = pcnt;
	unsigned int i;
	
	if (cnt<0) cnt += arraysz;
	if (cnt<0) return &res;
	
	unsigned int ucnt = cnt;
	if (ucnt>arraysz) ucnt = arraysz;
	
	for (i=0; i<ucnt; ++i)
	{
		if (array[(arraysz-1)-i]->id())
		{
			res[array[(arraysz-1)-i]->id()] = *(array[(arraysz-1)-i]);
		}
		else
		{
			res.newval() = *(array[(arraysz-1)-i]);
		}
	}
	return &res;
}

// ========================================================================
// METHOD value::splice
// ========================================================================
value *value::splice (int _pos, int _count) const
{
	if (_pos < 0) _pos += arraysz;
	if (_pos < 0) _pos = arraysz;
	
	size_t pos = _pos;
	
	if (pos > arraysz) pos = arraysz;
	
	if (_count < 0) _count = arraysz - pos;
	if (_count < 0) _count = 0;

	if (! _count) return NULL;
	
	size_t count = _count;

	size_t endpos = pos+count;
	if (endpos > arraysz) endpos = arraysz;
	
	returnclass (value) res retain;
	for (size_t i=pos; i<endpos; ++i)
	{
		if (array[i]->id())
		{
			res[array[i]->id()] = *(array[i]);
		}
		else
		{
			res.newval() = *(array[i]);
		}
	}
	
	return &res;
}

// ========================================================================
// METHOD ::zip
// ========================================================================
value *value::zip (void) const
{
	returnclass (value) res retain;
	int i = 0;
	
	foreach (node, (*this))
	{
		int j = 0;
		const statstring &id = node.id();
		foreach (child, node)
		{
			const statstring &cid = child.id();
			if (cid)
			{
				if (id) res[cid][id] = child;
				else res[cid][j] = child;
			}
			else
			{
				if (id) res[i][id] = child;
				else res[i][j] = child;
			}
			j++;
		}
		i++;
	}
	
	return &res;
}

// ========================================================================
// METHOD ::byvalue
// ========================================================================
value *value::byvalue (void) const
{
	returnclass (value) res retain;
	int i = 0;
	
	foreach (node, (*this))
	{
		if (node.sval()) res[node] = node.id();
		else res[i] = node.id();
		i++;
	}
	return &res;
}

// ========================================================================
// METHOD ::join
// ========================================================================
string *value::join (const string &sep, const string &l, const string &r)
{
	returnclass (string) res retain;
	if (l) res = l;
	bool first = true;
	foreach (node, (*this))
	{
		if (! first) res.strcat (sep);
		first = false;
		res.strcat (node.sval());
	}
	if (r) res.strcat (r);
	return &res;
}

// ========================================================================
// METHOD ::treecmp
// ========================================================================
bool value::treecmp (const value &other) const
{
	if (array)
	{
		if (arraysz != other.arraysz) return false;
		for (unsigned int i=0; i<arraysz; ++i)
		{
			if (array[i]->_name)
			{
				if (! other.exists (array[i]->_name)) return false;
				if (! array[i]->treecmp (other[array[i]->_name])) return false;
			}
			else
			{
				if (i >= (unsigned int) other.count()) return false;
				if (! array[i]->treecmp (other[i])) return false;
			}
		}
		return true;
	}
	
	if (attrib)
	{
		if (! other.attrib) return false;
		if (! attrib->treecmp (*(other.attrib))) return false;
	}
	
	return ((value &)(*this) == (value &)other);
}

// ========================================================================
// FUNCTION __parse_timestr
// ========================================================================
time_t __parse_timestr (const string &dt)
{
	timestamp t;
	t.iso (dt);
	return t.unixtime ();
}

// ========================================================================
// METHOD __make_timestr
// ========================================================================
string *__make_timestr (time_t ti)
{
	returnclass (string) res retain;
	timestamp t = ti;
	res = t.iso ();
	return &res;
}

// ========================================================================
// METHOD ::assign
// ========================================================================
void value::assign (const currency &c)
{
	t.lval = c.value();
	_itype = i_currency;
	if (_type == t_unset || isbuiltin (_type)) _type = t_currency;
}

void value::assign (currency *c)
{
	t.lval = c->value();
	delete c;
	_itype = i_currency;
	if (_type == t_unset || isbuiltin (_type)) _type = t_currency;
}

#define CHECK_BUILTIN(x) assert(x.strlen() == type.strlen() ); if( type == x ) return true; 

// ========================================================================
// METHOD ::isbuiltin
// ========================================================================
bool value::isbuiltin (const statstring &type)
{
	switch (type.strlen())
	{
	case 4:
		CHECK_BUILTIN(t_unset); // void
		CHECK_BUILTIN(t_char);
		CHECK_BUILTIN(t_bool);
		CHECK_BUILTIN(t_long);
		CHECK_BUILTIN(t_dict);
		CHECK_BUILTIN(t_date);
		return false;
	case 5:
		CHECK_BUILTIN(t_uchar);
		CHECK_BUILTIN(t_double); // float
		CHECK_BUILTIN(t_short);
		CHECK_BUILTIN(t_array);
		CHECK_BUILTIN(t_ulong);
		return false;
	case 6:
		CHECK_BUILTIN(t_ushort);
		CHECK_BUILTIN(t_string);
		// CHECK_BUILTIN(t_double); ????
		return false;
	case 7:
		CHECK_BUILTIN(t_int); // integer
		return false;
	case 8:
		CHECK_BUILTIN(t_currency);
		CHECK_BUILTIN(t_unsigned);
		return false;
	case 9:
		CHECK_BUILTIN(t_ipaddr); //ipaddress
		CHECK_BUILTIN(t_bool_true);
		return false;
	case 10:
		CHECK_BUILTIN(t_bool_false);
		return false;
	}

	return false;
}

#undef CHECK_BUILTIN

// ========================================================================
// METHOD ::init
// ========================================================================
void value::init (bool first)
{
	if (first)
	{
		threadref = getref();
		_type = t_unset;
		_itype = i_unset;
		
		t.lval = 0;
		key = 0;
		lower = higher = NULL;
		array = NULL;
		arraysz = 0;
		arrayalloc = 0;
		ucount = 0;
		attrib = NULL;
	}
	else
	{
		clear ();
	}
}

// ========================================================================
// METHOD ::isempty
// ========================================================================
bool value::isempty (void) const
{
	if (_itype != i_unset) return false;
	if (array) return false;
	if (attrib) return false;
	return true;
}

// ========================================================================
// METHOD ::operator[]
// ========================================================================
const value &value::operator[] (const char *str) const
{
	value *v;

	v = findchild (str);
	if (!v) return emptyvalue;
	return *v;
}

value &value::operator[] (const char *str)
{
	if (_type == t_unset || isbuiltin (_type)) _type = t_dict;
	value *v = findchild (str);
	return *v;
}

const value &value::operator[] (const string &str) const
{
	value *v;

	v = findchild (str.str());
	if (!v) return emptyvalue;
	return *v;
}

value &value::operator[] (const string &str)
{
	if (_type == t_unset)
		_type = t_dict;
		
	value *v = findchild (str.str());
	return *v;
}

const value &value::operator[] (const statstring &str) const
{
	value *v;
	
	v = findchild ((unsigned int) str.key(), (const char *) str.str());
	if (! v) return emptyvalue;
	return *v;
}

value &value::operator[] (const statstring &str)
{
	if (_type == t_unset || isbuiltin (_type)) _type = t_dict;
	
	value *v;
	v = findchild ((unsigned int) str.key(), (const char *) str.str());
	return *v;
}

value &value::operator[] (const value &va)
{
	value *v;
	if (va.type() == t_int)
	{
		if (_type == t_unset || isbuiltin (_type)) _type = t_array;
		v = getposition (va.uval());
	}
	else
	{
		if (_type == t_unset || isbuiltin (_type)) _type = t_dict;
		v = findchild (va.sval());
	}
	
	return *v;
}

const value &value::operator[] (const value &va) const
{
	value *v;
	if (va.type() == t_int)
	{
		v = getposition (va.uval());
	}
	else
	{
		v = findchild (va.sval());
	}
	
	if (! v) return emptyvalue;
	return *v;
}

// ========================================================================
// METHOD ::operator()
// ========================================================================
const value &value::operator() (const statstring &ki) const
{
	if (! attrib) return emptyvalue;
	return (*attrib)[ki];
}

value &value::operator() (const statstring &ki)
{
	if (! attrib) attrib = new value;
	return (*attrib)[ki];
}

// ========================================================================
// METHOD ::setattrib
// ========================================================================
void value::setattrib (const statstring &ki, const string &val)
{
	if (! attrib) attrib = new value;
	(*attrib)[ki] = val;
}

void value::setattrib (const statstring &ki, int val)
{
	if (! attrib) attrib = new value;
	(*attrib)[ki] = val;
}

void value::setattrib (const statstring &ki, const char *val)
{
	if (! attrib) attrib = new value;
	(*attrib)[ki] = val;
}

void value::setattrib (const statstring &ki, bool val)
{
	if (! attrib) attrib = new value;
	(*attrib)[ki] = val;
}

// ========================================================================
// METHOD ::operator<<
// ========================================================================
value &value::operator<< (value *v)
{
	for (int i=0; i<(*v).count(); ++i)
	{
		value &o = (*v)[i];
		const statstring &vid = o.id();
		if (vid) (*this)[vid] = o;
		else newval() = o;
	}
	delete v;
	return *this;
}

value &value::operator<< (const value &v)
{
	for (int i=0; i<v.count(); ++i)
	{
		const value &o = v[i];
		const statstring &vid = o.id();
		if (vid) (*this)[vid] = o;
		else newval() = o;
	}
	return *this;
}

// ========================================================================
// METHOD ::operator+=
// ========================================================================
value &value::operator+= (const string &str)
{
	newval() = str;
	return *this;
}

value &value::operator+= (const value &v)
{
	if (! v.id()) newval() = v;
	else (*this)[v.id()] = v;
	return *this;
}

// ========================================================================
// METHOD ::operator<
// ========================================================================
bool value::operator< (const value &other) const
{
	switch (other._itype)
	{
		case i_string:
			return (::strcmp (cval(), other.cval()) <0);
		case i_int:
			return (ival() < other.ival());
		case i_unsigned:
		case i_date:
			return (uval() < other.uval());
		case i_currency:
		case i_long:
			return (lval() < other.lval());
		case i_ulong:
			return (ulval() < other.ulval());
		
		default:
			return (dval() < other.dval());
	}
}

bool value::operator< (const value &other)
{
	switch (other._itype)
	{
		case i_string:
			return (::strcmp (cval(), other.cval()) <0);
		case i_int:
			return (ival() < other.ival());
		case i_unsigned:
		case i_date:
			return (uval() < other.uval());
		case i_currency:
		case i_long:
			return (lval() < other.lval());
		case i_ulong:
			return (ulval() < other.ulval());
		
		default:
			return (dval() < other.dval());
	}
}

// ========================================================================
// METHOD ::operator<=
// ========================================================================
bool value::operator<= (const value &other) const
{
	switch (other._itype)
	{
		case i_string:
			return (::strcmp (cval(), other.cval()) <=0);
		case i_int:
			return (ival() <= other.ival());
		case i_unsigned:
		case i_date:
			return (uval() <= other.uval());
		case i_currency:
		case i_long:
			return (t.lval <= other.t.lval);
		case i_ulong:
			return (ulval() <= other.ulval());
		
		default:
			return (dval() <= other.dval());
	}
}

bool value::operator<= (const value &other)
{
	switch (other._itype)
	{
		case i_string:
			return (::strcmp (cval(), other.cval()) <=0);
		case i_int:
			return (ival() <= other.ival());
		case i_unsigned:
		case i_date:
			return (uval() <= other.uval());
		case i_currency:
		case i_long:
			return (t.lval <= other.t.lval);
		case i_ulong:
			return (ulval() <= other.ulval());
		
		default:
			return (dval() <= other.dval());
	}
}

// ========================================================================
// METHOD ::operator>=
// ========================================================================
bool value::operator>= (const value &other) const
{
	switch (other._itype)
	{
		case i_string:
			return (::strcmp (cval(), other.cval()) >=0);
		case i_int:
			return (ival() >= other.ival());
		case i_unsigned:
		case i_date:
			return (uval() >= other.uval());
		case i_currency:
		case i_long:
			return (t.lval >= other.t.lval);
		case i_ulong:
			return (ulval() >= other.ulval());
		
		default:
			return (dval() >= other.dval());
	}
}

bool value::operator>= (const value &other)
{
	switch (other._itype)
	{
		case i_string:
			return (::strcmp (cval(), other.cval()) >=0);
		case i_int:
			return (ival() >= other.ival());
		case i_unsigned:
		case i_date:
			return (uval() >= other.uval());
		case i_currency:
		case i_long:
			return (t.lval >= other.t.lval);
		case i_ulong:
			return (ulval() >= other.ulval());
		
		default:
			return (dval() >= other.dval());
	}
}

// ========================================================================
// METHOD ::operator>
// ========================================================================
bool value::operator> (const value &other) const
{
	switch (other._itype)
	{
		case i_string:
			return (::strcmp (cval(), other.cval()) >0);
		case i_int:
			return (ival() > other.ival());
		case i_unsigned:
		case i_date:
			return (uval() > other.uval());
		case i_currency:
		case i_long:
			return (lval() > other.lval());
		case i_ulong:
			return (ulval() > other.ulval());
		
		default:
			return (dval() > other.dval());
	}
}

bool value::operator> (const value &other)
{
	switch (other._itype)
	{
		case i_string:
			return (::strcmp (cval(), other.cval()) >0);
		case i_int:
			return (ival() > other.ival());
		case i_unsigned:
		case i_date:
			return (uval() > other.uval());
		case i_currency:
		case i_long:
			return (lval() > other.lval());
		case i_ulong:
			return (ulval() > other.ulval());
		
		default:
			return (dval() > other.dval());
	}
}

// ========================================================================
// METHOD ::operator==
// ========================================================================
bool value::operator== (const value &other) const
{
	if (count() && other.count()) return treecmp (other);
	switch (other._itype)
	{
		case i_unset:
			if (_itype == i_unset) return true;
			if ((_itype == i_string) && (! s.strlen())) return true;
			return false;
		case i_string:
			return (::strcmp (cval(), other.cval()) ==0);
		case i_int:
			return (ival() == other.ival());
		case i_unsigned:
		case i_date:
			return (uval() == other.uval());
		case i_currency:
		case i_long:
			return (t.lval == other.t.lval);
		case i_ulong:
			return (ulval() == other.ulval());
		
		default:
			return (dval() == other.dval());
	}
}

bool value::operator== (const value &other)
{
	if (count() && other.count()) return treecmp (other);
	switch (other._itype)
	{
		case i_unset:
			if (_itype == i_unset) return true;
			if ((_itype == i_string) && (! s.strlen())) return true;
			return false;
		case i_string:
			return (::strcmp (cval(), other.cval()) ==0);
		case i_int:
			return (ival() == other.ival());
		case i_unsigned:
		case i_date:
			return (uval() == other.uval());
		case i_currency:
		case i_long:
			return (t.lval == other.t.lval);
		case i_ulong:
			return (ulval() == other.ulval());
		
		default:
			return (dval() == other.dval());
	}
}

// ========================================================================
// METHOD ::operator!=
// ========================================================================
bool value::operator!= (const value &other) const
{
	if (count() && other.count()) return (!treecmp (other));
	switch (other._itype)
	{
		case i_unset:
			if (_itype == i_unset) return false;
			if ((_itype == i_string) && (! s.strlen())) return false;
			return true;
		case i_string:
			return (::strcmp (cval(), other.cval()) !=0);
		case i_int:
			return (ival() != other.ival());
		case i_unsigned:
		case i_date:
			return (uval() != other.uval());
		case i_currency:
		case i_long:
			return (t.lval != other.t.lval);
		case i_ulong:
			return (ulval() != other.ulval());
		
		default:
			return (dval() != other.dval());
	}
}

bool value::operator!= (const value &other)
{
	if (count() && other.count()) return (!treecmp (other));
	switch (other._itype)
	{
		case i_unset:
			if (_itype == i_unset) return false;
			if ((_itype == i_string) && (! s.strlen())) return false;
			return true;
		case i_string:
			return (::strcmp (cval(), other.cval()) !=0);
		case i_int:
			return (ival() != other.ival());
		case i_unsigned:
		case i_date:
			return (uval() != other.uval());
		case i_currency:
		case i_long:
			return (t.lval != other.t.lval);
		case i_ulong:
			return (ulval() != other.ulval());
		
		default:
			return (dval() != other.dval());
	}
}

// ========================================================================
// METHOD ::setcurrency
// ========================================================================
void value::setcurrency (long long cnew)
{
	cleararray ();
	t.lval = cnew;
	_itype = i_currency;
	if (_type == t_unset || isbuiltin (_type)) _type = t_currency;
}

// ==========================================================================
// METHOD value::getcurrency
// ==========================================================================
long long value::getcurrency (void) const
{
	switch (_itype)
	{
		case i_currency:
			return t.lval;
		
		case i_double:
			return (unsigned long long) (t.dval * 1000.0);
		
		case i_string:
			return parsecurrency (s);

		case i_int:
			return (t.ival * 1000LL);
		
		case i_unsigned:
			return ((long long)t.uval) * 1000LL;
		
		case i_long:
		case i_ulong:
			return t.lval * 1000LL;
		
		default:
			return 0LL;
	}
	return 0LL; // unreachable
}

const value emptyvalue;
