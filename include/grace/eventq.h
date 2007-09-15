#ifndef _EVENTQ_H
#define _EVENTQ_H 1

#include <grace/lock.h>
#include <grace/value.h>

/// A class for sending event-data to a thread. It implements a fifo-queue
/// of value objects. The receiving thread can either poll or sleep on
/// incoming events.
class eventq
{
public:
			 /// Constructor. Nothing to see here.
			 eventq (void);
			 
			 // Destructor.
			~eventq (void);

			 /// Return number of pending events.			
	int		 count (void);
	
			 /// Send an event. Will be received with a type() of "event".
			 /// \param v The event-data.
	void	 send (const value &v);
	
			 /// Send an event with a specified type.
			 /// \param tp The type() of the event.
			 /// \param dt The event-data.
	void	 send (const statstring &tp, const value &dt);
	
			 /// Send an empty event with only a type. Will be received
			 /// as a boolean 'true' and the tprovided type().
			 /// \param tp The type() of the event.
	void	 send (const statstring &tp);
	
			 /// Send an empty event with only a type. Will be received
			 /// as a boolean 'true' and the tprovided type().
			 /// \param tp The type() of the event.
	void	 send (const char *tp) { send ((statstring) tp); }
	
			 /// Get the next event, if there is any. Will return an
			 /// empty value-object if no events are waiting.
	value	*nextevent (void);
	
			 /// Wait for a new event with a timeout. Will return an
			 /// empty value-object if the timeout was reached.
			 /// \param timeout_msec The timeout in milliseconds.
	value	*waitevent (int timeout_msec);
	
			 /// Wait indefinitely for a new event.
	value	*waitevent (void);
	
protected:
	lock<value>	ipc; ///< The actual event queue.
	conditional	event; ///< Will trigger if a new event is added.
};

#endif
