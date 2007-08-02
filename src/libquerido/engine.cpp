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
