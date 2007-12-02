#include <grace/value.h>
#include <grace/file.h>
#include <grace/stack.h>
#include <grace/strutil.h>
#include <grace/filesystem.h>

void value::encodegrace (string &into, int indent)
{
	string dent;
	if (! count())
	{
		if (itype == i_int)
		{
			into.printf ("%i", ival());
		}
		else if (itype == i_unsigned)
		{
			into.printf ("%u", uval());
		}
		else if (itype == i_double)
		{
			into.printf ("%f", dval());
		}
		else
		{
			into.strcat ('\"');
			encodejsonstring (into);
			into.strcat ('\"');
		}
	}
	else
	{
		int j = 0;
		if (attrib) foreach (a, (*attrib))
		{
			if (j) into.strcat (" ->");
			into.strcat ("\n");
			dent.pad (indent, ' ');
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
			into.strcat ("%s$(\"" %format (dent));
			array[i]->encodejsonid (into);
			into.strcat ("\",");
			array[i]->encodegrace (into, indent+4);
			into.strcat (")");
			j++;
		}
	}
}

string *value::tograce (void)
{
	returnclass (string) res retain;
	encodegrace (res, 0);
	res.strcat (";\n");
	return &res;
}