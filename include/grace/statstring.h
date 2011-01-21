// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _STATSTRING_H
#include <grace/value.h>
#define _STATSTRING_H 1

#include <grace/str.h>
#include <grace/checksum.h>
#include <grace/lock.h>
#include <grace/case.h>
#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>

#ifndef _STRINGREF_T
#define _STRINGREF_T 1

/// Memory structure for a string reference.
/// Statstrings are oftenly used as keys. To save memory, statstring
/// objects reference a global database of string data. Multiple
/// statstring objects containing the same literal string will point
/// to the same stringref data in memory. The stringrefdb class does
/// the bookkeeping.
struct stringref
{
	stringref					*parent; ///< Link to parent node in the hash tree.
	stringref					*lower, *higher; ///< Link to siblings in the hash tree.
	volatile unsigned int		 key; ///< This string's hash key.
	volatile unsigned int		 id; ///< This stringref's unique id.
	volatile unsigned int		 refcnt; ///< Reference count.
	string						 str; ///< Actual string data.
};

#endif

#ifndef _STRINGREFDB_T
#define _STRINGREFDB_T 1

/// Signal handler for USR1.
/// Dumps the stringref database to disk.
void dumpstringref (int signal);

extern statstring nokey;

/// Stringref database.
/// Keeps a global collection of stringref objects.
class stringrefdb
{
friend class process;
public:
							 stringrefdb (void)
							 {
							 	root = newref();
							 	root->key = 0x80000000;
							 	nukeroot = NULL;
							 	sequence = 0;
							 	dirtycount.o = 0;
							 	cleanups = 0;
							 	signal (SIGUSR1, dumpstringref);
							 }
							~stringrefdb (void);
	
							 /// Write tree to disk.
							 /// \param fn File name.
	void					 print (const char *fn);
	
							 /// Write node to a file.
							 /// Writes a stringref's data to a file.
							 /// Calls itself recursively for its
							 /// child links.
							 /// \param ref The stringref object.
							 /// \param f The stdio file.
	void					 print (stringref *ref, FILE *f);
	
							 /// Find or allocate reference.
							 /// Finds a stringref in its database, or
							 /// creates a new one for a unique string.
							 /// \param str The string data
							 /// \param key The calculated checksum key, or 0
							 ///            if this has not been calculated yet.
							 /// \return Pointer to the associated reference.
	stringref				*getref (const char *str, unsigned int key=0);
	
							 /// Copy a reference (add 1 to refcount).
	void					 cpref (stringref *ref);
	
							 /// Down reference count.
							 /// Lowers the reference count for an allocated
							 /// stringref object.
	void					 unref (stringref *r);
	
							 /// Remove unreferenced nodes.
							 /// Goes over a node and its children, de-
							 /// allocates any nodes that no longer have
							 /// live references.
	void					 reap (stringref *);
	
							 /// Generate new unique id.
							 /// Each new node in the stringref database
							 /// gets a unique id. This is implemented
							 /// using a simple sequence counter.
							 /// \return new id.
	unsigned int			 newid (void)
							 {
							 	unsigned int res;
							 	res = sequence++;
							 	
							 	return res;
							 }
							 
protected:
	stringref				*root; ///< Root node of the database.
	stringref				*nukeroot; ///< Root node of deleted entries
	unsigned int			 sequence; ///< Current sequence number.
	lock<int>				 treelock; ///< Lock for the database.
	
							 /// Remove a reference from the tree.
							 /// \param ref Pointer to the stringref.
	void					 rmref (stringref *ref);
	
							 /// Link in new stringref.
							 /// Adds a newly allocated stringref to
							 /// the hash tree.
	void					 linkref (stringref *);
	
							 /// Allocate a new stringref.
							 /// \return Pointer to the new object.
	stringref				*newref (void);
	
							 /// Number of bytes worth of de-allocated
							 /// nodes. Used by reap() to determine when
							 /// to spring into action (going over the
							 /// tree implies locking).
	lock<unsigned int>		 dirtycount;
	
	int						 cleanups; ///< Number of times reaped.
};

#endif

/// Returns reference to the global stringref database.
stringrefdb &STRINGREF(void);

#ifndef _STATSTRING_T
#define _STATSTRING_T 1

/// A static keyed string.
/// Storage for a mostly immutable string of text together with a
/// checksum key that makes it easy for keyed access methods of
/// other classes.
class statstring : public memory::retainable
{
public:
						 /// Constructor. Creates a 'nil' object.
						 statstring (void)
						 {
						 	ref = NULL;
						 }
						 
						 /// Constructor. Copy from C string.
						 statstring (const char *str)
						 {
						 	ref = NULL;
						 	assign (str);
						 }
						 
						 /// Cnstrucotr. Copy from unsigned C-string.
						 statstring (const unsigned char *str)
						 {
						 	ref = NULL;
						 	assign ((const char *) str);
						 }
						 
						 /// Copy-constructor.
						 statstring (const string &str)
						 {
						 	ref = NULL;
						 	assign (str);
						 }
						 
						 /// Copy-constructor. Deletes original.
						 statstring (string *str)
						 {
						 	ref = NULL;
						 	assign (str);
						 }
						 
						 /// Constructor. Copy from C string with known key.
						 /// \param str The string data.
						 /// \param k The calculated key.
						 statstring (const char *str, unsigned int k)
						 {
						 	ref = NULL;
						 	assign (str, k);
						 }
						 
						 /// Copy-constructor.
						 statstring (const statstring &str)
						 {
						 	ref = NULL;
						 	assign (str);
						 }
						 
						 /// Copy-constructor (retained).
						 statstring (statstring *str)
						 {
						 	ref = NULL;
						 	retainvalue (str);
						 }
						 
						 /// Copy-constructor (from retained value).
						 statstring (class value *);
						 
						 /// Copy-constructor (from value)
						 statstring (const class value &);
					 
					 	 /// Destructor. Remove reference from the
					 	 /// global ref table.
						~statstring (void)
						 {
						 	if (ref) STRINGREF().unref (ref);
						 	ref = NULL;
						 }
	
						 /// Assign to a string-less key.
	void				 assign (unsigned int);

						 /// Assign to data from a string object.
	void				 assign (const string &);
	
						 /// Assign to data from a string object.
						 /// Delete original after copying.
	void				 assign (string *);
	
						 /// Assign to C string.
	void				 assign (const char *);
	
						 /// Assign to C string with known key.
	void				 assign (const char *, unsigned int);
	
						 /// Assign from other statstring object.
	void				 assign (const statstring &);
	
						 /// Assign to a retained statstring.
	void				 assign (statstring *);
	
						 /// Clear the data.
	void				 clear (void) { init (false); }
	
						 /// Determine length of the string.
	inline unsigned int	 strlen (void) const
						 {
						 	if (! ref) return 0;
						 	assert (ref->refcnt > 0);
						 	return ref->str.strlen();
						 }
	
						 /// Return the hash key.
	inline unsigned int	 key (void) const
						 {
						 	if (! ref) return 0;
						 	assert (ref->refcnt > 0);
						 	return ref->key;
						 }
						 
						 /// Return our reference's unique id.
	unsigned int		 id (void) const
						 {
						 	if (! ref) return 0;
						 	assert (ref->refcnt > 0);
						 	return ref->id;
						 }
						 
						 //@{
						 /// Assignment operator.
	inline statstring	&operator= (unsigned int i)
						{
							assign (i);
							return *this;
						}
						
	statstring			&operator= (const class value &);
	statstring			&operator= (class value *);
	
	inline statstring	&operator= (const statstring &str)
						{
							assign (str);
						 	if (ref) assert (ref->refcnt > 0);
							return *this;
						}
	inline statstring	&operator= (statstring *str)
						 {
						 	assign (str);
						 	if (ref) assert (ref->refcnt > 0);
						 	return *this;
						 }
			
	inline statstring	&operator= (const char *str)
						{
							assign (str);
						 	if (ref) assert (ref->refcnt > 0);
							return *this;
						}
						
	inline statstring 	&operator= (const unsigned char *str)
					 	{
					 		assign ((const char *) str);
						 	if (ref) assert (ref->refcnt > 0);
					 		return *this;
					 	}
						
	inline statstring	&operator= (const string &str)
						{
							assign (str);
						 	if (ref) assert (ref->refcnt > 0);
							return *this;
						}
						
	inline statstring	&operator= (string *str)
						{
							assign (str);
						 	if (ref) assert (ref->refcnt > 0);
							return *this;
						}
						//@}
						
						//@{
						/// Equality operator.
	inline bool			operator== (const statstring &str) const
						{
						 	if (ref) assert (ref->refcnt > 0);
							if (key() != str.key()) return false;
							if (id() != str.id()) return false;
							return true;
						}
						
	inline bool			operator== (const string &str) const
						{
						 	if (ref) assert (ref->refcnt > 0);
							statstring sstr = str;
							if (key() != sstr.key()) return false;
							if (id() != sstr.id()) return false;
							if (! ref)
							{
								if (str.strlen()) return false;
								return true;
							}
							return (str == ref->str);
						}
	inline bool			operator== (const char *str) const
						{
						 	if (ref) assert (ref->refcnt > 0);
							size_t slen = ::strlen (str);
							if (slen != sval().strlen()) return false;
							return (::strncmp (str, cval(), slen) == 0);
						}
						
	inline bool			operator== (const string *str) const
						{
							if (! ref)
							{
								if (! str) return true;
								if ((*str).strlen()) return false;
								return true;
							}
							assert (ref->refcnt > 0);
							return ((*str) == ref->str);
						}
						
	bool				operator== (const class value &) const;
	bool				operator!= (const class value &) const;
						
    inline bool         operator!= (const statstring &str) const
                        {
						 	if (ref) assert (ref->refcnt > 0);
                            if ((key() == str.key()) && (id() == str.id()))
                            {
                            	return false;
                            }
							return true;
						}
						
	inline bool			operator!= (const string &str) const
						{
						 	if (ref) assert (ref->refcnt > 0);
							statstring sstr = str;
							if (key() == sstr.key()) return false;
							if (id() == sstr.id()) return false;
							if (! ref)
							{
								if (str.strlen()) return true;
								return false;
							}
							return !(str == ref->str);
						}
						
	inline bool			operator!= (const char *str) const
						{
						 	if (ref) assert (ref->refcnt > 0);
							size_t slen = ::strlen (str);
							if (slen != sval().strlen()) return true;
							return (::strncmp (str, cval(), slen) != 0);
						}
						
	inline bool			operator!= (const string *str) const
						{
							if (! ref)
							{
								if (str->strlen()) return true;
								return false;
							}
							assert (ref->refcnt > 0);
							return !((*str) == ref->str);
						}
						//@}
						
						/// Bool cast.
						/// Returns false if there's no reference.
	inline				operator bool (void) const
						{
							if (! ref) return false;
							assert (ref->refcnt > 0);
							return (ref->str.strlen());
						}
						
						/// Cast to c string.
	inline const char	*str (void) const
						{
							if (!ref) return "";
							assert (ref->refcnt > 0);
							return ref->str.str();
						}

						/// Cast to c string.
	inline const char	*cval (void) const
						{
							if (!ref) return "";
							assert (ref->refcnt > 0);
							return ref->str.str();
						}
												
						/// Cast to const string.
	const string		&sval (void) const
						{
							if (! ref) return emptystring;
							assert (ref->refcnt > 0);
							return ref->str;
						}

	virtual void		init (bool first);
protected:
	stringref		*ref; ///< This string's stringref struct (from stringrefdb).
};

#endif

#endif
