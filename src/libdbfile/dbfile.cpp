#include <dbfile/dbfile.h>
#include <grace/strutil.h>

dbrecord::dbrecord (dbfile *iowner)
{
	init (true);
	owner = iowner;
}

dbrecord::dbrecord (dbfile *iowner, dbrecord *iparent, const statstring &k)
{
	init (true);
	owner = iowner;
	parent = iparent;
	_id = k;
}

dbrecord::~dbrecord (void)
{
}

bool dbrecord::exists (const statstring &k)
{
	if (! owner) throw (dbrecordMemoryCorruption());
	
	return owner->recordexists (k);
}

dbrecord &dbrecord::operator[] (const char *c)
{
	return (*this)[(const statstring &)c];
}

dbrecord &dbrecord::operator[] (const statstring &k)
{
	dbrecord *r = NULL;
	
	// If we're looping, we are either the root node or a child of
	// one of its members.
	if (inloop)
	{
		// Create the member if it doesn't exist.
		if (! members.exists (k))
		{
			r = new dbrecord (owner, this, k);
			members.set (k, r);
		}
		
		// Set the member data from our internal storage: We'll copy
		// the sub-tree out of our value.
		dbrecord &tmp = members[k];
		tmp.inloop = true;
		tmp.v = v[k];
		return tmp;
	}
	
	// If we're not the root record, we're looking for member variables.
	if (parent)
	{
		// Do we already have one in our members dictionary?
		if (members.exists (k))
		{
			// Great, get a reference and refresh it with the latest data.
			dbrecord &tmp = members[k];
			tmp.v = v[k];
			return tmp;
		}
		
		// We're creating a new cache-record from scratch.
		r = new dbrecord (owner, this, k);
		r->v = v[k];
		members.set (k, r);
		return *r;
	}
	
	// We're the root-record.
	// First let's see if we already have this record in our cache,
	// return that if so.
	if (children.exists (k)) return children[k];
	
	// Bummer, we'll create it a new.
	r = new dbrecord (owner, this, k);
	
	// Does the record already exist in the database?
	if (exists (k))
	{
		// Yes, load it.
		string rec = owner->getrecord (k);
		owner->decode (rec, r->v);
	}
	else
	{
		// No, mark it as a new creation.
		r->create = true;
	}
	children.set (k, r);
	return *r;
}

void dbrecord::rmval (const statstring &k)
{
	if (inloop)
	{
		throw (dbrecordAssignDuringLoopException());
	}
	
	if (! parent) owner->rmval (k);
	members.clear();
	v.rmval (k);
	changed = true;
}

void dbrecord::fromvalue (const value &o)
{
	if (inloop)
	{
		throw (dbrecordAssignDuringLoopException());
	}
	
	if (! parent)
	{
		throw (dbrecordAssignRootException());
	}
	
	v = o;
	changed = true;
	
	dbrecord *crsr = this;
	
	while (crsr->parent->parent)
	{
		crsr->parent->changed = true;
		crsr->parent->v[crsr->_id] = crsr->v;
		crsr = crsr->parent;
	}
}

void dbrecord::tovalue (value &into)
{
	into = v;
}

void dbrecord::init (bool first)
{
	if (! first)
	{
		children.clear ();
		members.clear ();
	}
	else
	{
		parent = NULL;
		owner = NULL;
		_id = nokey;
		loopref = NULL;
		inloop = changed = create = false;
	}
}

dbfile::dbfile (void) : db (this)
{
	dbopen = false;
	encoding = attriblist;
	sep = ',';
}

dbfile::~dbfile (void)
{
}

bool dbfile::commit (void)
{
	for (int i=0; i<db.children.count(); ++i)
	{
		string enc;
		encode (enc, db.children[i].v);
			
		if (db.children[i].create)
		{
			if (!setrecord (db.children[i].id(), enc, true))
				return false;
		}
		else
		{
			if (!setrecord (db.children[i].id(), enc, false))
				return false;
		}
		
		db.children[i].changed = false;
		db.children[i].create = false;
		db.children[i].members.clear();
	}
	return true;
}

bool dbfile::rmval (const statstring &id)
{
	if (! removerecord (id)) return false;
	db.children.clear();
	return true;
}

bool dbfile::recordexists (const statstring &id)
{
	return false;
}

string *dbfile::getrecord (const statstring &id)
{
	return NULL;
}

bool dbfile::setrecord (const statstring &id, const string &s, bool c)
{
	return false;
}

bool dbfile::removerecord (const statstring &id)
{
	return false;
}

bool dbfile::startloop (void)
{
	return false;
}

bool dbfile::nextloop (void)
{
	return false;
}

dbrecord *dbrecord::visitchild (int pos)
{
	if (! pos)
	{
		if (! owner->startloop ())
			return NULL;
	}
	else
	{
		if (! owner->nextloop ())
			return NULL;
	}
	return this;
}

void dbfile::encode (string &into, value &v)
{
	into.crop ();
	switch (encoding)
	{
		case shox:
			into = v.toshox();
			break;
		
		case attriblist:
			foreach (node, v)
			{
				if (into) into.strcat (sep);
				into.printf ("%s=\"%S\"", node.id().str(), node.cval());
			}
			break;
		
		case courierdb:
			foreach (node, v)
			{
				if (into) into.strcat ('|');
				into.printf ("%s=%s", node.id().str(), node.cval());
			}
			break;
		
		default:
			into = v.sval();
			break;
	}
}

void dbfile::decode (const string &outof, value &v)
{
	value tmp;
	string l, r;
	v.clear ();
	
	switch (encoding)
	{
		case shox:
			v.fromshox (outof);
			break;
		
		case attriblist:
			v = strutil::splitquoted (outof, sep);
			break;
		
		case courierdb:
			tmp = strutil::split (outof, '|');
			foreach (word, tmp)
			{
				r = word;
				l = r.cutat ('=');
				v[l] = r;
			}
			break;
		
		default:
			v = outof;
			break;
	}
}
