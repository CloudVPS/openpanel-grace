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
	ipc.lockr();
	if (! ipc.o["running"])
	{
		ipc.unlock();
		if (pthread_create (&tid, &attr,
							thread::dorun, this))
		{
			ipc.unlock();
			throw (EX_THREAD_CREATE);
		}
		pthread_detach (tid);
		result = true;
	}
	else ipc.unlock();
	
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
	ipc.lockr();
	result = ipc.o["running"];
	ipc.unlock();
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
	value *res = new value;
	ipc.lockw ();
	if (! ipc.o["events"].count())
	{
		ipc.unlock ();
		return res;
	}
	(*res) = ipc.o["events"][0];
	ipc.o["events"].rmindex (0);
	ipc.unlock ();
	return res;
}

// ========================================================================
// METHOD thread::waitevent
// ------------------------
// Can be called from inside the thread to get the next event out of
// the queue, will sleep and wait indefinitely for one to arrive.
// ========================================================================
value *thread::waitevent (void)
{
	value *res = new value;
	ipc.lockw ();
	while (! ipc.o["events"].count())
	{
		ipc.unlock ();
		if (! event.wait())
		{
			return res;
		}
		ipc.lockw ();
	}
	(*res) = ipc.o["events"][0];
	ipc.o["events"].rmindex (0);
	ipc.unlock ();
	return res;
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
	lck.lockr();
	for (idx=0; idx<cnt; ++idx)
	{
		if (array[idx]->finished)
		{
			lck.unlock();
			delete array[idx];
			lck.lockr();
		}
	}
	lck.unlock();
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
