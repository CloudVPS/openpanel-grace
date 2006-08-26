#ifndef _DICTIONARY_H
#define _DICTIONARY_H 1

#include <stdlib.h>

#include <grace/str.h>
#include <grace/statstring.h>
#include <grace/exception.h>

/// A container. Keeps a pointer to a generic object, used in the
/// dictionary class.
class dictionaryEntry
{
public:
					 /// Constructor.
					 /// \param entryid The key for this object.
					 dictionaryEntry (const statstring &entryid);
					~dictionaryEntry (void);

	void			*ent; ///< Pointer to the entry.
	statstring		 id; ///< Key of this object.
	bool			 dynamic; ///< True if the pointer should be deleted.
	dictionaryEntry	*lower, *higher; ///< Hashtree pointers.
};

THROWS_EXCEPTION
(
	dictIndexOutOfBoundsException,
	0x1bef3ab0,
	"Dictionary array index out of range"
);

typedef class dictionaryEntry *dictPtr;

/// Keyed collection of objects.
/// Keeps an indexed collection of objects, using string keys.
template<class kind>
class dictionary
{
public:
					 dictionary (void)
					 {
					 	_array = NULL;
					 	_count = _arraysz = 0;
					 }
					~dictionary (void)
					 {
					 	clear ();
					 }

	void			 clear (void)
					 {
						for (int i=0; i<_count; ++i)
						{
							if (_array[i])
							{
								if (_array[i]->dynamic)
									delete (kind *) _array[i]->ent;
							}
						}
						free (_array);
						_count = _arraysz = 0;
						_array = NULL;
					 }
					 
					 /// Array access.
					 /// Looks up an entry by key.
					 /// \param st The entry's key.
					 /// \return Reference to the object.
	kind			&operator[] (const statstring &st)
					 {
						dictionaryEntry *de;
						
						de = demand (st);
						return *((kind *) de->ent);
					 }
					 /// Array access.
					 /// Looks up an entry by key.
					 /// \param st The entry's key.
					 /// \return Reference to the object.
	kind			&operator[] (const string &st)
					 {
					 	statstring id (st);
					 	dictionaryEntry *de;
					 	
					 	de = demand (id);
					 	return *((kind *) de->ent);
					 }
					 
					 /// Array access.
					 /// Looks up an entry by key.
					 /// \param st The entry's key.
					 /// \return Reference to the object.
	kind			&operator[] (const char *st)
					 {
					 	statstring id (st);
					 	dictionaryEntry *de;
					 	
					 	de = demand (id);
					 	return *((kind *) de->ent);
					 }
					 
					 /// Array access.
					 /// Looks up an entry by array index.
					 /// \param idnex The entry's index.).
					 /// \return Reference to the object.
	kind			&operator[] (int index)
					 {
					 	if ((! _array)||(index < 0)||(index >= _count))
					 		throw (dictIndexOutOfBoundsException());
					 	
					 	return *((kind *) _array[index]->ent);
					 }
					 
					 /// Associate an object with a key.
					 /// \param id The key for the object.
					 /// \param to Pointer to the object.
	void			 set (const statstring &id, kind *to)
					 {
					 	dictionaryEntry *de = demand (id, false);
					 	if (de->ent && de->dynamic) delete (kind *) de->ent;
					 	de->ent = to;
					 	de->dynamic = true;
					 }
	
	void			 set (const statstring &id, kind &to)
					 {
					 	dictionaryEntry *de = demand (id, false);
					 	if (de->ent && de->dynamic) delete de->ent;
					 	de->ent = &to;
					 	de->dynamic = false;
					 }
					 /// Check the dictionary for a key.
					 /// \param s The key.
					 /// \return Result, \b true if the key is in the dictionary.
	bool			 exists (const statstring &s)
					 {
						if (_count == 0)
						{
							return false;
						}
						dictionaryEntry *crsr = _array[0];
						while (crsr)
						{
							if (crsr->id == s) return true;
							if (crsr->id.key() < s.key())
							{
								if (crsr->higher) crsr = crsr->higher;
								else return false;
							}
							else
							{
								if (crsr->lower) crsr = crsr->lower;
								else return false;
							}
						}
						return false;
					 }
					 
					 /// Get item count.
					 /// \return Number of objects in the dictionary.
	int				 count (void) { return _count; }

protected:
	class dictionaryEntry	
				   **_array; ///< The internal array.
	int				 _arraysz; ///< Allocated size of the array.
	int				 _count; ///< Number of entries in the array.

					 /// Adds a new entry to the array (without linking it
					 /// into the tree).
					 /// \param id The object's key.
					 /// \param alloc If \e false, no new objects will be
					 ///              allocated - \b OBSOLETE.
					 /// \return Pointer to the new dictionaryEntry object
					 ///         which can contain a pointer to the
					 ///         element object.
dictionaryEntry 	*newentry (const statstring &id, bool alloc=true)
					 {
						if (! _arraysz)
						{
							_arraysz = 8;
							_array = (dictPtr *)
								calloc (_arraysz, sizeof (dictPtr));
							_count = 0;
						}
						if ( (_count+1) >= _arraysz)
						{
							_arraysz = _arraysz << 1;
							_array = (dictPtr *) realloc (_array, _arraysz * sizeof (dictPtr));
						}
						if ( (_count+1) < _arraysz)
						{
							_array[_count] = new dictionaryEntry(id);
							_array[_count]->ent = alloc ? new kind : NULL;
							_array[_count]->id = id;
							_array[_count]->dynamic = alloc ? true : false;
							_count++;
						}
						return _array[_count-1];
					 }
					 /// Look up an entry in the dictionary.
					 /// \param s The object's key.
					 /// \param alloc If \e false, no new objects will be
					 ///              allocated.
					 /// \return Pointer to the dictionaryObject associated
					 ///         with the key.
dictionaryEntry 	*demand (const statstring &s, bool alloc=true)
					 {
						if (_count == 0)
						{
							return newentry (s, alloc);
						}
						dictionaryEntry *crsr = _array[0];
						while (crsr)
						{
							if (crsr->id == s) return crsr;
							if (crsr->id.key() < s.key())
							{
								if (crsr->higher) crsr = crsr->higher;
								else
								{
									crsr->higher = newentry (s, alloc);
									return crsr->higher;
								}
							}
							else
							{
								if (crsr->lower) crsr = crsr->lower;
								else
								{
									crsr->lower = newentry (s, alloc);
									return crsr->lower;
								}
							}
						}
						
						return NULL;
					 }

					 /// Look up an entry in the dictionary by its numeric key.
					 /// \param key The object's numeric key.
					 /// \param alloc If \e false, no new objects will be
					 ///              allocated.
					 /// \return Pointer to the dictionaryObject associated
					 ///         with the key.
dictionaryEntry 	*demand (unsigned int key, bool alloc=true)
					 {
						statstring s;
						
						s = key;
						
						if (_count == 0)
						{
							return newentry (s);
						}
						dictionaryEntry *crsr = _array[0];
						while (crsr)
						{
							if (crsr->id.key() == key) return crsr;
							if (crsr->id.key() < key)
							{
								if (crsr->higher) crsr = crsr->higher;
								else
								{
									crsr->lower = newentry (s, alloc);
									return crsr->lower;
								}
							}
							else
							{
								if (crsr->lower) crsr = crsr->lower;
								else
								{
									crsr->higher = newentry (s, alloc);
									return crsr->higher;
								}
							}
						}
					 }
};

#include <grace/array.h>

/// Keeps a collection of text strings, with each unique string getting
/// a unique serial number. Used for keeping compact serialization.
class stringdict
{
public:
								 /// Constructor.
								 stringdict (void);
								 
								 /// Destructor.
								~stringdict (void);
	
								 /// Get the position for the provided
								 /// string, a new position will be created
								 /// if the string is new to the dict.
	unsigned int				 get (const statstring &);
	
								 /// Return the string for a numbered position.
	statstring					*get (unsigned int);
	
								 /// Return the number of strings in the dict.
	unsigned int				 count (void);

protected:
	dictionary<unsigned int>	 bystring; ///< Indexed by int id.
	array<statstring>			 byid; ///< Array of statstrings.
};

#endif
