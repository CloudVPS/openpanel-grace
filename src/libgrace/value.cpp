// ========================================================================
// value.cpp: Keyed generic data storage class
//
// (C) Copyright 2003-2006 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam 
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

#include <stdio.h>
#include <string.h>

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
// This constructor initializes an empty value object
// ========================================================================
value::value (const char *orig)
{
	init (true);
	(*this) = orig;
}

// ========================================================================
// CONSTRUCTOR (key)
// -----------------
// This constructor initializes value object with an integer key
// ========================================================================
value::value (creatorlabel l, unsigned int k)
{
	_type = t_unset;
	itype = i_unset;
	t.lval = 0;
	key = k;
	lower = higher = NULL;
	array = NULL;
	arraysz = 0;
	arrayalloc = 0;
	ucount = 0;
	attrib = NULL;
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
	itype = i_unset;
	
	t.lval = 0;
	key = ki;
	_name = k;
	lower = higher = NULL;
	array = NULL;
	ucount = 0;
	arraysz = 0;
	arrayalloc = 0;
	attrib = NULL;
}

// ========================================================================
// CONSTRUCTOR (string)
// --------------------
// This constructor initializes value object with a string key
// ========================================================================
value::value (creatorlabel l, const string &k)
{
	_type = t_unset;
	itype = i_unset;
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

	switch (v.itype)
	{
		case i_unsigned:
		case i_ipaddr:
		case i_date:
		case i_int:
		case i_bool:
			itype = v.itype;
			t.uval = v.t.uval;
			break;
		
		case i_string:
			itype = i_string;
			s = v.s;
			break;
		
		case i_long:
		case i_ulong:
		case i_currency:
			itype = v.itype;
			t.ulval = v.t.ulval;
			break;
			
		case i_double:
			itype = i_double;
			t.dval = v.t.dval;
			break;
			
		default:
			itype = i_unset;
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
	itype = i_unset;
	
	t.lval = 0;
	key = 0;
	lower = higher = NULL;
	array = NULL;
	arraysz = 0;
	arrayalloc = 0;
	ucount = 0;
	attrib = NULL;
	
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
value::value (valuable &v)
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
	itype = i_string;
	_type= t_string;
}

// ========================================================================
// COPY CONSTRUCTOR
// ========================================================================
value::value (const statstring &str)
{
	init ();
	s = str;
	itype = i_string;
	_type = t_string;
}

// ========================================================================
// COPY CONSTRUCTOR
// ========================================================================
value::value (string *str)
{
	init ();
	s = str;
	itype = i_string;
	_type = t_string;
}

// ========================================================================
// COPY CONSTRUCTOR
// ========================================================================
value::value (int orig)
{
	init ();
	t.ival = orig;
	itype = i_int;
	_type = t_int;
}

value::value (unsigned int orig)
{
	init ();
	t.uval = orig;
	itype = i_unsigned;
	_type = t_unsigned;
}

value::value (long long orig)
{
	init ();
	t.lval = orig;
	itype = i_long;
	_type = t_long;
}

value::value (unsigned long long orig)
{
	init ();
	t.ulval = orig;
	itype = i_ulong;
	_type = t_ulong;
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
	other.tovalue (*this);
	return *this;
}

value &value::operator= (valuable *other)
{
	other->tovalue (*this);
	delete other;
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
	
	dtenum vt = v.type();
	
	_type = vt;
	switch (v.itype)
	{
		case i_string: s.strclone (v.s); break;
		case i_bool:
		case i_ipaddr:
		case i_unsigned:
		case i_date:
		case i_int: t.uval = v.t.uval; break;
		case i_double: t.dval = v.t.dval; break;
		case i_currency:
		case i_ulong:
		case i_long: t.ulval = v.t.ulval; break;
	}
	itype = v.itype;
	_type = v.type();
	
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
			::printf ("FUCK!\n");
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
	clear();
	
	// Prefer to keep the original type
	
	if (v != NULL)
	{
		switch (v->itype)
		{
			case i_string: s = v->s; break;
			case i_bool:
			case i_unsigned:
			case i_int:
			case i_date:
			case i_ipaddr: t.uval = v->t.uval; break;
			case i_double: t.dval = v->t.dval; break;
			case i_long:
			case i_ulong: t.ulval = v->t.ulval; break;
		}
		itype = v->itype;
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
	// the itype is not i_string.
	//
	// This string representation is an internal affair of the class and
	// its side-effects  do not harm the principal constness, the code
	// the code 'owning' a value-object and passing it as const to another
	// function does not find the object in a functionally altered state.
    string &S = (string &) s;

	switch (itype)
	{
		case i_string:
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
		
		case i_ipaddr:
			S.crop();
			S.printf ("%i.%i.%i.%i",
					  (t.uval & 0xff000000) >> 24,
					  (t.uval & 0x00ff0000) >> 16,
					  (t.uval & 0x0000ff00) >> 8,
					  (t.uval & 0x000000ff));
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
	switch (itype)
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
	switch (itype)
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
	switch (itype)
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
	switch (itype)
	{
		case i_long:
		case i_ulong: return t.lval;
		case i_double: return (long long) t.dval;
		case i_int:
			if (t.ival >= 0) return (long long) t.ival;
			return ((long long) t.ival) | 0xffffffff00000000LL;
		case i_date:
		case i_ipaddr:
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
	switch (itype)
	{
		case i_long:
		case i_ulong: return t.ulval;
		case i_double: return (unsigned long long) t.dval;
		case i_int:
			if (t.ival >= 0) return (long long) t.ival;
			return ((long long) t.ival) | 0xffffffff00000000LL;
		case i_date:
		case i_ipaddr:
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
	if ((itype == i_bool) || (itype == i_int) || (itype == i_unsigned))
		return (bool) t.ival;
	if (itype == i_double)
		return (bool) t.dval;
	if (itype == i_string)
		return s.strcasecmp ("true") ? false : true;
	if ((itype == i_long) || (itype == i_ulong) || (itype == i_currency))
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

bool value::exists (const statstring &key) const
{
	if (! key) return false;
	if (havechild (key.key(), key.str())) return true;
	return false;
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
		if (rearrange)
		{
			unsigned int i;
			
			for (i=ucount; i<arraysz; ++i)
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
	static value emptyvalue;
	
	if ((i<0) && ((arraysz+i) >=0)) return *(array[arraysz+i]);
	value *v = getposition (i);
	if (!v) return emptyvalue;
	return *v;
}
value &value::operator[] (int i)
{
	if (_type == t_unset)
		_type = t_array;

	if ((i<0) && ((arraysz+i) >=0)) return *(array[arraysz+i]);
	value *v = getposition ((unsigned int) i);
	if (!v) return *this;
	return *v;
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
value &value::insertval (int atpos, dtenum typ)
{
	if (ucount > atpos)
	{
		++arraysz;
		alloc (arraysz);
		::memmove (array+atpos+1, array+atpos,
				   (arraysz - (atpos+1)) * sizeof (value *));
		array[atpos] = new value;
		array[atpos]->_type = typ;
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
	if (idx >= arraysz)
		return (value *)this;
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
	itype = i_unset;
	s.crop (0);
	if (attrib)
	{
		delete attrib;
		attrib = NULL;
	}
}

// ========================================================================
// METHOD ::alloc
// ========================================================================
void value::alloc (unsigned int count)
{
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
	string fm;
	string ent;
	struct tm mytm;
	
    fm = dt;
    ent = fm.cutat ('-');
    mytm.tm_year = atoi (ent.str()) - 1900;
    ent = fm.cutat ('-');
    mytm.tm_mon = atoi (ent.str()) -1;
    ent = fm.cutat (' ');
    mytm.tm_mday = atoi (ent.str());
    ent = fm.cutat (':');
    mytm.tm_hour = atoi (ent.str());
    ent = fm.cutat (':');
    mytm.tm_min = atoi (ent.str());
    mytm.tm_sec = atoi (fm.str());
    
    return mktime (&mytm);
}

// ========================================================================
// METHOD __make_timestr
// ========================================================================
string *__make_timestr (time_t ti)
{
	struct tm mytm;
	string *res = new string;
	
	gmtime_r (&ti, &mytm);
	res->printf ("%i-", mytm.tm_year + 1900);
	if (mytm.tm_mon<9) (*res).strcat ('0');
	res->printf ("%i-", mytm.tm_mon+1);
	if (mytm.tm_mday<10) (*res).strcat ('0');
	res->printf ("%i ", mytm.tm_mday);
	if (mytm.tm_hour<10) (*res).strcat (' ');
	res->printf ("%i:", mytm.tm_hour);
	if (mytm.tm_min<10) (*res).strcat ('0');
	res->printf ("%i:", mytm.tm_min);
	if (mytm.tm_sec<10) (*res).strcat ('0');
	res->printf ("%i", mytm.tm_sec);
	
	return res;
}

// ========================================================================
// METHOD ::assign
// ========================================================================
void value::assign (const currency &c)
{
	t.lval = c.value();
	itype = i_currency;
	if (_type == t_unset) _type = t_currency;
}

void value::assign (currency *c)
{
	t.lval = c->value();
	delete c;
	itype = i_currency;
	if (_type == t_unset) _type = t_currency;
}

// ========================================================================
// METHOD ::isbuiltin
// ========================================================================
bool value::isbuiltin (const statstring &type)
{
	static value tplist;
	if (! tplist.count())
	{
		tplist[t_char] = true;
		tplist[t_uchar] = true;
		tplist[t_short] = true;
		tplist[t_ushort] = true;
		tplist[t_int] = true;
		tplist[t_unsigned] = true;
		tplist[t_bool] = true;
		tplist[t_bool_true] = true;
		tplist[t_bool_false] = true;
		tplist[t_double] = true;
		tplist[t_string] = true;
		tplist[t_ipaddr] = true;
		tplist[t_unset] = true;
		tplist[t_long] = true;
		tplist[t_ulong] = true;
		tplist[t_array] = true;
		tplist[t_dict] = true;
		tplist[t_date] = true;
		tplist[t_currency] = true;
	}
	
	return tplist.exists (type);
}

// ========================================================================
// METHOD ::init
// ========================================================================
void value::init (bool first)
{
	if (first)
	{
		_type = t_unset;
		itype = i_unset;
		
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

bool value::isempty (void) const
{
	if (itype != i_unset) return false;
	if (array) return false;
	if (attrib) return false;
	return true;
}

const value emptyvalue;
