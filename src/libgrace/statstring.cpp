// ========================================================================
// statstring.cpp: Static key/string storage.
//
// (C) Copyright 2003-2006 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam 
// ========================================================================
#include <grace/statstring.h>
#include <grace/thread.h>

statstring nokey;

// ========================================================================
// FUNCTION dumpstringref
// ----------------------
// Bound to SIGUSR1 for debugging purposes.
// ========================================================================
void dumpstringref (int signal)
{
	STRINGREF().print ("stringrefs.dat");
}

void nukedb (void *thedb)
{
	delete (stringrefdb *) thedb;
}

stringrefdb *__REF;

stringrefdb &STRINGREF()
{
	static stringrefdb *REF = new stringrefdb;
	__REF = REF;
	return *REF;
	
	// not anymore?
	/*
	static bool createdkey = false;
	static pthread_key_t key; 
	if (! createdkey)
	{
		pthread_key_create (&key, nukedb);
	}
	stringrefdb *REF = (stringrefdb *) pthread_getspecific (key);
	if (! REF)
	{
		REF = new stringrefdb;
		pthread_setspecific (key, REF);
	}
	return *REF;*/
}

// ========================================================================
// DESTRUCTOR stringrefdb
// ----------------------
// Uses rmref to recursively delete the stringref tree
// ========================================================================
stringrefdb::~stringrefdb (void)
{
	if (root)
	{
		rmref (root);
		root = NULL;
	}
}

// ========================================================================
// METHOD stringrefdb::print
// -------------------------
// Write out the db contents to a file.
// ========================================================================
void stringrefdb::print (const char *fname)
{
	FILE *f = ::fopen (fname, "w");
	print (root, f);
	::fclose (f);
}

void stringrefdb::print (stringref *ref, FILE *output)
{
	fprintf (output,"%08x,%i,%i,\"%s\"\n", ref->key, ref->id,
			 ref->refcnt, ref->str.str());
	if (ref->lower) print (ref->lower, output);
	if (ref->higher) print (ref->higher, output);
}

// ========================================================================
// METHOD stringrefdb::rmref
// -------------------------
// The recursive tree eater
// ========================================================================
void stringrefdb::rmref (stringref *ref)
{
	if (ref->lower) rmref (ref->lower);
	if (ref->higher) rmref (ref->higher);
	delete ref;
}

// ========================================================================
// METHOD stringrefdb::unref
// -------------------------
// Decrease a stringref's reference count, if it is 0, remove it from
// the tree and relink any children.
// ========================================================================
void stringrefdb::unref (stringref *ref)
{
	exclusivesection (treelock)
	{
		ref->refcnt--;
		
		if (ref->parent && (ref->refcnt == 0))
		{
			unsigned int oldcount;
			unsigned int sz = ref->str.strlen();
			
			exclusivesection (dirtycount)
			{
				oldcount = dirtycount;
				dirtycount += ref->str.strlen();
				if ((dirtycount > 65536) || (sz && (dirtycount <= oldcount)))
				{
					dirtycount = 0;
					breaksection
					{
						reap (root);
						cleanups++;
					}
				}
			}
		}
	}
}

// ========================================================================
// METHOD stringrefdb::cpref
// -------------------------
// Increment a reference counter.
// ========================================================================
void stringrefdb::cpref (stringref *ref)
{
	exclusivesection (treelock) ref->refcnt++;
}

// ========================================================================
// METHOD stringrefdb::reap
// ------------------------
// Clean up free nodes.
// ========================================================================
void stringrefdb::reap (stringref *ref)
{
	stringref *lower = ref->lower;
	stringref *higher = ref->higher;
	
	if ( (ref->parent) && (ref->refcnt == 0) )
	{
		if (ref->parent->lower == ref)
		{
			ref->parent->lower = NULL;
		}
		else
		{
			ref->parent->higher = NULL;
		}

		if (ref->higher)
		{		
			ref->higher->parent = NULL;
			linkref (ref->higher);
		}
		
		if (ref->lower)
		{
			ref->lower->parent = NULL;
			linkref (ref->lower);
		}
		
		ref->lower = NULL;
		ref->higher = NULL;
		delete ref;
	}
	if (lower) reap (lower);
	if (higher) reap (higher);
	
}

// ========================================================================
// METHOD stringrefdb::newref
// --------------------------
// Allocate and initialize a new stringref structure
// ========================================================================
stringref *stringrefdb::newref (void)
{
	stringref *ref = new stringref;
	ref->parent = NULL;
	ref->lower = NULL;
	ref->higher = NULL;
	ref->key = 0;
	ref->id = newid();
	ref->refcnt = 0; 
	return ref;
}

// ========================================================================
// METHOD stringrefdb::getref
// --------------------------
// Finds a stringref matching our static string and key and returns a
// pointer with the correct refcount. Creates a new stringref if there
// was no suitable prior.
// ========================================================================
stringref *stringrefdb::getref (const char *str, unsigned int key)
{
	if (! key)
	{
		key = checksum (str);
	}
	
	stringref *crsr;
	stringref *oldcrsr;
	unsigned short cnt;
	
	// Need write-lock because we'll add stuff if there's no match.
	exclusivesection (treelock)
	{
		crsr = root;
		oldcrsr = root;
		
		while (crsr)
		{
			oldcrsr = crsr;
			
			if (crsr->key == key)
			{
				if (crsr->str.strcmp (str) == 0)
				{
					crsr->refcnt++;
					cnt = crsr->refcnt;
					breaksection
					{
						if (cnt == 1)
						{
							exclusivesection (dirtycount)
							{
								dirtycount -= crsr->str.strlen();
							}
						}
						return crsr;
					}
				}
				else crsr = crsr->lower;
			}
			else if (crsr->key < key) crsr = crsr->higher;
			else crsr = crsr->lower;
		}
		
		// The oldcrsr points to what will be our parent object,
		// by increasing the refcnt we make sure it will not
		// disappear from under our feet.
		
		crsr = newref ();
		crsr->str = str;
		crsr->refcnt = 1;
		crsr->key = key;
		crsr->parent = oldcrsr;
	
		if (oldcrsr->key < key) oldcrsr->higher = crsr;
		else oldcrsr->lower = crsr;
	}
	return crsr;
}

// ========================================================================
// METHOD stringrefdb::linkref
// ---------------------------
// Adds a stringref object into the tree
// ========================================================================
void stringrefdb::linkref (stringref *ref)
{
	stringref *crsr = root;
	
	while (crsr)
	{
		if (crsr->key == ref->key)
		{
			if (crsr->id == ref->id) return;
			if (crsr->lower) crsr = crsr->lower;
			else
			{
				crsr->lower = ref;
				ref->parent = crsr;
				return;
			}
		}
		else if (crsr->key < ref->key)
		{
			if (crsr->higher) crsr = crsr->higher;
			else
			{
				crsr->higher = ref;
				ref->parent = crsr;
				return;
			}
		}
		else
		{
			if (crsr->lower) crsr = crsr->lower;
			else
			{
				crsr->lower = ref;
				ref->parent = crsr;
				return;
			}
		}
	}
}

// ========================================================================
// METHOD statstring::assign
// -------------------------
// Various methods to copy string-type data into the object
// ========================================================================
void statstring::assign (const string &str)
{
	if (ref)
	{
		STRINGREF().unref (ref);
		ref = NULL;
	}
	
	ref = STRINGREF().getref (str);
}

void statstring::assign (const char *str)
{
	if (ref)
	{
		STRINGREF().unref (ref);
		ref = NULL;
	}
	
	ref = STRINGREF().getref (str);
}

void statstring::assign (string *str)
{
	if (ref)
	{
		STRINGREF().unref (ref);
		ref = NULL;
	}
	
	ref = STRINGREF().getref (str->str());
	delete str;
}

void statstring::assign (const statstring &str)
{
	if (ref)
	{
		STRINGREF().unref (ref);
	}
	
	if (str.ref)
	{
		STRINGREF().cpref (str.ref);
		ref = str.ref;
	}
	else
	{
		ref = NULL;
	}
}

void statstring::assign (statstring *str)
{
	retainvalue (str);
}

statstring &statstring::operator= (const value &orig)
{
	assign (orig.sval());
	return *this;
}

void statstring::assign (const char *str, unsigned int k)
{
	if (ref)
	{
		STRINGREF().unref (ref);
		ref = NULL;
	}
	
	ref = STRINGREF().getref (str,k);
}

void statstring::assign (unsigned int k)
{
	if (ref)
	{
		STRINGREF().unref (ref);
		ref = NULL;
	}
	
	ref = STRINGREF().getref ("",k);
}

void statstring::init (bool first)
{
	if (first) ref = NULL;
	else
	{
		if (ref) STRINGREF().unref (ref);
		ref = NULL;
	}
}
