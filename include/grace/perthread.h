#ifndef _PERTHREAD_H
#define _PERTHREAD_H 1

#include <pthread.h>
#include <grace/lock.h>

/// A node used by the perthread<kind> template class to keep a
/// thread-specific copy of an object.
template<class kind>
class perthreadnode
{
public:
	perthreadnode (void) { next = prev = NULL; }
	~perthreadnode (void) { }
	
	pthread_t     		  thr; ///< The creating thread.
						  //@{
						  /// List links.
	perthreadnode		 *next, *prev;
						 //@}
	kind		  		  obj; ///< Contained object.
};

/// A class to keep a collection of thread-specific objects. 
template<class kind>
class perthread
{
public:
	/// Constructor. Initializes array pointers.
	perthread (void)
	{
		first = last = NULL;
	}
	
	/// Destructor. Erases the array, which will delete the collected
	/// objects.
	~perthread (void)
	{
		perthreadnode<kind> *c, *nc;
		
		exclusivesection (lck)
		{
			c = first;
			while (c)
			{
				nc = c->next;
				delete c;
				c = nc;
			}
			first = last = NULL;
		}
	}
	
	/// Get a reference to the thread-specific object.
	kind &get (void)
	{
		// Get local thread-id
		pthread_t me = pthread_self();
		
		// First let's see if we already have a copy.
		sharedsection (lck)
		{
			perthreadnode<kind> *c = first;
			while (c)
			{
				// We do, return that copy.
				if (c->thr == me) breaksection return c->obj;
				c = c->next;
			}
		}
		
		// We will need to create a new one;
		perthreadnode<kind> *c = new perthreadnode<kind>;;
		
		// Now let's add it to the array. Take note that the model of
		// locking employed here (taking a read-lock to check for
		// existence, then pulling out and taking a write-lock to
		// create a new object) would be prone to race-conditions if
		// we were adding anything other than a bit of data that is
		// uniquely bound to the calling thread.
		exclusivesection (lck)
		{
			c->thr = me;
			c->next = NULL;
			if (last)
			{
				c->prev = last;
				last->next = c;
				last = c;
			}
			else
			{
				c->prev = NULL;
				first = last = c;
			}
		}
		return c->obj;
	}
	
	/// Assign the value. Whatever value you are keeping in this template
	/// will need to be able to deal with operator= for a reference to
	/// its own type.
	perthread &operator= (kind &i) { get() = i; }

protected:
	lock<bool> lck; ///< List lock.
	//@{
	///Linked list pointers.
	perthreadnode<kind> *first, *last;
	//@}
};

#endif
