#ifndef _DBFILE_H
#define _DBFILE_H 1

#include <grace/str.h>
#include <grace/value.h>
#include <grace/exception.h>
#include <grace/valuable.h>
#include <grace/dictionary.h>

THROWS_EXCEPTION
(
	dbfileAccessException,
	0x4032e187,
	"Error accessing db file"
);
	
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
							 dbrecord (void)
							 {
							 	throw (dbrecordUninitializedException());
							 }
							 
							 /// Constructor for the root record.
							 /// \param iowner The owning dbfile.
							 dbrecord (dbfile *iowner);
							 
							 /// Constructor for a child record.
							 /// \param iowner The owning dbfile.
							 /// \param iparent The parent record.
							 /// \param key The node's index key.
							 dbrecord (dbfile *iowner,
							 		   dbrecord *iparent,
							 		   const statstring &key);
							 							 
							 /// Destructor.
	virtual					~dbrecord (void);
	
							 /// Reports whether a child node exists.
							 /// \param k The child's key.
	bool					 exists (const statstring &k);
	
	
							 /// Access a child node or member data.
							 /// A child node will be automatically created
							 /// by querying the owning dbfile for an
							 /// object 
	dbrecord				&operator[] (const statstring &k);
	dbrecord				&operator[] (const char *);
	
							 /// Convenience operator for assigning from
							 /// a value. This should also cover strings,
							 /// integers and the likes.
	dbrecord				&operator= (const value &o) { fromvalue (o); }
	
							 /// Assignment operator for a retainable
							 /// value.
	dbrecord				&operator= (value *o) { fromvalue (o); }
	
							 /// Assignment operator for a retainable
							 /// string.
	dbrecord				&operator= (string *o) { value v = o; fromvalue (v); }

							 /// Convenience access to underlying value.
	const string			&sval (void) { return v.sval(); }
							 /// Convenience access to underlying value.
	const char				*cval (void) { return v.cval(); }
							 /// Convenience access to underlying value.
	int						 ival (void) { return v.ival(); }
							 /// Convenience access to underlying value.
	unsigned int			 uval (void) { return v.uval(); }
							 /// Convenience access to underlying value.
	long long				 lval (void) { return v.lval(); }
							 /// Convenience access to underlying value.
	unsigned long long		 ulval (void) { return v.ulval(); }
							 /// Convenience access to underlying value.
	bool					 bval (void) { return v.bval(); }

							 /// Casting operator to primary type.	
							 operator int (void) { return ival(); }
							 /// Casting operator to primary type.	
							 operator unsigned int (void) { return uval(); }
							 /// Casting operator to primary type.	
							 operator long long (void) { return lval(); }
							 /// Casting operator to primary type.	
							 operator unsigned long long (void) { return ulval(); }
							 /// Casting operator to primary type.	
							 operator bool (void) { return bval(); }

							 /// Return the object's key.
	const statstring		&id (void) { return _id; }

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
	void					 fromvalue (const value &outof);
	
							 /// Remove a record (root) or member
							 /// value (child).
							 /// \param key The key of the record/member.
	void					 rmval (const statstring &key);

							 /// Implementation for the visitor protocol,
							 /// allows foreach() to work its magic
							 /// on a root node.
	dbrecord				*visitchild (int pos);
							 
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
							courierdb = 0x6ab760c8 ///< Encode as courierdb
						 };

						 dbfile (void);
	virtual				~dbfile (void);
	
	dbrecord			 db;
	bool				 commit (void);
	bool				 rmval (const statstring &key);
	void				 setencoding (enctype t, char isep=',')
						 {
						 	encoding = t;
						 	sep = isep;
						 }

protected:
	virtual bool		 recordexists (const statstring &id);
	virtual string		*getrecord (const statstring &id);
	virtual bool		 setrecord (const statstring &id, const string &data,
									bool create=true);
	virtual bool		 removerecord (const statstring &id);
	virtual bool		 startloop (void);
	virtual bool		 nextloop (void);

	void				 encode (string &, value &v);
	void				 decode (const string &, value &);
	
	bool				 dbopen;
	enctype				 encoding;
	char				 sep;
};

#endif
