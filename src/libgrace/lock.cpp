// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#include <grace/lock.h>
#include <grace/system.h>

volatile bool __THREADED = false;

////////////////////////////////////////////////////////// WITH RWLOCK ////
#ifdef PTHREAD_HAVE_RWLOCK

// ========================================================================
// METHOD ::lockr
// ========================================================================
void lockbase::lockr (void)
{
	if (! __THREADED) return;
	int eno;
	if ((eno = pthread_rwlock_rdlock (rwlock)))
	{
		if (eno != EDEADLK)
		{
			throw lockException();
		}
	}
}

// ========================================================================
// METHOD ::lockw
// ========================================================================
void lockbase::lockw (void)
{
	if (! __THREADED) return;
	int eno;
	if ((eno = pthread_rwlock_wrlock (rwlock)))
	{
		if (eno != EDEADLK)
		{
			throw lockException();
		}
	}
}

// ========================================================================
// METHOD ::trylockr
// ========================================================================
bool lockbase::trylockr (int secs)
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
	int res = pthread_rwlock_timedrdlock (rwlock, &ts);
	if (res)
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

// ========================================================================
// METHOD ::trylockw
// ========================================================================
bool lockbase::trylockw (int secs)
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
	
	int res;
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

void __lockbase_unlock_breakme (void)
{
}

// ========================================================================
// METHOD ::unlock
// ========================================================================
void lockbase::unlock (void)
{
	int eno;
	if (! __THREADED) return;
	if ((eno = pthread_rwlock_unlock (rwlock)))
	{
		__lockbase_unlock_breakme ();
		throw lockException();
	}
}

/////////////////////////////////////////////////////// WITHOUT RWLOCK ////
#else

// ========================================================================
// METHOD ::lockr
// ========================================================================
void lockbase::lockr (void)
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

// ========================================================================
// METHOD ::lockw
// ========================================================================
void lockbase::lockw (void)
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

// ========================================================================
// METHOD ::unlock
// ========================================================================
void lockbase::unlock (void)
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

// ========================================================================
// METHOD ::trylockr
// ========================================================================
bool lockbase::trylockr (int secs)
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

// ========================================================================
// METHOD ::trylockw
// ========================================================================
bool lockbase::trylockw (int secs)
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

#endif
////////////////////////////////////////////////////////////// GENERIC ////

// ========================================================================
// CONSTRUCTOR conditional
// ========================================================================
conditional::conditional (void)
{
	attr = new pthread_mutexattr_t;
	mutex = new pthread_mutex_t;
	cond = new pthread_cond_t;
	queue = 0;
	
	pthread_mutexattr_init (attr);
	pthread_mutex_init (mutex, attr);
	pthread_cond_init (cond, NULL);
}

// ========================================================================
// DESTRUCTOR conditional
// ========================================================================
conditional::~conditional (void)
{
	pthread_mutex_destroy (mutex); delete mutex;
	pthread_mutexattr_destroy (attr); delete attr;
	pthread_cond_destroy (cond); delete cond;
}

// ========================================================================
// METHOD ::signal
// ========================================================================
void conditional::signal (void)
{
	pthread_mutex_lock (mutex);
	queue++;
	pthread_cond_signal (cond);
	pthread_mutex_unlock (mutex);
}

// ========================================================================
// METHOD ::broadcast
// ========================================================================
void conditional::broadcast (void)
{
	pthread_mutex_lock (mutex);
	queue++;
	pthread_cond_broadcast (cond);
	pthread_mutex_unlock (mutex);
}

// ========================================================================
// METHOD ::wait
// ========================================================================
bool conditional::wait (void)
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
	{
		--queue;
		result = true;
	}
	pthread_mutex_unlock (mutex);
	return result;
}

bool conditional::wait (int timeout)
{
	bool result = false;
	struct timespec ts;
	
	struct timeval otv;
	otv = core.time.unow();
	
	ts.tv_sec = otv.tv_sec;
	ts.tv_nsec = 1000 * otv.tv_usec;
	
	ts.tv_nsec += (1000000LL * (timeout % 1000));
	ts.tv_sec += timeout / 1000;
	
	if (ts.tv_nsec > 1000000000LL)
	{
		ts.tv_nsec -= 1000000000LL;
		ts.tv_sec++;
	}

	pthread_mutex_lock (mutex);
	if (queue)
	{
		--queue;
		pthread_mutex_unlock (mutex);
		return true;
	}
	if (! pthread_cond_timedwait (cond, mutex, &ts))
	{
		--queue;
		result = true;
	}
	pthread_mutex_unlock (mutex);
		
	return result;
}
