#include <grace/session.h>
#include <grace/strutil.h>
#include <grace/system.h>

// ==========================================================================
// CONSTRUCTOR sessionlist
// ==========================================================================
sessionlist::sessionlist (void)
{
}

// ==========================================================================
// DESTRUCTOR sessionlist
// ==========================================================================
sessionlist::~sessionlist (void)
{
}

// ==========================================================================
// METHOD sessionlist::exists
// ==========================================================================
bool sessionlist::exists (const statstring &id)
{
	bool result = false;
	
	sharedsection (db)
	{
		result = db.exists (id);	
	}
	
	return result;
}

// ==========================================================================
// METHOD sessionlist::create
// ==========================================================================
string *sessionlist::create (const value &sdat)
{
	returnclass (string) uuid retain;
	
	uuid = strutil::uuid();
	exclusivesection (db)
	{
		while (db.exists (uuid)) uuid = strutil::uuid();
		db[uuid] = sdat;
		db[uuid]("time") = (unsigned int) core.time.now();
	}
	
	return &uuid;
}

// ==========================================================================
// METHOD sessionlist::destroy
// ==========================================================================
void sessionlist::destroy (const statstring &id)
{
	exclusivesection (db)
	{
		db.rmval (id);
	}
}

// ==========================================================================
// METHOD sessionlist::get
// ==========================================================================
value *sessionlist::get (const statstring &id)
{
	returnclass (value) res retain;
	
	exclusivesection (db)
	{
		db[id]("time") = (unsigned int) core.time.now();
		res = db[id];
	}
	
	return &res;
}

// ==========================================================================
// METHOD sessionlist::set
// ==========================================================================
void sessionlist::set (const statstring &id, const value &dat)
{
	exclusivesection (db)
	{
		db[id] = dat;
		db[id]("time") = (unsigned int) core.time.now();
	}
}

// ==========================================================================
// METHOD sessionlist::expire
// ==========================================================================
void sessionlist::expire (int timeout)
{
	time_t now = core.time.now();
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
