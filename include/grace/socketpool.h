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
					 }
					 poolsocket (const poolsocket &s)
					 {
					 	serial = s.serial;
					 	sock = s.sock;
					 	connected = s.connected;
					 	when = kernel.time.now();
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
	int					 count;
	bool				 nativehandler;
	unsigned int		 serial;
};

#endif
