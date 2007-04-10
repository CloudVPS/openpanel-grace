#include <grace/session.h>
#include <grace/strutil.h>
#include <grace/system.h>

sessionlist::sessionlist (void)
{
}

sessionlist::~sessionlist (void)
{
}

bool sessionlist::exists (const statstring &id)
{
	bool result = false;
	
	sharedsection (db)
	{
		result = db.exists (id);	
	}
	
	return result;
}

string *sessionlist::create (const value &sdat)
{
	returnclass (string) uuid retain;
	
	uuid = strutil::uuid();
	exclusivesection (db)
	{
		while (db.exists (uuid)) uuid = strutil::uuid();
		db[uuid] = sdat;
		db[uuid]("time") = (unsigned int) kernel.time.now();
	}
	
	return &uuid;
}

value *sessionlist::get (const statstring &id)
{
	returnclass (value) res retain;
	
	exclusivesection (db)
	{
		db[id]("time") = (unsigned int) kernel.time.now();
		res = db[id];
	}
	
	return &res;
}

void sessionlist::set (const statstring &id, const value &dat)
{
	exclusivesection (db)
	{
		db[id] = dat;
		db[id]("time") = (unsigned int) kernel.time.now();
	}
}

void sessionlist::expire (int timeout)
{
	time_t now = kernel.time.now();
	time_t then;
	
	exclusivesection (db)
	{
		for (int i=0; i<db.count(); ++i)
		{
			then = db[i]("time").uval();
			if ((now-then) > timeout)
			{
				db.rmindex (i--);
			}
		}
	}
}
