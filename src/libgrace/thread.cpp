// ========================================================================
// thread.cpp: Thread and grouped thread abstracts
//
// (C) Copyright 2005-2006 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam 
// ========================================================================

#include <grace/thread.h>

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

	sharedsection (ipc)
	{
		if (! ipc["running"])
		{
			if (pthread_create (&tid, &attr, thread::dorun, this))
			{
				sectionlock.unlock();
				throw (EX_THREAD_CREATE);
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
	sharedsection (ipc)
	{
		result = ipc["running"];
	}
	return result;
}
	
// ========================================================================
// METHOD thread::nextevent
// ------------------------
// Can be called from inside the thread to get the next event out of
// the queue, or NULL if none are to be had.
// ========================================================================
value *thread::nextevent (void)
{
	returnclass (value) res retain;

	exclusivesection (ipc)
	{
		if (! ipc["events"].count())
		{
			breaksection
			{
				return &res;
			}
		}
		res = ipc["events"][0];
		ipc["events"].rmindex (0);
	}
	return &res;
}

// ========================================================================
// METHOD thread::waitevent
// ------------------------
// Can be called from inside the thread to get the next event out of
// the queue, will sleep and wait indefinitely for one to arrive.
// ========================================================================
value *thread::waitevent (void)
{
	returnclass (value) res retain;
	
	exclusivesection (ipc)
	{
		while (! ipc["events"].count())
		{
			breaksection
			{
				if (! event.wait())
				{
					return &res;
				}
			}
		}
		res = ipc["events"][0];
		ipc["events"].rmindex (0);
	}

	return &res;
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
