// ========================================================================
// value_php.cpp: Handling for serialized PHP arrays and objects.
//
// (C) Copyright 2005-2006 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam 
// ========================================================================

//      012345678 TAB-O-METER should measure 4
//      ^	^

#include <grace/value.h>
#include <grace/file.h>
#include <grace/stack.h>
#include <grace/strutil.h>
#include <grace/filesystem.h>

#include <stdio.h>
#include <string.h>

// Macro for skipping a cursor over an expected byte or chickening out
// if it was unexpectedly not found on the cursor location.
#define SKIPEXPECTED(ch) { if ((*crsr) == ch) ++crsr; else return NULL; }

// ========================================================================
// METHOD ::phpdeserialize
// -----------------------
// Loads the value's data from a serialized PHP string.
// ========================================================================
void value::phpdeserialize (const string &phpdata)
{
	phpdeserialize (phpdata.str(), false);
}

const char *value::phpdeserialize (const char *phpdata, bool recursed)
{
	enum pstate {
	  parse_key,
	  parse_value
	};
	
	statstring theKey;
	string theValue;
	string tmp;
	int itmp;
	double dtmp;
	bool numkey = false;
	bool datakey = false;
	
	pstate st = recursed ? parse_key : parse_value;
	
	const char *crsr = phpdata;
	const char *oldcrsr;
	char c;
	
	while (crsr && (*crsr))
	{
		if ((st == parse_value) && (theKey == ".data"))
		{
			datakey = true;
		}
		else datakey = false;
		
		c = (char) *crsr;
		if (c == '}')
		{
			if (st == parse_key) return crsr+1;
			return NULL;
		}
		else switch (c)
		{
			case 'N': // NULL
				// An value cannot have a null-key.
				if (st == parse_key) return NULL;
				crsr++;
				SKIPEXPECTED(';');
				if (datakey) (*this) = "";
				else if (numkey) (*this).newval() = "";
				else (*this)[theKey] = "";
				st = parse_key;
				break;
				
			case 'O': // OBJECT
				// An object can't be a key.
				if (st == parse_key) return NULL;
				
				crsr++;
				SKIPEXPECTED(':');

				while (::isdigit (*crsr)) crsr++;
				SKIPEXPECTED(':');
				
				// Skip class name, we don't care.
				while ((*crsr) && ((*crsr) != ':')) ++crsr;
				--crsr;
				
				// ACHTUNG! CASE ROLLS OVER BY DESIGN!
				
			case 'a': // ARRAY
				// An array can never be a key, give up.
			    if (st == parse_key) return NULL;
			    
				crsr++;
				SKIPEXPECTED(':');
				
				// The item count is completely boring to know,
				// it would've been more useful to get a byte count
				// here, but I'm getting the feeling here that the
				// PHP-coders added these 'size' fields to their
				// serialization data because it gave them a
				// hard-on, not because it's actually helping them
				// parse it more efficiently.
				while (::isdigit (*crsr)) crsr++;
				SKIPEXPECTED(':');
				SKIPEXPECTED('{');
	
				// Recurse
				if (theKey)
				{
					if (numkey)
					{
						crsr = (*this).newval().phpdeserialize (crsr,true);
					}
					else
					{
						if (theKey == ".attr")
						{
							if (! attrib) attrib = new value;
							crsr = attrib->phpdeserialize (crsr,true);
						}
						else
						{
							crsr = (*this)[theKey].phpdeserialize (crsr,true);
						}
					}
				}
				else
				{
					// TODO: I bet they don't handle numbered elements
					//       quite like this. Should test this with
					//       sparse PHP arrays on a rainy day.
					crsr = phpdeserialize (crsr, true);
				}
				
				// End of the line.
				if (! crsr) return NULL;
				
				// Next on the agenda, a fresh key to read.
				st = parse_key;
				break;
			
			case 'i': // INTEGER
				crsr++;
				SKIPEXPECTED(':');
				itmp = ::atoi (crsr);
				while (::isdigit (*crsr)) crsr++;
				SKIPEXPECTED(';');
				if (st == parse_key)
				{
					numkey = true;
					tmp.crop();
					tmp.printf ("%i", itmp);
					theKey = tmp;
					st = parse_value;
				}
				else
				{
					if (datakey) (*this) = itmp;
					else if (numkey) (*this).newval() = itmp;
					else (*this)[theKey] = itmp;
					st = parse_key;
				}
				break;
			
			case 's': // STRING
				crsr++;
				SKIPEXPECTED(':');
				itmp = ::atoi (crsr);
				while (::isdigit (*crsr)) crsr++;
				SKIPEXPECTED(':');
				SKIPEXPECTED('\"');
				oldcrsr = crsr;
				if (::strlen (crsr) < itmp) return NULL;
				tmp.strcpy (crsr, itmp);
				crsr += itmp;
				if (*crsr != '\"')
				{
					::printf ("Expected quote, got '%c'\n", *crsr);
					{
						printf ("size provided was %i, real size was %i\n",
								itmp, strstr (oldcrsr, "\";") - oldcrsr);
						
						return NULL;
					}
					/*if (*(crsr+1) == '\"')
					{
						::printf ("off-by-one, fixing\n");
						crsr++;
					}
					else
					{*/
						::printf ("fubar'ed\n");
						::printf ("text from cursor position:\n%s\n", crsr);
						return NULL;
					//}
				}
				SKIPEXPECTED('\"');
				SKIPEXPECTED(';');
				if (st == parse_key)
				{
					numkey = false;
					theKey = tmp;
					st = parse_value;
				}
				else
				{
					if (datakey) (*this) = tmp;
					else if (numkey) (*this).newval() = tmp;
					else (*this)[theKey] = tmp;
					st = parse_key;
				}
				break;
				
			case 'b': // BOOLEAN
				crsr++;
				SKIPEXPECTED(':');
				itmp = ::atoi (crsr);
				while (::isdigit (*crsr)) crsr++;
				SKIPEXPECTED(';');
				if (st == parse_key)
				{
					return NULL;
				}
				else
				{
					if (datakey) (*this) = (bool) itmp;
					else if (numkey)
						(*this).newval() = (bool) itmp;
					else
						(*this)[theKey] = (bool) itmp;
						
					st = parse_key;
				}
				break;
			
			case 'd': // FLOAT
				crsr++;
				SKIPEXPECTED(':');
				dtmp = ::atof (crsr);
				//::printf ("parsing 'd' value: %f\n", dtmp);
				while ((::isdigit (*crsr))||((*crsr)=='.')||((*crsr)=='-'))
					 crsr++;
				SKIPEXPECTED(';');
				if (st == parse_key)
				{
					numkey = true;
					tmp.crop();
					tmp.printf ("%f", dtmp);
					theKey = tmp;
					st = parse_value;
				}
				else
				{
					if (datakey)
					{
						(*this) = dtmp;
						(*this).cval();
					}
					else if (numkey)
					{
						(*this).newval() = dtmp;
						(*this)[-1].cval();
					}
					else
					{
						(*this)[theKey] = dtmp;
						(*this)[theKey].cval();
					}
					st = parse_key;
				}
				break;
				
			
			default:
				return NULL;
		}
	}
	
	return NULL;
}

// ========================================================================
// METHOD ::phpserialize
// ---------------------
// Tries to make a sensible representation of the data in serialized PHP
// talk. If the withattr argument is set to true, attributes are also
// taken into the equasion; In those cases every exported node will
// have an extra layer in the form of two children called ".data" and
// ".attr" for every exported object that has attributes.
// ========================================================================
string *value::phpserialize (bool withattr)
{
	string *result = new string;
	printphp (*result, withattr);
	return result;
}

// ========================================================================
// METHOD ::printphp
// -----------------
// The actual brains behind the phpserialize() operation.
// ========================================================================
void value::printphp (string &into, bool withattr)
{
	unsigned int marraysz;
	marraysz = arraysz + ((attrib&&withattr) ? attrib->count() : 0);
	if (withattr && (! arraysz)) marraysz++;
	if (marraysz)
	{
		into.printf ("a:%i:{", marraysz);
		for (int i=0; i<arraysz; ++i)
		{
			if (array[i]->id())
			{
				into.printf ("s:%i:\"%s\";", array[i]->id().strlen(),
							 array[i]->name());
			}
			else
			{
				into.printf ("i:%i;", i);
			}
			array[i]->printphp (into, withattr);
		}
		if (withattr && (attrib && attrib->count()))
		{
			into.printf ("s:5:\".attr\";");
			attrib->printphp (into, withattr);
		}
		if (arraysz)
		{
			into.printf ("}");
			return;
		}
		if (withattr) into.printf ("s:5:\".data\";");
	}
	switch (itype)
	{
		case i_unsigned:
		case i_int:
			into.printf ("i:%i;", ival());
			break;
		case i_bool:
			into.printf ("b:%i;", bval() ? 1 : 0);
			break;
		default:
			into.printf ("s:%i:\"", sval().strlen());
			into.strcat (sval());
			into.strcat ("\";");
			break;
	}
	if (marraysz && withattr) into.printf ("}");
}

