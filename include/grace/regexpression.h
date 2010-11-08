// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _EXPRESSION_H
#define _EXPRESSION_H 1

#include <sys/types.h>
#include <regex.h>
#include <grace/str.h>

class regex_statement
{
public:
				 regex_statement (const char *);
				~regex_statement (void);
				
	bool		 eval (const char *);
	int			 left (void);
	int			 right (void);

private:
	regex_t		 preg;
	regmatch_t	 pmatch[1];
};

class regex_clause
{
public:
					 regex_clause (const char *);
					~regex_clause (void);

	char			*parse (const char *);
	bool			 eval (const char *);
	char			*skipto (char *);
	char			*skipover (char *);
	char			*cutat (char *);
	
private:
	regex_statement	*st;
	char			*replace;
	int				 rsz;
	bool			 recurse;
};

class regexpression
{
public:
				  regexpression (const string &);
				 ~regexpression (void);
				
	string		 *parse (const string &);
	bool		  eval (const char *);
	char		 *skipto (char *);
	char		 *skipover (char *);
	char		 *cutat (char *);
	
protected:
	void		  Add (regex_clause *);
	regex_clause *FirstMatch (const char *);

private:
	int			  count;
	regex_clause **array;
};

#endif
