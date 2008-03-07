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
					
	pcre			*get (const statstring &expr);
	
protected:
	lock<pcredict>	 expressions;
};

class pcregexp
{
public:
					 pcregexp (void);
					 pcregexp (const statstring &expr);
					 
					~pcregexp (void);
	
	pcregexp		&operator= (const statstring &expr);
	void			 set (const statstring &expr);
	
	bool			 match (const string &to, value *outrefs = NULL);
	bool			 match (const string &to, value &outrefs);
	value			*capture (const string &from);
	string			*replace (const string &what, const string &with);
	
protected:
	pcre			*pobj;
};

string *$expr (const string &orig, const string &expr);
//string *$capture1 (const string &orig, const statstring &expr);
//string *$replace (const string &orig, const string &expr, const string &with);

#endif
