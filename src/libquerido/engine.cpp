#include <querido/engine.h>

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

bool dbhandle::query (const string &sql, value &into)
{
	return false;
}

bool dbhandle::listcolumns (const string &table, value &into)
{
	return false;
}

bool dbhandle::tableexists (const string &table)
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

bool dbengine::query (const string &sql)
{
	if (! hdl) return false;
	
	value tmp;
	return hdl->query (sql, tmp);
}

bool dbengine::query (const string &sql, value &into)
{
	if (! hdl) return false;
	
	return hdl->query (sql, into);
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

