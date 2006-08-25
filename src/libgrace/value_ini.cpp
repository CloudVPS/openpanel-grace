#include <grace/value.h>
#include <grace/file.h>
#include <grace/strutil.h>

// =======================================================================
// METHOD ::loadini
// ----------------
// Parses a Windows-style INI file into an associative array. Sections
// are parsed "flat", resulting in a two-dimensional array of
//
//    myvalue[sectionName][parameterName]
// =======================================================================
bool value::loadini (const string &filename)
{
	file f;
	string line;
	value split;
	statstring section;
	string pName, pVal;

	// No file? No data!	
	if (! f.openread (filename)) return false;
	
	try
	{
		while (! f.eof())
		{
			line = f.gets();
			if (line.strlen())
			{
				if (line[0]=='[') // section header
				{
					line = line.mid (1, line.strlen() - 2);
					section = line;
				}
				else if (line[0] != '#')  // No comment
				{
					int iVal;
					string isVal;
					
					split = iniparse (line);
					pName = split[0];
					pVal  = split[1];
					iVal  = split[1];
					
					// If we found an integer value, does that represent
					// the entirety of the string value?
					if (iVal)
					{
						isVal.printf ("%i", iVal);
						if (pVal != isVal) iVal = 0;
					}
					
					if (pName.strlen())
					{
						if (iVal) // cast as integer?
						{
							if (section)
								(*this)[section][pName] = iVal;
							else
								(*this)[pName] = iVal;
						}
						else // no, cast as string
						{
							if (section)
								(*this)[section][pName] = pVal;
							else
								(*this)[pName] = pVal;
						}
					}
				}
			}
		}
	}
	catch (...)
	{
	}
	f.close();
	return true;
}

// =======================================================================
// METHOD ::loadinitree
// ---------------------
// Loads a Windows style INI file. Sections can have a tree structure
// by using the colon (':') as a branch point. For example:
//
// [Main]
// foo=bar
// [Main:extras]
// quux=baz
//
// Would result in a tree like this:
//
// <dict id="Main">
//   <string id="foo">bar</string>
//   <dict id="extras">
//     <string id="quux">baz</string>
//   </dict>
// </dict>
// =======================================================================
bool value::loadinitree (const string &filename)
{
	file f;
	string line;
	value *section = this;
	value split;
	string pName, pVal;
	
	if (! f.openread (filename)) return false;
	
	try
	{
		while (! f.eof())
		{
			line = f.gets();
			if (line.strlen())
			{
				if (line[0]=='[') // section
				{
					line = line.mid (1, line.strlen() - 2);
					split = strutil::split (line, ':');
					section = this;
					for (int ii=0; ii<split.count(); ++ii)
					{
						section = section->findchild (split[ii].sval());
						section->type (t_dict);
					}
				}
				else if (line[0]!='#') 
				{
					int iVal;
					string isVal;
					
					split = iniparse (line);
					pName = split[0];
					pVal  = split[1];
					iVal = split[1];
					
					if (iVal)
					{
						isVal.printf ("%i", iVal);
						if (isVal != pVal) iVal = 0;
					}
					
					if (pName.strlen())
					{
						if (iVal)
							(*section)[pName] = iVal;
						else
							(*section)[pName] = pVal;
					}
				}
			}
		}
	}
	catch (...)
	{
	}
	f.close();
	return true;
}

// =======================================================================
// METHOD ::iniparse
// -----------------
// Parses a "name=value" line from a Windows style inifile paying
// attention to specific details. There is no formal specification for
// ini-files, so parsing has to take a number of things into account:
//
// * Parameter names might be prepended by whitespace
// * There may be whitespace before or after the '=' that is between the
//   parameter name and value
// * Parameter values may or may not be surrounded by quotes
//
// The method returns a temporary value object containing an array with
// two elements. The first element is the parameter name, the second
// is its value.
// =======================================================================
value *value::iniparse (const string &line)
{
	string nam;
	string val;
	value *result = new value;
	int nmleft, nmright;
	int lastnonspace;
	bool goteq, gotnmstart, gotvastart;
	char vaquote;
	
	nmleft = nmright = lastnonspace = 0;
	goteq = gotnmstart = gotvastart = false;
	vaquote = 0;
	
	for (unsigned int i=0; i<line.strlen(); ++i)
	{
		if (isspace (line[i]))
		{
			if (! goteq)
			{
				if (! gotnmstart) nmleft++;
			}
			else
			{
				if (gotvastart) val.strcat (' ');
			}
		}
		else if (line[i]=='=')
		{
			if (goteq) val.strcat ('=');
			else
			{
				nmright = lastnonspace+1;
				goteq = true;
				nam = line.mid (nmleft, nmright-nmleft);
			}
		}
		else if ( (!vaquote) && ((line[i] == ';')||(line[i] == '#')) )
		{
			i = line.strlen();
		}
		else
		{
			if (! goteq)
			{
				if (! gotnmstart)
				{
					gotnmstart = true;
					nmleft = i;
				}
				lastnonspace = i;
			}
			else
			{
				if (line[i] == '\\')
				{
					++i;
					val.strcat (line[i]);
				}
				else if (vaquote)
				{
					if (line[i] == vaquote)
						i = line.strlen();
					else
						val.strcat (line[i]);
				}
				else if (line[i] == '\"')
				{
					vaquote = line[i];
					gotvastart = true;
				}
				else
				{
					val.strcat (line[i]);
					gotvastart = true;
				}
			}
		}
	}
	if (! goteq)
	{
		if (nmright - nmleft > 0) nam = line.mid (nmleft, nmright-nmleft);
	}
	(*result).newval() = nam;
	(*result).newval() = val;

	return result;	
}
