#ifndef _CONFIGDB_H
#define _CONFIGDB_H 1

#include <grace/str.h>
#include <grace/value.h>
#include <grace/visitor.h>
#include <grace/statstring.h>
#include <grace/xmlschema.h>
#include <grace/validator.h>
#include <grace/system.h>

/// A class representing a chain of keys, or a complete path to an
/// item in a larger tree of value objects.
class keypath
{
public:
						 /// Constructor.
						 /// Creates an empty path.
						 keypath (void);
						 
						 /// Copy-constructor.
						 keypath (const keypath &);

						 /// Copy-constructor.
						 /// Deletes original.
						 keypath (keypath *);
						 
						 /// Copy-constructor.
						 /// Expects argument in the form of a
						 /// path string with elements separated by
						 /// a forward-slash.
						 keypath (const char *);
	
						 /// Access operator.
						 /// Returns the key at a given position, or
						 /// an empty statstring if there is no data.
						 /// \param pos The position, if negative
						 ///            as measured from the end.
	const statstring	&operator[] (int pos) const;
	
						 /// Get element count.
	int					 count (void) const;
	
						 /// Clear list.
	void				 clear ();
	
						 /// Add element.
	void				 add (const statstring &);

						 /// Add element.
	void				 add (const char *);

						 /// Add element.
	void				 add (const string &);

						 /// Add element.
	void				 add (const value &);

						 /// Remove tail element.
	void				 up (void);
	
						 /// Determine if a value object contains
						 /// this path.
	bool				 existsin (value &);
	
						 /// Begin loop tp apply path rule to a tree.
						 /// \param v The value object containing the data.
						 /// \param respos A keypath object that will
						 ///               point to the currently evaluated
						 ///               node for every round.
	void				 begin (value &v, keypath &respos);
	
						 /// Get current value for path (should be
						 /// prepended by begin() or next().
	const value			&get (void);
	
						 /// Go to the next tree match, if any. Returns
						 /// false if there are no more matches.
						 /// \param respos Key that will contain the
						 ///               current position being
						 ///               evaluated.
	bool				 next (keypath &respos);

protected:
	visitor<value>		*crsr; ///< Cursor for evaluation (begin/get/next)
	int					 cvpos; ///< Internal position.
	int					 cindex; ///< Internal index.
	
	statstring			**array; ///< Array containing the keys.
	unsigned int		  acount; ///< Number of elements in the array.
	unsigned int		  asize; ///< Allocated array size.
						  
						  /// Grow the array allocation by one.
	void				  grow1 (void);
};

/// A thread-bound local copy of a database.
struct tsdbentry
{
	tsdbentry	*next; ///< Linked list glue.
	tsdbentry	*prev; ///< Linked list glue.
	pthread_t	 assocthr; ///< Thread-id of associated thread
	time_t		 lastupdate; ///< Last update time of cached db
	value		 dat; ///< The cached db
};

/// A thread-specific database cache.
/// Useful for keeping a value object that is subject to change
/// during runtime. Every thread keeps a local copy of the data
/// to prevent threading mistakes. A timestamp is used to do a
/// possible refersh of the data.
class tsdb
{
public:
				 /// Constructor.
				 tsdb (void);
				 
				 /// Destructor.
				~tsdb (void);
				
				 /// Return a thread-specific copy of a value object.
				 /// \param from The source object.
				 /// \param ti The time the source object last changed.
	const value	&get (const value &from, time_t ti);

protected:
	lock<bool>	 lck; ///< Lock on the database.
	tsdbentry	*first; ///< First node in the tsdbentry linked list.
	tsdbentry	*last; ///< Last node in the tsdbentry linked list.
};

/// List of actions.
/// Entries in the configuration can be bound to watchers. A watcher is
/// a callback into the application object for transactions into a
/// defined part of the configuration tree. This enum lists the kinds
/// of actions with the data a callback should expect to handle.
namespace config
{
	/// Callback type.
	enum action
	{
		isvalid, ///< Validate sanity.
		create, ///< Create new configuration item.
		change, ///< Change existing configuration item.
		remove ///< Remove existing configuration item.
	};
};

#define __ACTIONARGS config::action, keypath &, const value &, const value &

/// A class to wrap application configuration parsing and logic.
/// Combines an XML schema and validator schema to load the application's
/// configuration out of conf. A callback mechanism allows specific 
/// actions to be bound to configuration changes.
template <class appclass> class configdb
{
public:

	/// Pointer to a method in the parent class for the callback.	
	typedef bool (appclass::*pmethod)(__ACTIONARGS);

	/// Constructor.
	/// \param papp Link back to the application object.
	configdb (appclass *papp)
	{
 		app = papp;
		first = last = NULL;
	}
	 
	/// Destructor.
	~configdb (void)
	{
	}
	
	/// Load, validate and process configuration data.
	/// It will load the schema and validator from schema: and the
	/// configuration file proper from conf:.
	/// \param cprefix Application prefix.
	/// \param error String to store error text.
	bool load (const string &cprefix, string &error)
	{
		string fn;
		fn.printf ("schema:%s.schema.xml", cprefix.str());
		
		if (! fs.exists (fn))
		{
			error = "Could not load schema";
			return false;
		}
		cschema.load (fn);
		
		fn.crop();
		fn.printf ("schema:%s.validator.xml", cprefix.str());
		if (! cval.load (fn))
		{
			error = "Could not load validator";
			return false;
		}
		
		fn.crop();
		fn.printf ("conf:%s.conf.xml", cprefix.str());
		if (! fs.exists (fn))
		{
			error = "Could not load configuration";
			return false;
		}
		ndb.loadxml (fn, cschema);
		
		if (! cval.check (ndb, error)) return false;
		
		error.crop();
		if (! handleactions (error)) return false;
		
		lck.lockw();
		db = ndb;
		lastloaded = kernel.time.now();
		lck.unlock();
		return true;
	}
	
	/// Add a watcher.
	/// \param kp The path to watch.
	/// \param m The method to call.
	void addwatcher (const keypath &kp, pmethod m)
	{
		keypath kpp (kp);
		configaction *a;
		a = new configaction (kpp, m);
		if (last)
		{
			a->next = NULL;
			a->prev = last;
			last->next = a;
			last = a;
		}
		else
		{
			a->next = a->prev = NULL;
			first = last = a;
		}
	}
	
	/// Get localized configuration section.
	/// Creates a thread-safe copy of the database for the current
	/// thread if needed. This configuration data is safe to be used
	/// from there on, even if the tree data is being reloaded at
	/// the same time.
	/// \param s Key of the section to return.
	const value &operator[] (const statstring &s)
	{
		lck.lockr();
		const value &v = localdb.get (db, lastloaded)[s];
		lck.unlock();
		return v;
	}
	
	/// Move a config change through all the watchers.
	/// \param err String to store errors in.
	bool handleactions (string &err)
	{
		keypath p;
		keypath pp;
		configaction *a;
		
		// First, loop over all the configactions and call them to
		// validate the new configuration data. If Any of them fails,
		// the party has been pooped and the old configuration will
		// remain effective.
		a = first;
		while (a)
		{
			// Iterate over all possible matches to this path
			a->path.begin (ndb, p);
			do
			{
				// Do the validation voodoo
				if (! a->call (app, config::isvalid, a->path.get(),
							   a->path.get()))
				{
					// Party pooper in the house.
					err = "Error in new configuration";
					return false;
				}
			} while (a->path.next (p));
			
			a = a->next;
		}
		
		// Now, let's do the real loop, applying the new configuration data.
		a = first;
		while (a)
		{
			// Iterate over all matches for this path.
			a->path.begin (ndb, p);
			do
			{
				// Does the path exist in the original database?
				if (p.existsin (db))
				{
					// Resolve the handle represented by p's path
					p.begin (db, pp);
					// Are the original and new trees not the same?
					if (! p.get().treecmp (a->path.get()))
					{
						// Call for a change
						(void) a->call (app, config::change, a->path.get(),
										p.get());
					}
				}
				else
				{
					// The path did not exist
					(void) a->call (app, config::create, a->path.get(),
									a->path.get());
				}
			} while (a->path.next (p));
			
			// Iterate the matches inside the old configuration tree
			a->path.begin (db, p);
			do
			{
				// Is this path gone in the new tree?
				if (! p.existsin (ndb))
				{
					// Call for its removal
					(void) a->call (app, config::remove, a->path.get(),
									a->path.get());
				}
			} while (a->path.next (p));
			
			a = a->next;
		}
		
		return true;
	}
	
	/// A callback registration for a specific key or list of
	/// subkeys in the configuration database.
	class configaction
	{
	public:
						 /// Constructor.
						 /// \param kp The matching path.
						 /// \param m The method to call.
						 configaction (keypath &kp, pmethod m)
						 {
						 	path = kp;
							_method = m;
						 }
		
						 /// Perform callback for this action.
						 /// \param app The parent object.
						 /// \param act The kind of configuration action
						 ///            (config::isvalid, config::create,
						 ///             config::change or config::remove)
						 /// \param newdb The configuration db after the change.
						 /// \param olddb The configuration db before the change.
		bool			 call (appclass *app,
							   config::action act,
							   const value &newdb,
							   const value &olddb)
						 {
							return (app->*_method) (act, path, newdb, olddb);
						 }
		
		configaction	*next; ///< Linked list glue.
		configaction	*prev; ///< Linked list glue.
		keypath			 path; ///< The matching path.
	protected:
		pmethod	 		_method; ///< The method to call.
	};

protected:
	appclass		*app; ///< Link back to application.
	configaction 	*first; ///< First callback.
	configaction	*last; ///< Last callback.
	xmlschema		 cschema; ///< Configuration schema.
	validator		 cval; ///< Configuration validator.
	value			 db; ///< Current configuration db.
	value			 ndb; ///< Candidate new configuration db.
	time_t			 lastloaded; ///< Time of last update.
	tsdb			 localdb; ///< Thread-safe db storage.
	lock<bool>		 lck; ///< Generic lock.
};

#endif
