#ifndef _GRACE_PCRE_H
#define _GRACE_PCRE_H 1

#include <pcre.h>

#include <grace/dictionary.h>
#include <grace/lock.h>
#include <grace/value.h>

typedef dictionary<pcre*> pcredict;

class pcredb
{
public:
					 pcredb (void);
					~pcredb (void);
					
	pcre			*get (const statstring &expr, int o);
	
protected:
	lock<pcredict>	 expressions;
};

class pcregexp
{
public:
					 pcregexp (void);
					 pcregexp (const statstring &expr, int o=0);
					 
					~pcregexp (void);
					
					 enum {
					 	anchored = PCRE_ANCHORED,
					 	ignorecase = PCRE_CASELESS,
					 	dollarendonly = PCRE_DOLLAR_ENDONLY,
					 	dotall = PCRE_DOTALL,
					 	extended = PCRE_EXTENDED,
					 	firstline = PCRE_FIRSTLINE,
					 	multiline = PCRE_MULTILINE,
					 	ungreedy = PCRE_UNGREEDY
					 };
	
	pcregexp		&operator= (const statstring &expr);
	pcregexp		&set (const statstring &expr);
	pcregexp		&setoptions (int o);
	
	bool			 match (const string &to, value *outrefs = NULL);
	bool			 match (const string &to, value &outrefs);
	value			*capture (const string &from);
	string			*replace (const string &what, const string &with,
							  bool g=false);
	
protected:
	pcre			*pobj;
	int				 options;
};

string *$expr (const string &orig, const string &expr);
string *$capture1 (const string &orig, const string &expr, int flags=0);
value *$capture (const string &orig, const string &expr, int flags=0);
//string *$capture1 (const string &orig, const statstring &expr);
//string *$replace (const string &orig, const string &expr, const string &with);

#endif
