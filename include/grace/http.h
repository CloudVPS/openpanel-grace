// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _HTTP_H
#define _HTTP_H 1

#include <grace/tcpsocket.h>
#include <grace/value.h>
#include <grace/str.h>
#include <grace/strutil.h>

$exception (httpMaxChunksizeException, "Max chunk-size exceeded");
$exception (httpTimeoutException, "Timeout in HTTP");

#define HTERR_INVALIDURL	0xbe6ca9fa
#define HTERR_CONNECTFAIL	0x9704f181
#define HTERR_TIMEOUT		0x931cc5a2
#define HTERR_BROKENPIPE	0x91094a03
#define HTERR_PROTO			0x93520a19

/// HTTP client class.
/// Implements the HTTP/1.1 standard POST and GET methods to submit data
/// to a webserver and read the results. When posting, it can either be
/// a properly serialized set of name/value pairs out of a value object,
/// or some arbitrary data with a content-type.
///
/// Example usage (using a GET request):
/// \verbinclude httpsocket_ex1.cpp
class httpsocket
{
public:
				 httpsocket (void);
				~httpsocket (void);
	
				 /// Perform a HTTP post.
				 /// \param url Full url of the resource to post to.
				 /// \param contenttype Mime-type of post data.
				 /// \param body Post data.
				 /// \param hdr Object to store return headers (NULL if none)
				 /// \return Body data.
	string		*post (const string &url, const string &contenttype,
					   const string &body, value *hdr = NULL);
					   
				 /// Perform a HTTP post.
				 /// \param url Full url of the resource to post to.
				 /// \param contenttype Mime-type of post data.
				 /// \param body Post data.
				 /// \param hdr Object to store return headers.
				 /// \return Body data.
	string		*post (const string &url, const string &contenttype,
					   const string &body, value &hdr)
				 {
				 	return post (url, contenttype, body, &hdr);
				 }
				 
				 /// Perform a HTTP post.
				 /// Posts variables using the x-www-urlencoding
				 /// scheme.
				 /// \param url Full url of the resource to post to.
				 /// \param postvar The variables to post.
				 /// \param hdr Object to store return headers (NULL if none)
				 /// \return Body data.
	string		*post (const string &url, const value &postvar,
					   value *headers = NULL);
					   
				 /// Perform a HTTP post.
				 /// Posts variables using the x-www-urlencoding
				 /// scheme.
				 /// \param url Full url of the resource to post to.
				 /// \param postvar The variables to post.
				 /// \param hdr Object to store return headers
				 /// \return Body data.
	string		*post (const string &url, const value postvar, value &hdr)
				 {
				 	return post (url, postvar, &hdr);
				 }
				 
				 /// Perform a HTTP get.
				 /// \param url Full url of the resource to get.
				 /// \param hdr Object to store return headers (NULL if none)
				 /// \return Body data.
	string		*get (const string &url, value *hdr = NULL);
	
				 /// Perform a HTTP get.
				 /// \param url Full url of the resource to get.
				 /// \param hdr Object to store return headers
				 /// \return Body data.
	string		*get (const string &url, value &hdr)
				 {
				 	return get (url, &hdr);
				 }
				 
				 /// Make connections through a proxy server
				 /// \param host Proxy server it's host name
				 /// \param port Proxy port to connect through
	void		 setproxy (const string &host, int port)
				 {
				 	_useproxy 	= true;
				 	_proxyhost	= host;
				 	
				 	// When port is null set to default squid
				 	// port for the linux nerds
				 	_proxyport	= port == 0 ? 3128 : port;
				 }
				 
				 /// Set the value an output header.
				 /// \param name The header name.
				 /// \param value The header value.
	void		 setheader (const statstring &name, const string &value)
				 {
				 	postheaders[name] = value;
				 }

	value		 postheaders; ///< Custom headers for POST/GET.
	string		 error; ///< Error string storage.
	int			 errorcode; ///< Error code.
	int			 status; ///< HTTP status of last request.
	
				 /// Set HTTP keepalive.
	void		 keepalive (bool k=false) { _keepalive=k; }
	
				 /// Disable HTTP keepalive.
	void		 nokeepalive (void) { _keepalive=false; }
	
				 /// Set HTTP basic authentication credentials.
				 /// \param user Username.
				 /// \param pass Password.
	void		 authentication (const string &user, const string &pass);
	
				 /// Access to the underlying tcpsocket.
				 /// \return Reference to the socket.
	tcpsocket	&sock (void) { return _sock; };
	
				 /// Get current timeout value in ms.
	int			 timeout (void) { return _timeout; };
	
				 /// Set current timeout value.
				 /// \param to New timeout in ms, 0 disables.
	void		 timeout (int to) { _timeout = to; };

protected:
				 /// Internal connect method.
				 /// Acts smart by reusing an open tcpsocket if the
				 /// host did not change and keepalive is permitted.
	bool		 connectToHost (const string &hostname, int port);
	bool		 getData (string &into, size_t bytes);
	bool		 getChunked (string &into);
	string		*getResult (value *hdr);

	tcpsocket	 _sock; ///< The tcp socket used for connections.
	string		 _host; ///< The current host.
	int			 _port; ///< The current port.
	bool		 _keepalive; ///< Flag for HTTP keepalive.
	int			 _timeout; ///< Timeout value for non-blocking, 0 for blocking.
	
	bool		 _useproxy;		///< Use a proxy server to connect
	string 		 _proxyhost;	///< Proxy server host name or IP
	int 		 _proxyport;	///< Proxy server port to connect
};

#endif
