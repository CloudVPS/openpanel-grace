#ifndef _LOCK_H
#define _LOCK_H 1

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

enum lockExceptions {
	exLockError ///< Generic locking exception.
};

inline void __musleep (int ms)
{
	struct timeval tv;
	tv.tv_sec = ms/1000;
	tv.tv_usec = 1000 * (ms%1000);
	(void) ::select (0, NULL, NULL, NULL, &tv);
}

extern bool __THREADED;

#ifdef PTHREAD_HAVE_RWLOCK

/// A thread lock.
/// Allows mixed readers/writers.
template<class kind>
class lock
{
public:
					 lock (void)
					 {
						attr = new pthread_rwlockattr_t;
						rwlock = new pthread_rwlock_t;

						pthread_rwlockattr_init (attr);
						pthread_rwlock_init (rwlock, NULL);
					 }
					~lock (void)
					 {
			 			pthread_rwlock_destroy (rwlock);
						pthread_rwlockattr_destroy (attr);

						delete attr;
						delete rwlock;
					 }
					 
					 /// Perform a read-lock.
					 /// Will block if there is a current or
					 /// pending write-block.
	inline void	 	 lockr (void)
					 {
					 	if (! __THREADED) return;
					 	int eno;
			 			if ((eno = pthread_rwlock_rdlock (rwlock)))
			 			{
							if (eno != EDEADLK)
							{
								::printf ("lockr fail eno=%i %s\n",
										  eno, strerror (eno));
								throw (exLockError);
							}
						}
					 }
					 
					 /// Perform a write-lock.
					 /// Will block if there are current read-locks.
					 /// Pending read-locks are blocked until
					 /// this write-lock is acquired and released.
	inline void		 lockw (void)
					 {
					 	if (! __THREADED) return;
					 	int eno;
			 			if ((eno = pthread_rwlock_wrlock (rwlock)))
			 			{
							if (eno != EDEADLK)
							{
								::printf ("lockw fail eno=%i %s\n",
										  eno, strerror (eno));
								throw (exLockError);
							}
						}
					 }
					 
					 /// Remove earlier lock.
	inline void		 unlock (void)
					 {
					 	int eno;
					 	if (! __THREADED) return;
			 			if ((eno = pthread_rwlock_unlock (rwlock)))
						{
							::printf ("unlock fail eno=%i %s\n",
									  eno, strerror (eno));
							throw (exLockError);
						}
					 }
					 
					 /// Attempt a read-lock.
					 /// Tries to acquire a lock. Returns true
					 /// if one could be acquired within the timeout
					 /// limit.
					 /// \param secs Timeout in seconds.
	inline bool		 trylockr (int secs=0)
					 {
					 	if (! __THREADED) return true;
					 	if (! secs)
					 	{
			 				if (pthread_rwlock_tryrdlock (rwlock)) return false;
							return true;
						}
#ifdef PTHREAD_HAVE_TIMEDLOCK
						struct timespec ts;
						ts.tv_sec = secs + ::time (NULL);
						ts.tv_nsec = 0;
						int res;
						if (res = pthread_rwlock_timedrdlock (rwlock, &ts))
						{
							if (res == ETIMEDOUT)
								return false;
							sleep (1);
							return false;
						}
						return true;
#else
						if (! pthread_rwlock_tryrdlock (rwlock)) return true;
						for (int i=0; i<10; ++i)
						{
							__musleep (100 * secs);
							if (! pthread_rwlock_tryrdlock (rwlock))
								return true;
						}
						return false;
#endif
					 }

					 /// Attempt a write-lock.
					 /// Tries to acquire a lock. Returns true
					 /// if one could be acquired within the timeout
					 /// limit.
					 /// \param secs Timeout in seconds.
	inline bool		 trylockw (int secs = 0)
					 {
					 	if (! __THREADED) return true;
					 	if (! secs)
					 	{
				 			if (pthread_rwlock_trywrlock (rwlock)) return false;
							return true;
						}
#ifdef PTHREAD_HAVE_TIMEDLOCK
						struct timespec ts;
						ts.tv_sec = secs + ::time(NULL);
						ts.tv_nsec = 0;
						if ((res = pthread_rwlock_timedwrlock (rwlock, &ts)))
						{
							if (res == ETIMEDOUT)
								return false;
							sleep (1);
						}
						return true;
#else
						if (! pthread_rwlock_trywrlock (rwlock)) return true;
						for (int i=0; i<10; ++i)
						{
							__musleep (100 * secs);
							if (! pthread_rwlock_tryrdlock (rwlock))
								return true;
						}
						if (pthread_rwlock_trywrlock (rwlock)) return false;
						return true;
#endif
					 }
	
	kind			 o;
protected:
	pthread_rwlockattr_t	*attr;
	pthread_rwlock_t		*rwlock;
};

#else

#define LOCK_NONE           0x00
#define LOCK_READ           0x01
#define LOCK_WRITE          0x02
#define LOCK_PREWRITE       0x03
#define LOCK_READYWRITE     0x04
#define LOCK_WRITEPREWRITE  0x05

/// A thread lock.
/// Allows mixed readers/writers.
template<class kind>
class lock
{
public:
					 lock (void)
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
					~lock (void)
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
	inline void	 	 lockr (void)
					 {
					 	if (! __THREADED) return;
					 	int goahead = 0;
						pthread_t self = pthread_self();
						//::printf ("%08x@[%d] ::lockr()\n", this, self);
		
						pthread_mutex_lock (mutex);
						if (addme())
						{
							while (!goahead)
							{
								if ((status == LOCK_NONE) || (status == LOCK_READ))
								{
									//::printf ("@[%d] st->lockread\n", self);
									status = LOCK_READ;
									++numlocks;
									goahead = 1;
									pthread_mutex_unlock (mutex);
								}
								else
								{
									//::printf ("@[%d] lockr failed, condwait\n", self);
									pthread_cond_wait (cond, mutex);
								}
							}
						}
						else pthread_mutex_unlock (mutex);
					 }

					 /// Perform a write-lock.
					 /// Will block if there are current read-locks.
					 /// Pending read-locks are blocked until
					 /// this write-lock is acquired and released.
	inline void		 lockw (void)
					 {
					 	if (! __THREADED) return;
					 	int goahead = 0;
					 	bool upgrading = false;
						pthread_t self = pthread_self();
						//::printf ("%08x@[%d] ::lockw()\n", this, self);
						
						pthread_mutex_lock (mutex);
						
						if (! addme())
						{
							if (self != writer) upgrading = true;
							else
							{
								pthread_mutex_unlock (mutex);
								return;
							}
						}
						
						while (!goahead)
						{
							if ((upgrading) && (status == LOCK_PREWRITE) && (numlocks = 1))
							{
								status = LOCK_READYWRITE;
							}
							if ((status == LOCK_NONE)||(status == LOCK_READYWRITE))
							{
								//::printf ("@[%d] %s -> write\n", self, (status==LOCK_NONE) ? "none" : "readywrite");
								status = LOCK_WRITE;
								++numlocks;
								goahead = 1;
								writer = self;
								writerupgraded = upgrading;
							}
							else if (status == LOCK_READ)
							{
								//::printf ("@[%d] read -> prewrite\n", self);
								status = LOCK_PREWRITE;
							}
							else
							{
								//::printf ("@[%d] other status\n", self);
							}
							
							if (! goahead)
							{
								//::printf ("@[%d] lockw failed, cond_wait\n");
								pthread_cond_wait (cond, mutex);
							}
							else pthread_mutex_unlock (mutex);
						}
					 }
	inline void		 unlock (void)
					 {
					 	if (! __THREADED) return;
					 	pthread_mutex_lock (mutex);
						pthread_t self = pthread_self();
						//::printf ("%08x@[%d] ::unlock()\n", this, self);
						
						if (((status == LOCK_WRITE) && 
						     (writer == self) && 
						     (writerupgraded)) || delme())
						{
							--numlocks;
							// We're releasing a read lock
							if ((status == LOCK_READ)||(status == LOCK_PREWRITE))
							{
								//::printf ("@[%d] end readlock, newnum=%i\n", self, numlocks);
								if (! numlocks)
								{
									//::printf ("@[%d] newst=%s\n", self, (status==LOCK_PREWRITE) ? "readywrite" : "none");
									status = (status == LOCK_PREWRITE) ? LOCK_READYWRITE : LOCK_NONE;
								}
							}
							else // releasing a write lock
							{
								//::printf ("@[%d] end writelock\n", self);
								if (writerupgraded)
									status = LOCK_READ;
								else
									status = LOCK_NONE;
								
								writer = 0;
								writerupgraded = false;
							}
						}
						pthread_cond_broadcast (cond);
						pthread_mutex_unlock (mutex);
					 }
					 
					 /// Attempt a read-lock.
					 /// Tries to acquire a lock. Returns true
					 /// if one could be acquired within the timeout
					 /// limit.
					 /// \param secs Timeout in seconds.
	inline bool		 trylockr (int secs = 0)
					 {
					 	if (! __THREADED) return true;
					 	int goahead = 0;
						
						for (int i=0; i<10; ++i)
						{
							pthread_mutex_lock (mutex);
							if (addme())
							{
								if ((status == LOCK_NONE) || (status == LOCK_READ))
								{
									status = LOCK_READ;
									++numlocks;
									goahead = 1;
								}
								else
								{
									delme();
								}
							}
							pthread_mutex_unlock (mutex);
							if (goahead) return true;
							if (! secs) return false;
							
							__musleep (100 * secs);
						}
						return false;
					 }

					 /// Attempt a write-lock.
					 /// Tries to acquire a lock. Returns true
					 /// if one could be acquired within the timeout
					 /// limit.
					 /// \param secs Timeout in seconds.
	inline bool		 trylockw (int secs = 0)
					 {
					 	if (! __THREADED) return true;
					 	int goahead = 0;
					 	bool upgrading = false;
						pthread_t self = pthread_self();
						
						for (int i=0; i<10; ++i)
						{
							pthread_mutex_lock (mutex);
							if (! addme())
							{
								if (self != writer) upgrading = true;
								else
								{
									pthread_mutex_unlock (mutex);
									return true;
								}
							}
							if ((status == LOCK_NONE)||(status == LOCK_READYWRITE))
							{
								status = LOCK_WRITE;
								goahead = 1;
								writer = self;
								writerupgraded = upgrading;
							}
							else
							{
								if (! upgrading) delme();
							}

							pthread_mutex_unlock (mutex);
							if (goahead) return true;
							if (! secs) return false;
							
							__musleep (100 * secs);
						}
						return false;
					 }
	
	kind			 o;
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

#endif

#define exclusiveaccess(lname) if (bool __section_flip = true) \
	for (lname.lockw(); __section_flip; lname.unlock()) \
	  for (; __section_flip;) if (! (__section_flip = false))

#define sharedaccess(lname) if (bool __section_flip = true) \
	for (lname.lockw(); __section_flip; lname.unlock()) \
	  for (; __section_flip;) if (! (__section_flip = false))

#define exclusivesection(lname) if (bool __section_shared = false) {} else \
  if (bool __section_flip = true) \
	for (lname.lockw(); __section_flip; lname.unlock()) \
	  for (typeof( lname ) &sectionlock = lname; __section_flip;) \
		for (typeof(sectionlock.o) &lname = sectionlock.o;__section_flip;) if (! (__section_flip = false))

#define sharedsection(lname) if (bool __section_shared = true) \
  if (bool __section_flip = true) \
	for (lname.lockr(); __section_flip; lname.unlock()) \
	  for (typeof( lname ) &sectionlock = lname; __section_flip;) \
		for (typeof(sectionlock.o) &lname = sectionlock.o;__section_flip;) if (! (__section_flip = false))

#define unprotected(lname) if (bool __section_flip = true) \
	for (typeof( lname ) &sectionlock = lname; __section_flip;) \
		for (typeof(sectionlock.o) &lname = sectionlock.o; __section_flip;) \
			if (! (__section_flip = false))

#define breaksection if (bool __break_flip = true) \
	for (sectionlock.unlock(); __break_flip; (void)(__section_shared ? sectionlock.lockr() : sectionlock.lockw())) \
		for (; __break_flip; ) if (! (__break_flip = false))

/// Conditional.
/// Implies a condition that one or more threads can wait on that
/// other threads can raise. 
class conditional
{
public:
				 /// Constructor.
				 /// Sets up POSIX attributes.
				 conditional (void)
				 {
					attr = new pthread_mutexattr_t;
					mutex = new pthread_mutex_t;
					cond = new pthread_cond_t;
					queue = 0;
					
					pthread_mutexattr_init (attr);
					pthread_mutex_init (mutex, attr);
					pthread_cond_init (cond, NULL);
				 }
				 
				 /// Destructor.
				 /// Cleans up POSIX structures.
				~conditional (void)
				 {
					pthread_mutex_destroy (mutex); delete mutex;
					pthread_mutexattr_destroy (attr); delete attr;
					pthread_cond_destroy (cond); delete cond;
				 }
				
				 /// Wake up one thread that is waiting.
	void		 signal (void)
				 {
				 	pthread_mutex_lock (mutex);
				 	queue++;
				 	pthread_cond_signal (cond);
				 	pthread_mutex_unlock (mutex);
				 }
				 
				 /// Wake up all threads that are waiting.
	void		 broadcast (void)
				 {
				 	pthread_mutex_lock (mutex);
				 	queue++;
				 	pthread_cond_broadcast (cond);
				 	pthread_mutex_unlock (mutex);
				 }
	
				 /// Sleep until the condition is raised.
	bool		 wait (void)
				 {
				 	bool result = false;
				 	
				 	pthread_mutex_lock (mutex);
				 	if (queue)
				 	{
				 		--queue;
				 		pthread_mutex_unlock (mutex);
				 		return true;
				 	}
				 	if (! pthread_cond_wait (cond, mutex))
				 		result = true;
				 	pthread_mutex_unlock (mutex);
				 	return result;
				 }
				 
				 /// Sleep for a number of microseconds or until
				 /// the condition is raised.
				 /// \param timeout Timeout in 10^-4 seconds.
	bool		 wait (int timeout)
				 {
				 	bool result = false;
				 	struct timespec ts;
				 	
				 	ts.tv_sec = timeout/10000;
				 	ts.tv_nsec = 10 * (timeout % 10000);
				 	
				 	pthread_mutex_lock (mutex);
				 	if (queue)
				 	{
				 		--queue;
				 		pthread_mutex_unlock (mutex);
				 		return true;
				 	}
				 	if (! pthread_cond_timedwait (cond, mutex, &ts))
				 		result = true;
				 	pthread_mutex_unlock (mutex);
				 		
				 	return result;
				 }
	
protected:
	pthread_mutexattr_t		*attr; ///< POSIX mutex attribute.
	pthread_mutex_t			*mutex; ///< POSIX mutex.
	pthread_cond_t			*cond; ///< POSIX conditional.
	int						 queue; ///< Queue counter.
};

#endif
