#include <grace/eventq.h>

eventq::eventq (void)
{
}

eventq::~eventq (void)
{
}

int eventq::count (void)
{
	int res;
	sharedsection (ipc)
	{
		res = ipc.count();
	}
	return res;
}

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
				if (! event.wait(timeout_msec * 10))
				{
					return &res;
				}
			}
		}
		res = ipc[0];
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
		res = ipc[0];
		ipc.rmindex (0);
	}
	return &res;
}

value *eventq::nextevent (void)
{
	
}
