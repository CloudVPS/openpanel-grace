#ifndef _UDPSOCKET_H
#define _UDPSOCKET_H 1

#include <grace/str.h>
#include <grace/ipaddress.h>

class udpsocket
{
public:
					 udpsocket (void);
					~udpsocket (void);
					
	bool			 bind (int port);
	bool			 bind (ipaddress addr, int port);
	
	string			*receive (int timeout_ms = 0);
	string			*receive (ipaddress &remoteip, int timeout_ms = 0);
	bool			 sendto (ipaddress addr, int port, const string &data);
	
protected:
	int				 sock;
};

#endif
