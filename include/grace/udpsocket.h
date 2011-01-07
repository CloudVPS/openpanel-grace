// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _UDPSOCKET_H
#define _UDPSOCKET_H 1

#include <grace/str.h>
#include <grace/ipaddress.h>

$exception (udpAddressFamilyException, "Unknown address family");

/// Class for sending and receiving data using UDP.
class udpsocket
{
public:
					 /// Constructor. Sets up a SOCK_DGRAM socket on
					 /// sock.
					 udpsocket (void);
					 
					 /// Destructor. Closes the socket.
					~udpsocket (void);
					
					 /// Bind to a port on INADDR_ANY.
					 /// \param port The UDP port.
	bool			 bind (int port);
	
					 /// Bind to a specific port/ip.
					 /// \param addr The IP address.
					 /// \param port The UDP port.
	bool			 bind (ipaddress addr, int port);
	
					 /// Receive a single packet.
					 /// \param timeout_ms Optional timeout in milliseconds,
					 ///                   if nothing is received you end
					 ///                   up with an empty string result.
	string			*receive (int timeout_ms = -1);
	
					 /// Receive a single packet with remote host information.
					 /// \param remoteip The ip address of the sending
					 ///                 host will be written to this reference.
					 /// \param timeout_ms Timeout in milliseconds.
	string			*receive (ipaddress &remoteip, int timeout_ms = -1);
	
					 /// Send a single packet.
					 /// \param addr IPv4 address of host to send to.
					 /// \param port UDP port to send to.
					 /// \param data Data to send (should fit in a packet).
	bool			 sendto (ipaddress addr, int port, const string &data);
	
					 /// Indicate that we will want to send to a broadcast
					 /// address.
	void			 setbroadcast (bool to = true);
	
					 /// Get the bound port if none was given at binding
					 /// time. 
	int				 getport (void) { return boundport; }
	
protected:
	int				 sock; ///< The actual datagram socket.
	int				 boundport; ///< The port we bound to if we gave none.
};

#endif
