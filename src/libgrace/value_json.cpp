#include <grace/value.h>
#include <grace/file.h>
#include <grace/stack.h>
#include <grace/strutil.h>
#include <grace/filesystem.h>

const char *value::readjsonstring (const char *pos, string &into)
{
	const char *atpos = pos;
	if (*atpos != '\"') return NULL;
	++atpos;
	while ((*atpos) && (*atpos != '\"'))
	{
		if (*atpos == '\\')
		{
			switch (atpos[1])
			{
				case '\"' :
					into.strcat ('\"');
					atpos++;
					break;
				
				case '\\' :
					into.strcat ('\\');
					atpos++;
					break;
				
				case 'b' :
					into.strcat ((char) 8);
					atpos++;
					break;
				
				case 'f' :
					into.strcat ('\f');
					atpos++;
					break;
				
				case 'n' :
					into.strcat ('\n');
					atpos++;
					break;
				
				case 'r' :
					into.strcat ('\r');
					atpos++;
					break;
					
				case 't' :
					into.strcat ('\t');
					atpos++;
					break;
			}
		}
		else
		{
			into.strcat (*atpos);
		}
		atpos++;
	}
	
	atpos++;
	
	if (*atpos) return atpos;
	return NULL;
}

const char *value::readjsonnumber (const char *crsr, string &into)
{
	const char *c = crsr;
	
	if (*c == '-')
	{
		into.strcat ('-');
		c++;
	}
	
	while (isdigit (*c))
	{
		into.strcat (*c);
		c++;
	}
	
	if (*c == ',') return c;
	if (*c == '}') return c;
	if (isspace (*c)) return c;
	if (*c != '.') return NULL;
	
	into.strcat (*c);
	++c;
	
	while (isdigit (*c))
	{
		into.strcat (*c);
		c++;
	}
	
	if (*c == ',') return c;
	if (*c == '}') return c;
	if (isspace (*c)) return c;

	if ((*c != 'e') && (*c != 'E')) return NULL;
	into.strcat (*c);
	++c;
	if ((*c == '+') || (*c == '-'))
	{
		into.strcat (*c);
		c++;
	}
	while (isdigit (*c))
	{
		into.strcat (*c);
		c++;
	}
	
	return c;
}

const char *value::decodejson (const char *objpos)
{
	const char *crsr = objpos;
	if (*crsr == '{')
	{
		while (*crsr != '}')
		{
			string nam, val;
			
			crsr++;
			while (isspace(*crsr) || (*crsr == '\n')) crsr++;
			
			if (*crsr == ',')
			{
				crsr++;
				while (isspace(*crsr) || (*crsr == '\n')) crsr++;
			}
			
			if (*crsr == '\"')
			{
				crsr = readjsonstring (crsr, nam);
				if (! crsr) return NULL;
				
				while (isspace(*crsr) || (*crsr == '\n')) crsr++;
			}
			else if (*crsr == '}')
			{
				return crsr+1;
			}
			else
			{
				return NULL;
			}
			
			if (*crsr != ':')
			{
				return NULL;
			}
			crsr++;
			while (isspace (*crsr) || (*crsr == '\n')) crsr++;
			
			if (*crsr == '\"')
			{
				crsr = readjsonstring (crsr, val);
				if (! crsr) return NULL;
				(*this)[nam] = val;
			}
			else if ((*crsr=='+')||(*crsr=='-')||
					 (isdigit (*crsr)))
			{
				crsr = readjsonnumber (crsr, val);
				if (! crsr) return NULL;
				(*this)[nam] = val;
				if (val.strchr ('.') > 0)
				{
					(*this)[nam] = (*this)[nam].dval();
				}
				else
				{
					(*this)[nam] = (*this)[nam].ival();
				}
			}
			else if ((*crsr == '{')||(*crsr == '['))
			{
				crsr = (*this)[nam].decodejson (crsr);
				if (! crsr) return NULL;
			}
			else if (::strncmp (crsr, "true", 4) == 0)
			{
				(*this)[nam] = true;
				crsr += 4;
			}
			else if (::strncmp (crsr, "false", 5) == 0)
			{
				(*this)[nam] = false;
				crsr += 5;
			}
			else
			{
				return NULL;
			}
		}
	}
	else if (*crsr == '[')
	{
		++crsr;

		while (*crsr != ']')
		{
			string val;

			while (isspace (*crsr) || (*crsr == '\n')) crsr++;
			
			if (*crsr == ',')
			{
				crsr++;
				while (isspace (*crsr) || (*crsr == '\n')) crsr++;
			}
			
			if (*crsr == '\"')
			{
				crsr = readjsonstring (crsr, val);
				if (! crsr) return NULL;
				(*this).newval() = val;
			}
			else if ((*crsr=='+')||(*crsr=='-')||
					 (isdigit (*crsr)))
			{
				crsr = readjsonnumber (crsr, val);
				if (! crsr) return NULL;
				(*this).newval() = val;
				if (val.strchr ('.') > 0)
				{
					(*this)[-1] = (*this)[-1].dval();
				}
				else
				{
					(*this)[-1] = (*this)[-1].ival();
				}
			}
			else if ((*crsr == '{')||(*crsr == '['))
			{
				crsr = (*this).newval().decodejson (crsr);
				if (! crsr) return NULL;
			}
			else if (::strncmp (crsr, "true", 4) == 0)
			{
				(*this).newval() = true;
				crsr += 4;
			}
			else if (::strncmp (crsr, "false", 5) == 0)
			{
				(*this).newval() = false;
				crsr += 5;
			}
			else
			{
				return NULL;
			}
			
		}
	}
	
	if (crsr == objpos) return NULL;
	return crsr+1;
}

bool value::fromjson (const string &code)
{
	int pos = 0;
	while (isspace (code[pos])) pos++;

	decodejson (code.cval() + pos);	
}

void value::encodejsonstring (string &into) const
{
	const char *c = cval();
	int len = sval().strlen();
	
	for (int i=0; i<len; ++i)
	{
		char cc = c[i];
		if (cc == '\n') into.strcat ("\\n");
		else if (cc == '\t') into.strcat ("\\t");
		else if (cc == '\"') into.strcat ("\\\"");
		else if (cc == '\r') into.strcat ("\\r");
		else into.strcat (cc);
	}
}

void value::encodejsonid (string &into) const
{
	const char *c = name();
	int len = id().sval().strlen();
	
	for (int i=0; i<len; ++i)
	{
		char cc = c[i];
		if (cc == '\n') into.strcat ("\\n");
		else if (cc == '\t') into.strcat ("\\t");
		else if (cc == '\"') into.strcat ("\\\"");
		else if (cc == '\r') into.strcat ("\\r");
		else into.strcat (cc);
	}
}

void value::encodejson (string &into) const
{
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
		else if (itype == i_bool)
		{
			into.strcat (bval() ? "true" : "false");
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
		if (ucount == count())
		{
			into.strcat ('[');
			for (int i=0; i<count(); ++i)
			{
				if (i) into.strcat (',');
				array[i]->encodejson (into);
			}
			into.strcat (']');
		}
		else
		{
			into.strcat ('{');
			for (int i=0; i<count(); ++i)
			{
				if (i) into.strcat (',');
				into.strcat ('\"');
				array[i]->encodejsonid (into);
				into.strcat ("\":");
				array[i]->encodejson (into);
			}
			into.strcat ('}');
		}
	}
}

string *value::tojson (void) const
{
	returnclass (string) res retain;
	encodejson (res);
	return &res;
}
