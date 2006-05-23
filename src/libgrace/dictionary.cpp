// ========================================================================
// dictionary.cpp: GRACE generic object dictionary with string keys
//
// (C) Copyright 2003-2004 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam 
// ========================================================================

#include <grace/dictionary.h> // This is where the fun is

// ========================================================================
// CONSTRUCTOR
// -----------
// Creates a dictionaryEntry object with a provided key
// ========================================================================

dictionaryEntry::dictionaryEntry (const statstring &iid)
{
	ent = NULL;
	lower = NULL;
	higher = NULL;
	id = iid;
}

// ========================================================================
// DESTRUCTOR
// ----------
// No work to bedone right now.
// ========================================================================

dictionaryEntry::~dictionaryEntry (void)
{
}

