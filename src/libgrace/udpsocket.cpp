// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#include <grace/udpsocket.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

$exception (udpSocketException, "Could not create UDP socket");

// ==========================================================================
// CONSTRUCTOR udpsocket
// ==========================================================================
udpsocket::udpsocket (void)
{
	boundport = 0;
	sock = socket (AF_INET, SOCK_DGRAM, 0);
	if (sock<0) throw (udpSocketException());
}

// ==========================================================================
// DESTRUCTOR udpsocket
// ==========================================================================
udpsocket::~udpsocket (void)
{
	if (sock >= 0) close (sock);
}

// ==========================================================================
// METHOD udpsocket::bind
// ==========================================================================
bool udpsocket::bind (int port)
{
	struct sockaddr_in local_addr;

	local_addr.sin_addr.s_addr = INADDR_ANY;
	local_addr.sin_port = htons (port);
	local_addr.sin_family = AF_INET;

	if (::bind (sock, (struct sockaddr *) &local_addr, sizeof (local_addr)))
	{
		return false;
	}
	
	if (port) boundport = port;
	else
	{
		socklen_t slen = sizeof (local_addr);
		if (! getsockname (sock, (struct sockaddr *) &local_addr, &slen))
		{
			boundport = local_addr.sin_port;
		}
		else
		{
			boundport = 0;
		}
	}
	
	return true;
}

bool udpsocket::bind (ipaddress addr, int port)
{
	struct sockaddr_in local_addr;

	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl (addr);
	local_addr.sin_port = htons (port);

	if (::bind (sock, (struct sockaddr *) &local_addr, sizeof (local_addr)))
	{
		return false;
	}
	
	if (port) boundport = port;
	else
	{
		socklen_t slen = sizeof (local_addr);
		if (! getsockname (sock, (struct sockaddr *) &local_addr, &slen))
		{
			boundport = ntohs (local_addr.sin_port);
		}
		else
		{
			boundport = 0;
		}
	}
	
	return true;
}

// ==========================================================================
// METHOD udpsocket::setbroadcast
// ==========================================================================
void udpsocket::setbroadcast (bool to)
{
	int pram = 1;
	setsockopt (sock, SOL_SOCKET, SO_BROADCAST, (char *) &pram, sizeof (int));
}

// ==========================================================================
// METHOD udpsocket::sendto
// ==========================================================================
bool udpsocket::sendto (ipaddress addr, int port, const string &data)
{
	struct sockaddr_in remote_addr;
	remote_addr.sin_addr.s_addr = htonl (addr);
	remote_addr.sin_port = htons (port);
	remote_addr.sin_family = AF_INET;
	
	if (::sendto (sock, data.str(), data.strlen(), 0,
				(struct sockaddr *) &remote_addr, sizeof (remote_addr)))
	{
		return false;
	}
	
	return true;
}

// ==========================================================================
// METHOD udpsocket::receive
// ==========================================================================
string *udpsocket::receive (int timeout_ms)
{
	returnclass (string) res retain;
	
	char buf[2048];
	int sz;
	
	if (timeout_ms>=0)
	{
		fd_set fds;
		struct timeval tv;
		tv.tv_sec = timeout_ms/1000;
		tv.tv_usec = (timeout_ms % 1000) * 1000;
		
		FD_ZERO (&fds);
		FD_SET (sock, &fds);
		
		if (select (sock+1, &fds, NULL, NULL, &tv) < 1) return &res;
	}
	
	sz = recv (sock, buf, 2048, 0);
	if (sz>0) res.strcpy (buf, sz);
	return &res;
}

string *udpsocket::receive (ipaddress &addr, int timeout_ms)
{
	returnclass (string) res retain;
	struct sockaddr_in remote_addr;
	char buf[2048];
	int sz;
	socklen_t addrsz = sizeof (remote_addr);
	
	if (timeout_ms>=0)
	{
		fd_set fds;
		struct timeval tv;
		tv.tv_sec = timeout_ms/1000;
		tv.tv_usec = (timeout_ms % 1000) * 1000;
		
		FD_ZERO (&fds);
		FD_SET (sock, &fds);
		
		if (select (sock+1, &fds, NULL, NULL, &tv) < 1)
		{
			addr = 0;
			return &res;
		}
	}
	
	sz = recvfrom (sock, buf, 2048, 0, (struct sockaddr *) &remote_addr,
				   &addrsz);
	
	addr = ntohl (remote_addr.sin_addr.s_addr);
	if (sz>0) res.strcpy (buf, sz);
	return &res;
}
