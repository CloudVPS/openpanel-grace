// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _THREAD_H
#define _THREAD_H 1

#include <grace/str.h>
#include <grace/value.h>
#include <grace/lock.h>
#include <grace/eventq.h>
#include <grace/dictionary.h>

#include <pthread.h>
#include <signal.h>

typedef dictionary<class thread> globalthreadlist;
extern lock<globalthreadlist> THREADLIST;
extern volatile bool __THREADED;

$exception (threadCreateException, "Could not create thread");
$exception (threadGroupIndexException, "Invalid group index");

/// Base class for threads.
/// Derived classes can use this class to spawn into a background thread.
class thread
{
public:
					 thread (void);
					 thread (const string &nm);
					 
	virtual			~thread (void)
					 {
					 }
					 
					 /// Start the thread (alias for spawn).
					 /// \throw threadCreateException Error creating a thread.
	inline void		 start (void)
					 {
					 	spawn();
					 }
					 
					 /// Thread implementation (virtual method).
	virtual	void	 run (void);
	
					 /// Static method to start thread.
					 /// Used as an argument for the system thread create.
					 /// \param param Pointer to the thread object.
					 /// \return NULL (return value mandated by POSIX API).
	static void		*dorun (void *param);
					 
					 /// Spawn the thread in the background.
					 /// \throw threadCreateException Error creating a thread.
	bool			 spawn (void);
	
					 /// Check thread's running status.
					 /// \return Status, \b true if thread runs.
	bool			 runs (void);
	
					 /// Get the next event from the queue.
					 /// The run method can call this to peek at any
					 /// incoming events.
					 /// \return The value object containing the event data,
					 ///         which may be empty.
	value			*nextevent (void)
					 {
					 	return events.nextevent ();
					 }

					 /// Block waiting for an event on the queue.
					 /// The run method can call this to wait for work.
					 /// \return The value object containing the event data,
					 ///         which should not be empty.
	value			*waitevent (void)
					 {
					 	return events.waitevent ();
					 }
					 
					 /// Block waiting for an event on the queue, with
					 /// timeout.
					 /// The run method can call this to wait for work.
					 /// \param timeout_msec The timeout in milliseconds.
					 /// \return The value object containing the event data,
					 ///         which should not be empty.
	value			*waitevent (int timeout_msec)
					 {
					 	return events.waitevent (timeout_msec);
					 }
					 
					 /// Send an event to the thread.
					 /// Other threads can call this method to dispatch
					 /// commands or other events. The receiving thread
					 /// will get this value with a type() of "event".
					 /// \param v The event data.
	void			 sendevent (const value &v,
								eventq::priority p = eventq::normal)
					 {
					 	events.send (v, p);
					 }
					 
					 /// Send an event without data to the thread.
					 /// The receiving thread will get a value-object
					 /// with a a type() set to your liking and a
					 /// boolean value of 'true'.
					 /// \param type The event-type.
	void			 sendevent (const statstring &type,
								eventq::priority p = eventq::normal)
					 {
					 	events.send (type, p);
					 }
					 
					 /// Send an event without data to the thread.
					 /// The receiving thread will get a value-object
					 /// with a a type() set to your liking and a
					 /// boolean value of 'true'.
					 /// \param type The event-type.
	void			 sendevent (const string &type,
								eventq::priority p = eventq::normal)
					 {
					 	statstring tp = type;
					 	events.send (tp, p);
					 }
					 
					 /// Send an event without data to the thread.
					 /// The receiving thread will get a value-object
					 /// with a a type() set to your liking and the
					 /// data from the value you supply.
					 /// \param type The event-type.
					 /// \param data The event data.
	void			 sendevent (const statstring &type, const value &data,
								eventq::priority p = eventq::normal)
					 {
					 	events.send (type, data, p);
					 }
					 
	void			 sendevent (const char *type,
								eventq::priority p = eventq::normal)
					 {
					 	statstring tp = type;
					 	events.send (tp);
					 }
					 
					 /// Measure the queue size.
					 /// \return Number of events in the queue.
	int			 	 eventqueue (void)
					 {
					 	return events.count();
					 }
					 
	void			 shutdown (void)
					 {
					 	sendevent ("shutdown");
					 	hasfinished.wait();
					 }

					 /// Find out thread's unique id.
					 /// \return The unique id number.
	unsigned int	 threadid (void)
					 {
					 	void *me = this;
					 	void *tmp = &me;
					 	if (sizeof (void *) == 4)
					 	{
					 		unsigned int *o = (unsigned int *) tmp;
					 		return *o;
					 	}
					 	else
					 	{
					 		unsigned long long *o = (unsigned long long *) tmp;
					 		unsigned int r;
					 		r =  (unsigned int) (((*o) & (0xffffffff00000000LL)) >> 32);
					 		r ^= (unsigned int) ((*o) & 0xffffffffLL);
					 		return r;
					 	}
					 }
					 
					 /// Set the scheduling priority.
					 /// \param prio The new priority.
	void			 setpriority (int prio)
					 {
					 	schedparam.sched_priority = prio;
					 	pthread_setschedparam (tid, SCHED_OTHER, &schedparam);
					 }
					 
					 /// Kill this thread dead.
	void			 kill (void)
					 {
					 	pthread_kill (tid, SIGKILL);
					 }
	
	bool			 finished; ///< True if the run method has finished.
	string			 threadname; ///< Name of the thread.
	
					 /// Return a list of spawned thread objects.
	static value	*getlist (void)
					 {
					 	returnclass (value) res retain;
					 	sharedsection (THREADLIST)
					 	{
					 		foreach (t, THREADLIST)
					 		{
					 			statstring i;
					 			i = "%08x" %format ((unsigned long long)&t);
					 			res[i] = t.threadname;
					 		}
					 	}
					 	return &res;
					 }
					 
					 /// Shutdown all threads.
					 /// \param timeout_s Timeout in seconds before
					 ///                  all threads are killed, 0 for
					 ///                  infinite timeout.
	static void		 shutdownall (int timeout_s=0)
					 {
					 	exclusivesection (THREADLIST)
					 	{
					 		foreach (t, THREADLIST)
					 		{
					 			t.sendevent ("shutdown");
					 		}
					 	}
					 	
					 	for (int i=0; (timeout_s == 0) || (i<timeout_s); ++i)
					 	{
					 		sharedsection (THREADLIST)
					 		{
					 			if (! THREADLIST.count()) return;
					 		}
					 		sleep (1);
					 	}
					 	
					 	exclusivesection (THREADLIST)
					 	{
					 		foreach (t, THREADLIST) t.kill();
					 	}
					 }

protected:
	pthread_t		 tid; ///< Local thread id.
	pthread_attr_t	 attr; ///< POSIX thread attributes.
	sched_param		 schedparam; ///< POSIX scheduling parameters.
	bool			 spawned; ///< True if thread was spawned.
	eventq			 events; ///< Event queue.
	lock<bool>		 isrunning; ///< True if thread runs.
	conditional		 hasfinished; ///< Will trigger if thread exits.
};

/// Array group of threads.
/// Storage for situations where you have a group of worker threads that
/// you want to keep track of. Collects thread objects that are derived
/// from the groupthread class.
class threadgroup
{
public:
					 threadgroup (void)
					 {
					 	__THREADED = true;
					 	arraysz = cnt = 0;
						array = 0;
					 }
					~threadgroup (void)
					 {
					 	if (array) free (array);
					 }
					 
					 /// Add a new thread to the array.
					 /// Called from the groupthread constructor.
	void			 add (class groupthread *t);
					 
					 /// Remove a thread object from the array.
	void			 remove (class groupthread *t);
					 
					 /// Array access operator.
					 /// \param idx The index number.
					 /// \return Reference to object at index.
					 /// \throw threadGroupIndexException Index was out of bounds.
	inline groupthread &operator[] (int idx)
					 {
						if (idx<0) throw threadGroupIndexException();
						if (idx>cnt) throw threadGroupIndexException();
						return *(array[idx]);
					 }
					 
					 /// Send an event to all member threads.
					 /// Starts with the last thread in the array
					 /// \param ev The event data.
	void			 broadcastevent (const value &ev);
	void			 broadcastevent (const string &tp);
	void			 broadcastevent (const statstring &tp);
	void			 broadcastevent (const char *tp);
	void			 broadcastevent (const statstring &t, const value &ev);
	
					 /// Get a thread count.
					 /// \return Number of threads in the array.
	inline int		 count (void)
					 {
					 	return cnt;
					 }
					 
					 /// Remove threads from the list that have finished.
	void			 gc (void);
	
	lock<int>		 lck; ///< Lock for the array.
	
protected:
	class groupthread	**array; ///< Storage array.
	int					  cnt; ///< Number of groupthreads in the array.
	int					  arraysz; ///< Allocated array size.
};

/// Grouped worker thread.
/// Implements a thread that can be used in a threadgroup.
class groupthread : public thread
{
public:
					 /// Constructor.
					 /// \param gr Parent threadgroup to link into.
					 groupthread (threadgroup &gr) : thread("groupthread")
					 {
					 	__THREADED = true;
					 	group = &gr;
					 	(*group).add (this);
					 }
					 
					 /// Constructor.
					 /// \param gr Parent threadgroup to link into.
					 /// \param nm Thread name in the thread list.
					 groupthread (threadgroup &gr, const string &nm)
					 	: thread (nm)
					 {
					 	__THREADED = true;
					 	group = &gr;
					 	(*group).add (this);
					 }
					 
					 /// Destructor.
	virtual			~groupthread (void)
					 {
						(*group).remove (this);
					 }
					 
					 /// Main run method for the thread.
	virtual void	 run (void);
	
	lock<value>		 data; ///< Misc. storage.
protected:
	threadgroup		*group;	///< Link to parent group.
};

#endif
