// ========================================================================
// httpd.cpp: Classes implementing a flexible httpd interface
//
// (C) Copyright 2004-2006 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam
// ========================================================================

//      012345678 TAB-O-METER should measure 4
//      ^	^

#include <grace/httpd.h>
#include <grace/value.h>
#include <grace/lock.h>
#include <grace/strutil.h>
#include <grace/filesystem.h>
#include <grace/system.h>
#include <grace/timestamp.h>
#include <grace/xmlschema.h>
#include <grace/case.h>

// ========================================================================
// DESTRUCTOR httpd
// ----------------
// Nothing interesting to do (yet)
// ========================================================================
httpd::~httpd (void)
{
}

// ========================================================================
// METHOD httpd::run
// -----------------
// Implements the main thread. Starts the configured number of httpdworker
// threads, then starts keeping track of the connection load and spawns
// new threads up to a configured maximum if needed. If the load gets low,
// it will ask some threads to end their life the next time they wake up.
// ========================================================================
void httpd::run (void)
{
	int skimcount = 0;
	int tdelay = 0;
	
	// Spawn the worker threads
	for (int i=0; i<minthr; ++i)
		new httpdworker (this);
	
	// Main loop
	while (! _shutdown)
	{
		sleep (tune::httpd::mainthread::idle); // No rush
		int tload;
		
		// Get the current load
		sharedsection (load) { tload = load; }
		
		// Can we still take an extra connection?
		if ( (tload+1) >= workers.count() )
		{
			// No, are we maxed out yet?
			if (workers.count() < maxthr)
			{
				// No, start a new drone
				new httpdworker (this);
				
				// Don't kill this drone any time during the next
				// five rounds.
				tdelay = tune::httpd::wkthread::minrounds;
			}
		}
		else if (! tdelay) // There's room, did we recently spawn?
		{
			// No, let's see if we're carrying extra weight
			if (workers.count() > minthr)
			{
				// Do we carry more than two threads over the load?
				if ((workers.count() - tload) >
								tune::httpd::wkthread::minoverhead);
				{
					// Have we not already asked one of these threads
					// to die as of late?
					if ((workers.count() != skimcount))
					{
						// Ask the first thread to sod off at the
						// next convenient occasion.
						skimcount = workers.count();
						value ev;
						ev["command"] = "die";
						workers[0].sendevent (ev);
						tdelay = 5;
					}
				}
			}
		}
		if (tdelay) tdelay--;
	}
	
	int i;
	for (i=workers.count(); i; --i)
	{
		value ev;
		ev["command"] = "die";
		workers[i-1].sendevent (ev);
	}
	while (workers.count())
	{
		sleep (1);
		workers.gc ();
	}
}

void httpd::shutdown (void)
{
	while (workers.count())
	{
		_shutdown = true;
		sleep (1);
	}
}

// ========================================================================
// METHOD httpd::addobject
// -----------------------
// Normally called from the httpdobject creator, links a httpdobject to
// the end of the chain.
// ========================================================================
void httpd::addobject (httpdobject *obj)
{
	obj->next = NULL;
	if (! first)
	{
		first = obj;
		return;
	}
	
	httpdobject *crsr = first;
	while (crsr->next) crsr = crsr->next;
	crsr->next = obj;
}

// ========================================================================
// METHOD httpd::addeventhandler
// -----------------------------
// Adds a httpdeventhandler to the end of the chain and updates the
// event mask.
// ========================================================================
void httpd::addeventhandler (httpdeventhandler *obj)
{
	obj->next = NULL;
	eventmask |= obj->classmatch;
	
	if (! firsthandler)
	{
		firsthandler = obj;
		return;
	}
	
	httpdeventhandler *crsr = firsthandler;
	while (crsr->next) crsr = crsr->next;
	crsr->next = obj;
}

// ========================================================================
// FUNCTION httpstatusstr
// ----------------------
// Quick utility function to translate a http status code to a terse
// description string as used in the HTTP reply.
// ========================================================================
const char *httpstatusstr (int st)
{
	switch (st)
	{
		case 100: return "CONTINUE";
		case 200: return "OK";
		case 201: return "CREATED";
		case 202: return "ACCEPTED";
		case 203: return "NONAUTH";
		case 204: return "NO CONTENT";
		case 205: return "RESET CONTENT";
		case 206: return "PARTIAL CONTENT";
		case 300: return "MULTIPLE CHOICES";
		case 301: return "MOVED PERM";
		case 302: return "FOUND";
		case 303: return "SEE OTHER";
		case 400: return "BAD REQUEST";
		case 401: return "UNAUTHORIZED";
		case 403: return "FORBIDDEN";
		case 404: return "NOT FOUND";
		case 405: return "METHOD NOT ALLOWED";
		case 406: return "NOT ACCEPTABLE";
		case 413: return "ENTITY TOO LARGE";
		case 414: return "URI TOO LONG";
		case 500: return "INTERNAL ERROR";
		case 501: return "NOT IMPLEMENTED";
		default: return "WHATEVER";
	}
}

// ========================================================================
// METHOD httpd::eventhandle
// -------------------------
// Takes a value object and passes it to the chain of httpdeventhandler
// objects attached earlier. The event object should have an attribute
// named "class" to distinguish between the three possible classes of
// httpd events. This is what they are like:
//
// <event class="access">
//   <string id="method">GET</string>
//   <string id="httpver">1.1</string>
//   <string id="uri">/index.html</string>
//   <string id="file">/web/site1/index.html</string>
//   <string id="ip">10.42.69.5</string>
//   <string id="user"/>
//   <string id="referrer">http://site1/test.html</string>
//   <string id="useragent">Wget/1.8.2</string>
//   <integer id="status">200</integer>
//   <integer id="bytes">194</integer>
// </event>
//
// <event class="error">
//   <string id="ip">10.42.69.5</string>
//   <string id="text">Client tried to authenticate as user root</string>
// </event>
//
// <event class="info">
//   <string id="type">threadstarted</string>
//   <string id="thread">httpdworker/10827</string>
// </event>
//
// <event class="info">
//   <string id="type">connectionaccepted</string>
//   <string id="thread">httpdworker/10827</string>
//   <integer id="load">3</integer>
//   <string id="ip">10.42.69.5</string>
// </event>
//
// The types 'threadstopped' and 'connectionclosed' have the same payload
// as their counterparts listed above.
// ========================================================================
void httpd::eventhandle (value &ev)
{
	httpdeventhandler *hdl;
	int eventclass = 0;
	
	caseselector (ev("class"))
	{
		incaseof ("access") : eventclass = HTTPD_ACCESS; break;
		incaseof ("error") : eventclass = HTTPD_ERROR; break;
		incaseof ("info") : eventclass = HTTPD_INFO; break;
		defaultcase : return;
	}
	
	hdl = firsthandler;
	while (hdl)
	{
		if (hdl->classmatch & eventclass)
		{
			if (hdl->handle (ev)) return;
		}
		hdl = hdl->next;
	}
}

unsigned int httpd::sendfile (tcpsocket &s, const string &fn)
{
	if (! fs.exists (fn)) return 0;
	unsigned int sz = fs.size (fn);
	
	s.printf ("Content-length: %i\r\n\r\n", sz);
	s.sendfile (fn, sz);
	return sz;
}

// ========================================================================
// METHOD httpd::handle
// --------------------
// Takes a httpd request processed by httpdworker and feeds it through
// the chain of httpdobjects. Each object that has an urimatch that
// applies to our uri is called until the first one that returns a
// non-zero status.
//
// When a httpdobject returns a negative status, this indicates that
// the object already took care of the http enchilada. Changes to the
// possibility of keepalive are picked up out of env["keepalive"] and
// the number of bytes transferred should be stored by such an object
// in env["sentbytes"].
//
// On a positive status return, this method picks up any provided
// headers out of the outhdr value object, adds a content-length and
// returns the headers + outbody to the client socket.
// ========================================================================
void httpd::handle (string &uri, string &postbody, value &inhdr,
					const string &method, const string &httpver,
					tcpsocket &s, bool &keepalive)
{
	httpdobject *crsr = first;
	value env;
	value outhdr;
	string outbody;
	
	unprotected (load)
	{
		if ( (tune::httpd::keepalive::trigger * load) > workers.count() )
		keepalive = false;
	}
		
	// Give the objects access to the keepalive status
	env["keepalive"] = keepalive;
	env["method"] = method;
	env["ip"] = s.peer_name;
	env["referrer"] = inhdr["Referrer"];
	
	// Loop over the chain
	while (crsr)
	{
		// Does this one match?
		if (uri.globcmp (crsr->urimatch))
		{
			int res;
			
			// Let it do its magic
			res = crsr->run (uri, postbody, inhdr, outbody, outhdr, env, s);
			
			// Positive non-zero reply?
			if (res > 0)
			{
				// Send the http response, headers and body
				s.printf ("HTTP/1.1 %i %s\r\n", res, httpstatusstr (res));
				outhdr["Content-length"] = outbody.strlen();
				
				foreach (hdr, outhdr)
				{
					s.printf ("%s: %s\r\n", hdr.name(), hdr.cval());
				}
				s.printf ("\r\n");
				s.puts (outbody);
				
				// Create a log-event if needed
				if (eventmask & HTTPD_ACCESS)
				{
					value logdata;
					logdata("class") = "access";
					logdata["method"] = method;
					logdata["httpver"] = httpver;
					logdata["uri"] = uri;
					logdata["file"] = env["file"];
					logdata["ip"] = s.peer_name;
					logdata["user"] = env["user"];
					logdata["referrer"] = inhdr["Referrer"];
					logdata["useragent"] = inhdr["User-Agent"];
					logdata["status"] = res;
					logdata["bytes"] = outbody.strlen();
					
					keepalive = env["keepalive"].bval();
					eventhandle (logdata);
				}
				
				return;
			}
			if (res < 0) // Non-zero negative reply
			{
				// Create a log-event if needed
				if (eventmask & HTTPD_ACCESS)
				{
					value logdata;
					logdata("class") = "access";
					logdata["method"] = method;
					logdata["httpver"] = httpver;
					logdata["uri"] = uri;
					logdata["file"] = env["file"];
					logdata["ip"] = s.peer_name;
					logdata["user"] = env["user"];
					logdata["referrer"] = inhdr["Referer"];
					logdata["useragent"] = inhdr["User-Agent"];
					logdata["status"] = -res;
					logdata["bytes"] = env["sentbytes"].ival();
					
					eventhandle (logdata);
				}
				
				// Retain the keepalive status
				keepalive = env["keepalive"].bval();
				return;
			}
		}
		crsr = crsr->next;
	}
	
	// Tough luck, fall back to ugliness
	s.printf ("HTTP/1.1 404 NOT FOUND\r\n");
	
	int fbytes;
	
	if (havedefault (404))
	{
		s.printf ("Content-type: text/html\r\n");
		fbytes = sendfile (s, defaultdocument (404));
	}
	else
	{
		s.printf ("Content-length: 36\r\n");
		s.printf ("Content-type: text/html\r\n\r\n");
		s.printf ("<html><h1>404 Not Found</h1></html>\n");
		fbytes = 36;
	}
	
	if (eventmask & HTTPD_ACCESS)
	{
		value logdata;
		logdata("class") = "access";
		logdata["method"] = method;
		logdata["httpver"] = httpver;
		logdata["uri"] = uri;
		logdata["file"] = "";
		logdata["ip"] = s.peer_name;
		logdata["user"] = "";
		logdata["referrer"] = inhdr["Referer"];
		logdata["useragent"] = inhdr["User-Agent"];
		logdata["status"] = 404;
		logdata["bytes"] = fbytes;
		
		eventhandle (logdata);
	}
	
	keepalive = false;
}

// ========================================================================
// METHOD httpdworker::run
// -----------------------
// Accepts a connection, scoops the HTTP request, any headers and
// any posted body data.
// ========================================================================
void httpdworker::run (void)
{
	tcpsocket s;
	value ev;
	string line;
	string threadid;
	bool run = true;
	
	threadid.printf ("httpdworker/%i", tid);
	
	// If anyone cares, shout out that we're alive
	if (parent->eventmask & HTTPD_INFO)
	{
		value ev;
		ev("class") = "info";
		ev["type"] = "threadstarted";
		ev["thread"] = threadid;
		parent->eventhandle (ev);
	}
	
	// As long as we weren't asked to die
	while (run)
	{
		while (! parent->tcplock.trylockw(5))
		{
			// Anyone calling us?
			ev = nextevent();
			if (ev.count())
			{
				if (ev["command"] == "die") // time to go
				{
					run = false;
					if (parent->eventmask & HTTPD_INFO)
					{
						value ev;
						ev("class") = "info";
						ev["type"] = "threadstopped";
						ev["thread"] = threadid;
						parent->eventhandle (ev);
					}
					return;
				}
			}
		}
		
		while (! s)
		{
			s = parent->listener.tryaccept(2.0);
			if (!s)
			{
				ev = nextevent();
				if (ev.count())
				{
					if (ev["command"] == "die") // time to go
					{
						run = false;
						parent->tcplock.unlock();
						if (parent->eventmask & HTTPD_INFO)
						{
							value ev;
							ev("class") = "info";
							ev["type"] = "threadstopped";
							ev["thread"] = threadid;
							parent->eventhandle (ev);
						}
						return;
					}
				}
			}
		}
		parent->tcplock.unlock();
		
		// Handle the parent httpd's load
		int nload;
		
		exclusiveaccess (parent->load) { nload = parent->load.o++; }

		// Should we tell anyone?
		if (parent->eventmask & HTTPD_INFO)
		{
			value ev;
			ev("class") = "info";
			ev["type"] = "connectionaccepted";
			ev["thread"] = threadid;
			ev["load"] = nload;
			ev["ip"] = s.peer_name;
			parent->eventhandle (ev);
		}
		
		bool keepalive = true;
		
		try
		{
			// Startup state, wait for a http command
			// and read headers. Will keep coming back to
			// the top until the connection is gone or
			// somewhere we decided to ditch keepalive.
			
			while ( (! s.eof()) && (keepalive) )
			{
				bool gotCommand = false;
				string httpCommand;
				value httpHeaders;
				string bodyData;
				
				while (! s.eof())
				{
					line = s.gets();
					
					// Empty line, our turn
					if (gotCommand && (! line.strlen())) break;
					
					// Ok, but did we get anything useful?
					if (line.strlen())
					{
						// The first line, this is the http command
						if (! gotCommand)
						{
							gotCommand = true;
							httpCommand = line;
						}
						else // No, these are headers. Munge.
						{
							if (httpHeaders.count() < 48)
								httpHeaders << strutil::parsehdr (line);
						}
					}
				}
				
				// If we got nothing useful, totally drop out of
				// the session.
				if (! gotCommand) throw (httpdWorkerException());
				if (s.eof()) throw (httpdWorkerException());
				
				// Now we'll start interpreting the http comand
				string cmd;
				string uri;
				
				cmd = httpCommand.cutat (' ');
				cmd.ctoupper();
				
				// If it was a post, get the post body.
				if (cmd.strcasecmp ("post") == 0)
				{
					size_t sz = httpHeaders["Content-length"].uval();
					
					// It's not over size, is it?
					if (sz > (unsigned int) parent->maxpostsize())
					{
						// tis? Whine!
						s.printf ("HTTP/1.1 413 ENTITY TOO LARGE\r\n");
						s.printf ("Content-type: text/html\r\n\r\n");
						s.printf (errortext::httpd::html_body,
								  errortext::httpd::html_413);
						keepalive = false;
						
						// Whine upstream if needed
						if (parent->eventmask & HTTPD_ERROR)
						{
							value ev;
							string ertxt;
							ertxt.printf (errortext::httpd::toolarge, sz);
							
							ev("class") = "error";
							ev["ip"] = s.peer_name;
							ev["text"] = ertxt;
							parent->eventhandle (ev);
						}
						break;
					}
					
					// If we're here, it means we may suck data
					if (sz)
					{
						bodyData = s.read (sz);
					}
				}
				else if (cmd.strcasecmp ("get") == 0)
				{
					if (httpHeaders.exists ("Content-length"))
					{
						if (httpHeaders["Content-length"].ival() > 0)
						{
							s.printf ("HTTP/1.1 400 BAD REQUEST\r\n");
							s.printf ("Content-type: text/html\r\n\r\n");
							s.printf ("<html><h1>GET request with POST body");
							s.printf (" not allowed</h1></html>\n");
							keepalive = false;
							
							if (parent->eventmask & HTTPD_ERROR)
							{
								value ev;
								ev("class") = "error";
								ev["ip"] = s.peer_name;
								ev["text"] = errortext::httpd::getbody;
								parent->eventhandle (ev);
							}
							break;
						}
					}
				}
				
				// Get the URI part. Compensate for http/0.9's lack of
				// well-formedness.
				if (httpCommand.strchr (' ') < 0)
				{
					uri = httpCommand;
					httpCommand.crop (0);
				}
				else uri = httpCommand.cutat (' ');
				
				// Set the default keepalive scheme for the protocol
				// version.
				if (httpCommand.strcasecmp ("http/1.1") == 0)
					keepalive = true;
				else
					keepalive = false;
				
				// The httpCommand now hosts the actual version string
				// like '1.0' or '1.1'.
				delete httpCommand.cutat ('/');
				
				// No version? Assume 0.9.
				if (! httpCommand.strlen())
				{
					httpCommand = "0.9";
				}
				
				// Change all this explicitly if the client asked
				// for a differnet scheme using the Connection header
				if (httpHeaders["Connection"] == "close")
					keepalive = false;
				else if (httpHeaders["Connection"] == "keepalive")
					keepalive = true;
				
				// As of now, we only recognize get and post requests
				if ((cmd.strcasecmp ("post") == 0) ||
				    (cmd.strcasecmp ("get") == 0))
				{
					parent->handle (uri, bodyData, httpHeaders,
									cmd, httpCommand,
									s, keepalive);
				}
				else // The rest gets the 500 EFINGER
				{
					s.printf ("HTTP/1.1 500 UNKNOWN METHOD '%S'\r\n", cmd.str());
					s.printf ("Content-type: text/html\r\n\r\n");
					s.printf (errortext::httpd::html_body,
							  errortext::httpd::html_500_method);
					keepalive = false;
					
					// Tell the world if it cares
					if (parent->eventmask & HTTPD_ERROR)
					{
						value ev;
						string ertxt;
						ertxt.printf (errortext::httpd::method, cmd.str());
						
						ev("class") = "error";
						ev["ip"] = s.peer_name;
						ev["text"] = ertxt;
						parent->eventhandle (ev);
					}
				}
			}
		}
		catch (...)
		{
		}
		
		s.close();
		
		// Downgrade to DEFCON foo-1
		exclusiveaccess (parent->load) { nload = parent->load.o--; }

		// Inform those who want to be informed
		if (parent->eventmask & HTTPD_INFO)
		{
			value ev;
			ev("class") = "info";
			ev["type"] = "connectionclosed";
			ev["thread"] = threadid;
			ev["load"] = nload;
			ev["ip"] = s.peer_name;
			parent->eventhandle (ev);
		}
	}
}

// ========================================================================
// CONSTRUCTOR httpdobject
// -------------------
// Couldn't stick this in the .h or I'd be in class dependency hell.
// ========================================================================
httpdobject::httpdobject (httpd &pparent, const string &purimatch)
{
	next = NULL;
	parent = &pparent;
	urimatch = purimatch;
	pparent.addobject (this);
}

// ========================================================================
// DESTRUCTOR httpdobject
// ----------------------
// Bo-ring!
// ========================================================================
httpdobject::~httpdobject (void)
{
}

// ========================================================================
// METHOD httpdobject::run
// -----------------------
// Someone forgot to derive a class!
// ========================================================================
int httpdobject::run (string &uri, string &postbody,
					  value &inhdr, string &out, value &outhdr,
					  value &env, tcpsocket &s)
{
	return 404;
}

// ========================================================================
// CONSTRUCTOR httpdbasicauth
// ----------------------
// Initialize the user database. Pass it with a boolean value of 'crypt'
// to enable md5/des crypts for the passwords.
// ========================================================================
httpdbasicauth::httpdbasicauth (httpd &pparent, const string &purimatch,
								const string &authrealm, httpdauthenticator &a)
	: httpdobject (pparent, purimatch)
{
	auth = &a;
	realm = authrealm;
}

// ========================================================================
// DESTRUCTOR httpdbasicauth
// -------------------------
// This space for rent
// ========================================================================
httpdbasicauth::~httpdbasicauth (void)
{
}

// ========================================================================
// METHOD httpdbasicauth::run
// --------------------------
// This method will return 0 on succesful authentication. The authenticated
// user will be stuck in env["user"]. If it fails, the return value
// will be 401 with the WWW-Authenticate header set.
// ========================================================================
int httpdbasicauth::run (string &uri, string &postbody,
						 value &inhdr, string &out, value &outhdr,
						 value &env, tcpsocket &s)
{
	// Format the WWW-Authenticate header
	string authhdr;
	authhdr.printf ("Basic realm=\"%S\"", realm.str());
	outhdr["WWW-Authenticate"] = authhdr;
	
	// No authorization header sent, at all. Educate the client.
	if (! inhdr.exists ("Authorization"))
	{
		outhdr["Content-type"] = "text/html";
		out.printf ("<html><body>"
					"<H1>Please Authenticate</H1>"
					"</body></html>\n");
		return 401;
	}
	
	// This is where we parse the authorization header
	string authStringEnc;
	string authString;
	string username;
	string password;
	
	authStringEnc = inhdr["Authorization"].sval();
	
	if (authStringEnc.strchr (' ') >0)
		delete authStringEnc.cutat (' ');
	
	authString = authStringEnc.decode64();
	
	username = authString.cutat (':');
	if (! username.strlen())
	{
		// No ':' means only a username provided
		username = authString;
	}
	else
	{
		password = authString;
	}
	
	// Find the user and check the password
	
	bool authenticated = auth->authenticate (username, password, uri);
	
	if (! authenticated) // No dough, whine and return 401
	{
		if (redirurl && (username || password))
		{
			outhdr["Location"] = redirurl;
			outhdr["Content-type"] = "text.html";
			out.printf ("<html><body><a href=\"%s\">", redirurl.str());
			out.printf ("Redirected</a></body></html>\n");
			return 200;
		}
		
		outhdr["Content-type"] = "text/html";
		
		if (parent->havedefault (401))
		{
			out = fs.load (parent->defaultdocument (401));
		}
		else
		{
			out.printf (errortext::httpd::html_body,
						errortext::httpd::html_401);
		}
		
		// Whine to some log object if there's any that care
		if (username.strlen() && (parent->eventmask & HTTPD_ERROR))
		{
			value outev;
			string errtxt;
			
			errtxt.printf (errortext::httpd::authfail,
						   username.str(), realm.str(), uri.str());
			outev("class") = "error";
			outev["ip"] = s.peer_name;
			outev["text"] = errtxt;
			parent->eventhandle (outev);
		}
		return 401;
	}
	
	env["user"] = username;
	
	return 0;
}

// ========================================================================
// METHOD httpdbasicauth::redirectto
// ========================================================================
void httpdbasicauth::redirectto (const string &uri)
{
	redirurl = uri;
}

// ========================================================================
// CONSTRUCTOR httpdeventhandler
// -----------------------------
// Sets the classmatch, adds the object to the httpd chain.
// ========================================================================
httpdeventhandler::httpdeventhandler (httpd &pparent,
									  httpdeventclass inclasses)
{
	classmatch = inclasses;
	next = NULL;
	pparent.addeventhandler (this);
	parent = &pparent;
}

// ========================================================================
// METHOD httpdeventhandler::handle
// --------------------------------
// I guess Someone forgot to override a virtual method. Now Someone will
// be facing a default method. I guess Someone will feel stupid.
// ========================================================================
int httpdeventhandler::handle (value &ev)
{
	return 0;
}

// ========================================================================
// METHOD httpdlogger::handle
// --------------------------
// Writes access events to an apache-style accesslog. Writes error events
// to an apache-style errorlog. Life is simple.
// ========================================================================
int httpdlogger::handle (value &ev)
{
	timestamp ti = kernel.time.now();
	string timestr = ti.format ("%d/%b/%Y:%H:%M:%S %z");
	string uri = ev["uri"];
	if (uri.strchr ('?') >= 0) delete uri.cutafter ('?');

	if (ev ("class") == "access")
	{
		string remuser = ev["user"];
		if (! remuser.strlen()) remuser = "-";
		
		// We'll be called from any thread, lock the file access.
		exclusivesection (faccess)
		{
			faccess.printf ("%s - %S [%s] \"%s %S HTTP/%S\" "
							"%i %i \"%S\" \"%S\"\n",
							ev["ip"].cval(),
							remuser.str(),
							timestr.str(),
							ev["method"].cval(),
							uri.str(),
							ev["httpver"].cval(),
							ev["status"].ival(),
							ev["bytes"].ival(),
							ev["referrer"].cval(),
							ev["useragent"].cval());
		}
	}
	else if (haserrorlog)
	{
		// We'll be called from any thread, lock the file access.
		exclusivesection (ferror)
		{
			ferror.printf ("[%s] [error] [client %s] %s\n",
						   timestr.str(),
						   ev["ip"].cval(),
						   ev["text"].cval());
		}
	}
	return 1;
}

// ========================================================================
// CONSTRUCTOR httpdrewrite
// ------------------------
// Boring. Constructs base class and copies regexp parameter.
// ========================================================================
httpdrewrite::httpdrewrite (httpd &pparent, const string &purimatch,
							const string &pregexp)
	: httpdobject (pparent, purimatch)
{
	rule = pregexp;
}

// ========================================================================
// DESTRUCTOR httpdrewrite
// -----------------------
// Yawn!
// ========================================================================
httpdrewrite::~httpdrewrite (void)
{
}

// ========================================================================
// METHOD httpdrewrite::run
// ------------------------
// Does a fairly straightforward regular expression parse on the uri
// and stores it right back.
// ========================================================================
int httpdrewrite::run (string &uri, string &postbody, value &inhdr,
					   string &out, value &outhdr, value &env,
					   tcpsocket &s)
{
	uri = strutil::regexp (uri, rule);
	return 0;
}

// ========================================================================
// CONSTRUCTOR httpdvhost
// ----------------------
// Loads the vhost database.
// ========================================================================
httpdvhost::httpdvhost (httpd &pparent, const value &phostdb)
	: httpdobject (pparent, "*")
{
	hostdb = phostdb;
}

// ========================================================================
// DESTRUCTOR httpdvhost
// ---------------------
// blink.
// ========================================================================
httpdvhost::~httpdvhost (void)
{
}

// ========================================================================
// METHOD httpdvhost::run
// ----------------------
// Reads the 'Host' header. Cuts off any port designation, then looks
// it up in hostdb. If there is no match it will look up an entry
// with key "*". Prepends the string therein to the uri. So the following:
//
// GET /favicon.ico HTTP/1.0
// Host: www.foo.com
//
// Would have its uri rewritten from "/favicon.ico" to the path
// "/foo.com/favicon.ico" provided that hostdb["www.foo.com"] exists and
// contains the string "foo.com".
// ========================================================================
int httpdvhost::run (string &uri, string &postbody, value &inhdr,
					 string &out, value &outhdr, value &env,
					 tcpsocket &s)
{
	int colonpos;
	string host = inhdr["Host"];
	
	// Strip off port designation
	colonpos = host.strchr (':');
	if (colonpos >= 0) host = host.left (colonpos);
	
	// No usable host header, assume default
	if (! host.strlen()) host = "*";
	
	// Host doesn't exist? Assume default.
	if (! hostdb.exists (host))
		host = "*";
	
	// If it exists now, use it for the rewrite
	if (hostdb.exists (host))
	{
		string newuri;
		newuri = "/";
		newuri.strcat (hostdb[host].sval());
		if (uri[0] != '/') newuri.strcat ('/');
		newuri.strcat (uri);
		uri = newuri;
		env["host"] = host;
		return 0; // Return 0, let the rest of the chain figure it out
	}
	
	// Ok we really couldn't find anything useful to do, return an error.
	outhdr["Content-type"] = "text/html";
	if (parent->havedefault (404))
	{
		out = fs.load (parent->defaultdocument (404));
	}
	else
	{
		out.printf (errortext::httpd::html_404_vhost, host.str());
	}

	// Also whine to the errorlog, if it's there.
	if (parent->eventmask & HTTPD_ERROR)
	{
		value outev;
		string errtxt;
		
		errtxt.printf (errortext::httpd::novhost, host.str());
		outev("class") = "error";
		outev["ip"] = s.peer_name;
		outev["text"] = errtxt;
		parent->eventhandle (outev);
	}
	
	// We'll file this under case number 404: "The Lost Case"
	return 404;
}

// ========================================================================
// CONSTRUCTOR httpdauthenticator
// ========================================================================
httpdauthenticator::httpdauthenticator (void)
{
}

// ========================================================================
// DESTRUCTOR httpdauthenticator
// ========================================================================
httpdauthenticator::~httpdauthenticator (void)
{
}

// ========================================================================
// METHOD ::authenticate
// ========================================================================
bool httpdauthenticator::authenticate (const string &u, const string &p,
									   const string &uri)
{
	return false;
}

// ========================================================================
// METHOD ::getuser
// ========================================================================
value *httpdauthenticator::getuser (const string &u)
{
	returnclass (value) result retain;
	return &result;
}

// ========================================================================
// CONSTRUCTOR valueauth
// ========================================================================
valueauth::valueauth (const value &db) : httpdauthenticator ()
{
	userdb = db;
}

// ========================================================================
// DESTRUCTOR valueauth
// ========================================================================
valueauth::~valueauth (void)
{
}

// ========================================================================
// METHOD ::authenticate
// ========================================================================
bool valueauth::authenticate (const string &username, const string &passwd,
							  const string &uri)
{
	bool checkedcrypted = false;
	bool authenticated = false;
	
	if (userdb ("crypt") == true)
	{
		checkedcrypted = true;
		authenticated = kernel.pwcrypt.verify (passwd,
											   userdb[username].sval());
	}
	else
	{
		if (passwd == userdb[username].sval())
			authenticated = true;
	}
	
	return authenticated;
}

// ========================================================================
// METHOD ::getuser
// ========================================================================
value *valueauth::getuser (const string &username)
{
	returnclass (value) res retain;

	if (userdb.exists (username))
	{
		res = userdb[username].attributes();
	}
	return &res;
}

// ========================================================================
// CONSTRUCTOR pwfileauth
// ========================================================================
pwfileauth::pwfileauth (const string &fname) : httpdauthenticator ()
{
	filename = fname;
	lastmod = 0;
	lastcheck = 0;
}

pwfileauth::pwfileauth (const string &fname,
						const string &fieldOne) : httpdauthenticator ()
{
	filename = fname;
	lastmod = 0;
	lastcheck = 0;
	fieldnames[0] = fieldOne;
}

pwfileauth::pwfileauth (const string &fname,
						const string &fieldOne,
						const string &fieldTwo) : httpdauthenticator ()
{
	filename = fname;
	lastmod = 0;
	lastcheck = 0;
	fieldnames.newval() = fieldOne;
	fieldnames.newval() = fieldTwo;
}

pwfileauth::pwfileauth (const string &fname,
						const string &fieldOne,
						const string &fieldTwo,
						const string &fieldThree) : httpdauthenticator ()
{
	filename = fname;
	lastmod = 0;
	lastcheck = 0;
	fieldnames.newval() = fieldOne;
	fieldnames.newval() = fieldTwo;
	fieldnames.newval() = fieldThree;
}

pwfileauth::pwfileauth (const string &fname,
						const string &fieldOne,
						const string &fieldTwo,
						const string &fieldThree,
						const string &fieldFour) : httpdauthenticator ()
{
	filename = fname;
	lastmod = 0;
	lastcheck = 0;
	fieldnames.newval() = fieldOne;
	fieldnames.newval() = fieldTwo;
	fieldnames.newval() = fieldThree;
	fieldnames.newval() = fieldFour;
}

pwfileauth::pwfileauth (const string &fname,
						const string &fieldOne,
						const string &fieldTwo,
						const string &fieldThree,
						const string &fieldFour,
						const string &fieldFive) : httpdauthenticator ()
{
	filename = fname;
	lastmod = 0;
	lastcheck = 0;
	fieldnames.newval() = fieldOne;
	fieldnames.newval() = fieldTwo;
	fieldnames.newval() = fieldThree;
	fieldnames.newval() = fieldFour;
	fieldnames.newval() = fieldFive;
}

pwfileauth::pwfileauth (const string &fname,
						const string &fieldOne,
						const string &fieldTwo,
						const string &fieldThree,
						const string &fieldFour,
						const string &fieldFive,
						const string &fieldSix) : httpdauthenticator ()
{
	filename = fname;
	lastmod = 0;
	lastcheck = 0;
	fieldnames.newval() = fieldOne;
	fieldnames.newval() = fieldTwo;
	fieldnames.newval() = fieldThree;
	fieldnames.newval() = fieldFour;
	fieldnames.newval() = fieldFive;
	fieldnames.newval() = fieldSix;
}

// ========================================================================
// DESTRUCTOR valueauth
// ========================================================================
pwfileauth::~pwfileauth (void)
{
}

// ========================================================================
// METHOD ::checkrecord
// ========================================================================
void pwfileauth::checkrecord (void)
{
	time_t ti = kernel.time.now();
	value v;
	if ((ti - lastcheck) > 10)
	{
		v = fs.getinfo (filename);
		if (v["mtime"].uval() > (unsigned int) lastmod)
		{
			loadfile ();
			lastmod = v["mtime"].uval();
		}
		lastcheck = ti;
	}
}

// ========================================================================
// METHOD ::loadfile
// ========================================================================
void pwfileauth::loadfile (void)
{
	file f;
	string line;
	string username;
	string pwhash;
	value splt;
	int i;
	
	lck.lockw();
	userdb.clear();
	
	if (! f.openread (filename)) return;
	try
	{
		while (! f.eof())
		{
			line = f.gets();
			splt = strutil::split (line, ':');
			username = splt[0];
			pwhash = splt[1];
			
			userdb[username]["passwd"] = pwhash;
			for (i=0; i<fieldnames.count(); ++i)
			{
				userdb[-1][fieldnames[i].sval()] = splt[2+i];
			}
		}
	}
	catch (...)
	{
	}
	f.close ();
	lck.unlock();
}

// ========================================================================
// METHOD ::authenticate
// ========================================================================
bool pwfileauth::authenticate (const string &u, const string &pw,
							   const string &uri)
{
	bool res;
	checkrecord ();
	lck.lockr ();
	if (! userdb.exists (u))
	{
		lck.unlock();
		return false;
	}
	res = kernel.pwcrypt.verify (pw, userdb[u]["passwd"].sval());
	lck.unlock();
	
	return res;
}

// ========================================================================
// METHOD ::getuser
// ========================================================================
value *pwfileauth::getuser (const string &u)
{
	returnclass (value) res retain;

	checkrecord ();
	lck.lockr ();
	if (userdb.exists (u))
	{
		// FIXME: thread bullshit this should not be necessary
		foreach (e, userdb[u])
		{
			res[e.name()] = e.cval();
		}
	}
	lck.unlock ();
	return &res;
}

// ========================================================================
// COSNTRUCTOR serverpage
// ========================================================================
serverpage::serverpage (httpd &pparent, const string &uri)
	: httpdobject (pparent, uri)
{
}

// ========================================================================
// DESTRUCTOR serverpage
// ========================================================================
serverpage::~serverpage (void)
{
}

// ========================================================================
// METHOD ::run
// ========================================================================
int serverpage::run (string &uri, string &postbody, value &inhdr,
					 string &out, value &outhdr, value &env, tcpsocket &s)
{
	value reqenv;
	int res;
	
	if (env["method"] == "GET")
	{
		string reqdata;
		int pos;
		if ((pos = uri.strchr ('?')) >= 0)
		{
			reqdata = uri.mid (pos+1);
			reqenv = strutil::httpurldecode (reqdata);
		}
	}
	else if (inhdr["Content-type"] == "application/x-www-form-urlencoded")
	{
		reqenv = strutil::httpurldecode (postbody);
	}

	outhdr["Content-type"] = "text/html";

	res = execute (env, reqenv, out, outhdr);
	if (res>0) return res;
	
	out.crop();
	out.printf (errortext::httpd::html_body, errortext::httpd::html_spage);
	
	return 500;
}

// ========================================================================
// METHOD ::execute
// ========================================================================
int serverpage::execute (value &env, value &argv, string &out, value &outhdr)
{
	// Useless method in base class.
	return 0;
}
