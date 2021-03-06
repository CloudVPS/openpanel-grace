// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _DBCONN_H
#define _DBCONN_H 1

#include <grace/value.h>

#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>

class dbconnection
{
public:
					 dbconnection (void);
					~dbconnection (void);

	value			 resource;
	bool			 open (void);
	void			 close (void);
	value			*query (string &);
	value			*queryf (const char *, ...);
	unsigned int	 insert_id (void);
	const string	&error (void);
	
	value			*getrec (const char *tb, int id, const char *inam = "id");
	value			*getrec (const char *tb, const char *id,
							 const char *inam = "id");
	void			 update (const char *tb, value &val,
							 const char *inam = "id");
	unsigned int	 insert (const char *tb, value &val);
	
	
protected:
	int				 (*db_init) (void);
	void			*(*db_open) (const value &);
	value			*(*db_exec) (void *, string &);
	char			*(*db_error) (void *);
	void			 (*db_close) (void *);
	unsigned int	 (*db_insertid) (void *);
	
	string			 _error;
	void			*handle;
	void			*dlhandle;
	lock<bool>		 lck;
};

#endif
