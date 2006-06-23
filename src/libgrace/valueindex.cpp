// ========================================================================
// valueindex.cpp: GRACE indexing class on arbitrary keys of value objects
//
// (C) Copyright 2003-2006 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam 
// ========================================================================

#include <grace/valueindex.h>

typedef value *valueptr_t;

// ========================================================================
// CONSTRUCTOR
// -----------
// Initializes an indexreference instance with the string 'id' as
// the referenced key and a pointer-to-valueindex 'p' as the parent
// valueindex instance.
// 
// The indexreference class acts as a storage for pointers to
// value objects that all share a common key attribute.
// ========================================================================
indexreference::indexreference (const statstring &id, class valueindex *p) :
                _refvalue (id)
{
	// Initialize values
	higher = lower = NULL;
	_refarray = (value **) calloc (sizeof (valueptr_t), 4);
	_count = 0;
	_arraysz = 4;
	higher = NULL;
	lower = NULL;
	_refvalue = id;
	
	// If the parent valueindex has no root, we are the firstborn
	if (p->root == NULL)
	{
		p->root = this;
	}
	else
	{
		// Appearantly there are others, so let's find ourselves
		// a cozy place in the tree where there's not too much
		// rain or falling acorns.
		
		indexreference *i = p->root;
		while (i)
		{
			if (_refvalue.key() < i->_refvalue.key())
			{
				if (i->lower) i = i->lower;
				else
				{
					i->lower = this;
					i = NULL;
				}
			}
			else
			{
				if (i->higher) i = i->higher;
				else
				{
					i->higher = this;
					i = NULL;
				}
			}
		}
	}
}

// ========================================================================
// DESTRUCTOR
// ----------
// Nuke the array. Trigger our siblings to kill themselves too.
// Cultish isn't it?
// ========================================================================
indexreference::~indexreference (void)
{
	free (_refarray);
	if (higher) delete higher;
	if (lower) delete lower;
}

// ========================================================================
// METHOD addreference
// -------------------
// Adds a value pointer to the array, growing it as needed.
// ========================================================================
void indexreference::addreference (value *v)
{
	if (_count >= _arraysz)
	{
		_arraysz = _arraysz * 2;
		_refarray = (value **) 
			realloc (_refarray, _arraysz * sizeof (valueptr_t));
	}
	_refarray[_count++] = v;
}

// ========================================================================
// METHOD operator[]
// -----------------
// Returns a specific value in the array by reference.
// ========================================================================
value &indexreference::operator[] (int i)
{
	static value *nilvalue = new value;
	if (i<0) return (*nilvalue);
	if (i>_count) return (*nilvalue);
	
	return *(_refarray[i]);
}

// ========================================================================
// METHOD count
// ------------
// Returns the number of items in the array.
// ========================================================================
int indexreference::count (void)
{
	return _count;
}

// ========================================================================
// CONSTRUCTOR
// -----------
// Initializes values, not very exciting.
// ========================================================================
valueindex::valueindex (void)
{
	root = NULL;
}

// ========================================================================
// DESTRUCTOR
// ----------
// If there is a 'root' indexreference instance, delete it. It will
// take care of the others.
// ========================================================================
valueindex::~valueindex (void)
{
	if (root)
	{
		delete root;
		root = NULL;
	}
}

// ========================================================================
// METHOD indextypes
// -----------------
// For a given value, indexes its direct child nodes by their
// type().
// ========================================================================
void valueindex::indextypes (value &v)
{
	statstring typ;
	
	if (root)
	{
		delete root;
		root = NULL;
	}
	
	foreach (node, v)
	{
		indexreference *r;
		typ = node.type();
		
		r = find (typ);
		if (!r)
		{
			r = new indexreference (typ, this);
		}
		r->addreference (&node);
	}	
}

// ========================================================================
// METHOD indexproperty
// --------------------
// For a given value object, index its direct child nodes by a given
// property.
// ========================================================================
void valueindex::indexproperty (value &v, const statstring &prop)
{
	if (root)
	{
		delete root;
		root = NULL;
	}
	
	foreach (node, v)
	{
		indexreference *r;
		statstring pval = node(prop).sval();
		
		r = find (pval);
		if (! r)
		{
			r = new indexreference (pval, this);
		}
		r->addreference (&node);
	}
}

// ========================================================================
// METHOD indexrecord
// ------------------
// For a given value object, index its direct children by the
// data of a specified child node.
// ========================================================================
void valueindex::indexrecord (value &v, const statstring &rec)
{
	statstring rval;
	
	if (root)
	{
		delete root;
		root = NULL;
	}
	
	foreach (node, v)
	{
		indexreference *r;
		rval = node[rec].sval();
		
		r = find (rval);
		if (! r)
		{
			r = new indexreference (rval, this);
		}
		r->addreference (&node);
	}
}

// ========================================================================
// METHOD operator[]
// -----------------
// Returns the indexreference object for a specific key, creating
// a new one if it didn't exist yet.
// ========================================================================
indexreference &valueindex::operator[] (const statstring &id)
{
	indexreference *r;
	
	r = find (id);
	if (! r)
	{
		r = new indexreference (id, this);
	}
	
	return *r;
}

// ========================================================================
// METHOD first
// ------------
// Returns the first result of an indexreference set.
// ========================================================================
value &valueindex::first (const statstring &id)
{
	indexreference *r;
	
	r = find(id);
	if (! r)
	{
		r = new indexreference (id, this);
	}
	
	return (*r)[0];
}

// ========================================================================
// METHOD exists
// -------------
// Indicates whether a given key has been recorded in the index.
// ========================================================================
bool valueindex::exists (const statstring &id)
{
	indexreference *r;
	
	r = find (id);
	if (! r) return false;
	return true;
}

// ========================================================================
// METHOD find
// -----------
// Returns a pointer to the indexreference object holding the array
// for a given key, or NULL if none exist.
// ========================================================================
indexreference *valueindex::find (const statstring &id)
{
	indexreference *r = root;
	
	while (r)
	{
		if (r->_refvalue == id) return r;
		if (id.key() < r->_refvalue.key()) r = r->lower;
		else r = r->higher;
	}
	return NULL;
}
