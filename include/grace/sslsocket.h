// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _SSLSOCKET_H
#define _SSLSOCKET_H 1

#include <grace/tcpsocket.h>
#include <grace/str.h>
#include <grace/file.h>
#include <grace/http.h>


/// Exception values emitted by sslcodec
enum sslException
{
	EX_SSL_BUFFER_SNAFU		= 0xf95877be, ///< Buffer management error.
	EX_SSL_PROTOCOL_ERROR	= 0xa3ecd69e, ///< SSL protocol related error.
	EX_SSL_CLIENT_ALERT		= 0xe6950536, ///< MatrixSSL client alert.
	EX_SSL_NO_HANDSHAKE		= 0xcc30b80b, ///< Action after failed handshake.
	EX_SSL_INIT 			= 0x9fcc10c0  ///< Could not init SSL.
};


/// Implementation of tcpsocket with SSL support.
/// Uses sslclientcodec to encapsulate with MatrixSSL.
class sslsocket : public tcpsocket
{
public:
					 sslsocket (void);
					~sslsocket (void);

					 /// Copy operator (useful for listener::accept).
					 /// \param orig Original socket. It will be deleted
					 ///             after the codec and other associated
					 ///             structures have been copied.
	sslsocket		&operator= (sslsocket *orig);
	
					 /// Return any codec-related errors.
	const string	&codecerror (void)
					 {
					 	return codec->error();
					 }
					 
	void			 nocertcheck (void)
					 {
					 	codec->nocertcheck ();
					 }
};

/// Listening TCP socket.
/// Opens a socket that listens for tcp connections on a configured
/// port. Acts as a factory for connected tcpsocket objects.
class ssllistener : public tcplistener
{
public:
				 ssllistener() : keys(0), allowpassthrough(false) {}

	void		 loadkeyfile( const string& cert, const string& priv = "" );
	void		 loadkeystring( const string& cert, const string& priv = "" );
	
	void		 setallowpassthrough( bool newvalue ) {allowpassthrough=newvalue;}
	bool		 getallowpassthrough() const { return allowpassthrough;}

				 /// Wait for a new connection.
				 /// \return Pointer to a new tcpsocket bound to the connection.
				 /// \throw socketCreateAcception Error creating a BSD socket.
	virtual tcpsocket	*accept (void);
	
				 /// Wait for a new connection.
				 /// \param timeout Timeout in seconds.
				 /// \return Pointer to a new tcpsocket bound to the connection,
				 ///         or NULL when it failed.
	virtual tcpsocket *tryaccept (double timeout);
private:
	void      	*keys;
	bool		allowpassthrough;
};


/// Implementation of httpsocket with SSL support.
/// A lightweight wrapper that sets up a httpsocket with an
/// sslclientcodec. The base class is aware of URLs in the
/// https namespace if the tcpsocket's codec has been set.
class httpssocket : public httpsocket
{
public:
					 /// Constructor.
					 httpssocket (void);
	
					 /// Return any codec-related errors.
	const string	&codecerror (void);
					 
	void			 nocertcheck (void);
};

#endif
