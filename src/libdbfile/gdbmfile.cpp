#include <dbfile/gdbmfile.h>

datum gdbm_datum_from_string (const string &s)
{
	datum d;
	d.dsize = s.strlen();
	d.dptr = (char *) ::malloc (s.strlen());
	::memcpy (d.dptr, s.str(), s.strlen());
	return d;
}

void gdbm_datum_to_string (datum &d, string &s)
{
	if (! d.dptr) return;
	s.strcpy (d.dptr, d.dsize);
	::free (d.dptr);
	d.dptr = NULL;
	d.dsize = 0;
}

void gdbm_free_datum (datum &d)
{
	if (! d.dptr) return;
	::free (d.dptr);
	d.dptr = NULL;
	d.dsize = 0;
}

bool gdbmfile::open (const string &dbfile)
{
	dbopen = false;
	f = gdbm_open ((char *) dbfile.str(), 512, GDBM_WRITER, 0666, NULL);
	if (! f) return false;
	return (dbopen = true);
}

void gdbmfile::close (void)
{
	if (f) gdbm_close (f);
	f = NULL;
	dbopen = false;
}

bool gdbmfile::recordexists (const statstring &id)
{
	int ret;
	datum key = gdbm_datum_from_string (id.sval());
	
	ret = gdbm_exists (f, key);
	gdbm_free_datum (key);
	return ret ? true : false;
}

string *gdbmfile::getrecord (const statstring &id)
{
	if (! dbopen) throw (dbfileNotOpenException());

	returnclass (string) res retain;
	
	datum key = gdbm_datum_from_string (id.sval());
	datum rec = gdbm_fetch (f, key);
	gdbm_datum_to_string (rec, res);
	gdbm_free_datum (key);
	return &res;
}

bool gdbmfile::setrecord (const statstring &id,
						  const string &data,
						  bool create)
{
	if (! dbopen) throw (dbfileNotOpenException());

	int r;
	datum key = gdbm_datum_from_string (id.sval());
	datum rec = gdbm_datum_from_string (data);
	
	if (create)
	{
		r = gdbm_store (f, key, rec, GDBM_INSERT);
	}
	else
	{
		r = gdbm_store (f, key, rec, GDBM_REPLACE);
	}
	
	gdbm_free_datum (key);
	gdbm_free_datum (rec);
	
	if (! r) return true;
	return false;
}

bool gdbmfile::removerecord (const statstring &id)
{
	if (! dbopen) throw (dbfileNotOpenException());

	int r;
	datum key = gdbm_datum_from_string (id.sval());
	
	r = gdbm_delete (f, key);
	gdbm_free_datum (key);
	
	return (r == 0);
}

bool gdbmfile::startloop (void)
{
	if (! dbopen) throw (dbfileNotOpenException());

	db.inloop = true;
	db.loopref = ::malloc (sizeof (datum));
	((datum*)db.loopref)->dsize = 0;
	((datum*)db.loopref)->dptr = NULL;
	
	(*((datum*)db.loopref)) = gdbm_firstkey (f);
	if (((datum*)db.loopref)->dptr)
	{
		datum content;
		string enc;
		
		content = gdbm_fetch (f, *((datum*)db.loopref));
		if (! content.dptr)
		{
			::free (((datum*)db.loopref)->dptr);
			::free (((datum*)db.loopref));
			db.loopref = NULL;
			db.inloop = false;
			return false;
		}
		
		enc.strcpy (((datum*)db.loopref)->dptr, ((datum*)db.loopref)->dsize);
		db._id = enc;
		
		gdbm_datum_to_string (content, enc);
		decode (enc, db.v);
		
		return true;
	}
	::free (db.loopref);
	db.loopref = NULL;
	db.inloop = false;
	return false;
}

bool gdbmfile::nextloop (void)
{
	if (! dbopen) throw (dbfileNotOpenException());

	datum d;
	d = gdbm_nextkey (f, (*((datum*)db.loopref)));
	::free (((datum*)db.loopref)->dptr);
	
	if (d.dptr)
	{
		datum content;
		
		content = gdbm_fetch (f, d);
		if (! content.dptr)
		{
			gdbm_free_datum (d);
			::free (db.loopref);
			db.loopref = NULL;
			return false;
		}
		
		string enc;
		enc.strcpy (d.dptr, d.dsize);
		db._id = enc;

		gdbm_datum_to_string (content, enc);
		decode (enc, db.v);
		((datum*)db.loopref)->dsize = d.dsize;
		((datum*)db.loopref)->dptr = d.dptr;
		return true;
	}

	::free (db.loopref);
	db.loopref = NULL;
	return false;
}
