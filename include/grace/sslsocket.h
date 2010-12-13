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
					 httpssocket (void);
	
					 /// Return any codec-related errors.
	const string	&codecerror (void);
					 
	void			 nocertcheck (void);
};

#endif
