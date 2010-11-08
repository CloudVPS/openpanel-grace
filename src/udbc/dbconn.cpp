// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#include <udbc/dbconn.h>
#include <grace/str.h>
#include <grace/filesystem.h>

dbconnection::dbconnection (void)
{
	handle = NULL;
	dlhandle = NULL;
	resource["driver"] = "lib:udbc_mysql.so";
}

dbconnection::~dbconnection (void)
{
	if (handle) db_close (handle);
	handle = NULL;
	if (dlhandle) dlclose (dlhandle);
	dlhandle = NULL;
}

value *dbconnection::query (string &s)
{
	value *res;
	lck.lockw();
	if (!handle)
	{
		lck.unlock();
		return NULL;
	}
	res = db_exec (handle, s);
	lck.unlock();
	return res;
}

value *dbconnection::queryf (const char *fmt, ...)
{
	if (!handle) return NULL;
	value *res;
	
	string  st;
	va_list ap;
	
	va_start (ap, fmt);
	st.printf_va (fmt, &ap);
	va_end (ap);
	
	lck.lockw();
	if (! handle)
	{
		lck.unlock();
		return NULL;
	}
	res = db_exec (handle, st);
	lck.unlock();
	return res;
}

bool dbconnection::open (void)
{
	string driver;
	string drivervol;
	string driverpath;

	lck.lockw();	
	if (handle)
	{
		lck.unlock();
		return true;
	}
	
	driver = resource["driver"];
	
	if (driver[0] != '/')
	{
		if (driver.strchr (':') < 0)
		{
			drivervol = "lib";
		}
		else
		{
			drivervol = driver.cutat (':');
		}
		
		driverpath = drivervol;
		driverpath += ":";
		driverpath += driver;
	
		driver = fs.transr (driverpath);
	}
	dlhandle = dlopen (driver.str(), RTLD_GLOBAL | RTLD_NOW);
	if (!dlhandle)
	{
		_error.printf ("could not open %s :%s\n", resource["driver"].cval(), dlerror());
		lck.unlock();
		return false;
	}
	
	db_open = (void *(*)(const value &)) dlsym (dlhandle, "dbdriver_open");
	if (! db_open)
	{
		_error.printf ("could not resolve (dbdriver_open): %s\n", dlerror());
		dlclose (dlhandle);
		dlhandle = NULL;
		lck.unlock();
		return false;
	}
	
	db_exec = (value *(*)(void *, string &)) dlsym (dlhandle, "dbdriver_execute");
	if (! db_exec)
	{
		dlclose (dlhandle);
		dlhandle = NULL;
		_error.printf ("could not resolve (dbdriver_execute)\n");
		lck.unlock();
		return false;
	}
	
	db_error = (char *(*)(void *)) dlsym (dlhandle, "dbdriver_error");
	if (! db_error)
	{
		dlclose (dlhandle);
		dlhandle = NULL;
		_error.printf ("could not resolve (dbdriver_error)\n");
		lck.unlock();
		return false;
	}
	
	db_close = (void (*)(void *)) dlsym (dlhandle, "dbdriver_close");
	if (! db_close)
	{
		dlclose (dlhandle);
		dlhandle = NULL;
		_error.printf ("could not resolve (dbdriver_close)\n");
		lck.unlock();
		return false;
	}
	
	db_init = (int (*)(void)) dlsym (dlhandle, "dbdriver_init");
	if (! db_init)
	{
		dlclose (dlhandle);
		dlhandle = NULL;
		_error.printf ("could not resolve (dbdriver_init)\n");
		lck.unlock();
		return false;
	}

	db_insertid = (unsigned int (*)(void *)) dlsym (dlhandle, "dbdriver_insert_id");
	if (! db_insertid)
	{
		dlclose (dlhandle);
		dlhandle = NULL;
		_error.printf ("could not resolve (dbdriver_insert_id)\n");
		lck.unlock();
		return false;
	}
	
	if (! db_init())
	{
		dlclose (dlhandle);
		dlhandle = NULL;
		if (! _error.strlen())
			_error = "db_init failed";
		
		lck.unlock();
		return false;
	}
	
	handle = db_open (resource);
	if (!handle)
	{
		if (! _error.strlen())
			_error = "db_open failed";
		lck.unlock();
		return false;
	}
	lck.unlock();
	return true;
}

void dbconnection::close (void)
{
	lck.lockw();
	if (handle)
	{
		db_close (handle);
		handle = NULL;
	}
	lck.unlock();
}

unsigned int dbconnection::insert_id (void)
{
	unsigned int res;
	if (! handle) return 0;
	lck.lockw();
	res = db_insertid (handle);
	lck.unlock();
	
	return res;
}

const string &dbconnection::error (void)
{
	if (! handle) return _error;
	_error = db_error (handle);
	return _error;
}

value *dbconnection::getrec (const char *tb, int id, const char *inam)
{
	value *res;
	value qres;
	
	qres = queryf ("select * from %s where %s=%i", tb, inam, id);
	res = new value;
	(*res) = qres[0];
	return res;
}

value *dbconnection::getrec (const char *tb, const char *id, const char *inam)
{
	value *res;
	value qres;
	
	qres = queryf ("select * from %s where %s='%S'", tb, inam, id);
	res = new value;
	(*res) = qres[0];
	return res;
}

void dbconnection::update (const char *tb, value &v, const char *inam)
{
	string qry;
	statstring str;
	bool first = true;
	
	str = inam;
	
	qry.printf ("update %s set ", tb);
	for (int i=0; i<v.count(); ++i)
	{
		if (v[i].label() != str)
		{
			if (!first) qry.printf (",");
			first = false;
			if (v.type() == t_int)
				qry.printf ("%s=%i", v[i].name(), v[i].ival());

			else if (v.type() == t_double)
				qry.printf ("%s=%f", v[i].name(), v[i].dval());

			else if (v.type() == t_ipaddr)
				qry.printf ("%s=%d", v[i].name(), v[i].ipval());

			else
				qry.printf ("%s='%S'", v[i].name(), v[i].cval());
		}
	}
	qry.printf (" where %s=", inam);
	if (v[inam].type() == t_int)
		qry.printf ("%i", v[inam].ival());

	else if (v[inam].type() == t_ipaddr)			
		qry.printf ("%d", v[inam].ipval());

	else		
		qry.printf ("'%S'", v[inam].cval());
	
	delete query (qry);
}

unsigned int dbconnection::insert (const char *tb, value &v)
{
	string qry;
	bool first=true;
	int i;
	
	qry.printf ("insert into %s(", tb);
	for (i=0; i<v.count(); ++i)
	{
		if (! first) qry += ",";
		first = false;
		qry += v[i].name();	
	}
	
	qry.printf (") values (");
	first = true;
	
	for (i=0; i<v.count(); ++i)
	{
		if (! first) qry += ",";
		first = false;

		if (v[i].type() == t_int)
			qry.printf ("%i", v[i].ival());

		else if (v[i].type() == t_double)			
			qry.printf ("%f", v[i].dval());

		else				
			qry.printf ("'%S'", v[i].cval());
	}
	
	qry += ")";
	
	delete queryf ("%s", qry.str());
	return insert_id ();
}
