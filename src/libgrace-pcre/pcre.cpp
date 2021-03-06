// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#include <grace/pcre.h>
#include <grace/strutil.h>
#include <grace/file.h>

// ========================================================================
// CONSTRUCTOR pcredb
// ========================================================================
pcredb::pcredb (void)
{
}

// ========================================================================
// DESTRUCTOR pcredb
// ========================================================================
pcredb::~pcredb (void)
{
}

// ========================================================================
// METHOD pcredb::get
// ========================================================================
pcre *pcredb::get (const statstring &expr, int options)
{
	pcre *res = NULL;
	
	exclusivesection (expressions)
	{
		if (expressions.exists (expr))
		{
			res = expressions[expr];
			return res;
		}

		if (! expressions.exists (expr))
		{
			int erroffset;
			const char *errptr;
			res = pcre_compile (expr.str(), options, &errptr, &erroffset, NULL);
			if (res)
			{
				expressions[expr] = res;
			}
		}
	}
	
	return res;
}

pcredb __PCREDB;

// ========================================================================
// CONSTRUCTOR pcregexp
// ========================================================================
pcregexp::pcregexp (void)
{
	pobj = NULL;
	options = 0;
}

pcregexp::pcregexp (const statstring &expr, int o)
{
	pobj = NULL;
	options = o;
	set (expr);
}

// ========================================================================
// DESTRUCTOR pcregexp
// ========================================================================
pcregexp::~pcregexp (void)
{
}

// ========================================================================
// METHOD pcregexp::operator=
// ========================================================================
pcregexp &pcregexp::operator= (const statstring &to)
{
	return set (to);
}

// ========================================================================
// METHOD pcregexp::set
// ========================================================================
pcregexp &pcregexp::set (const statstring &to)
{
	pobj = __PCREDB.get (to, options);
	return *this;
}

// ========================================================================
// METHOD pcregexp::setoptions
// ========================================================================
pcregexp &pcregexp::setoptions (int o)
{
	options = o;
}

#define MAXBACKREF 256

// ========================================================================
// METHOD pcregexp::match
// ========================================================================
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

// ========================================================================
// METHOD pcregexp::capture
// ========================================================================
value *pcregexp::capture (const string &from)
{
	returnclass (value) res retain;
	match (from, res);
	return &res;
}

// ========================================================================
// METHOD pcregexp::replace
// ========================================================================
string *pcregexp::replace (const string &_orig, const string &with,
						   bool replaceall)
{
	returnclass (string) res retain;
	int thevector[MAXBACKREF*2];
	value R;

	string orig;
	res = _orig;
	int r;
	bool doloop = true;
	
	while (doloop)
	{
		doloop = replaceall;
		orig = res;
		
		r = pcre_exec (pobj, NULL, orig.str(), orig.strlen(),
					   0, 0, thevector, MAXBACKREF*2);
		
		if (r < 1)
		{
			return &res;
		}

		res.crop();
		R.clear ();
		
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
	}
	
	
	return &res;
}

// ========================================================================
// FUNCTION $expr
// ========================================================================
string *$expr (const string &orig, const string &expr)
{
	char split = expr[1];
	string esc;
	esc.strcat (split);
	value arg = strutil::splitescaped (expr, split, esc);
	if (arg[0] != "s") return NULL;
	
	int opts = 0;
	bool replaceall = false;
	
	string flags = arg[3];
	for (int i=0; i<flags.strlen(); ++i)
	{
		switch (flags[i])
		{
			case 'i' : opts |= pcregexp::ignorecase; break;
			case 'g' : replaceall = true; break;
			default : break;
		}
	}
	
	pcregexp ex (arg[1], opts);
	return ex.replace (orig, arg[2], replaceall);
}

// ========================================================================
// FUNCTION $capture1
// ========================================================================
string *$capture1 (const string &orig, const string &expr, int flags)
{
	returnclass (string) res retain;
	pcregexp re (expr, flags);
	value v = re.capture (orig);
	res = v[0];
	return &res;
}

// ========================================================================
// FUNCTION $capture
// ========================================================================
value *$capture (const string &orig, const string &expr, int flags)
{
	pcregexp re (expr, flags);
	return re.capture (orig);
}
