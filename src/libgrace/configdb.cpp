// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

// ========================================================================
// configdb.cpp: Utility classes for the configdb template class.
// ========================================================================
#include <grace/configdb.h>

// ========================================================================
// CONSTRUCTOR keypath
// ========================================================================
keypath::keypath (void)
{
	array = NULL;
	acount = 0;
	asize = 0;
	crsr = NULL;
	cvpos = cindex = 0;
}

// ========================================================================
// COPY CONSTRUCTOR keypath
// ========================================================================
keypath::keypath (const keypath &orig)
{
	array = NULL;
	acount = 0;
	asize = 0;
	crsr = NULL;
	cvpos = cindex = 0;
	for (int i=0; i<orig.count(); ++i) add (orig[i]);
}

// ========================================================================
// COPY CONSTRUCTOR keypath
// ========================================================================
keypath::keypath (keypath *orig)
{
	crsr = NULL;
	cvpos = cindex = 0;
	array = orig->array;
	acount = orig->acount;
	asize = orig->asize;
	orig->array = NULL;
	orig->acount = orig->asize = 0;
	delete orig;
}

// ========================================================================
// CONSTRUCTOR keypath (from c-string)
// ========================================================================
keypath::keypath (const char *v)
{
	value splt;

	array = NULL;
	acount = 0;
	asize = 0;
	crsr = NULL;
	cvpos = cindex = 0;

	splt = strutil::split (v, '/');
	foreach (e, splt) add (e);
}

keypath::~keypath (void)
{
	if (array)
	{
		for (unsigned i=0; i<acount; ++i)
		{
			delete array[i];
			array[i] = NULL;
		}
		acount = 0;
		::free (array);
	}
}

// ========================================================================
// ARRAY OPERATOR keypath
// ========================================================================
const statstring &keypath::operator[] (int ppos) const
{
	static statstring null;
	int pos = ppos;
	
	if (! array)
	{
		return null;
	}
	if (pos>((int)acount))
	{
		return null;
	}
	if (pos<0)
	{
		pos += acount;
		if (pos<0)
		{
			return null;
		}
	}
	return *(array[pos]);
}

// ========================================================================
// METHOD keypath::count
// ---------------------
// Return the number of nodes in this path.
// ========================================================================
int keypath::count (void) const
{
	return acount;
}

// ========================================================================
// METHOD keypath::clear
// ---------------------
// De-allocate the path array.
// ========================================================================
void keypath::clear (void)
{
	if (! array) return;

	for (unsigned i=0; i<acount; ++i)
	{
		delete array[i];
		array[i] = NULL;
	}
	acount = 0;
}

// ========================================================================
// METHOD keypath::add
// ========================================================================
void keypath::add (const statstring &s)
{
	grow1();
	array[acount] = new statstring (s);
	++acount;
}

void keypath::add (const string &s)
{
	grow1();
	array[acount] = new statstring (s);
	++acount;
}

void keypath::add (const char *s)
{
	grow1();
	array[acount] = new statstring (s);
	++acount;
}

void keypath::add (const value &s)
{
	grow1();
	array[acount] = new statstring (s.sval());
	++acount;
}

// ========================================================================
// METHOD keypath::up
// ------------------
// Go up one directory (deleting the bottom node).
// ========================================================================
void keypath::up (void)
{
	if (acount)
	{
		delete array[acount-1];
		array[acount-1] = NULL;
		--acount;
	}
}

// ========================================================================
// METHOD keypath::existsin
// ------------------------
// Returns true if the keypath can be found inside the tree of a value
// object.
// ========================================================================
bool keypath::existsin (value &val) const
{
	visitor<value> probe (val);
	unsigned int x;
		
	for (x=0; x<acount; ++x)
	{
		if ((*(array[x])) == "*")
		{
			if (! probe.first()) return false;
		}
		else
		{
			if (! probe.enter (*(array[x]))) return false;
		}
	}
	return true;
}

// ========================================================================
// METHOD keypath::begin
// ---------------------
// Begin a round of iteration over multiple possibilities in a keypath
// (where a node is called "*" indicating the concept of a 'for each').
// ========================================================================
void keypath::begin (value &val, keypath &respos)
{
	crsr = new visitor<value> (val);
	respos.clear();
	cindex = 0;
	
	for (unsigned int i=0; i<acount; ++i)
	{
		if ( (*(array[i])) == "*")
		{
			respos.add ("*");
			crsr->first();
		}
		else
		{
			respos.add (*(array[i]));
			crsr->enter (*(array[i]));
		}
	}
}

// ========================================================================
// METHOD keypath::get
// -------------------
// When inside a loop started by keypath::begin() this returns the
// object at the full path under the current cursor.
// ========================================================================
const value &keypath::get (void) const
{
	return crsr->obj();
}

// ========================================================================
// METHOD keypath::next
// --------------------
// Checks for the next possibility when going over a 'for each' loop on
// a value object. If we're at the last possibility, it returns false.
// ========================================================================
bool keypath::next (keypath &respos)
{
	while ((respos.count()) && (respos[-1] != "*"))
	{
		respos.up();
		crsr->up();
	}

	if (! respos.count()) return false;
	respos.up();
	crsr->up();
	
	while ((unsigned int) respos.count() < acount)
	{
		if ( (*(array[respos.count()])) == "*")
		{
			respos.add ("*");
			crsr->first();
			cindex++;
			for (int i=0; i<cindex; ++i)
			{
				if (! crsr->next())
				{
					delete crsr;
					crsr = NULL;
					cindex = 0;
					return false;
				}
			}
		}
		else
		{
			crsr->enter (*(array[respos.count()]));
			respos.add (*(array[respos.count()]));
		}
	}
	return true;
}

// ========================================================================
// METHOD keypath::grow1
// ---------------------
// Increase the array size and counter.
// ========================================================================
void keypath::grow1 (void)
{
	unsigned int oasize;
	if (! asize)
	{
		asize = 8;
		array = (statstring **) malloc (asize * sizeof (statstring*));
		for (int i=0; i<8; ++i) array[i] = NULL;
	}
	else
	{
		if ((acount+1)>= asize)
		{
			oasize = asize;
			asize = asize*2;
			array = (statstring **) realloc (array, asize*sizeof (statstring*));
			for (unsigned int i=oasize; i<asize; ++i)
				array[i] = NULL;
		}
	}
}

// ========================================================================
// CONSTRUCTOR tsdb
// ========================================================================
tsdb::tsdb (void)
{
	first = last = NULL;
}

// ========================================================================
// DESTRUCTOR keypath
// ========================================================================
tsdb::~tsdb (void)
{
}

// ========================================================================
// METHOD tsdb::get
// ----------------
// Access a specific node inside the thread-specific database.
// ========================================================================
const value &tsdb::get (const value &from, time_t ti)
{
	pthread_t whom = __THREADED ? pthread_self() : (pthread_t) 1;
	lck.lockw();
	tsdbentry *c = first;
	while (c)
	{
		if (c->assocthr == whom)
		{
			if (c->lastupdate != ti)
			{
				c->dat = from;
				c->lastupdate = ti;
			}
			lck.unlock();
			return c->dat;
		}
		c = c->next;
	}
	
	c = new tsdbentry;
	c->assocthr = whom;
	c->lastupdate = ti;
	c->dat = from;
	if (last)
	{
		c->next = NULL;
		c->prev = last;
		last->next = c;
		last = c;
	}
	else
	{
		c->next = c->prev = NULL;
		first = last = c;
	}

	lck.unlock();	
	return c->dat;
}
