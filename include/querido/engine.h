#ifndef _QUERIDO_ENGINE_H
#define _QUERIDO_ENGINE_H 1

#include <grace/str.h>
#include <grace/value.h>

class dbhandle
{
public:
					 dbhandle (class dbengine *owner);
					~dbhandle (void);

	virtual bool	 open (const value &details);
	virtual bool	 close (void);
	virtual bool	 query (const string &sql, value &into);
	virtual bool	 listcolumns (const string &table, value &into);
	virtual bool	 tableexists (const string &table);
	virtual bool	 listtables (value &into);
	
	const string	&error (void) { return errstr; }
	int				 errorcode (void) { return errcode; }
	
protected:
	string			 drivername;
	string			 errstr;
	int				 errcode;
};

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
	bool			 query (const string &sql);
	bool			 query (const string &sql, value &into);
	bool			 listcolumns (const string &table, value &into);
	bool			 tableexists (const string &table);
	bool			 listtables (value &into);

protected:
	dbhandle		*hdl;
	string			 engine;
	
	void			 attachbuiltin (localengine t);
};

#endif