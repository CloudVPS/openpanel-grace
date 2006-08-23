#ifndef _EVENTQ_H
#define _EVENTQ_H 1

#include <grace/lock.h>
#include <grace/value.h>

class eventq
{
public:
			 eventq (void);
			~eventq (void);
			
	int		 count (void);
	void	 send (const value &);
	void	 send (const statstring &tp, const value &dt);
	void	 send (const statstring &tp);
	value	*nextevent (void);
	value	*waitevent (int timeout_msec);
	value	*waitevent (void);
	
protected:
	lock<value>	ipc;
	conditional	event;
};

#endif
