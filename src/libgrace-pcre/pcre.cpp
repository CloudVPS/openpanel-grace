#include <grace/pcre.h>
#include <grace/strutil.h>
#include <grace/file.h>

pcredb::pcredb (void)
{
}

pcredb::~pcredb (void)
{
}

pcre *pcredb::get (const statstring &expr)
{
	pcre *res = NULL;
	
	sharedsection (expressions)
	{
		if (expressions.exists (expr))
		{
			res = expressions[expr];
			breaksection return res;
		}
	}
	
	exclusivesection (expressions)
	{
		int erroffset;
		const char *errptr;
		res = pcre_compile (expr.str(), 0, &errptr, &erroffset, NULL);
		if (res)
		{
			expressions[expr] = res;
		}
	}
	
	return res;
}

pcredb __PCREDB;

pcregexp::pcregexp (void)
{
	pobj = NULL;
}

pcregexp::pcregexp (const statstring &expr)
{
	pobj = NULL;
	set (expr);
}

pcregexp::~pcregexp (void)
{
}

pcregexp &pcregexp::operator= (const statstring &to)
{
	set (to);
}

void pcregexp::set (const statstring &to)
{
	pcre *r = __PCREDB.get (to);
	pobj = r;
}

#define MAXBACKREF 16

bool pcregexp::match (const string &to, value *outref)
{
	if (! pobj) return true;
	
	int thevector[MAXBACKREF*2];
	value tref;
	value &R = outref ? *outref : tref;
	
	int r;
	r = pcre_exec (pobj, NULL, to.str(), to.strlen(),
				   0, 0, thevector, MAXBACKREF*2);
	
	if (r < 1) return false;
	if (r>1)
	{
		for (int i=1; i<r; ++i)
		{
			int pos = thevector[2*i];
			int sz = thevector[(2*i)+1] - pos;
			R.newval() = to.mid (pos, sz);
		}
	}
	
	return true;
}

bool pcregexp::match (const string &to, value &outref)
{
	return match (to, &outref);
}

value *pcregexp::capture (const string &from)
{
	returnclass (value) res retain;
	match (from, res);
	return  &res;
}

string *pcregexp::replace (const string &orig, const string &with)
{
	returnclass (string) res retain;
	int thevector[MAXBACKREF*2];
	value R;

	int r;
	r = pcre_exec (pobj, NULL, orig.str(), orig.strlen(),
				   0, 0, thevector, MAXBACKREF*2);
	
	if (r < 1)
	{
		res = orig;
		return &res;
	}
	
	if (r > 1)
	{
		for (int i=1; i<r; ++i)
		{
			int pos = thevector[2*i];
			int sz = thevector[(2*i)+1] - pos;
			R.newval() = orig.mid (pos, sz);
		}
	}
	
	if (thevector[0])
	{
		res = orig.left (thevector[0]);
	}
	
	if (with.strchr ('\\') >= 0)
	{
		int toi;
		
		const char *p = with.cval();
		while (p && *p)
		{
			if ((*p == '\\') && (toi = atoi (p+1)))
			{
				res.strcat (R[toi-1]);
				p++;
				while (isdigit (*p)) p++;
			}
			else
			{
				res.strcat (*p);
				p++;
			}
			
		}
	}
	else
	{
		res.strcat (with);
	}
	
	if (thevector[1] < orig.strlen())
	{
		res.strcat (orig.mid (thevector[1]));
	}
	
	return &res;
}

string *$expr (const string &orig, const string &expr)
{
	char split = expr[1];
	value arg = strutil::splitescaped (expr, split);
	if (arg[0] != "s") return NULL;
	pcregexp ex (arg[1]);
	return ex.replace (orig, arg[2]);
}
