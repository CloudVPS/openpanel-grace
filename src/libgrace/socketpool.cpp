// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

// ========================================================================
// socketpool.cpp: Classes implementing a pool of open connections to a
//                 host.
// ========================================================================
#include <grace/socketpool.h>

// ========================================================================
// DESTRUCTOR poolsocket
// ========================================================================
poolsocket::~poolsocket (void)
{
	if (! iscopy)
	{
		if (sock) delete sock;
	}
	sock = NULL;
}

poolsocket &poolsocket::operator= (const poolsocket &s)
{
	sock = s.sock;
	iscopy = true;
	connected = s.connected;
	when = s.when;
	inuse = s.inuse;
	serial = s.serial;
	
	return *this;
}

tcpsocket &poolsocket::s (void)
{
	static tcpsocket nosock;
	if (! sock) return nosock;
	return *sock;
}

// ========================================================================
// METHOD basicpoolhandler::isvalid
// ========================================================================
bool basicpoolhandler::isvalid (tcpsocket &s, time_t last)
{
	if (! timeout) return true;
	return false;
}

// ========================================================================
// METHOD basicpoolhandler::close
// ========================================================================
void basicpoolhandler::close (tcpsocket &s)
{
	s.close();
}

// ========================================================================
// METHOD basicpoolhandler::open
// ========================================================================
bool basicpoolhandler::open (tcpsocket &s)
{
	return s.connect (host, port);
}

// ========================================================================
// CONSTRUCTOR socketpool
// ========================================================================
socketpool::socketpool (const string &hostname, int port, int timeout)
{
	handler = new basicpoolhandler (hostname, port, timeout);
	first = last = NULL;
	nativehandler = true;
	count = 0;
}

// ========================================================================
// CONSTRUCTOR socketpool
// ========================================================================
socketpool::socketpool (socketpoolhandler &hdl)
{
	handler = &hdl;
	first = last = NULL;
	nativehandler = false;
	count = 0;
}

// ========================================================================
// DESTRUCTOR socketpool
// ========================================================================
socketpool::~socketpool (void)
{
	poolsocket *s, *ns;
	
	s = first;
	while (s)
	{
		ns = s->next;
		delete s;
		s = ns;
	}
	first = last = NULL;
	
	if (nativehandler) delete handler;
}

// ========================================================================
// METHOD socketpool::setsize
// ========================================================================
bool socketpool::setsize (unsigned int ncount)
{
	time_t now = core.time.now();
	lck.lockw();
	
	if (ncount < count) return false;
	
	while (count < ncount)
	{
		poolsocket *s = new poolsocket;

		s->when = now;
		s->connected = false;
		s->serial = serial++;
		
		if (last)
		{
			s->prev = last;
			s->next = NULL;
			last->next = s;
			last = s;
		}
		else
		{
			s->next = s->prev = NULL;
		}
		count++;
	}
	lck.unlock();
	return true;
}

// ========================================================================
// METHOD sovketpool::getsocket
// ========================================================================
bool socketpool::getsocket (poolsocket &into)
{
	poolsocket *crsr;
	time_t now = core.time.now();
	lck.lockr();
	
	crsr = first;
	while (crsr)
	{
		if (! crsr->inuse)
		{
			if (! crsr->connected)
			{
				if (! handler->open (crsr->s()))
				{
					lck.unlock();
					return false;
				}
				crsr->connected = true;
				crsr->when = now;
			}
			else if (! handler->isvalid (crsr->s(), crsr->when))
			{
				handler->close (crsr->s());
				if (! handler->open (crsr->s()))
				{
					lck.unlock();
					crsr->connected = false;
					return false;
				}
				crsr->when = now;
			}
			crsr->inuse = true;
			lck.unlock();
			into = (*crsr);
			return true;
		}
		crsr = crsr->next;
	}
	lck.unlock();
	return false;
}

// ========================================================================
// METHOD socketpool::done
// ========================================================================
void socketpool::done (poolsocket &s)
{
	poolsocket *crsr;
	lck.lockr();
	
	crsr = first;
	while (crsr)
	{
		if (crsr->serial == s.serial)
		{
			crsr->inuse = false;
			lck.unlock();
			return;
		}
		crsr = crsr->next;
	}
	lck.unlock();
}

socketpoolhandler::~socketpoolhandler (void)
{
}

bool socketpoolhandler::isvalid (tcpsocket &s, time_t last)
{
	return false;
}

void socketpoolhandler::close (tcpsocket &s)
{
}

bool socketpoolhandler::open (tcpsocket &s)
{
	return false;
}
