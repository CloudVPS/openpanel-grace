// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#include <grace/value.h>
#include <grace/file.h>
#include <grace/stack.h>
#include <grace/strutil.h>
#include <grace/filesystem.h>

// ========================================================================
// METHOD ::encodegrace
// ========================================================================
void value::encodegrace (string &into, int indent)
{
	string dent;
	if (! count())
	{
		int j = 0;
		dent.pad (indent, ' ');
		
		if (! isbuiltin (type()))
		{
			into.strcat ("\n");
			into.strcat ("%s$type(\"%S\")" %format (dent, type()));
			j = 1;
		}
		if (attrib) foreach (a, (*attrib))
		{
			if (j) into.strcat (" ->");
			into.strcat ("\n");
			into.strcat ("%s$attr(\"" %format (dent));
			a.encodejsonid (into);
			into.strcat ("\",");
			a.encodegrace (into, indent);
			into.strcat (")");
			j++;
		}
		if (j && (_itype != i_unset))
		{
			into.strcat (" ->\n%s$val(" %format (dent));
		}
		if (j && (_itype == i_unset))
		{
			j = 0;
		}
		else if (_itype == i_int)
		{
			into.printf ("%i", ival());
		}
		else if (_itype == i_unsigned)
		{
			into.printf ("%u", uval());
		}
		else if (_itype == i_double)
		{
			into.printf ("%f", dval());
		}
		else if (_itype == i_date)
		{
			into.printf ("(time_t) %i", ival());
		}
		else if (_itype == i_ipaddr)
		{
			into.printf ("(ipaddress) \"%s\"", cval());
		}
		else
		{
			into.strcat ('\"');
			encodejsonstring (into);
			into.strcat ('\"');
		}
		if (j) into.strcat (")");
	}
	else
	{
		int j = 0;
		dent.pad (indent, ' ');
		
		if (! isbuiltin (type()))
		{
			into.strcat ("\n");
			into.strcat ("%s$type(\"%S\")" %format (dent, type()));
			j = 1;
		}
		
		if (attrib) foreach (a, (*attrib))
		{
			if (j) into.strcat (" ->");
			into.strcat ("\n");
			into.strcat ("%s$attr(\"" %format (dent));
			a.encodejsonid (into);
			into.strcat ("\",");
			a.encodegrace (into, indent);
			into.strcat (")");
			j++;
		}
		for (int i=0; i<count(); ++i)
		{
			if (j) into.strcat (" ->");
			into.strcat ("\n");
			dent.pad (indent, ' ');
			if (i >= ucount)
			{
				into.strcat ("%s$(\"" %format (dent));
				array[i]->encodejsonid (into);
				into.strcat ("\",");
			}
			else
			{
				into.strcat ("%s$(" %format (dent));
			}
			array[i]->encodegrace (into, indent+4);
			into.strcat (")");
			j++;
		}
	}
}

// ========================================================================
// METHOD ::tograce
// ========================================================================
string *value::tograce (void)
{
	returnclass (string) res retain;
	encodegrace (res, 0);
	res.strcat (";\n");
	return &res;
}
