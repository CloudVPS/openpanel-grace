// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

// ========================================================================
// thread.cpp: Thread and grouped thread abstracts
// ========================================================================

#include <grace/thread.h>

// ========================================================================
// CONSTRUCTOR thread
// ========================================================================
thread::thread (void)
{
	threadname = "thread";
	spawned = false;
	finished = false;
	__THREADED = true;
	memset (&schedparam, 0, sizeof(schedparam));
	unprotected (isrunning) { isrunning = false; }
	pthread_attr_init (&attr);
	tid = NULL;
}

thread::thread (const string &nm)
{
	threadname = nm;
	spawned = false;
	finished = false;
	__THREADED = true;
	unprotected (isrunning) { isrunning = false; }
	pthread_attr_init (&attr);
	tid = NULL;
}

// ========================================================================
// METHOD ::dorun
// ========================================================================
void *thread::dorun (void *param)
{
	thread *me = (thread *) param;
	sigset_t sigs;
	
	sigemptyset (&sigs);
	sigaddset (&sigs, SIGPIPE);
	pthread_sigmask (SIG_BLOCK, &sigs, NULL);
	pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, NULL);

	(*me).isrunning.lockw();
	(*me).isrunning.o = true;
	(*me).isrunning.unlock();

	statstring mtid = "%08x" %format ((unsigned long long) me);

	exclusivesection (THREADLIST)
	{
		THREADLIST.set (mtid, *me);
	}

	try
	{
		(*me).run();
	}
	catch (...)
	{
	}
	
	(*me).isrunning.lockw();
	(*me).isrunning.o = false;
	(*me).isrunning.unlock();
	(*me).finished = true;
	(*me).hasfinished.broadcast();
	
	exclusivesection (THREADLIST)
	{
		THREADLIST.rmval (mtid);
	}
	
	return NULL;
}

// ========================================================================
// METHOD thread::run
// ------------------
// How nice, a base class instance. Let's croak.
// ========================================================================
void thread::run (void)
{
	::printf ("%% Spawned thread base class\n");
}

// ========================================================================
// METHOD groupthread::run
// -----------------------
// How nice, a base class instance. Let's croak.
// ========================================================================
void groupthread::run (void)
{
	::printf ("%% Spawned groupthread base class\n");
}

// ========================================================================
// METHOD thread::spawn
// --------------------
// Spawns the thread into the background, if it hasn't already been done.
// ========================================================================
bool thread::spawn (void)
{
	if (spawned)
	{
		return true;
	}
	bool result = false;

	sharedsection (isrunning)
	{
		if (! isrunning)
		{
			if (pthread_create (&tid, &attr, thread::dorun, this))
			{
				sectionlock.unlock();
				throw threadCreateException();
			}
			pthread_detach (tid);
			result = true;
		}
	}
	
	spawned = result;
	return result;
}

// ========================================================================
// METHOD thread::runs
// -------------------
// Can be called by other threads to see if the thread is still running.
// ========================================================================
bool thread::runs (void)
{
	bool result;
	sharedsection (isrunning)
	{
		result = isrunning;
	}
	return result;
}
	
// ========================================================================
// METHOD threadgroup::gc
// ----------------------
// Clean up exited threads. This cannot be done at the moment the thread
// exits because it will mess up the pool data or create horrendous
// deadlocks, or some giant space goat will eat its home planet, I forgot
// the exact circumstances but they were terribly depressing.
// ========================================================================
void threadgroup::gc (void)
{
	int idx;
	sharedsection (lck)
	{
		for (idx=0; idx<cnt; ++idx)
		{
			if (array[idx]->finished)
			{
				breaksection { delete array[idx]; }
			}
		}
	}
}

// ========================================================================
// METHOD threadgroup::broadcastevent
// ----------------------------------
// Send an event to every member thread in the group. Spammer!
// ========================================================================
void threadgroup::broadcastevent (const value &ev)
{
	gc ();
	for (int idx=(cnt-1); idx>=0; --idx)
	{
	   array[idx]->sendevent (ev);
	}
}

void threadgroup::broadcastevent (const string &ev)
{
	gc ();
	for (int idx=(cnt-1); idx>=0; --idx)
	{
	   array[idx]->sendevent (ev);
	}
}

void threadgroup::broadcastevent (const statstring &ev)
{
	gc ();
	for (int idx=(cnt-1); idx>=0; --idx)
	{
	   array[idx]->sendevent (ev);
	}
}

void threadgroup::broadcastevent (const char *ev)
{
	gc ();
	for (int idx=(cnt-1); idx>=0; --idx)
	{
	   array[idx]->sendevent (ev);
	}
}

void threadgroup::broadcastevent (const statstring &tp, const value &ev)
{
	gc ();
	for (int idx=(cnt-1); idx>=0; --idx)
	{
	   array[idx]->sendevent (tp, ev);
	}
}

// ========================================================================
// METHOD threadgroup::add
// ========================================================================
void threadgroup::add (class groupthread *t)
{
	lck.lockw();
	if (! array)
	{
		arraysz = 8;
		array = (class groupthread **)
			malloc (8 * sizeof (class groupthread *));
	}
	if ( (cnt+1) > arraysz )
	{
		array = (class groupthread **)
			realloc (array, (arraysz*2) *
							sizeof (class groupthread *));
		arraysz *= 2;
	}
	array[cnt++] = t;
	lck.unlock();
}

// ========================================================================
// METHOD threadgroup::remove
// ========================================================================
void threadgroup::remove (class groupthread *t)
{
	lck.lockw();
	for (int i=0; i<cnt; ++i)
	{
		if (array[i] == t)
		{
			for (int j=i+1; j<cnt; ++j)
			{
				array[i++] = array[j];
			}
			--cnt;
		}
	}
	lck.unlock();
}
