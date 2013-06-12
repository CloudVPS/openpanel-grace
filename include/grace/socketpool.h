// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _SOCKETPOOL_H
#define _SOCKETPOOL_H 1

#include <grace/tcpsocket.h>
#include <grace/str.h>
#include <grace/system.h>

class poolsocket
{
friend class socketpool;
public:
					 poolsocket (void)
					 {
					 	next = prev = NULL;
					 	connected = false;
					 	iscopy = false;
					 	when = 0;
					 	inuse = false;
					 	sock = new tcpsocket;
					 	serial = 0;
					 }
					 poolsocket (const poolsocket &s)
					 {
					 	serial = s.serial;
					 	sock = s.sock;
					 	connected = s.connected;
					 	when = core.time.now();
					 	iscopy = true;
					 	inuse = false;
					 }
					~poolsocket (void);
					 
	poolsocket		&operator= (const poolsocket &s);
	tcpsocket		&s (void);

protected:				 
	unsigned int	 serial;
	tcpsocket		*sock;
	bool			 connected;
	bool			 iscopy;
	time_t			 when;
	bool			 inuse;

	poolsocket	*prev, *next;
};

class socketpoolhandler
{
public:
	virtual				~socketpoolhandler (void);
	virtual bool		 isvalid (tcpsocket &s, time_t last);
	virtual void		 close (tcpsocket &s);
	virtual bool		 open (tcpsocket &s);
};

class basicpoolhandler : public socketpoolhandler
{
public:
						 basicpoolhandler (const string &h, int p, int to)
						 {
						 	host = h;
						 	port = p;
						 	timeout = to;
						 }
						~basicpoolhandler (void) {}
	bool				 isvalid (tcpsocket &s, time_t last);
	void				 close (tcpsocket &s);
	bool				 open (tcpsocket &s);
	
protected:
	string				 host;
	int					 port;
	int					 timeout;
};

class socketpool
{
public:
						 socketpool (const string &, int, int);
						 socketpool (socketpoolhandler &hdl);
						~socketpool (void);
				
	bool				 setsize (unsigned int count);
	bool				 getsocket (poolsocket &);
	void				 done (poolsocket &);

protected:
	poolsocket			*first, *last;
	lock<bool>			 lck;
	socketpoolhandler	*handler;
	unsigned int		 count;
	bool				 nativehandler;
	unsigned int		 serial;
};

#endif
