// ========================================================================
// eventq.cpp: GRACE event queue for thread communication.
//
// (C) Copyright 2006 Pim van Riezen <pi@madscience.nl>
//                    Madscience Labs, Rotterdam 
// ========================================================================
#include <grace/eventq.h>

// ========================================================================
// CONSTRUCTOR eventq
// ========================================================================
eventq::eventq (void)
{
}

// ========================================================================
// DESTRUCTOR eventq
// ========================================================================
eventq::~eventq (void)
{
}

// ========================================================================
// METHOD ::count
// ========================================================================
int eventq::count (void)
{
	int res;
	sharedsection (ipc)
	{
		res = ipc.count();
	}
	return res;
}

// ========================================================================
// METHOD ::send
// ========================================================================
void eventq::send (const value &ev)
{
	exclusivesection (ipc)
	{
		ipc.newval() = ev;
		ipc[-1].type ("event");
	}
	event.signal ();
}

void eventq::send (const statstring &tp, const value &ev)
{
	exclusivesection (ipc)
	{
		ipc.newval() = ev;
		ipc[-1].type (tp);
	}
	event.signal ();
}

void eventq::send (const statstring &tp)
{
	exclusivesection (ipc)
	{
		ipc.newval() = true;
		ipc[-1].type (tp);
	}
	event.signal();
}

// ========================================================================
// METHOD ::waitevent
// ========================================================================
value *eventq::waitevent (int timeout_msec)
{
	if (timeout_msec == 0) return waitevent ();
	returnclass (value) res retain;
	
	exclusivesection (ipc)
	{
		while (! ipc.count())
		{
			breaksection
			{
				if (! event.wait(timeout_msec))
				{
					return &res;
				}
			}
		}
		res = (const value &) ipc[0];
		ipc.rmindex (0);
	}
	return &res;
}

value *eventq::waitevent (void)
{
	returnclass (value) res retain;
	
	exclusivesection (ipc)
	{
		while (! ipc.count())
		{
			breaksection
			{
				if (! event.wait())
				{
					return &res;
				}
			}
		}
		res = (const value &) ipc[0];
		ipc.rmindex (0);
	}
	
	return &res;
}

// ========================================================================
// METHOD ::nextevent
// ========================================================================
value *eventq::nextevent (void)
{
	returnclass (value) res retain;
	
	exclusivesection (ipc)
	{
		if (! ipc.count()) breaksection return &res;
		res = (const value &) ipc[0];
		ipc.rmindex (0);
	}
	return &res;
}
