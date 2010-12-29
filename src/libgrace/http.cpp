// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

// ========================================================================
// http.cpp: GRACE httpsocket class
// ========================================================================

#include <grace/http.h>
#include <grace/defaults.h>
#include <grace/application.h>

// ========================================================================
// CONSTRUCTOR
// -----------
// Defaults to HTTP/1.1 keep-alive. Apache stupidly doesn't close the
// connection in all cases, even if it didn't send a Content-Length header,
// in such cases it may be better to turn it off.
// ========================================================================
httpsocket::httpsocket (void)
{
	_keepalive = false;
	status = 0;
	errorcode = 0;
	_timeout = 0;
	_useproxy = false;
}

// ========================================================================
// DESTRUCTOR
// ----------
// Closes the http socket if there was one open
// ========================================================================
httpsocket::~httpsocket (void)
{
	if (_host.strlen()) _sock.close();
}

// ========================================================================
// METHOD ::post
// -------------
// Encodes the top-level array of a value-object, posting it as name/
// value pairs.
//
// If not NULL, the hdr value object will be filled with all the glorious
// HTTP headers we received back.
//
// An empty object is returned if something failed. The error property
// may be of use in such cases.
// ========================================================================
string *httpsocket::post (const string &url, const value &postvar,
						  value *hdr)
{
	string postbody;
	string encoded;
	int i=0;
	
	foreach (nmval, postvar)
	{
		if (i) postbody.strcat ('&');
		encoded = strutil::urlencode (nmval.sval());
		postbody.strcat ("%s=%s" %format (nmval.id(), encoded));
		++i;
	}
	
	return post (url, "application/x-www-form-urlencoded", postbody, hdr);
}

// ========================================================================
// METHOD ::post
// -------------
// Posts any data with a given content-type.
// ========================================================================
string *httpsocket::post (const string &url, const string &ctype,
						  const string &body, value *hdr)
{
	error.crop (0);
	errorcode = 0;
	string re ("s#http://##");
	status = 0;
	bool unixdomain = false;
	
	if (! url.globcmp ("http://*"))
	{
		if ((_sock.codec) && (url.globcmp ("https://*")))
		{
			re = "s#https://##";
		}
		else if (url.globcmp ("unix://*"))
		{
			re = "s#unix://##";
			unixdomain = true;
		}
		else
		{
			errorcode = HTERR_INVALIDURL;
			error = errortext::http::invalidurl %format (url);
			return NULL;
		}
	}
	string rawuri;
	string hostpart;
	string portpart;
	int port = 80;
	
	rawuri = strutil::regexp (url, re);
	if (rawuri[0] == '[')
	{
		hostpart = rawuri.cutat ("]");
		hostpart = hostpart.mid (1);
		portpart = rawuri.cutat ('/');
		if (portpart.strlen())
		{
			if (portpart[0] == ':')
				port = atoi (portpart.cval() +1);
		}
	}
	else
	{
		hostpart = rawuri.cutat ('/');
		
		if (hostpart.strchr (':') >= 0)
		{
			portpart = hostpart;
			hostpart = portpart.cutat (':');
			port = atoi (portpart);
		}
	}
	
	// If a proxy server was set we where
	// not supposed to open a direct connection
	if (unixdomain)
	{
		if (_host.strlen())
		{
			_sock.close();
			_host.crop(0);
			_port = 0;
		}
		_host = hostpart;
		_keepalive = false;
		
		if (! _sock.uconnect (hostpart))
		{
			errorcode = HTERR_CONNECTFAIL;
			error = errortext::http::connect_usock %format (hostpart);
			return NULL;
		}
	}
	else if (_useproxy)
	{
		if (! connectToHost (_proxyhost, _proxyport))
		{
			errorcode = HTERR_CONNECTFAIL;
			error = errortext::http::connect_proxy %format (hostpart, port);
			return NULL;
		}	
	}
	else
	{
		if (! connectToHost (hostpart, port))
		{
			errorcode = HTERR_CONNECTFAIL;
			error = errortext::http::connect %format (hostpart, port);
			return NULL;
		}
	}
	
	if (postheaders.exists ("Connection")) postheaders.rmval ("Connection");
	if (postheaders.exists ("Content-type")) postheaders.rmval ("Content-type");
	if (postheaders.exists ("Content-length")) postheaders.rmval ("Content-length");
	
	try
	{
		// If we use a proxy we will insert the whole url
		if (! _useproxy)
			_sock.puts ("POST /%s HTTP/1.1\r\n" %format (rawuri));
		else
			_sock.puts ("POST %s HTTP/1.1\r\n" %format (url));

		if (! postheaders.exists ("Host"))
		{
			_sock.puts ("Host: %s\r\n" %format (hostpart));
		}
		
		foreach (hdr, postheaders)
		{
			_sock.puts ("%s: %s\r\n" %format (hdr.id(), hdr));
		}
		
		if (_keepalive)
		{
			_sock.puts ("Connection: keep-alive\r\n");
		}
		else
		{
			_sock.puts ("Connection: close\r\n");
		}
		
		_sock.puts ("Content-type: %s\r\n" %format (ctype));
		_sock.puts ("Content-length: %u\r\n" %format (body.strlen()));
		_sock.puts ("\r\n");
		_sock.puts (body);
		
		return getResult (hdr);
	}
	catch (...)
	{
		return NULL;
	}
}

// ========================================================================
// METHOD ::get
// ------------
// Uses a HTTP/1.1 GET request to collect a resource as a string object.
// ========================================================================
string *httpsocket::get (const string &url, value *hdr)
{
	errorcode = 0;
	error.crop (0);
	string re ("s#http://##");
	status = 0;
	bool unixdomain = false;
	
	if (! url.globcmp ("http://*"))
	{
		if ((_sock.codec) && (url.globcmp ("https://*")))
		{
			re = "s#https://##";
		}
		else if (url.globcmp ("unix://*"))
		{
			re = "s#unix://##";
			unixdomain = true;
		}
		else
		{
			errorcode = HTERR_INVALIDURL;
			error = errortext::http::invalidurl %format (url);
			return NULL;
		}
	}
	string rawuri;
	string hostpart;
	string portpart;
	int port = 80;
	rawuri = strutil::regexp (url, re);
	if (rawuri[0] == '[')
	{
		hostpart = rawuri.cutat ("]");
		hostpart = hostpart.mid (1);
		portpart = rawuri.cutat ('/');
		if (portpart.strlen())
		{
			if (portpart[0] == ':')
				port = atoi (portpart.cval() +1);
		}
	}
	else
	{
		hostpart = rawuri.cutat ('/');
		if (hostpart.strchr (':') >= 0)
		{
			portpart = hostpart;
			hostpart = portpart.cutat (':');
			port = atoi (portpart);
		}
	}
	
	int attempt = 0;
	
tryagain:
	if (unixdomain)
	{
		if (_host.strlen())
		{
			_sock.close();
			_host.crop(0);
			_port = 0;
		}
		_host = hostpart;
		_keepalive = false;
		
		if (! _sock.uconnect (hostpart))
		{
			errorcode = HTERR_CONNECTFAIL;
			error = errortext::http::connect_usock %format (hostpart);
			return NULL;
		}
	}
	else if (_useproxy)
	{
		if (! connectToHost (_proxyhost, _proxyport))
		{
			errorcode = HTERR_CONNECTFAIL;
			error = errortext::http::connect_proxy %format (_proxyhost,_proxyport);
			return NULL;
		}	
	}
	else
	{
		if (! connectToHost (hostpart, port))
		{
			errorcode = HTERR_CONNECTFAIL;
			error = errortext::http::connect %format (hostpart, port);
			return NULL;
		}
	}
	
	while (true)
	{
		if (! _useproxy)
		{
			if (!_sock.puts ("GET /%s HTTP/1.1\r\n" %format (rawuri))) break;
		}
		else
		{
			if (!_sock.puts ("GET %s HTTP/1.1\r\n" %format (url.str()))) break;
		}
	
		if (! postheaders.exists ("Host"))
		{
			if (!_sock.puts ("Host: %s\r\n" %format (hostpart))) break;
		}
		
		// RFC2616 14.3 states that HTTP/1.1 servers may assume that
		// clients not specifying an Accept-Encoding header are capable
		// of handling gzip and compress.
		
		if (! postheaders.exists ("Accept-Encoding"))
		{
			if (! _sock.puts ("Accept-Encoding: \r\n")) break;
		}
		
		foreach (hdr, postheaders)
		{
			if (! _sock.puts ("%s: %s\r\n" %format (hdr.id(), hdr))) break;
		}
		
		if (_keepalive)
		{
			if (! _sock.puts ("Connection: keep-alive\r\n")) break;
		}
		else
		{
			if (! _sock.puts ("Connection: close\r\n")) break;
		}
			
		if (! _sock.puts ("\r\n")) break;
		
		string *r = getResult (hdr);
		if (status !=0) return r;
		delete r;
		break;
	}
	
	++attempt;
	_host.crop ();
	_port = 0;
	_sock.close ();
	if (attempt<2) goto tryagain;
	return NULL;
}

// ========================================================================
// METHOD ::connectToHost
// ----------------------
// Internal method, opens the connection to a given host, or uses a
// cached connection if it is still open.
// ========================================================================
bool httpsocket::connectToHost (const string &hostname, int port)
{
	if ((_keepalive) && (_host == hostname) && (_port == port) && (! _sock.eof()))
	{
		return true;
	}
	if (_host.strlen())
	{
		_sock.close();
		_host.crop(0);
		_port = 0;
	}
	if (_sock.connect (hostname, port))
	{
		_host = hostname;
		_port = port;
		return true;
	}
	return false;
}

// ========================================================================
// METHOD ::getChunked
// -------------------
// Receives chunked data. Returns false if a timeout or other condition
// didn't allow us to get the full object.
// ========================================================================
bool httpsocket::getChunked (string &into)
{
	int chunksz;
	int todo;
	string ln;
	error.crop ();
	errorcode = 0;
	
	if (! _host.strlen()) return false;
	
	try
	{
		do
		{
			if (! _timeout) ln = _sock.gets();
			else
			{
				ln.crop();
				if (! _sock.waitforline (ln, _timeout, 16))
				{
					return false;
				}
			}
			chunksz = ln.toint (16);
			if (chunksz>defaults::lim::httpd::chunksize)
			{
				error = errortext::http::chunksz %format (defaults::lim::httpd::chunksize, chunksz);
				throw httpMaxChunksizeException();
			}
			if (chunksz>0)
			{
				todo = chunksz;
				
				if (! _timeout)
				{
					while (todo > 0)
					{
						ln = _sock.read (todo);
						if (ln.strlen())
						{
							into.strcat (ln);
							todo -= ln.strlen();
						}
						else
						{
							errorcode = HTERR_BROKENPIPE;
							error = errortext::http::connbroken %format (_sock.error());
							return false;
						}
					}
					ln = _sock.gets(); // get rid of trailing crlf;
				}
				else
				{
					while (todo>0)
					{
						ln = _sock.read (chunksz, _timeout);
						if (ln.strlen())
						{
							todo -= ln.strlen();
							into.strcat (ln);
						}
						else
						{
							errorcode = HTERR_TIMEOUT;
							error = errortext::http::timeout;
							return false;
						}
					}
					if (! _sock.waitforline (ln, _timeout, 16))
					{
						errorcode = HTERR_TIMEOUT;
						error = errortext::http::timeout;
						return false;
					}
				}
			}
		} while (chunksz > 0);
		
		return true;
	}
	catch (...)
	{
		// apache seems to prefer hanging up over sending a 0 chunk
		// at least some of the times. So if we triggered no earlier
		// error, see this exception as an end-of-file rather than
		// a failure per se.
		if (! error.strlen())
		{
			errorcode = HTERR_BROKENPIPE;
			error = errortext::http::connbroken %format (_sock.error());
			_sock.close();
			_host.crop (0);
			_port = 0;
			return true;
		}

		_sock.close();
		_host.crop (0);
		_port = 0;
		return false;
	}
}

// ========================================================================
// METHOD ::getData
// ----------------
// Sucks data of a specified size into a string object. If the specified
// size is '0', it will suck till the connection drops.
// ========================================================================
bool httpsocket::getData (string &into, size_t contentLength)
{
	if (! _host.strlen()) return false;
	//::printf ("getdata content-length %i\n", contentLength);
	if (contentLength)
	{
		size_t bytesLeft = contentLength;
		size_t bytesWanted;
		string inbuf;
		
		try
		{
			while (bytesLeft>0)
			{
				bytesWanted = (bytesLeft<4096) ? bytesLeft : 4096;
				
				if (! _timeout) inbuf = _sock.read (bytesWanted);
				else inbuf = _sock.read (bytesWanted, _timeout);
				
				if (inbuf.strlen())
				{
					into.strcat (inbuf);
					bytesLeft -= inbuf.strlen();
				}
				else
				{
					errorcode = HTERR_TIMEOUT;
					error = errortext::http::timeout;
					return false;
				}
			}
		}
		catch (...)
		{
			_sock.close();
			_host.crop(0);
			_port = 0;
		}
		if (bytesLeft>0)
		{
			errorcode = HTERR_BROKENPIPE;
			error = errortext::http::prebroken;
			error = _sock.error ();
			return false;
		}
		return true;
	}
	else
	{
		string inbuf;
		try
		{
			while (1)
			{
				inbuf = _sock.read (4096);
				into.strcat (inbuf);
			}
		}
		catch (...)
		{
			_sock.close();
			_host.crop(0);
			_port = 0;
		}
		return true;
	}
}

// ========================================================================
// METHOD ::getResult
// ------------------
// Parses HTTP/1.1 return headers and collects the body data.
// ========================================================================
string *httpsocket::getResult (value *hdr)
{
	returnclass (string) result retain;
	value *headers;
	bool gotHeaders = false;
	string line;
	value mySplit;
	size_t csz = 0;
	int thestatus = 100;
	
	headers = hdr ? hdr : new value;
	
	try
	{
		while (thestatus == 100)
		{
			if (! _timeout) line = _sock.gets();
			else
			{
				line.crop();
				if (! _sock.waitforline (line, _timeout, 256))
				{
					errorcode = HTERR_TIMEOUT;
					error = errortext::http::timeout;
					status = 0;
					_sock.close();
					_host.crop(0);
					_port = 0;
					delete headers;
					return &result;
				}
			}
			//::printf ("***>%s<***\n", line.str());
			
			mySplit = strutil::split (line, ' ');
		
			thestatus = mySplit[1].ival();
		
			if (thestatus != 100)
				headers->setattrib ("status", mySplit[1].sval());
			else
			{
				_sock.puts ("\r\n");
			}
			
			gotHeaders = false;
			
			while (! gotHeaders)
			{
				line = _sock.gets();
				if (! line.strlen())
				{
					gotHeaders = true;
				}
				else if (thestatus != 100)
				{
					if (headers->count() < 48)
						(*headers) << strutil::parsehdr (line);
		
				//::printf ("***>%s<***\n", line.str());
				}
			}
		}
		
		status = thestatus;
		
		if (headers->exists ("Content-Length"))
			csz = (*headers)["Content-Length"].uval();

		//::printf ("hdr: content-length %i\n", csz);

		if (headers->exists ("Transfer-Encoding"))
		{
			if ((*headers)["Transfer-Encoding"] == "chunked")
			{
				if (! hdr) delete headers;
				
				try
				{
					if (getChunked (result)) return &result;
				}
				catch (...)
				{
					errorcode = HTERR_PROTO;
					error = errortext::http::chunk;
					status = 0;
					_sock.close ();
					_host.crop (0);
				}
				result.crop (0);
				return &result;
			}
		}

		if (! hdr) delete headers;					
		
		if (getData (result, csz))
		{
			return &result;
		}
		result.crop (0);
		return &result;
	}
	catch (...)
	{
		errorcode = HTERR_BROKENPIPE;
		error = errortext::http::prebroken;
		error = _sock.error();
		status = 0;
		_sock.close();
		_host.crop(0);
		_port = 0;
		delete headers;
		return &result;
	}
}

// ========================================================================
// METHOD ::authentication
// -----------------------
// Set username and password for HTTP basic authorization on the socket.
// ========================================================================
void httpsocket::authentication (const string &user, const string &pass)
{
	string authdata;
	string authtoken;
	
	// Create the base64-encoded monster
	authdata = "%s:%s" %format (user, pass);
	authtoken = authdata.encode64();
	authdata = "Basic ";
	authdata.strcat (authtoken);
	
	postheaders["Authorization"] = authdata;
}
