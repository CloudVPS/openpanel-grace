// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.
#include <dbfile/db4file.h>

// ========================================================================
// METHOD db4_DBT_from_string
// ========================================================================
static DBT db4_DBT_from_string(const string &s)
{
	DBT d;
	::memset(&d, 0, sizeof(d));
    d.size = s.strlen();
    d.data = (char *) ::malloc (s.strlen());
    ::memcpy (d.data, s.str(), s.strlen());
    return d;
}

// ========================================================================
// FUNCTION db4_DBT_to_string
// ========================================================================
static void db4_DBT_to_string (DBT &d, string &s)
{
	if (! d.data) return;
	s.strcpy ((char *) d.data, d.size);
	::free (d.data);
	::memset(&d, 0, sizeof(d));
}

// ========================================================================
// FUNCTION db4_DBT_free_datum
// ========================================================================
static void db4_DBT_free (DBT &d)
{
	if (d.data)
		::free (d.data);
	::memset(&d, 0, sizeof(d));
}

// ========================================================================
// METHOD ::open
// ========================================================================
bool db4file::open (const string &dbfile)
{
	dbopen = false;
	if ( 0 != db_create(&f, NULL, 0))
	{
		f=NULL;
		return false;
	}
	// bloody bloody why not change the bloody name of the bloody call then!
#if DB_VERSION_MINOR > 0
	if ( 0 != f->open(f, NULL, dbfile.str(), NULL, DB_HASH, DB_CREATE, 0666))
#else
	if ( 0 != f->open(f, dbfile.str(), NULL, DB_HASH, DB_CREATE, 0666))
#endif
	{	
		f->close(f, 0);
		f=NULL;
		return false;
	}

	// f->set_errfile(f, stderr);
	return (dbopen = true);
}

// ========================================================================
// METHOD ::close
// ========================================================================
void db4file::close (void)
{
	if (f) f->close (f, 0);
	f = NULL;
	dbopen = false;
}

// ========================================================================
// METHOD ::recordexists
// ========================================================================
bool db4file::recordexists (const statstring &id)
{
	int ret;
	if (! dbopen) throw (dbfileNotOpenException());

	returnclass (string) res retain;
	
	DBT key = db4_DBT_from_string (id.sval());
	DBT rec;
	memset(&rec, 0, sizeof(rec));
	
	rec.flags = DB_DBT_MALLOC;
	ret = f->get (f, NULL, &key, &rec, 0);

	db4_DBT_free (key);
	db4_DBT_free (rec);
	return (ret==0);
}

// ========================================================================
// METHOD ::getrecord
// ========================================================================
string *db4file::getrecord (const statstring &id)
{
	if (! dbopen) throw (dbfileNotOpenException());

	returnclass (string) res retain;
	
	DBT key = db4_DBT_from_string (id.sval());
	DBT rec;
	
	rec.flags = DB_DBT_MALLOC;
	if( 0 != f->get (f, NULL, &key, &rec, 0))
	{
		db4_DBT_free (key);
		res.clear(); // FIXME: exception?
		return &res;
	}
	db4_DBT_to_string (rec, res);
	db4_DBT_free (key);
	return &res;
}

// ========================================================================
// METHOD ::setrecord
// ========================================================================
bool db4file::setrecord (const statstring &id,
						  const string &data,
						  bool create)
{
	if (! dbopen) throw (dbfileNotOpenException());

	int r;
	DBT key = db4_DBT_from_string (id.sval());
	DBT rec = db4_DBT_from_string (data);
	
	r = f->put (f, NULL, &key, &rec, 0);
	
	db4_DBT_free (key);
	db4_DBT_free (rec);
	
	return (r==0);
}

// ========================================================================
// METHOD ::removerecord
// ========================================================================
bool db4file::removerecord (const statstring &id)
{
	if (! dbopen) throw (dbfileNotOpenException());

	int r;
	DBT key = db4_DBT_from_string (id.sval());
	
	r = f->del (f, NULL, &key, 0);
	db4_DBT_free (key);
	
	return (r == 0 || r == DB_NOTFOUND);
}

// ========================================================================
// METHOD ::startloop
// ========================================================================
bool db4file::startloop (void)
{
	DBC *dbcursor;

	if (! dbopen) throw (dbfileNotOpenException());

	if (0 != f->cursor(f, NULL, &dbcursor, 0))
		return false;
	
	db.loopref = dbcursor;
	
	db.inloop = true;

	return nextloop();
}

// ========================================================================
// METHOD ::nextloop
// ========================================================================
bool db4file::nextloop (void)
{
	if (! dbopen) throw (dbfileNotOpenException());

	DBC *dbcursor = (DBC *) db.loopref;

	DBT key, rec;
	memset(&key, 0, sizeof(key));
	memset(&rec, 0, sizeof(rec));
	key.flags =	rec.flags = DB_DBT_MALLOC;
	
	int ret;
	ret = dbcursor->c_get (dbcursor, &key, &rec, DB_NEXT_NODUP);
	if (0 != ret)
	{
		dbcursor->c_close(dbcursor);
		db.loopref = NULL;
		db.inloop = false;
		return false;
	}

	string enc;
	enc.strcpy ((char *) key.data, key.size);
	db._id = enc;

	db4_DBT_to_string (rec, enc);
	decode (enc, db.v);
	return true;
}

// ========================================================================
// METHOD ::filesync
// ========================================================================
bool db4file::filesync (void)
{
	if (! dbopen) throw (dbfileNotOpenException());
	return (f->sync(f, 0) == 0);
}
