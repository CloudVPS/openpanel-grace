#include <grace/regexpression.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <grace/str.h>

regex_statement::regex_statement (const char *stm)
{
	int err;

	pmatch[0].rm_so = 0;
	pmatch[0].rm_eo = 0;
	
	if ((err=regcomp (&preg, stm, REG_EXTENDED|REG_ICASE)))
	{
		char ebuff[256];
		regerror (err, &preg, ebuff, 255);
	}
}

regex_statement::~regex_statement (void)
{
	regfree (&preg);
}

bool regex_statement::eval (const char *str)
{
	return ((regexec (&preg, str, 1, &pmatch[0], 0) == 0));
}

int regex_statement::left (void)
{
	return (int) pmatch[0].rm_so;
}

int regex_statement::right (void)
{
	return (int) pmatch[0].rm_eo;
}

regex_clause::regex_clause (const char *expr)
{
	replace = NULL;
	st = NULL;
	rsz = 0;
	recurse = false;

	char *ex = new char[strlen(expr)+1];
	strcpy (ex, expr);
	
	if ((*ex == 's')&&(ex[1]))
	{
		char *left = ex+1;
		char *mid = left+1;
		char *right = left+1;
		
		char *s; // temp vars
		int   c;
		
		mid = left+1;
		while (*mid && (*mid != *left))
		{
			if ((*mid=='\\') && (mid[1] == *left)) ++mid;
			++mid;
		}
		if (*mid)
		{
			*mid = '\0';
			right = mid+1;
			while (*right && (*right != *left))
			{
				if ((*right=='\\') && (right[1] == *left)) ++right;
				++right;
			}
			if (*right)
			{
				*right = '\0';
				if (right[1]=='g') recurse = true;
				replace = new char[(right-mid)+1];
				
				s = mid+1;
				
				for (c=0; *s; ++c)
				{
					if ((*s == '\\')&&(s[1] == *left)) ++s;
					replace[c] = *s++;
				}
				replace[c] = '\0';
				rsz = c;
			}
			
			char *tmp = new char[(mid-left)];
			
			s = left+1;
			
			for (c=0; *s; ++c)
			{
				if ((*s == '\\')&&(s[1] == *left)) ++s;
				tmp[c] = *s++;
			}
			tmp[c] = '\0';
			
			st = new regex_statement (tmp);
			delete[] tmp;
		}
		else
		{
			st = new regex_statement (".*");
		}
	}
	else
	{
		st = new regex_statement (expr);
	}
	
	delete[] ex;
}

regex_clause::~regex_clause (void)
{
	if (st) delete st;
	if (replace) delete[] replace;
}

char *regex_clause::parse (const char *iorig)
{
	char *orig = (char *) iorig;
	int offset=0;
	char *result = (char *) orig;
	int x=0;
	
	while (((x==0)||(recurse)) && (st->eval (orig+offset)))
	{
		int l = st->left() + offset;
		int r = st->right() + offset;
		
		int sz = strlen (orig);
		int newsz = sz - (r-l) + rsz;
		
		if (replace)
		{
			char *curResult = result;
			char *curOrig = orig;
			result = new char[newsz+1];
			memcpy (result, orig, l);
			memcpy (result+l, replace, rsz);
			memcpy (result+l+rsz, orig+r, sz-r);
			result[newsz] = '\0';
			orig = result;
			if ( ( curResult != (char *) iorig ) &&
			     ( curResult != result ) &&
			     ( curResult != orig ) )
			{
				delete[] curResult;
			}
			if ( ( curOrig != (char *) iorig ) &&
			     ( curOrig != curResult ) &&
			     ( curOrig != orig ) )
			{
				delete[] curOrig;
			}
			offset = l+rsz;
		}
		else
		{
			char *curOrig = orig;
			char *curResult = result;
			result = new char[2];
			result[0] = '1';
			result[1] = '\0';
			orig = result;
			recurse = false;
			if ( ( curResult != (char *) iorig ) &&
			     ( curResult != result ) &&
			     ( curResult != orig ) )
			{
				delete[] curResult;
			}
			if ( ( curOrig != (char *) iorig ) &&
			     ( curOrig != curResult ) &&
			     ( curOrig != orig ) )
			{
				delete[] curOrig;
			}
		}
		++x;
	}
	
	return result;
}

bool regex_clause::eval (const char *orig)
{
	return st->eval (orig);
}

char *regex_clause::skipto (char *orig)
{
	if (! orig) return NULL;
	if (! st->eval (orig))
	{
		delete[] orig;
		return NULL;
	}
	
	char *result = new char[(strlen(orig) - st->left())+1];
	strcpy (result, orig + st->left());
	
	delete[] orig;
	return result;
}

char *regex_clause::skipover (char *orig)
{
	if (! orig) return NULL;
	if (! st->eval (orig))
	{
		delete[] orig;
		return NULL;
	}
	
	char *result = new char[(strlen(orig) - st->right())+1];
	strcpy (result, orig + st->right());
	
	delete[] orig;
	return result;
}

char *regex_clause::cutat (char *orig)
{
	if (! orig) return NULL;
	if (! st->eval (orig))
	{
		delete[] orig;
		return NULL;
	}
	
	orig[st->left()] = '\0';
	return orig;
}

regexpression::regexpression (const string &statement)
{
	count = 0;
	array = NULL;
	const char *stm = statement.str();

	char *tmp = new char[statement.strlen()+1];
	strcpy (tmp, stm);
	
	char *left, *right;
	char sep = (*tmp == 's') ? ';' : '|';
	
	left = right = tmp;
	while (*left)
	{
		right = left+1;
		while (*right && (*right != sep))
		{
			if ((*right=='\\')&&(right[1]==sep)) ++right;
			++right;
		}
		if (*right)
		{
			*right = '\0';
			Add (new regex_clause (left));
			left = right+1;
		}
		else
		{
			if (left != right) Add (new regex_clause (left));
			left = right;
		}
	}
	
	delete[] tmp;
}

regexpression::~regexpression (void)
{
	for (int i=0; i<count; ++i) delete array[i];
	if (array) free (array);
}

string *regexpression::parse (const string &orig)
{
	char *res = (char *) orig.str();
	
	for (int i=0; i<count; ++i)
	{
		char *oldres = res;
		res = array[i]->parse (res);
		if ( (res != oldres) && (oldres != (char *) orig.str()) )
		{
			delete[] oldres;
		}
	}
	
	string *result = new string (res);
	if (res != (char *) orig.str())
	{
		delete[] res;
	}
	return result;
}

bool regexpression::eval (const char *orig)
{
	return (FirstMatch (orig) != NULL);
}

char *regexpression::skipto (char *orig)
{
	regex_clause *r = FirstMatch (orig);
	return r ? r->skipto (orig) : orig;
}

char *regexpression::skipover (char *orig)
{
	regex_clause *r = FirstMatch (orig);
	return r ? r->skipover (orig) : orig;
}

char *regexpression::cutat (char *orig)
{
	regex_clause *r = FirstMatch (orig);
	return r ? r->cutat (orig) : orig;
}

void regexpression::Add (regex_clause *r)
{
	if (! array)
	{
		array = (regex_clause **) malloc (sizeof (regex_clause *));
		count = 1;
		array[0] = r;
		return;
	}
	
	array = (regex_clause **) realloc (array, (count+1) * sizeof (regex_clause *));
	array[count++] = r;
}

regex_clause *regexpression::FirstMatch (const char *str)
{
	for (int i=0; i<count; ++i)
		if (array[i]->eval (str)) return array[i];
	
	return NULL;
}

#ifdef TEST

/* TESTING CODE ----------------------------------------------------------------- */

int main (int argc, char *argv[])
{
	if (argc<2)
	{
		fprintf (stderr, "Usage: %s <expression>\n", argv[0]);
		return 1;
	}
	regexpression expr(argv[1]);
	
	while (!feof (stdin))
	{
		char *buf = new char[256];
		*buf = 0;
		fgets (buf, 255, stdin);
		if (*buf)
		{
			buf[strlen(buf)-1] = '\0';
			buf = expr.parse (buf);
			printf ("%s\n", buf); fflush (stdout);
		}
		delete[] buf;
	}
}

/*-------------------------------------------------------------------------------*/

#endif
