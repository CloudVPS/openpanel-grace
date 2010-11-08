// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

// ========================================================================
// dictionary.cpp: GRACE generic object dictionary with string keys
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

