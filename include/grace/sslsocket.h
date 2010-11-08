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

#include <matrixssl/matrixSsl.h>

extern void setupMatrixSSL (void);

/// Exception values emitted by sslclientcodec
enum sslException
{
	EX_SSL_BUFFER_SNAFU		= 0xf95877be, ///< Buffer management error.
	EX_SSL_PROTOCOL_ERROR	= 0xa3ecd69e, ///< SSL protocol related error.
	EX_SSL_CLIENT_ALERT		= 0xe6950536, ///< MatrixSSL client alert.
	EX_SSL_NO_HANDSHAKE		= 0xcc30b80b, ///< Action after failed handshake.
	EX_SSL_INIT 			= 0x9fcc10c0  ///< Could not init SSL.
};

/// An iocodec implementing SSL client traffic through MatrixSSL.
/// Used by sslsocket as the iocodec of choice.
class sslclientcodec : public iocodec
{
public:
					 sslclientcodec (void);
					~sslclientcodec (void);
	
					 /// Initiates the SSL handshake.
	bool			 setup (void);
	
					 /// Reset all buffers and handshake data.
	void			 reset (void);
	
					 /// Adds SSL-coded data to the input buffer.
	bool			 addinput (const char *, size_t);
	
					 /// Adds plain data to the output buffer for SSL coding.
	bool			 addoutput (const char *, size_t);
	
					 /// Adds an SSL close event to the output buffer.
	void			 addclose (void);
	
					 /// Loads decoded data into a ringbuffer.
	bool			 fetchinput (ringbuffer &);
	
					 /// Copies currently encoded output data to a string.
	void			 peekoutput (string &);
	
					 /// Used to report the number of coded output bytes
					 /// that have been succesfully flushed.
	void			 doneoutput (unsigned int);
	
					 /// Determines if there is enough room in the output
					 /// buffer for a given number of bytes.
	bool			 canoutput (unsigned int);
	
					 /// If called, certificate checking will be disabled.
	void			 nocertcheck (void);
	
protected:
	ssl_t			*ssl;
	sslBuf_t		 inbuf;
	sslBuf_t		 insock;
	sslBuf_t		 outsock;
	sslSessionId_t	 session;
	bool			 disablecerts;
	
	bool			 handshakedone;
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

/// Implementation of httpsocket with SSL support.
/// A lightweight wrapper that sets up a httpsocket with an
/// sslclientcodec. The base class is aware of URLs in the
/// https namespace if the tcpsocket's codec has been set.
class httpssocket : public httpsocket
{
public:
					 /// Constructor.
					 httpssocket (void)
					 	: httpsocket ()
					 {
					 	_sock.codec = new sslclientcodec;
					 }
	
					 /// Return any codec-related errors.
	const string	&codecerror (void)
					 {
					 	return _sock.codec->error();
					 }
					 
	void			 nocertcheck (void)
					 {
					 	_sock.codec->nocertcheck();
					 }
};

#endif
