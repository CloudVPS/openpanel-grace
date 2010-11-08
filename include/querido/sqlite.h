// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

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
