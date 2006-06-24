// ========================================================================
// http.cpp: GRACE httpsocket class
//
// (C) Copyright 2004-2006 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam 
// ========================================================================

#include <grace/http.h>
#include <grace/defaults.h>

// ========================================================================
// CONSTRUCTOR
// -----------
// Defaults to HTTP/1.1 keep-alive. Apache stupidly doesn't close the
// connection in all cases, even if it didn't send a Content-Length header,
// in such cases it may be better to turn it off.
// ========================================================================
httpsocket::httpsocket (void)
{
	_keepalive = true;
	status = 0;
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
		postbody.printf ("%s=%s", nmval.name(), encoded.str());
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
	string re ("s#http://##");
	status = 0;
	
	if (! url.globcmp ("http://*"))
	{
		if ((_sock.codec) && (url.globcmp ("https://*")))
		{
			re = "s#https://##";
		}
		else
		{
			error.crop(0);
			error.printf ("%s is not a valid url", url.str());
			return NULL;
		}
	}
	string rawuri;
	string hostpart;
	string portpart;
	int port = 80;
	
	rawuri = strutil::regexp (url, re);
	hostpart = rawuri.cutat ('/');
		
	if (hostpart.strchr (':') >= 0)
	{
		portpart = hostpart;
		hostpart = portpart.cutat (':');
		port = atoi (portpart);
	}
	
	// If a proxy server was set we where
	// not supposed to open a direct connection
	if (_useproxy)
	{
		if (! connectToHost (_proxyhost, _proxyport))
		{
			error.crop(0);
			error.printf ("Could not connect to proxy at address: '%s' port %i",
					hostpart.str(), port);
			return NULL;
		}	
	}
	else
	{
		if (! connectToHost (hostpart, port))
		{
			error.crop(0);
			error.printf ("Could not connect to '%s' port %i",
					hostpart.str(), port);
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
			_sock.printf ("POST /%s HTTP/1.1\r\n", rawuri.str());
		else
			_sock.printf ("POST %s HTTP/1.1\r\n", url.str());

		if (! postheaders.exists ("Host"))
		{
			_sock.printf ("Host: %s\r\n", hostpart.str());
		}
		
		foreach (hdr, postheaders)
		{
			_sock.printf ("%s: %s\r\n", hdr.name(), hdr.cval());
		}
		if (_keepalive)
		{
			_sock.printf ("Connection: keep-alive\r\n");
		}
		else
		{
			_sock.printf ("Connection: close\r\n");
		}
		
		_sock.printf ("Content-type: %s\r\n", ctype.str());
		_sock.printf ("Content-length: %u\r\n", body.strlen());
		_sock.printf ("\r\n");
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
	error.crop (0);
	string re ("s#http://##");
	status = 0;
	
	if (! url.globcmp ("http://*"))
	{
		if ((_sock.codec) && (url.globcmp ("https://*")))
		{
			re = "s#https://##";
		}
		else
		{
			error.printf ("%s is not a valid url", url.str());
			return NULL;
		}
	}
	string rawuri;
	string hostpart;
	string portpart;
	int port = 80;
	rawuri = strutil::regexp (url, re);
	hostpart = rawuri.cutat ('/');
	if (hostpart.strchr (':') >= 0)
	{
		portpart = hostpart;
		hostpart = portpart.cutat (':');
		port = atoi (portpart);
	}
	
		// If a proxy server was set we where
	// not supposed to open a direct connection
	if (_useproxy)
	{
		if (! connectToHost (_proxyhost, _proxyport))
		{
			error.printf ("Could not connect to proxy at address: '%s' port %i",
					hostpart.str(), port);
			return NULL;
		}	
	}
	else
	{
		if (! connectToHost (hostpart, port))
		{
			error = "Connection failed";
			return NULL;
		}
	}
	
	try
	{
		if (! _useproxy)
			_sock.printf ("GET /%s HTTP/1.1\r\n", rawuri.str());
		else
			_sock.printf ("GET %s HTTP/1.1\r\n", url.str());
	
		if (! postheaders.exists ("Host"))
		{
			_sock.printf ("Host: %s\r\n", hostpart.str());
		}
		
		if (! postheaders.exists ("Accept-Encoding"))
		{
			_sock.printf ("Accept-Encoding: \r\n");
		}
		
		foreach (hdr, postheaders)
		{
			_sock.printf ("%s: %s\r\n", hdr.name(), hdr.cval());
		}
		
		if (_keepalive)
			_sock.printf ("Connection: keep-alive\r\n");
		else
			_sock.printf ("Connection: close\r\n");
		_sock.printf ("\r\n");
		
		return getResult (hdr);
	}
	catch (...)
	{
		return NULL;
	}
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
			if (chunksz>DEFAULT_LIM_HTTP_CHUNKSIZE)
			{
				error = "Max chunksize ";
				error.printf ("%i < %i", DEFAULT_LIM_HTTP_CHUNKSIZE, chunksz);
				throw (EX_HTTP_MAX_CHUNKSIZE);
			}
			if (chunksz>0)
			{
				todo = chunksz;
				
				if (! _timeout)
				{
					ln = _sock.read (chunksz);
					if (ln.strlen())
					{
						into.strcat (ln);
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
							error = "Timeout";
							return false;
						}
					}
					if (! _sock.waitforline (ln, _timeout, 16))
					{
						error = "Timeout";
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
			error = "Connection broken ";
			error.printf ("(%s)", _sock.error().str());
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
					error = "Connection timed out";
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
			error = "Premature end of connection";
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
					error = "Connection timed out getting headers";
					status = 0;
					_sock.close();
					_host.crop(0);
					_port = 0;
					delete headers;
					return &result;
				}
			}
			
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
				}
			}
		}
		
		status = thestatus;
		
		if (headers->exists ("Content-Length"))
			csz = (*headers)["Content-Length"].uval();

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
					error = "Error getting chunked data";
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
		error = "Connection failure while getting headers";
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
	authdata.printf ("%s:%s", user.cval(), pass.cval());
	authtoken = authdata.encode64();
	authdata = "Basic ";
	authdata.strcat (authtoken);
	
	postheaders["Authorization"] = authdata;
}
