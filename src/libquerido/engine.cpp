// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#include <querido/engine.h>
#ifdef HAVE_SQLITE3
  #include <querido/sqlite.h>
#endif

dbhandle::dbhandle (dbengine *owner)
	: drivername (owner->engine)
{
	errcode = 0;
}

dbhandle::~dbhandle (void)
{
}

bool dbhandle::open (const value &details)
{
	return false;
}

bool dbhandle::close (void)
{
	return true;
}

bool dbhandle::query (const string &sql, value &into, const statstring &i)
{
	return false;
}

bool dbhandle::listcolumns (const string &table, value &into)
{
	return false;
}

bool dbhandle:: tableexists (const string &table)
{
	return false;
}

bool dbhandle::listtables (value &into)
{
	return false;
}

int dbhandle::rowsaffected (void)
{
	return 0;
}

dbengine::dbengine (const string &driver)
{
	caseselector (driver)
	{
		incaseof ("sqlite") :
			attachbuiltin (SQLite);
			break;
		
		defaultcase :
			hdl = NULL;
			break;
	}
}

dbengine::dbengine (localengine t)
{
	attachbuiltin (t);
}

dbengine::~dbengine (void)
{
}

bool dbengine::open (const value &parm)
{
	if (! hdl) return false;
	return hdl->open (parm);
}

bool dbengine::close (void)
{
	if (! hdl) return true;
	hdl->close ();
}

bool dbengine::query (const string &sql, const statstring &idxby)
{
	if (! hdl) return false;
	
	value tmp;
	return hdl->query (sql, tmp, idxby);
}

bool dbengine::query (const string &sql, value &into, const statstring &idxby)
{
	if (! hdl) return false;
	
	return hdl->query (sql, into, idxby);
}

bool dbengine::listcolumns (const string &table, value &into)
{
	if (! hdl) return false;
	
	return hdl->listcolumns (table, into);
}

bool dbengine::tableexists (const string &table)
{
	if (! hdl) return false;

	return hdl->tableexists (table);
}

bool dbengine::listtables (value &into)
{
	if (! hdl) return false;
	
	return hdl->listtables (into);
}

int dbengine::rowsaffected (void)
{
	if (! hdl) return 0;
	
	return hdl->rowsaffected ();
}

void dbengine::attachbuiltin (localengine t)
{
	switch (t)
	{
#ifdef HAVE_SQLITE3
		case SQLite:
			hdl = new sqlitehandle (this);
			break;
#endif
		default:
			throw (unknownEngineException());
	}
}
