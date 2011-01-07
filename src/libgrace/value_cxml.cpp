// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

// ========================================================================
// value_cxml.cpp: Binary compressed XML dumper/parser
// ========================================================================

#include <grace/str.h>
#include <grace/statstring.h>
#include <grace/value.h>
#include <grace/valueindex.h>
#include <grace/xmlschema.h>

// ------------------------------------------------------------------------
// I could theoretically combine these with the _itype enum, but CXML/DAAP
// use slightly different type conventions, so this will make it easier
// to stay compatible.
// ------------------------------------------------------------------------
#define T_CXML_CHAR 1
#define T_CXML_UCHAR 2
#define T_CXML_SHORT 3
#define T_CXML_USHORT 4
#define T_CXML_LONG 5
#define T_CXML_ULONG 6
#define T_CXML_LONG_LONG 7
#define T_CXML_ULONG_LONG 8
#define T_CXML_STRING 9
#define T_CXML_DATE 10
#define T_CXML_VERSION 11
#define T_CXML_CONTAINER 12
#define T_CXML_CURRENCY 13

// ------------------------------------------------------------------------
// foo-2-little-endian macros
// ------------------------------------------------------------------------
#define CV64(foo,offs) (((long long)((unsigned char)foo[offs])<<56)|\
						((long long)((unsigned char)foo[offs+1])<<48)|\
						((long long)((unsigned char)foo[offs+2])<<40)|\
                        ((long long)((unsigned char)foo[offs+3])<<32)|\
                        ((long long)((unsigned char)foo[offs+4])<<24)|\
                        ((long long)((unsigned char)foo[offs+5])<<16)|\
                        ((long long)((unsigned char)foo[offs+6])<<8)|\
                        ((long long)((unsigned char)foo[offs+7])))
#define CVU64(foo,offs) (((unsigned long long)((unsigned char)foo[offs])<<56)|\
						 ((unsigned long long)((unsigned char)foo[offs+1])<<48)|\
						 ((unsigned long long)((unsigned char)foo[offs+2])<<40)|\
                         ((unsigned long long)((unsigned char)foo[offs+3])<<32)|\
                         ((unsigned long long)((unsigned char)foo[offs+4])<<24)|\
                         ((unsigned long long)((unsigned char)foo[offs+5])<<16)|\
                         ((unsigned long long)((unsigned char)foo[offs+6])<<8)|\
                         ((unsigned long long)((unsigned char)foo[offs+7])))
#define CV32(foo,offs) ((((unsigned char)foo[offs])<<24)|(((unsigned char)foo[offs+1])<<16)|\
						(((unsigned char)foo[offs+2])<<8)|(((unsigned char)foo[offs+3])))
#define CVU32(foo,offs)(((unsigned)((unsigned char)foo[offs])<<24)|\
						((unsigned)((unsigned char)foo[offs+1])<<16|\
						((unsigned)((unsigned char)foo[offs+2])<<8)|\
						((unsigned)((unsigned char)foo[offs+3]))))
#define CV16(foo,offs) (((short)((unsigned char)foo[offs])<<8)|((short)((unsigned char)foo[offs+1])))
#define CVU16(foo,offs) (((unsigned short)((unsigned char)foo[offs])<<8)|\
						((unsigned short)((unsigned char)foo[offs+1])))


// ========================================================================
// FUNCTION cxmltype
// -----------------
// Resolves a statstring type to its CXML integer counterpart
// ========================================================================
int cxmltype (const string &type)
{
	static value *types = new value;
	if (! types->exists (t_unset))
	{
		(*types)[t_unset] = 0;
		(*types)[t_char] = 1;
		(*types)[t_bool] = 1;
		(*types)[t_uchar] = 2;
		(*types)[t_short] = 3;
		(*types)[t_ushort] = 4;
		(*types)[t_int] = 5;
		(*types)[t_unsigned] = 6;
		(*types)[t_long] = 7;
		(*types)[t_date] = 10;
		(*types)[t_ulong] = 8;
		(*types)[t_bool] = 1;
		(*types)[t_ipaddr] = 9;
		(*types)[t_array] = 12;
		(*types)[t_dict] = 12;
		(*types)[t_string] = 9;
		(*types)[t_currency] = 13;
	}
	if (! types->exists (type)) return 0;
	return (*types)[type].ival();
}

// ========================================================================
// METHOD ::loadxml
// ----------------
// Open an xml-file and translate it.
// ========================================================================
void value::fromcxml (string &from, xmlschema &schema)
{
	if (from.strlen() < 8) return;
	
	parsecompressed (0, from, schema);
}

// ========================================================================
// METHOD ::parsecompressed
// ------------------------
// Load in a compressed CXML bytestream.
// ========================================================================
size_t value::parsecompressed (size_t offs, string &from, xmlschema &schema)
{
	char opcode[5];
	char copcode[5];
	size_t vsz;
	unsigned int val;
	size_t crsr;
	
	// Do we fit?
	if (from.strlen() < (offs+8)) return from.strlen();
	
	// Read our opcode
	opcode[0] = from[offs];
	opcode[1] = from[offs+1];
	opcode[2] = from[offs+2];
	opcode[3] = from[offs+3];
	opcode[4] = 0;
	
	// Read our size
	vsz = CVU32(from,offs+4);
	
	// Do we still fit?
	if (from.strlen() < (offs+8+vsz))
	{
		return from.strlen();
	}
	
	// Set the cursor to the content part
	crsr = offs+8;
	
	_type = schema.xmlclass (opcode);
	
	if (schema.hasimplicit (opcode))
	{
		_name = schema.implicitid (opcode);
	}
	
	int opcodeType = cxmltype(schema.xmltype (opcode));
	
	// Handle the simple builtin types if applicable
	if ((vsz>0) && (opcodeType == T_CXML_CHAR))
	{
		char val = from[crsr];
		(*this) = (int) val;
		return crsr+1;
	}
	if ((vsz>1) && (opcodeType == T_CXML_SHORT))
	{
		short val = CV16(from,crsr);
		(*this) = val;
		return crsr+2;
	}
	if ((vsz>3) && (opcodeType == T_CXML_LONG))
	{
		int val = CV32(from,crsr);
		(*this) = val;
		return crsr+4;
	}
	if (opcodeType == T_CXML_STRING)
	{
		string tstr;
		tstr.strcpy (from.cval() + crsr, vsz);
		(*this) = tstr;
		return crsr+vsz;
	}
	if ((vsz>7) && (opcodeType == T_CXML_LONG_LONG))
	{
		long long lval = CV64(from,crsr);
		(*this) = lval;
		return crsr+8;
	}
	if ((vsz>7) && (opcodeType == T_CXML_CURRENCY))
	{
		long long lval = CV64(from,crsr);
		(*this).setcurrency (lval);
		return crsr+8;
	}
	if ((vsz>0) && (opcodeType == T_CXML_UCHAR))
	{
		unsigned char val = from[crsr];
		(*this) = (unsigned int) val;
		return crsr+1;
	}
	if ((vsz>1) && (opcodeType == T_CXML_USHORT))
	{
		unsigned short val = CVU16(from,crsr);
		(*this) = val;
		return crsr+2;
	}
	if ((vsz>3) && (opcodeType == T_CXML_ULONG))
	{
		unsigned int val = CVU32(from,crsr);
		(*this) = val;
		return crsr+4;
	}
	if ((vsz>7) && (opcodeType == T_CXML_ULONG_LONG))
	{
		unsigned long long lval = CVU64(from,crsr);
		(*this) = lval;
		return crsr+8;
	}
	
	// The CXML_CONTAINER type is a bit more complicated
	if (opcodeType == T_CXML_CONTAINER)
	{
		string resvOpcode;
		unsigned int csz;
		bool wasprecedent = false;
		
		// Read child nodes until we're out of space
		while ((crsr+8) < (vsz+offs+8))
		{
			value *mychild = NULL;
			
			// Get the child opcode
			copcode[0] = from[crsr];
			copcode[1] = from[crsr+1];
			copcode[2] = from[crsr+2];
			copcode[3] = from[crsr+3];
			copcode[4] = 0;
			
			// Get the child size
			csz = CVU32(from,crsr+4);			
			
			crsr+= 8;
			// Bail if the child won't fit
			if ((crsr+csz) > (vsz+offs+8)) break;
			
			if (schema.isattribute (copcode)) // is an attribute
			{
				string aval;
				int tval;
				short sval;
				unsigned short usval;
				long long llval;
				unsigned int utval;
				unsigned long long ullval;
				
				// get the attribute as into a string
				
				switch (cxmltype(schema.xmltype (copcode)))
				{
					case T_CXML_CHAR:
						if (csz) tval = from[crsr];
						aval.printf ("%i", tval);
						break;
					
					case T_CXML_SHORT:
						if (csz>1) sval = CV16(from,crsr);
						aval.printf ("%i", sval);
						break;
						
					case T_CXML_LONG:
						if (csz>3) tval = CV32(from,crsr);
						aval.printf ("%i", tval);
						break;
						
					case T_CXML_LONG_LONG:
						if (csz>7) llval = CV64(from,crsr);
						aval.printf ("%L", llval);
						break;
						
					case T_CXML_STRING:
						if (csz)
						{
							aval.strcpy (from.cval()+crsr, csz);
						}
						break;
					
					case T_CXML_UCHAR:
						if (csz) utval = from[crsr];
						aval.printf ("%u", utval);
						break;
					
					case T_CXML_USHORT:
						if (csz>1) usval = CVU16(from,crsr);
						aval.printf ("%u", usval);
						break;
						
					case T_CXML_ULONG:
						if (csz>3) utval = CVU32(from,crsr);
						aval.printf ("%u", utval);
						break;
					
					case T_CXML_ULONG_LONG:
						if (csz>7) ullval = CVU64(from,crsr);
						aval.printf ("%U", ullval);
						break;
					
					case T_CXML_CURRENCY:
						if (csz>7) llval = CV64(from,crsr);
						printcurrency (aval, llval);
						break;
						
					default:
						aval.crop (0);
						break;
				}
				
				// is this an id field?
				if (schema.isindex (copcode))
				{
					if (schema.isprecedent (copcode))
					{
						(*this)[aval].type
							(schema.precedentclass (copcode));
						wasprecedent = true;
					}
					else
					{
						_name = aval;
					}
				}
				else // no, a regular attribute
				{
					if (schema.isprecedent (copcode))
					{
						if (! wasprecedent)
							(*this).newval (schema.precedentclass (copcode));
						
						(*this)[-1].setattrib
							(schema.attributelabel (copcode), aval);
						wasprecedent = true;
					}
					else
					{
						setattrib (schema.attributelabel (copcode), aval);
					}
				}
			}
			else // not an attribute, so it's a child node
			{
				if (_type == t_unset) _type = t_dict;
				value *v = new value;
				v->parsecompressed (crsr-8, from, schema);
				if (wasprecedent)
				{
					(*this)[-1] = v;
					wasprecedent = false;
				}
				else
				{
					if (v->_name) (*this)[v->_name] = v;
					else this->newval(v->type()) = v;
				}
			}
			crsr += csz;
		}
		return crsr;
	}
	return from.strlen();
}

string *value::tocxml (xmlschema &schema)
{
	returnclass (string) into retain;
	value *root = new value;
	
	printcompressed (0, into, *root, schema);
	delete root;
	return &into;
}

// ========================================================================
// METHOD ::compressbuiltin
// ------------------------
// Prints a builtin valuetype with an opcode to a CXML bytestream.
// ========================================================================
size_t value::compressbuiltin (size_t offs, string &into,
								 const char *opcode,
								 const statstring &ptype,
								 const value &v) const
{
	int ctype = cxmltype(ptype);
	size_t crsr = offs;
	
	// Not much to consider about signedness, we're doing a binary
	// dump with the endian nastiness tucked away.
	
	switch (ctype)
	{
		case T_CXML_CHAR:
		case T_CXML_UCHAR:
			crsr = into.binputnum8 (crsr,opcode,v.uval() & 0xff);
			break;
		case T_CXML_SHORT:
		case T_CXML_USHORT:
			crsr = into.binputnum16 (crsr,opcode,v.uval() & 0xffff);
			break;
		case T_CXML_LONG:
		case T_CXML_ULONG:
		case T_CXML_DATE:
			crsr = into.binputnum32 (crsr,opcode,v.uval());
			break;
		case T_CXML_STRING:
			crsr = into.binputstr (crsr,opcode,v.sval());
			break;
		case T_CXML_LONG_LONG:
		case T_CXML_ULONG_LONG:
			crsr = into.binputnum64 (crsr,opcode,v.ulval());
			break;
		case T_CXML_CURRENCY:
			crsr = into.binputnum64 (crsr,opcode,v.getcurrency());
			break;
		default:
			break;
	}
	
	return crsr;
}

// ========================================================================
// METHOD ::printcompressed
// ------------------------
// Writes all elements of the object to a CXML bytestream.
// ========================================================================
size_t value::printcompressed (size_t _offs, string &into, const value &parent,
							   xmlschema &schema) const
{
	size_t crsr = _offs;
	size_t offs = _offs;
	statstring opcodelabel;
	statstring exportid;
	bool needprecedents = false;
	
	// Only T_CXML_CONTAINER objects can have their key/attributes inside,
	// so for other basic types an attribute is defined as a value
	// preceeding the actual value. The parser on the other end will have
	// to know these attributes are 'precedents' so they can keep them on 
	// ice until the actual value arrives.
	//
	// The fact that a container has its attributes defined within is to 
	// make it easier to deal with Apple's cute but definitely asshole 
	// implementation of the compressed protocol in DAAP, which is derived 
	// from their butthole plists, which are also cute in their way but 
	// lack the entire concept of XML attributes so they could be lazy and 
	// use the much less flexible NSDictionary class and its sibling plist 
	// format as the root of all evil.
	
	schema.resolveclass (_name, _type, parent._name,
						 parent._type, opcodelabel);
	
	if (! schema.knownclass (opcodelabel)) return crsr;
	
	needprecedents = schema.wouldneedprecedents (opcodelabel);
	
	exportid = schema.resolveidexport
					(_name, _type, parent._type, parent._name);
	
	if (needprecedents) // see above
	{
		if (exportid) // is there a key associated with this object?
		{
			statstring indexfield;
			statstring indextype;
			value theindex;
			
			// although index is a string, CXML may define it to be an int
			// type, in that case we must convert it. I may resurrect the
			// ability to get a purely key-based child in the near future
			// then this will start making more sense.
			//
			// for now, it satisfies at least the conditions to communicate
			// with, say, DAAP clients.
			
			indexfield = schema.resolveindexname (opcodelabel);
			indextype = schema.resolvetypeattrib
							(opcodelabel, indexfield);
			
			// put it in a value object, the quickest way to get it
			// converted to whatever the schema defines
			theindex = _name.str();
			const char *thecode = schema.resolvecodeid (opcodelabel);
			
			if (thecode)
			{
				crsr = compressbuiltin ((unsigned int) crsr, into,
										thecode,
										indextype,
										theindex);
				offs = crsr; // these are precedents, they are not to be part
							 // of the main object's size calculations
			}
		}
		if (attrib)
		{
			for (int i=0; i < attrib->count(); ++i)
			{
				statstring atype;
				const char *thecode;
				
				// convert the attributes to the schema-define CXML format
				// and write it down.
				atype = schema.resolvetypeattrib
							(opcodelabel, (*attrib)[i]._name);
				
				thecode = schema.resolvecodeattrib
							(opcodelabel,(*attrib)[i]._name);
				
				if (thecode)
				{
					crsr = compressbuiltin (crsr,
								into, thecode,
								atype, (*attrib)[i]);
					offs = crsr;
				}
			}
		}
	}
	
	// put in the header for the main object, including a 32-bit placeholder
	// that will later be used to store the object's size.
	crsr = into.binputopc (offs, schema.resolvecode (opcodelabel));
	crsr = into.binput32u (crsr, 0);
	
	if (! needprecedents) // this is a dictionary type
	{
		if (exportid) // first dump our key if we have it
		{
			statstring indexfield;
			statstring indextype;
			value theindex;
			
			indexfield = schema.resolveindexname (opcodelabel);
			indextype = schema.resolvetypeattrib
							(opcodelabel, indexfield);
			theindex = _name.str();

			crsr = compressbuiltin (crsr, into,
									schema.resolvecodeid (opcodelabel),
									indextype,
									theindex);
		}
		if (attrib) // also dump any attributes
		{
			for (int i=0; i < attrib->count(); ++i)
			{
				statstring atype;
				atype = schema.resolvetypeattrib
							(opcodelabel, (*attrib)[i]._name);
				crsr = compressbuiltin (crsr, into,
							schema.resolvecodeattrib (opcodelabel,
									(*attrib)[i]._name),
							atype, (*attrib)[i]);
			}
		}
		if (arraysz) // for god's sake, think of the children!
		{
			for (unsigned int i=0; i < arraysz; ++i)
			{
				// recursion: see recursion
				crsr = array[i]->printcompressed (crsr, into, *this, schema);
			}
		}
	}
	else // ok so we are a basic builtin type, let's move on then shall we?
	{
		// Note how we will not ponder about the array. There can be no
		// array here. If there is, why was it tagged as a <string> or
		// whatever? Sheez, make an attribute called "data" or something,
		// we're not in HTMLala-land anymore.
		
		crsr = compressbuiltin (offs, into, schema.resolvecode (opcodelabel),
								schema.resolvetype (opcodelabel), *this);
	}
	
	into.binput32u (offs+4, crsr-(offs+8));
	return crsr;
}

