// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _TCPSOCKET_H
#define _TCPSOCKET_H 1

#include <grace/file.h>
#include <grace/str.h>
#include <grace/lock.h>
#include <grace/system.h>
#include <grace/ipaddress.h>

$exception (socketException, "Generic socket exception");
$exception (socketCreateException, "Could not create socket");

/// A tcp connection class.
/// Implements blocking and non-blocking interaction with another host
/// over an inbound or outbound tcp connection.
///
/// Usage example:
/// \verbinclude tcpsocket_ex1.cpp
class tcpsocket : public file
{
public:
				 /// Constructor.
				 tcpsocket (void);
				 
				 /// Destructor.
				~tcpsocket (void);
	
				 /// Connect to an IPv4 host by name.
				 /// Returns true if connection succeeded.
				 /// \param host Hostname or dotted quad to connect to.
				 /// \param hport TCP port to use.
				 /// \return Status, \b true if connection succeeded.
				 /// \throw socketCreateException Error creating a BSD socket.
				 /// \throw EX_SSL_INIT Error initializing sslclientcodec.
	bool		 connect (const string &host, int hport);
	
				 /// Connect to an IPv4 host by name.
	bool		 connect (const char *host, int port)
				 {
				 	return connect ((string) host, port);
				 }
	
				 /// Connect to an IPv4 host by address.
				 /// Returns true if connection succeeded.
				 /// \param host Hostname or dotted quad to connect to.
				 /// \param hport TCP port to use.
				 /// \return Status, \b true if connection succeeded.
				 /// \throw socketCreateException Error creating a BSD socket.
				 /// \throw EX_SSL_INIT Error initializing sslclientcodec.
	bool		 connect (ipaddress addr, int hport);
	
				 /// Connect to a Unix Domain socket.
				 /// \return Status, \b true if connection succeeded.
				 /// \throw socketCreateException Error creating a BSD socket.
				 /// \throw EX_SSL_INIT Error initializing sslclientcodec.
	bool		 uconnect (const string &);

				 /// Set address to connect from when using connect( .. )
				 /// First use this before connecting
				 /// \param address Address to bind
	bool		 bindtoaddr (ipaddress);
	
				 /// Get peer credentials (for unix sockets on some OSes).
	void		 getcredentials (void);
	
				 /// Send peer credentials (for unix sockets on some OSes).
	void		 sendcredentials (void);
	
				 /// Receive a file descriptor.
				 /// This uses special features of Unix Domain sockets
				 /// that may not be available on all platforms. The
				 /// program on the other side of the socket can use
				 /// file::sendfd() to initiate this transfer.
				 /// \return Pointer to new file object bound to the descriptor.
	file		*getfd (void);
	
				 /// Hand over a file descriptor.
				 /// Uses special features in the implementation of
				 /// Unix Domain sockets that allows file descriptors
				 /// to be passed from one process to another. This
				 /// feature is only available for the Linux kernel
				 /// family.
				 /// \param fl The file to share with the other process.
	void		 sendfd (file &fl);
	
				 /// Use sendfile to send a disk file.
				 /// Some kernels have a sendfile system call that
				 /// can be used to instruct the kernel to send the
				 /// contents of a disk file directly to a socket,
				 /// bypassing expensive context-switches.
				 /// \param path Absolute path of the file.
				 /// \param sz Number of bytes to send.
	void		 sendfile (const string &path, unsigned int sz);
	
				 /// Derive from other tcpsocket.
				 /// \param orig The original socket.
				 /// \return Reference to self.
	tcpsocket	&operator= (tcpsocket &orig);
	
				 /// Derive from other tcpsocket. Deletes original.
				 /// \param orig Pointer to the original object.
				 /// \return Reference to self.
	tcpsocket	&operator= (tcpsocket *orig);
	
	pid_t		 peer_pid; ///< Peer pid credentials.
	uid_t		 peer_uid; ///< Peer uid credentials.
	gid_t		 peer_gid; ///< Peer gid credentials.
	ipaddress	 peer_addr; ///< IPv4 address of peer.
	string		 peer_name; ///< String-annotated address of peer.
	int			 peer_port; ///< TCP port for peer.
	int			 local_port; ///< Local TCP port
	time_t		 ti_established; ///< Connection time.
	ipaddress 	 localbindaddr;		  ///< Address to bind.

protected:
				 /// Backend implementation of all derivation methods.
				 /// Flushes the internal buffer. Takes over the other
				 /// object's filno and eof status. Copies the original's
				 /// buffer contents. Then gets rid of the original.
				 /// \param s Pointer to the original object.
	void		 derive (tcpsocket *s);

				 /// Backend implementation of all derivation methods.
				 /// Flushes the internal buffer. Takes over the other
				 /// object's filno and eof status. Copies the original's
				 /// buffer contents. 
				 /// \param s Reference to the original object.
	void		 derive (tcpsocket &s);
};

/// Base class for tcplistener implementations.
class listenerbase
{
public:
						 listenerbase (void);
	virtual				~listenerbase (void);
	virtual tcpsocket	*accept (void);
	virtual tcpsocket	*tryaccept (double timeout);
};

/// Listening TCP socket.
/// Opens a socket that listens for tcp connections on a configured
/// port. Acts as a factory for connected tcpsocket objects.
class tcplistener : public listenerbase
{
public:
				 /// Constructor.
				 /// \param port TCP port to listen to.
				 /// \throw socketCreateException Error creating a BSD socket.
				 tcplistener (int port);
				 
				 /// Constructor.
				 /// \param path Path for the Unix Domain socket.
				 /// \throw socketException Error creating a BSD socket.
				 tcplistener (const string &path);

				 /// Delayed initialization constructor.
				 tcplistener (void);
				 
				 /// Constructor.
				 /// Delayed initialization constructor. You must
				 /// excplicitly call one of the listento() methods
				 /// before going for accept().
				~tcplistener (void);

				 /// Start listening on a TCP port.
				 /// \param port The TCP port.
				 /// \throw socketCreateException Error creating a BSD socket.
	void		 listento (int port);
	
				 /// Start listening on a TCP port at a specific address.
				 /// \param addr The listening address.
				 /// \param port The listening tcp port.
				 /// \throw socketCreateException Error creating a BSD socket.
	void		 listento (ipaddress addr, int port);
	
				 /// Start listening on a Unix socket.
				 /// \param path The Unix path.
				 /// \throw socketCreateException Error creating a BSD socket.
	void		 listento (const string &path);
	
				 /// Wait for a new connection.
				 /// \return Pointer to a new tcpsocket bound to the connection.
				 /// \throw socketCreateAcception Error creating a BSD socket.
	virtual tcpsocket *accept (void);
	
				 /// Wait for a new connection.
				 /// \param timeout Timeout in seconds.
				 /// \return Pointer to a new tcpsocket bound to the connection,
				 ///         or NULL when it failed.
	virtual tcpsocket *tryaccept (double timeout);

protected:
	bool		 listening; ///< True if the socket is listening.
	bool		 tcpdomain; ///< True if we're AF_INET/tcp
	int			 tcpdomainport; ///< Listen port for tcp.
	ipaddress	 bindaddress; ///< Listen address (0 for INADDR_ANY)
	string		 unixdomainpath; ///< Listen path for AF_UNIX.
	lock<int>	 sock; ///< Lock to allow for cross-thread non blocking.
};

#endif
