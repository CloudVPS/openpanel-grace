// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

// ========================================================================
// eventq.cpp: GRACE event queue for thread communication.
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
