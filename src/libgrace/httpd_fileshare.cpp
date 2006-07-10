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

// ========================================================================
// CONSTRUCTOR httpdfiletypehandler
// --------------------------------
// Set link to parent object and add ourselves as a filetype handler.
// ========================================================================
httpdfiletypehandler::httpdfiletypehandler (httpdfileshare &pparent,
											const string &filetype)
{
	parent = &pparent;
	parent->addhandler (this, filetype);
}

// ========================================================================
// DESTRUCTOR httpdfiletypehandler
// ========================================================================
httpdfiletypehandler::~httpdfiletypehandler (void)
{
}

// ========================================================================
// METHOD httpdfiletypehandler::run
// --------------------------------
// This is a base class, so send an error event and give out a 500.
// ========================================================================
int httpdfiletypehandler::run (string &path, string &postbody, value &inhdr,
						       string &out, value &outhdr, value &env,
							   tcpsocket &s)
{
	if (parent->parent->eventmask & HTTPD_ERROR)
	{
		value ev;
		string ertxt;
		ertxt.printf ("Unoverloaded filetypehandler invoked for file %S",
					  path.str());
		
		ev("class") = "error";
		ev["ip"] = s.peer_name;
		ev["text"] = ertxt;
		parent->parent->eventhandle (ev);
	}
	return 500;
}

// ========================================================================
// CONSTRUCTOR httpdscriptparser
// -----------------------------
// Load base configuration. Sets up either with the .script extension
// or the extension set in inconfig["scriptextension"].
// ========================================================================
httpdscriptparser::httpdscriptparser (httpdfileshare &pparent, value &inconfig)
	: httpdfiletypehandler (pparent, inconfig.exists ("scriptextension") ?
								inconfig["scriptextension"].cval() :
								"script")
{
	config = inconfig;
}

// ========================================================================
// DESTRUCTOR httpdscriptparser
// ========================================================================
httpdscriptparser::~httpdscriptparser (void)
{
}

// ========================================================================
// METHOD httpdscriptparser::run
// -----------------------------
// Loads the script either from disk or the cache. Parses inbound
// data. Sets up the scriptenv and then runs the script.
// ========================================================================
int httpdscriptparser::run (string &path, string &postbody, value &inhdr,
						    string &out, value &outhdr, value &env,
							tcpsocket &s)
{
	string scripttext;
	value scriptenv;
	
	if (! scriptcache.exists (path))
	{
		scripttext = fs.load (path);
		scriptcache[path].build (scripttext);
	}
	
	if (inhdr["Content-type"] == "application/x-www-form-urlencoded")
	{
		scriptenv = strutil::httpurldecode (postbody);
	}
	else if (config["permitxml"] == true)
	{
		if (inhdr["Content-type"] == "application/xml")
		{
			string schemapath = path;
			schemapath.strcat (".schema.xml");
			if (fs.exists (schemapath))
			{
				xmlschema formSchema (schemapath);
				scriptenv.fromxml (postbody, formSchema);
			}
			else if (config["mandatoryschema"] == false)
			{
				scriptenv.fromxml (postbody);
			}
		}
	}
	
	scriptenv["REMOTE_ADDR"] = s.peer_name;
	scriptenv["REMOTE_USER"] = env["user"];
	scriptenv["REFERRER"] = inhdr["Referer"];
	scriptenv["USER_AGENT"] = inhdr["User-Agent"];
	
	if (config.exists ("defaultvars"))
		scriptenv << config["defaultvars"];
		
	if (env.exists ("scriptenv"))
		scriptenv << env["scriptenv"];
		
	scriptcache[path].run (scriptenv, out, "main");
	
	if (scriptenv.exists ("CONTENT_TYPE"))
		outhdr["Content-type"] = scriptenv["CONTENT_TYPE"];
	else
		outhdr["Content-type"] = "text/html";
	
	return 200;
}

// ========================================================================
// CONSTRUCTOR httpdfileshare
// ----------------------
// Set root property. Figure out if it is a path volume. Set default
// extension-to-mime mappings.
// ========================================================================
httpdfileshare::httpdfileshare (httpd &pparent, const string &purimatch,
								const string &proot)
	: httpdobject (pparent, purimatch)
{
	root = proot;
	if (root.strchr (':') >= 0) roothasvolume = true;
	else roothasvolume = false;

	mimedb["html"] = "text/html";
	mimedb["jpg"] = "image/jpeg";
	mimedb["gif"] = "image/gif";
	mimedb["png"] = "image/png";
	mimedb["xml"] = "application/xml";
	mimedb["txt"] = "text/plain";
	mimedb["gz"] = "application/gzip";
	mimedb["tar"] = "application/tar";
	mimedb["rpm"] = "application/x-rpm";
	mimedb["htm"] = "text/html";
	mimedb["cpp"] = "text/plain";
	mimedb["h"] = "text/plain";
	mimedb["c"] = "text/plain";
}

// ========================================================================
// DESTRUCTOR httpdfileshare
// -------------------------
// Zen
// ========================================================================
httpdfileshare::~httpdfileshare (void)
{
}

// ========================================================================
// METHOD httpdfileshare::run
// --------------------------
// Checks for obvious nastiness. Maps the uri to a path. Opens the file
// and sends it using tcpsocket::sendfile().
// ========================================================================
int httpdfileshare::run (string &uri, string &postbody,
						 value &inhdr, string &out, value &outhdr,
						 value &env, tcpsocket &s)
{
	// Weed out illegal URI elements
	if ((uri.strstr ("//") > 0) || (uri.strstr ("/..") >= 0))
	{
		outhdr["Content-type"] = "text/html";
		out.printf ("<html><body>"
					"<H1>Illegal URI Requested</H1>"
					"</body></html>");

		if (parent->eventmask & HTTPD_ERROR)
		{
			value ev;
			string ertxt;
			ertxt.printf ("Illegal URI %S", uri.str());
			
			ev("class") = "error";
			ev["ip"] = s.peer_name;
			ev["text"] = ertxt;
			parent->eventhandle (ev);
		}
		return 500;
	}
	
	string createdpath; //< String to build up the translated uri.
	string realpath; //< The translated uri with translated pathvolume.
	
	createdpath = root;
	createdpath.printf ("%s%s", uri[0]=='/' ? "" : "/", uri.str());
	realpath = fs.transr (createdpath);

	// If the root path is not a pathvolume, compare it to the final
	// translated path to make sure we're matching with the root.
	// This complete check is redundant.
	if ( (root[0] == '/') && 
	     (!roothasvolume) && 
	     (realpath.strncmp (root, root.strlen()) != 0) )
	{
		outhdr["Content-type"] = "text/html";
		out.printf ("<html><body><H1>Illegal URI Requested</H1>\n"
					"</body></html>");
					
		if (parent->eventmask & HTTPD_ERROR)
		{
			value ev;
			string ertxt;
			ertxt.printf ("Illegal URI %S root=<%s> realpath=<%s>",
						  uri.str(), root.str(), realpath.str());
			
			ev("class") = "error";
			ev["ip"] = s.peer_name;
			ev["text"] = ertxt;
			parent->eventhandle (ev);
		}
		
		return 500;
	}
	
	// No file, no ride.
	if ( (realpath.strlen() == 0) || (! fs.exists (realpath)) )
	{
		// TODO add option for a 404 document
		if (parent->havedefault (404))
		{
			unsigned int bytes;
			s.printf ("HTTP/1.1 404 NOT FOUND\r\n"
					  "Content-type: text/html\r\n");
			
			bytes = parent->sendfile (s, parent->defaultdocument (404));
			env["sentbytes"] = bytes;
			return -404;
		}
		outhdr["Content-type"] = "text/html";
		out.printf ("<html><body><H1>404 Not Found</H1></body></html>\n");
		return 404;
	}
	
	// TODO: change default document behavior
	if (fs.isdir (realpath)) realpath.strcat ("/index.html");
	
	unsigned int outsz;
	
	outsz = fs.size (realpath);
	
	// Figure out the extension
	int extpos = realpath.strchr ('.');
	int nextpos;
	string mimetype = "application/octet-stream"; // default mime-type
	
	// Figured that, try to map a mimetype.
	if (extpos>=0)
	{
		while ((nextpos = realpath.strchr ('.', extpos+1))>0)
			extpos = nextpos;
			
		string ext = realpath.mid (extpos+1);
		
		if (filetypes.exists (ext))
		{
			return filetypes[ext]->run (realpath, postbody, inhdr, out,
										outhdr, env, s);
		}
		
		if (mimedb.exists (ext))
			mimetype = mimedb[ext];
	}
	
	bool keepalive = env["keepalive"].bval();
	
	s.printf ("HTTP/1.1 200 OK\r\n");
	s.printf ("Connection: %s\r\n", keepalive ? "keep-alive" : "close");
	s.printf ("Content-type: %s\r\n", mimetype.str());
	s.printf ("Content-length: %i\r\n\r\n", outsz);
	
	s.sendfile (realpath, outsz);
	env["sentbytes"] = outsz;
	return -200;
}

// ========================================================================
// METHOD httpdfileshare::addhandler
// ---------------------------------
// Adds a handler to the filetypes db for a given extension.
// ========================================================================
void httpdfileshare::addhandler (httpdfiletypehandler *handler,
								 const statstring &forextension)
{
	filetypes[forextension] = handler;
}
