// ========================================================================
// value_ascii.cpp: Keyed generic data storage class
//
// (C) Copyright 2003-2004 Pim van Riezen <pi@madscience.nl>
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

const char *___SPC = "                                                                                                                                ";

// ========================================================================
// METHOD ::load
// -------------
// Loads the value's data from a file.
// ========================================================================
void value::load (const string &fname)
{
	file f;
	try
	{
		f.openread (fname);
		load (f);
		return;
	}
	catch (...)
	{
		throw valueFileNotFoundException();
	}
	
}

void value::load (file &f)
{
	if (arraysz)
	{
		for (unsigned int i=0; i<arraysz; ++i)
		{
			delete array[i];
		}
		::free (array);
		array = NULL;
		arraysz = 0;
	}
	
	stack<class value> treestack;
	value *crsr = this;
	
	string ln;
	string name;
	value tmp;
	int i;
	
	try
	{
		while (1)
		{
			ln = f.gets();
			
			// Strip leading whitespace
			if (ln)
			{
				for (i=0; (ln[i]==' ')||(ln[i]=='\t'); ++i);
				ln = ln.mid(i);
			}
			
			// Split up the line
			tmp = strutil::splitquoted (ln, ' ');
			
			// Unnamed node of unspecified type
			if (tmp[0] == "+=")
			{
				// Make it that the parser for "=" can pick this up
				tmp[2] = tmp[1];
				tmp[1] = "=";
			}
			
			// Unnamed node with children
			else if (tmp[0] == "+{")
			{
				// Update the stack and cursor
				treestack.push (crsr);
				crsr = &(crsr->newval());
			}
			
			// End of a subtree?
			else if (tmp[0] == "}")
			{
				// Pull back position from the stack
				crsr = treestack.pull();
			}
			
			// Or perhaps it is a named child with its own children?
			else if (tmp[1] == "{")
			{
				if (tmp[0].sval()[0] == '<')
				{
					string dst;
					dst = tmp[0];
					dst = strutil::regexp (dst, "s/<//;s/>//");
					
					treestack.push (crsr);
					crsr = crsr->findchild(resid (dst.str()), NULL);
				}
				else
				{
					treestack.push (crsr);
					crsr = crsr->findchild(tmp[0]);
				}
			}
			
			// Standard assignment
			if (tmp[1] == "=")
			{
				bool named = true;
				bool mnenomic = false;
				string nam;
				nam = tmp[0];
				
				string dst;
				dst = tmp[2];
				
				string mn;
				
				if (nam[0] == '<')
				{
					mn = strutil::regexp (tmp[0].sval(), "s/<//;s/>//");
					mnenomic = true;
					named = false;
				}
				
				// Using this macro we can use the named boolean to determine
				// whether to update a named child node or an unnamed one
				// that has to be created with newval().
				#define CCHILD (named ? (*(crsr->findchild(tmp[0].cval()))) : \
							    mnenomic ? (*(crsr->findchild(resid (mn.str()), NULL))) : \
								crsr->newval())
				
				// Check for unnamed condition
				if (tmp[0] == "+=")
					named = false;
				
				// Check if the value is an ip-address
				if (dst[0] == '<')
				{
					dst = strutil::regexp (dst, "s/<//;s/>//");
					value v;
					v = dst;
					
					CCHILD.setip (v.ipval());
				}
				
				// If it was a string there should be a quote in the source 
				// line. This quote gets yanked by strutil::splitquoted, 
				// which is why we check ln and not tmp[2].
				else if (ln.strchr ('\"') >= 0)
				{
					CCHILD = /*strutil::unescape*/ (tmp[2].sval());
				}
								
				// If we're still in the loop, it means we've seen no quotes
				// and no braces, so it must be an integer or floating point
				// assignment.
				else
				{
					// A decimal point is a sure giveaway
					if (dst.strchr('.') >= 0)
					{
						CCHILD = tmp[2].dval();
					}
					else
					{
						CCHILD = tmp[2].ival();	
					}
				}
				
			}
		}
	}
	catch (...)
	{
		f.close();
		
		// If the stack is not empty, the file was fscked. Whine.
		if (treestack.count())
		{
			throw valueParsingException();
		}
	}
}

// ========================================================================
// METHOD ::decode
// ========================================================================
void value::decode (string &f)
{
	stack<value> treestack;
	value *crsr = this;
	
	string ln;
	string name;
	value tmp;
	int i;
	bool mnenomic = false;
    bool named = true;
	string mn;
	
	while (f.strlen() && (f.strchr('\n')>=0))
	{
		ln = f.cutat('\n');
		
		if (ln.strlen())
		{
			// Strip leading whitespace
			for (i=0; (ln[i]==' ')||(ln[i]=='\t'); ++i);
			ln = ln.mid(i);
			
			// Split up the line
			tmp = strutil::splitquoted (ln, ' ');
			
			// Unnamed node of unspecified type
			if (tmp[0] == "+=")
			{
				// Make it that the parser for "=" can pick this up
				tmp[2] = tmp[1];
				tmp[1] = "=";
			}
			
			// Unnamed node with children
			else if (tmp[0] == "+{")
			{
				// Update the stack and cursor
				treestack.push (crsr);
				crsr = &(crsr->newval());
			}
			
			// End of a subtree
			else if (tmp[0] == "}")
			{
				// Pull back position from the stack
				crsr = treestack.pull();
			}
			
			// Standard assignment
			if (tmp[1] == "=")
			{
				bool named = true;
				string dst;
				dst = tmp[2];
				
				// Check for unnamed condition
				if (tmp[0] == "+=")
					named = false;
				
				// Check if the value is an ip-address
				if (dst[0] == '<')
				{
					dst = strutil::regexp (dst, "s/<//;s/>//");
					value v;
					v = dst;
					
					CCHILD.setip (v.ipval());
				}
				
				// If it was a string there should be a quote in the source line. This
				// quote gets yanked by strutil::splitquoted, which is why we check
				// ln and not tmp[2].
				else if (ln.strchr ('\"') >= 0)
				{
					CCHILD = /*strutil::unescape*/ (tmp[2].sval());
				}
				
				// Or perhaps it is a named child with its own children?
				else if (tmp[1].sval() == "{")
				{
					treestack.push (crsr);
					crsr = crsr->findchild(tmp[0]);
				}
				
				// If we're still in the loop, it means we've seen no quotes
				// and no braces, so it must be an integer or floating point
				// assignment.
				else
				{
					// A decimal point is a sure giveaway
					if (dst.strchr('.') >= 0)
					{
						CCHILD = tmp[2].dval();
					}
					else
					{
						CCHILD = tmp[2].ival();	
					}
				}
			}	
		}
	}
}

// ========================================================================
// METHOD ::save
// -------------
// Stores the value's data in a file.
// ========================================================================
void value::save (const string &fname, bool compact) const
{
	file f;
	if (f.openwrite (fname))
	{
		save (f, compact);
	}
}

void value::save (file &f, bool compact) const
{
	for (unsigned int x=0; x<arraysz; ++x)
	{
		array[x]->print (0, f, compact);
	}
	f.close ();
}

// ========================================================================
// METHOD ::encode
// ========================================================================
string *value::encode (bool compact) const
{
	returnclass (string) res retain;
	
	for (unsigned int x=0; x<arraysz; ++x)
	{
		array[x]->printstr (0, res, compact);
	}
	
	return &res;
}


// ========================================================================
// METHOD ::print
// --------------
// Formats and idents a human-readable printout of the value's data.
// ========================================================================
void value::print (int indent, file &out, bool compact) const
{
	string outstr;
	string nm;
	
	// Part 1, define the name/value of the key, if any
	
	if (_name) // A string was defined
	{
		nm = _name;
		nm += " ";
	}
	else // Numeric or no key
	{
		if (key & 0xff000000) // Full numeric key
		{
			nm.printf ("#%08x ", key);
		}
		else if (key) // Mnenomic key
		{
			nm.printf ("<%c%c%c%c> ",
					   restochar ((key & 0x00fc0000) >> 18),
					   restochar ((key & 0x0003f000) >> 12),
					   restochar ((key & 0x00000fc0) >> 6 ),
					   restochar ((key & 0x0000003f)      ));
		}
		else // No key
		{
			nm = "+";
		}
	}
	
	// Print out the data according to its type
	
	if (itype == i_double)
	{
		if (!compact) outstr.printf (___SPC+(128-indent));
		outstr.printf ("%s= %f\n", nm.str(), t.dval);
		
	}
	else if (itype == i_string)
	{
		if ((!arraysz)||(s.strlen()))
		{
			outstr.crop();
			if (!compact) outstr.printf (___SPC+(128-indent));
			outstr.printf ("%s= \"%S\"\n", nm.str(), s.str());
		}
		
	}
	else if (itype == i_int)
	{
		if (!compact) outstr.printf (___SPC+(128-indent));
		outstr.printf ("%s= %i\n", nm.str(), t.ival);
	}
	else if (itype == i_ipaddr)
	{
		if (!compact) outstr.printf (___SPC+(128-indent));
		outstr.printf ("%s= <%i.%i.%i.%i>\n",
						nm.str(),
						(t.uval & 0xff000000) >> 24,
						(t.uval & 0x00ff0000) >> 16,
						(t.uval & 0x0000ff00) >> 8,
						t.uval & 0x000000ff);
	}
	else if (! arraysz)
	{
		if (!compact) outstr.printf (___SPC+(128-indent));
		outstr.printf ("%s= \"\"\n", nm.str());
	}
		
	// Print out any children/array members
	
	if (arraysz)
	{
		if (!compact) outstr.printf (___SPC+(128-indent));
		outstr.printf ("%s{\n", nm.str());
		
		out.puts (outstr);
		
		for (unsigned int i=0; i<arraysz; ++i)
		{
			array[i]->print (indent+2, out, compact);
		}
		
		if (!compact) out.printf ("%s}\n", ___SPC+(128-indent));
		else out.printf ("}\n");
	}
	else
	{
		out.puts (outstr);
	}
}

// ========================================================================
// METHOD ::printstr
// ========================================================================
void value::printstr (int indent, string &out, bool compact) const
{
	string outstr;
	string nm;
	
	// Part 1, define the name/value of the key, if any
	
	if (_name) // A string was defined
	{
		nm = _name;
		nm += " ";
	}
	else // Numeric or no key
	{
		if (key & 0xff000000) // Full numeric key
		{
			nm.printf ("#%08x ", key);
		}
		else if (key) // Mnenomic key
		{
			nm.printf ("<%c%c%c%c> ",
					   restochar ((key & 0x00fc0000) >> 18),
					   restochar ((key & 0x0003f000) >> 12),
					   restochar ((key & 0x00000fc0) >> 6 ),
					   restochar ((key & 0x0000003f)      ));
		}
		else // No key
		{
			nm = "+";
		}
	}
	
	// Print out the data according to its type
	
	if (itype == i_double)
	{
		if (!compact) outstr.printf (___SPC+(128-indent));
		outstr.printf ("%s= %f\n", nm.str(), t.dval);
	}
	else if (itype == i_string)
	{		
		if ((!arraysz)||(s.strlen()))
		{
			if (!compact) outstr.printf (___SPC+(128-indent));
			outstr.printf ("%s= \"%S\"\n", nm.str(), s.str());
		}
	}
	else if (itype == i_int)
	{			
		if (!compact) outstr.printf (___SPC+(128-indent));
		outstr.printf ("%s= %i\n", nm.str(), t.ival);
	}
	else if (itype == i_ipaddr)
	{	
		if (!compact) outstr.printf (___SPC+(128-indent));
		outstr.printf ("%s= <%i.%i.%i.%i>\n",
						nm.str(),
						(t.uval & 0xff000000) >> 24,
						(t.uval & 0x00ff0000) >> 16,
						(t.uval & 0x0000ff00) >> 8,
						t.uval & 0x000000ff);
	}
	else if (! arraysz)
	{		
		if (!compact) outstr.printf (___SPC+(128-indent));
		outstr.printf ("%s= \"\"\n", nm.str());
	}
	
	// Print out any children/array members
	
	if (arraysz)
	{
		if (!compact) outstr.printf (___SPC+(128-indent));
		outstr.printf ("%s{\n", nm.str());
		
		if (outstr.strlen())
			out.strcat (outstr.str());
		
		for (unsigned int i=0; i<arraysz; ++i)
		{
			array[i]->printstr (indent+2, out);
		}
		
		if (!compact) out.printf ("%s}\n", ___SPC+(128-indent));
		else out.strcat ("}\n");
	}
	else
	{
		if (outstr.strlen()) 
			out.strcat (outstr.str());
	}
}
