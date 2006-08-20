// ========================================================================
// value_shox.cpp: Keyed binary data storage class
//
// (C) Copyright 2006 Pim van Riezen <pi@madscience.nl>
//                    Madscience Labs, Rotterdam 
// ========================================================================

//      012345678 TAB-O-METER should measure 4
//      ^	^


#include <grace/value.h>
#include <grace/file.h>
#include <grace/stack.h>
#include <grace/strutil.h>
#include <grace/filesystem.h>
#include <grace/stringdict.h>

#include <stdio.h>
#include <string.h>

// ========================================================================
// METHOD ::loadshox
// ========================================================================
void value::loadshox (const string &fname)
{
	string shox;
	shox = fs.load (fname);
	if (shox.strlen()) fromshox (shox);
}

// ========================================================================
// METHOD ::saveshox
// ========================================================================
bool value::saveshox (const string &fname) const
{
	string shox;
	shox = toshox();
	return fs.save (fname, shox);
}

// ========================================================================
// METHOD ::fromshox
// ========================================================================
void value::fromshox (const string &shox)
{
	clear();

	stringdict sdict;
	
	// Verify the 'magic' header
	if (shox.strlen() < 8) return;
	if ((shox[0] != 'S')||(shox[1] != 'H') ||
		(shox[2] != 'o')||(shox[3] != 'X'))
	{
		return;
	}
	
	size_t offs = 6;
	unsigned int t_uint;
	unsigned short t_ushort;
	
	// Read the 'required version' block at offset 6, we're 01.00
	// ourselves.
	offs = shox.binget16u (offs, t_ushort);
	if (! offs) return; // read error
	if (t_ushort > 0x0100) return; // wrong version
	
	// read the number of stringdict entries
	offs = shox.bingetvint (offs, t_uint);
	if (! offs) return;
	
	// read all stringdict strings
	for (unsigned int i=0; i<t_uint; ++i)
	{
		statstring tstat;
		string t;
		
		offs = shox.bingetvstr (offs, t);
		if (! offs) return;
		
		tstat = t;
		sdict.get (tstat);
	}
	
	// Every shox entry has a key, even the root node, although
	// this key is ignored (and actually expected to be 0 although
	// this is not checked or enforced).
	unsigned int ik;
	if (! (offs = shox.bingetvint (offs, ik))) return;
	
	// The readshox function parses every other aspect of a serialized
	// value in the stream except for the key-id.
	readshox (sdict, offs, shox);
}

// ========================================================================
// METHOD ::readshox
// ========================================================================
void value::readshox (stringdict &sdict, size_t &offs, const string &shox)
{
	unsigned char dtype;
	unsigned int ikey;
	unsigned int attrcount;
	unsigned int chcount;
	unsigned int i;
	value *crsr;
	statstring tskey;
	
	// Read the type indicator.
	if (! (offs = shox.binget8u (offs, dtype))) return;
	
	if (dtype & SHOX_HAS_CLASSNAME) // bit 7: object has custom class
	{
		if (! (offs = shox.bingetvint (offs, ikey))) return;
		_type = sdict.get(ikey-1);
	}
	
	if (dtype & SHOX_HAS_ATTRIB) // bit 6: object has attributes
	{
		// Create attributes if necessary
		if (! attrib) attrib = new value;
		
		// Read the number of attributes for the object
		if (! (offs = shox.bingetvint (offs, attrcount))) return;
		
		for (i=0; i<attrcount; ++i)
		{
			// Get the key-id (which is an sdict index) out of the stream.
			if (! (offs = shox.bingetvint (offs, ikey))) return;
			
			// If the key is '0' we deserialize an object with no id,
			// otherwise initialize it with the id looked up in the sdict.
			tskey = sdict.get (ikey-1);
			if (ikey) crsr = &((*attrib)[tskey]);
			else crsr = &(attrib->newval());
			
			// Recurse to read the data into the node.
			crsr->readshox (sdict, offs, shox);
			if (! offs) return;
		}
	}
	
	if (dtype & SHOX_HAS_CHILDREN) // bit 5: object has children
	{
		// Read the child count int.
		if (! (offs = shox.bingetvint (offs, chcount))) return;
		
		for (i=0; i<chcount; ++i)
		{
			// Figure out if there is a key.
			statstring skey;
			if (! (offs = shox.bingetvint (offs, ikey))) return;
			if (ikey)
			{
				// Yes, find the actual string key
				skey = sdict.get (ikey-1);
				crsr = &((*this)[skey]); //findchild (skey.id(), skey.str());
			}
			else crsr = &(newval()); // No, unkeyed value.
			
			// Recurse to read the object data.
			crsr->readshox (sdict, offs, shox);
			if (! offs) return;
		}
	}
	else // No children, read data member.
	{
		int pival;
		unsigned char ucval;
		unsigned int vunsigned;
		int vint;
		long long vlong;
		unsigned long long vulong;
		double vdouble;
		
		switch (dtype & 0x1f)
		{
			case i_unset:
				break;
			
			case i_int:
				if (! (offs = shox.binget32 (offs, vint))) return;
				t.ival = vint;
				break;
			
			case i_ipaddr:
			case i_date:
			case i_unsigned:
				if (! (offs = shox.binget32u (offs, vunsigned))) return;
				t.uval = vunsigned;
				break;
				
			case i_double:
				if (! (offs = shox.bingetieee (offs, vdouble))) return;
				t.dval = vdouble;
				break;
			
			case i_long:
				if (! (offs = shox.binget64 (offs, vlong))) return;
				t.lval = vlong;
				break;
			
			case i_ulong:
				if (! (offs = shox.binget64u (offs, vulong))) return;
				t.ulval = vulong;
				break;
			
			case i_bool:
				if (! (offs = shox.binget8u (offs, ucval))) return;
				t.uval = ucval;
				break;
				
			case i_string:
				if (! (offs = shox.bingetvstr (offs, s))) return;
				break;
		}
		
		itype = (dtype & 0x1f);
	}
}

// ========================================================================
// METHOD ::toshox
// ========================================================================
string *value::toshox (void) const
{
	returnclass (string) res retain;
	size_t offs;
	
	res.strcat ("SHoX");
	offs = res.binput16u (4, 0x0100); // Data format version
	offs = res.binput16u (6, 0x0100); // Minimum required version
	
	string		 shoxdata;
	stringdict	 shoxdict;
	statstring	 tkey;
	
	printshox (shoxdata, shoxdict);
	
	offs = res.binputvint (offs, shoxdict.count());
	for (unsigned int i=0; i<shoxdict.count(); ++i)
	{
		tkey = shoxdict.get (i);
		offs = res.binputvstr (offs, tkey);
	}
	
	res.strcat (shoxdata);
	return &res;
}

// ========================================================================
// METHOD ::printshox
// ========================================================================
void value::printshox (string &outstr, stringdict &sdict) const
{
	if (_name)
	{
		outstr.binputvint (outstr.strlen(), sdict.get (_name) +1);
	}
	else
	{
		outstr.binputvint (outstr.strlen(), 0);
	}
	
	unsigned char xtype = itype;
	if (! value::isbuiltin (_type)) xtype |= SHOX_HAS_CLASSNAME;
	if (attrib && attrib->count()) xtype |= SHOX_HAS_ATTRIB;
	if (arraysz) xtype |= SHOX_HAS_CHILDREN;
	
	outstr.binput8u (outstr.strlen(), xtype);
	
	if (xtype & SHOX_HAS_CLASSNAME)
	{
		outstr.binputvint (outstr.strlen(), sdict.get (_type) +1);
	}
	
	if (xtype & SHOX_HAS_ATTRIB)
	{
		outstr.binputvint (outstr.strlen(), attrib->count());
		
		for (int i=0; i<attrib->count(); ++i)
		{
			attrib->array[i]->printshox (outstr, sdict);
		}
	}
	
	if (xtype & SHOX_HAS_CHILDREN)
	{
		outstr.binputvint (outstr.strlen(), arraysz);
		
		for (unsigned int i=0; i<arraysz; ++i)
		{
			array[i]->printshox (outstr, sdict);
		}
	}
	else
	{
		switch (itype)
		{
			case i_unset:
				break;
			
			case i_int:
				outstr.binput32 (outstr.strlen(), ival());
				break;
				
			case i_ipaddr:
			case i_date:
			case i_unsigned:
				outstr.binput32u (outstr.strlen(), uval());
				break;
			
			case i_double:
				outstr.binputieee (outstr.strlen(), dval());
				break;
				
			case i_long:
				outstr.binput64 (outstr.strlen(), lval());
				break;
				
			case i_ulong:
				outstr.binput64u (outstr.strlen(), ulval());
				break;
				
			case i_bool:
				outstr.binput8u (outstr.strlen(), bval() ? 0xff : 0x00);
				break;
			
			case i_string:
				outstr.binputvstr (outstr.strlen(), sval());
				break;
		}
	}
}
