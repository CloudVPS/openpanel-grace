#ifndef _THREAD_H
#define _THREAD_H 1

#include <grace/str.h>
#include <grace/value.h>
#include <grace/lock.h>

#include <pthread.h>
#include <signal.h>

extern bool __THREADED;

/// Thread related exceptions.
enum threadException {
	EX_THREAD_CREATE		= 0xd40398bb, ///< Error creating thread.
	EX_THREAD_INVALID_INDEX = 0xd079dc79  ///< Wrong index threadgroup array.
};

extern lock<value> THREADLIST;

/// Base class for threads.
/// Derived classes can use this class to spawn into a background thread.
class thread
{
public:
					 thread (void)
					 {
					 	threadname = "thread";
					 	spawned = false;
					 	finished = false;
					 	__THREADED = true;
					 	unprotected (ipc) { ipc["running"] = false; }
					 	pthread_attr_init (&attr);
					 }
					 
					 thread (const string &nm)
					 {
					 	threadname = nm;
					 	spawned = false;
					 	finished = false;
					 	__THREADED = true;
					 	unprotected (ipc) { ipc["running"] = false; }
					 	pthread_attr_init (&attr);
					 }
					 
	virtual			~thread (void)
					 {
					 }
					 
					 /// Start the thread (alias for spawn).
					 /// \throw threadException Error creating a thread.
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
	static void		*dorun (void *param)
					 {
					 	thread *me = (thread *) param;
					 	sigset_t sigs;
					 	
					 	sigemptyset (&sigs);
					 	sigaddset (&sigs, SIGPIPE);
					 	pthread_sigmask (SIG_BLOCK, &sigs, NULL);
					 	pthread_setcanceltype (PTHREAD_CANCEL_DISABLE, NULL);

					 	(*me).ipc.lockw();
						(*me).ipc.o["running"] = true;
						(*me).ipc.unlock();

						string mtid;
						mtid.printf ("%08x", me);

						exclusivesection (THREADLIST)
						{
							THREADLIST[mtid] = me->threadname;
						}

						(*me).run();
						
						(*me).ipc.lockw();
						(*me).ipc.o["running"] = false;
						(*me).ipc.unlock();
						(*me).finished = true;
						
						exclusivesection (THREADLIST)
						{
							THREADLIST.rmindex (mtid);
						}
						
						return NULL;
					 }
					 
					 /// Spawn the thread in the background.
					 /// \throw threadException Error creating a thread.
	bool			 spawn (void);
	
					 /// Check thread's running status.
					 /// \return Status, \b true if thread runs.
	bool			 runs (void);
	
					 /// Get the next event from the queue.
					 /// The run method can call this to peek at any
					 /// incoming events.
					 /// \return The value object containing the event data,
					 ///         which may be empty.
	value			*nextevent (void);

					 /// Block waiting for an event on the queue.
					 /// The run method can call this to wait for work.
					 /// \return The value object containing the event data,
					 ///         which should not be empty.
	value			*waitevent (void);
					 
					 /// Send an event to the thread.
					 /// Other threads can call this method to dispatch
					 /// commands or other events.
					 /// \param v The event data.
	void			 sendevent (const value &v)
					 {
					 	exclusivesection (ipc)
					 	{
					 		ipc["events"].newval () = v;
					 	}
					 	event.signal ();
					 }
					 
					 /// Measure the queue size.
					 /// \return Number of events in the queue.
	int			 	 eventqueue (void)
					 {
					 	int result = 0;
					 	sharedsection (ipc)
						{
						 	result = ipc["events"].count ();
						}
					 	return result;
					 }

					 /// Find out thread's unique id.
					 /// \return The unique id number.
	unsigned int	 threadid (void)
					 {
					 	if (! tid) return (unsigned int) this;
					 	return (unsigned int) tid;
					 }
					 
					 /// Set the scheduling priority.
					 /// \param prio The new priority.
	void			 setpriority (int prio)
					 {
					 	schedparam.sched_priority = prio;
					 	pthread_setschedparam (tid, SCHED_OTHER, &schedparam);
					 }
	
	bool			 finished; ///< True if the run method has finished.
	lock<value>		 ipc; ///< Event queue.
	conditional		 event; ///< Event trigger.
	lock<value>		 data; ///< More internal storage.
	
	string			 threadname;
	static value	*getlist (void)
					 {
					 	returnclass (value) res retain;
					 	sharedsection (THREADLIST)
					 	{
					 		res = THREADLIST;
					 	}
					 	return &res;
					 }
	
protected:
	pthread_t		 tid; ///< Local thread id.
	pthread_attr_t	 attr; ///< POSIX thread attributes.
	sched_param		 schedparam; ///< POSIX scheduling parameters.
	bool			 spawned; ///< True if thread was spawned.
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
	inline void		 add (class groupthread *t)
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
					 
					 /// Remove a thread object from the array.
	inline void		 remove (class groupthread *t)
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
					 
					 /// Array access operator.
					 /// \param idx The index number.
					 /// \return Reference to object at index.
					 /// \throw threadException Index was out of bounds.
	inline groupthread &operator[] (int idx)
					 {
						if (idx<0) throw (EX_THREAD_INVALID_INDEX);
						if (idx>cnt) throw (EX_THREAD_INVALID_INDEX);
						return *(array[idx]);
					 }
					 
					 /// Send an event to all member threads.
					 /// Starts with the last thread in the array
					 /// \param ev The event data.
	void			 broadcastevent (const value &ev);
	
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
					 
					 groupthread (threadgroup &gr, const string &nm)
					 	: thread (nm)
					 {
					 	__THREADED = true;
					 	group = &gr;
					 	(*group).add (this);
					 }
					 
	virtual			~groupthread (void)
					 {
						(*group).remove (this);
					 }
					 
	virtual void	 run (void);
					 
protected:
	threadgroup		*group;	///< Link to parent group.
};

#endif
