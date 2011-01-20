// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _LOCK_H
#define _LOCK_H 1

#include <grace/exception.h>
#include <pthread.h>
#include <grace/platform.h>
//#include <grace/system.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

/// Lock types.
enum locktype {
	lockRead, ///< Read lock.
	lockWrite ///< Write lock.
};

$exception (lockException, "Error locking");

inline void __musleep (int ms)
{
	struct timeval tv;
	tv.tv_sec = ms/1000;
	tv.tv_usec = 1000 * (ms%1000);
	(void) ::select (0, NULL, NULL, NULL, &tv);
}

extern volatile bool __THREADED;

#ifdef PTHREAD_HAVE_RWLOCK

/// A base class for a thread lock (for pthread implementations with native
/// rwlocks).
/// Allows mixed readers/writers. The template lock<kind> class should
/// normally be used to wrap a lock around a specific object.
class lockbase
{
public:
					 /// Constructor. Sets up pthread data.
					 lockbase (void)
					 {
						attr = new pthread_rwlockattr_t;
						rwlock = new pthread_rwlock_t;

						pthread_rwlockattr_init (attr);
						pthread_rwlock_init (rwlock, NULL);
					 }
					 
					 /// Destructor. Frees pthread objects.
					~lockbase (void)
					 {
			 			pthread_rwlock_destroy (rwlock);
						pthread_rwlockattr_destroy (attr);

						delete attr;
						delete rwlock;
					 }
					 
					 /// Perform a read-lock.
					 /// Will block if there is a current or
					 /// pending write-block.
	void			 lockr (void);
					 
					 /// Perform a write-lock.
					 /// Will block if there are current read-locks.
					 /// Pending read-locks are blocked until
					 /// this write-lock is acquired and released.
	void			 lockw (void);
					 
					 /// Remove earlier lock.
	void			 unlock (void);
					 
					 /// Attempt a read-lock.
					 /// Tries to acquire a lock. Returns true
					 /// if one could be acquired within the timeout
					 /// limit.
					 /// \param secs Timeout in seconds.
	bool			 trylockr (int secs=0);

					 /// Attempt a write-lock.
					 /// Tries to acquire a lock. Returns true
					 /// if one could be acquired within the timeout
					 /// limit.
					 /// \param secs Timeout in seconds.
	bool			 trylockw (int secs = 0);
	
protected:
	pthread_rwlockattr_t	*attr;
	pthread_rwlock_t		*rwlock;
};

/// Lock template class. Use this to guard your objects.
template<typename kind>
class lock : public lockbase
{
public:
	kind o;
};

#else

#define LOCK_NONE           0x00
#define LOCK_READ           0x01
#define LOCK_WRITE          0x02
#define LOCK_PREWRITE       0x03
#define LOCK_READYWRITE     0x04
#define LOCK_WRITEPREWRITE  0x05

/// A base class for a thread lock (for pthread implementations without native
/// rwlocks).
/// Allows mixed readers/writers. The template lock<kind> class should
/// normally be used to wrap a lock around a specific object.
class lockbase
{
public:
					 /// Constructor.
					 lockbase (void)
					 {
					 	locks = NULL;
						locksz = 0;
						
						attr = new pthread_mutexattr_t;
						mutex = new pthread_mutex_t;
						cond = new pthread_cond_t;
						
						pthread_mutexattr_init (attr);
						pthread_mutex_init (mutex, attr);
						pthread_cond_init (cond, NULL);
						
						status = LOCK_NONE;
						numlocks = 0;
					 }
					 
					 /// Destructor.
					~lockbase (void)
					 {
					 	pthread_mutex_destroy (mutex); delete mutex;
						pthread_mutexattr_destroy (attr); delete attr;
						pthread_cond_destroy (cond); delete cond;
						status = LOCK_NONE;
						numlocks = 0;
					 }

					 /// Perform a read-lock.
					 /// Will block if there is a current or
					 /// pending write-block.
	void			 lockr (void);
	
					 /// Perform a write-lock.
					 /// Will block if there are current read-locks.
					 /// Pending read-locks are blocked until
					 /// this write-lock is acquired and released.
	void			 lockw (void);
	
					 /// Unlcok an earlier read- or write-lock.
	void			 unlock (void);
					 
					 /// Attempt a read-lock.
					 /// Tries to acquire a lock. Returns true
					 /// if one could be acquired within the timeout
					 /// limit.
					 /// \param secs Timeout in seconds.
	bool			 trylockr (int secs = 0);

					 /// Attempt a write-lock.
					 /// Tries to acquire a lock. Returns true
					 /// if one could be acquired within the timeout
					 /// limit.
					 /// \param secs Timeout in seconds.
	bool			 trylockw (int secs = 0);
	
protected:
	pthread_mutexattr_t		*attr;
	pthread_mutex_t			*mutex;
	pthread_cond_t			*cond;
	
	pthread_t				*locks;
	pthread_t				 writer;
	bool					 writerupgraded;
	int						 locksz;
	
	inline bool 			 addme (void)
							 {
							 	pthread_t me = pthread_self();
							 	int i;
								
								// Create lock array if it wasn't there yet
							 	if (! locks)
								{
									locks = (pthread_t *) malloc (8*sizeof (pthread_t));
									for (i=0; i<8; ++i) locks[i] = 0;
									locksz = 8;
								}
								
								for (i=0; i<locksz; ++i)
								{
									if (locks[i] == me) return false;
								}
								for (i=0; i<locksz; ++i)
								{
									if (! locks[i]) // free slot
									{
										locks[i] = me;
										return true;
									}
								}
								locks = (pthread_t *) realloc (locks, 2*locksz*sizeof(pthread_t));
								for (i=locksz+1; i<(2*locksz); ++i) locks[i] = 0;
								locks[locksz] = me;
								locksz *= 2;
								return true;
							 }
	inline bool 			 delme (void)
							 {
							 	pthread_t me = pthread_self();
							 	int i;
																
								// No lock array is no lock :)
							 	if (! locks) return false;
								
								for (i=0; i<locksz; ++i)
								{
									if (locks[i] == me)
									{
										locks[i] = 0;
										return true;
									}
								}
								return false;
							 }
	
	int						 status;
	int						 numlocks;
};

/// Lock template class. Use this to guard your objects.
template<typename kind>
class lock : public lockbase
{
public:
	kind o;
};

#endif


/// Locks the provided lock for writing as long as the object is alive. 
class scopedlock 
{
    friend class scopedunlock;
public:
    scopedlock( lockbase& inlock, bool exclusive )
    : thelock(inlock)
    , exclusive( exclusive )
    , locked( false )
    {
        lock();      
    } 
    ~scopedlock()
    {
        unlock();
    }

private:
    void lock()
    {
        if( !locked )
        {
            if( exclusive )
                thelock.lockw();
            else
                thelock.lockr();
                
            locked = true;
        }
    }

    void unlock()
    {
        if( locked ) 
        {
            thelock.unlock();
            locked = false;
        }
    }

    lockbase& thelock;
    bool exclusive;
    bool locked;
};

class scopedunlock 
{
public:
    scopedunlock( scopedlock& target, bool shouldrelock = true )
    : target( target )
    , relocktarget( shouldrelock )
    {
        target.unlock();
    }
    
    ~scopedunlock()
    {
        if( relocktarget )
        {
            target.lock();
        }
    }

    void shouldrelock()
    {
        relocktarget = true;
    }
    
private:
    scopedlock& target;
    bool relocktarget;
};

#define exclusiveaccess(lname) \
    for( bool __macrohelper = true;                     __macrohelper; __macrohelper = false ) \
    for( typeof( lname ) &sectionlock = lname;          __macrohelper; __macrohelper = false ) \
    for( scopedlock __scopelock( sectionlock, false );  __macrohelper; __macrohelper = false ) \

#define sharedaccess(lname) \
    for( bool __macrohelper = true;                     __macrohelper; __macrohelper = false ) \
    for( typeof( lname ) &sectionlock = lname;          __macrohelper; __macrohelper = false ) \
    for( scopedlock __scopelock( sectionlock, false );  __macrohelper; __macrohelper = false ) \

#define exclusivesection(lname) \
    for( bool __macrohelper = true;                     __macrohelper; __macrohelper = false ) \
    for( typeof( lname ) &sectionlock = lname;          __macrohelper; __macrohelper = false ) \
    for( scopedlock __scopelock( sectionlock, true );   __macrohelper; __macrohelper = false ) \
    for( typeof(sectionlock.o) &lname = sectionlock.o;  __macrohelper; __macrohelper = false ) 

#define sharedsection(lname) \
    for( bool __macrohelper = true;                     __macrohelper; __macrohelper = false ) \
    for( typeof( lname ) &sectionlock = lname;          __macrohelper; __macrohelper = false ) \
    for( scopedlock __scopelock( sectionlock, false );  __macrohelper; __macrohelper = false ) \
    for( typeof(sectionlock.o) &lname = sectionlock.o;  __macrohelper; __macrohelper = false ) 

#define unprotected(lname) \
    for( bool __macrohelper = true;                     __macrohelper; __macrohelper = false ) \
    for( typeof( lname ) &sectionlock = lname;          __macrohelper; __macrohelper = false ) \
    for( typeof(sectionlock.o) &lname = sectionlock.o;  __macrohelper; __macrohelper = false ) 

#define breaksection \
    for( bool __macrohelper = true;                     __macrohelper; __macrohelper = false ) \
    for( scopedunlock __scopeunlock( __scopelock, 0 );  __macrohelper; __scopeunlock.shouldrelock(),  __macrohelper = false ) 

/// Conditional.
/// Implies a condition that one or more threads can wait on that
/// other threads can raise. 
class conditional
{
public:
				 /// Constructor.
				 /// Sets up POSIX attributes.
				 conditional (void);
				 
				 /// Destructor.
				 /// Cleans up POSIX structures.
				~conditional (void);
				
				 /// Wake up one thread that is waiting.
	void		 signal (void);
				 
				 /// Wake up all threads that are waiting.
	void		 broadcast (void);
	
				 /// Sleep until the condition is raised.
	bool		 wait (void);
				 
				 /// Sleep for a number of microseconds or until
				 /// the condition is raised.
				 /// \param timeout Timeout in 10^-4 seconds.
	bool		 wait (int timeout);
	
protected:
	pthread_mutexattr_t		*attr; ///< POSIX mutex attribute.
	pthread_mutex_t			*mutex; ///< POSIX mutex.
	pthread_cond_t			*cond; ///< POSIX conditional.
	int						 queue; ///< Queue counter.
};

#endif
