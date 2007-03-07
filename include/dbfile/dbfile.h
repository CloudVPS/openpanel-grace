#ifndef _DBFILE_H
#define _DBFILE_H 1

#include <grace/str.h>
#include <grace/value.h>
#include <grace/exception.h>
#include <grace/valuable.h>
#include <grace/dictionary.h>

THROWS_EXCEPTION
(
	dbfileNotOpenException,
	0x774801d4,
	"Db file not open"
);
	
THROWS_EXCEPTION
(
	dbrecordMemoryCorruption,
	0x097022f0,
	"Internal memory corruption"
);

THROWS_EXCEPTION
(
	dbrecordAssignDuringLoopException,
	0x423a8934,
	"Record assignment during a loop execution"
);

THROWS_EXCEPTION
(
	dbrecordAssignRootException,
	0x6490748c,
	"Assignment to root record prohibited"
);

THROWS_EXCEPTION
(
	dbrecordUninitializedException,
	0x04f88236,
	"Uninitialized dbrecord created"
);

/// A virtual placeholder for a position in a database file. A dbrecord
/// object can fill any of the following functions:
/// - The 'root' record of a dbfile object is used to access and cache
///   specific keyed records from the database file through the
///   operator[].
/// - A first-level child-record is created under the root-node as
///   a cached copy of the record with the same key in the database.
/// - A second-level child represents a member variable of a record if
///   the database format exports more than one field.
/// - In a special case, the root-node is used to implement a part of
///   the visitor-protocol that enables the foreach-macro to work.
/// The class inherits from valuable, making it easy to convert it to
/// and from a value-object.
class dbrecord : public valuable
{
friend class dbfile;
friend class visitor<dbrecord>;
public:
							 /// Default constructor.
							 /// This constructor should never be called;
							 /// a dbrecord needs a parent.
							 /// \throw dbrecordUninitializedException
							 ///        Didn't I just tell you never to use
							 ///        the default constructor?
							 dbrecord (void)
							 {
							 	throw (dbrecordUninitializedException());
							 }
							 
							 /// Constructor for the root record.
							 /// \param iowner The owning dbfile.
							 dbrecord (class dbfile *iowner);
							 
							 /// Constructor for a child record.
							 /// \param iowner The owning dbfile.
							 /// \param iparent The parent record.
							 /// \param key The node's index key.
							 dbrecord (class dbfile *iowner,
							 		   dbrecord *iparent,
							 		   const statstring &key);
							 							 
							 /// Destructor.
	virtual					~dbrecord (void);
	
							 /// Reports whether a child node exists.
							 /// \param k The child's key.
	bool					 exists (const statstring &k);
	
	
							 //@{
							 /// Access a child node or member data.
							 /// A child node will be automatically created
							 /// by querying the owning dbfile for an
							 /// object 
	dbrecord				&operator[] (const statstring &k);
	dbrecord				&operator[] (const char *);
							 //@}
	
							 /// Convenience operator for assigning from
							 /// a value. This should also cover strings,
							 /// integers and the likes.
	dbrecord				&operator= (const value &o) { fromvalue (o); return *this; }
	
							 /// Assignment operator for a retainable
							 /// value.
	dbrecord				&operator= (value *o) { fromvalue (o); return *this; }
	
							 /// Assignment operator for a retainable
							 /// string.
	dbrecord				&operator= (string *o) { value v = o; fromvalue (v); return *this; }

							 /// Assignment operator for a const string.
	dbrecord 				&operator= (const string &o) { value v = o; fromvalue (v); return *this; }

							 //@{
							 /// Convenience access to underlying value.
	const string			&sval (void) { return v.sval(); }
	const char				*cval (void) { return v.cval(); }
	int						 ival (void) { return v.ival(); }
	unsigned int			 uval (void) { return v.uval(); }
	long long				 lval (void) { return v.lval(); }
	unsigned long long		 ulval (void) { return v.ulval(); }
	bool					 bval (void) { return v.bval(); }
							 //@}

							 //@{
							 /// Casting operators to primary type.	
							 operator int (void) { return ival(); }
							 operator unsigned int (void) { return uval(); }
							 operator long long (void) { return lval(); }
							 operator unsigned long long (void) { return ulval(); }
							 operator bool (void) { return bval(); }
							 //@}

							 /// Return the object's key.
	const statstring		&id (void) { return _id; }

							 /// Implementation for the visitor protocol,
							 /// allows foreach() to work its magic
							 /// on a root node.
	dbrecord				*visitchild (int pos);

protected:
							 /// Serialize ourselves into a value.
							 /// \param into The destination object.
	virtual void			 tovalue (value &into);
	
							 /// Deserialize ourselves from a retained value.
							 ///\param outof The source object.
	void					 fromvalue (value *outof)
							 {
							 	fromvalue (*outof);
							 	delete outof;
							 }
							 
							 /// Deserialize ourselves from a value.
							 ///\param outof The source object.
							 /// \throw dbrecordAssignDuringLoopException 
							 ///        Method was called during a foreach()
							 ///		loop which is not allowed.
							 /// \throw dbrecordAssignRootException
							 ///		Method was called on the root record.
	void					 fromvalue (const value &outof);
	
							 /// Remove a record (root) or member
							 /// value (child).
							 /// \param key The key of the record/member.
							 /// \throw dbrecordAssignDuringLoopException 
							 ///        Method was called during a foreach()
							 ///		loop which is not allowed.
	void					 rmval (const statstring &key);
							 
	virtual void			 init (bool first);

public:
	value					 v; ///< Loaded/cached value.
	
							 /// This flag is set to true if the record
							 /// was changed. The dbfile::commit() method
							 /// inspects this flag to determine which
							 /// records should be flushed back to the
							 /// database file.
	bool					 changed;
	
							 /// This flag is set to true if the record
							 /// didn't exist in the database file prior
							 /// to 
	bool					 create; ///< If true, record was newly created.
	void 					*loopref; ///< Internal storage for looping.
	bool					 inloop; ///< True if this object is looping.
	statstring				 _id; ///< Record's or member's key.

protected:
	dbrecord 				*parent; ///< Parent object or NULL for root.
	dbfile					*owner; ///< Owning dbfile.
	dictionary<dbrecord>	 children; ///< Cached children (root only)
	dictionary<dbrecord>	 members; ///< Member variables (non-root only)
};

/// Base class for implementations of file-based databases. 
class dbfile
{
friend class dbrecord;
public:
						 /// Encoding-type for the records
	enum				 enctype {
							flat = 0x7c3c444d, ///< Encode flat string
							shox = 0x5c2445b4, ///< Encode as SHoX
							attriblist = 0xb2cac808, ///< Encode as quoted attribute-list
							courierdb = 0x6ab760c8, ///< Encode as courierdb
							valuelist = 0x19abf996 ///< Encode right-hand values as list
						 };
						
						 /// Default constructor.
						 dbfile (void);
						 
						 /// Virtual destructor.
	virtual				~dbfile (void);
	
	dbrecord			 db; ///< The root record.
	
						 /// Commit all outstanding changes to the database
						 /// layer.
	bool				 commit (void);
	
						 /// Remove a record.
						 /// \param key The id of the record to remove.
	bool				 rmval (const statstring &key);
	
						 /// Set the encoding type for data within a
						 /// record.
						 /// \param t The encoding type.
						 /// \param isep An optional separator character.
	void				 setencoding (enctype t, char isep=',')
						 {
						 	encoding = t;
						 	sep = isep;
						 	db.children.clear ();
						 }
						 
						 /// Set the maximum size of the cache. The class
						 /// currently uses a somewhat naive implementation
						 /// for clearing the cache: If the number of
						 /// cached entries exceeds the maximum size, the
						 /// entire cache is invalidated, without any
						 /// due consideration for uncommitted records.
						 /// For this reason, the number is set to 0 by
						 /// default (cache entries never expire).
						 /// \param i The new maximum size (0 for no limits).
	void				 setcachesize (unsigned int i) { maxcache = i; }
	
						 /// Clear the cache explicitly (will also remove
						 /// any uncommitted records).
	void				 clearcache (void) { db.children.clear(); }

protected:
						 /// Check with the database layer if the file
						 /// has a record with a provided id.
						 /// \param id The id to look for.
	virtual bool		 recordexists (const statstring &id);
	
						 /// Get the string-encoded record from the
						 /// database layer.
						 /// \param id The id of the record.
	virtual string		*getrecord (const statstring &id);
	
						 /// Create or update a record in the database
						 /// file.
						 /// \param id The id of the record to change.
						 /// \param data The string-encoded data.
	virtual bool		 setrecord (const statstring &id, const string &data,
									bool create=true);
									
						 /// Remove a record from the database file.
						 /// \param id The id of the record to remove.
	virtual bool		 removerecord (const statstring &id);
	
						 /// Initiate a loop over all records in the file.
						 /// This method should set up the 'db' rootnode
						 /// with its 'inloop' flag. Its 'loopref' pointer
						 /// should be used to point at a custom object
						 /// you may need to save state between iterations.
						 /// After initialization, the contents of the
						 /// first database record should be stored in
						 /// db.v and db._id. 
						 /// \return True if the first record was
						 ///         succesfully loaded.
	virtual bool		 startloop (void);
	
						 /// Takes the state information stored in the db
						 /// member and uses it to read the next record
						 /// from the database file. If there is no next
						 /// record, all loop-related information inside
						 /// db (db.inloop, db.loopref) should be cleared.
						 /// \return True if the next record was succesfully
						 ///         loaded into db.
	virtual bool		 nextloop (void);
	
						 /// Tells the database layer to write any and all
						 /// changes irrevocably to disk.
	virtual bool		 filesync (void);

						 /// Encodes a value-object to a string using the
						 /// requested coding method.
						 /// \param s The destination string.
						 /// \param v The source value.
	void				 encode (string &s, value &v);
	
						 /// Ddecodes a string object into a value.
						 /// \param s The source string.
						 /// \param v The destination value.
	void				 decode (const string &s, value &v);
	
	bool				 dbopen; ///< True if the database file is open.
	enctype				 encoding; ///< Selected encoding type.
	char				 sep; ///< Optional encoding-related separator.
	unsigned int		 maxcache; ///< Maximum size of the cache.
};

#endif
