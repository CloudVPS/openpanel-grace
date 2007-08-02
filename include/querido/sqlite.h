#ifndef _QUERIDO_SQLITE_H
#define _QUERIDO_SQLITE_H 1

#ifdef HAVE_SQLITE3
#include <sqlite3.h>
#include <querido/engine.h>
#include <grace/value.h>

class sqlitehandle : public dbhandle
{
public:
				 sqlitehandle (dbengine *owner);
				~sqlitehandle (void);
				
	bool		 open (const value &details);
	bool		 close (void);
	bool		 query (const string &, value &, const statstring &);
	bool		 listcolumns (const string &, value &);
	bool		 tableexists (const string &);
	bool		 listtables (value &into);
	int			 rowsaffected (void);

protected:
	sqlite3 	*hdl;
};

#endif
#endif
