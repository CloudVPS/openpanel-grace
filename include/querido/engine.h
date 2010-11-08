// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _QUERIDO_ENGINE_H
#define _QUERIDO_ENGINE_H 1

#include <grace/str.h>
#include <grace/value.h>

class dbhandle
{
public:
					 dbhandle (class dbengine *owner);
	virtual			~dbhandle (void);

	virtual bool	 open (const value &details);
	virtual bool	 close (void);
	virtual bool	 query (const string &sql, value &into, const statstring &i);
	virtual bool	 listcolumns (const string &table, value &into);
	virtual bool	 tableexists (const string &table);
	virtual bool	 listtables (value &into);
	virtual int		 rowsaffected (void);
	
	const string	&error (void) { return errstr; }
	int				 errorcode (void) { return errcode; }
	
protected:
	string			 drivername;
	string			 errstr;
	int				 errcode;
};

THROWS_EXCEPTION (
	unknownEngineException,
	0x677e40e9,
	"Unknown database engine"
);

class dbengine
{
friend class dbhandle;
public:
					 enum localengine
					 {
					 	UDBC,
					 	MySQL,
					 	SQLite
					 };

					 dbengine (const string &driver);
					 dbengine (localengine builtintype);
					~dbengine ();
					
	bool			 open (const value &details);
	bool			 close (void);
	bool			 query (const string &sql, const statstring &idxby="");
	bool			 query (const string &sql, value &into,
							const statstring &idxby="");
	bool			 listcolumns (const string &table, value &into);
	bool			 tableexists (const string &table);
	bool			 listtables (value &into);
	int				 rowsaffected (void);
	
	const string	&error (void) { return hdl->error(); }
	int				 errorcode (void) { return hdl->errorcode(); }

protected:
	dbhandle		*hdl;
	string			 engine;
	
	void			 attachbuiltin (localengine t);
};

#endif
