// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

// ========================================================================
// httpd.cpp: Classes implementing a flexible httpd interface
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
		ertxt = errortext::httpd::ftype_base %format (path);
		
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

	if (fs.exists ("/etc/mime.types"))
	{
		file fm;
		string ln;
		value splt;
		
		fm.openread ("/etc/mime.types");
		while (! fm.eof ())
		{
			ln = fm.gets ();
			splt = strutil::splitspace (ln);
			if (splt.count() > 1)
			{
				for (int i=1; i<splt.count(); ++i)
				{
					mimedb[splt[i]] = splt[0];
				}
			}
		}
		fm.close ();
	}

	if (! mimedb.count())
	{
		mimedb = $("html", "text/html") ->
				 $("htm", "text/html") ->
				 $("js", "application/x-javascript") ->
				 $("gif", "image/gif") ->
				 $("jpg", "image/jpeg") ->
				 $("jpeg", "image/jpeg") ->
				 $("xml", "application/xml") ->
				 $("txt", "text/plain") ->
				 $("gz", "application/gzip") ->
				 $("tar", "application/tar") ->
				 $("rpm", "application/x-rpm") ->
				 $("cpp", "text/plain") ->
				 $("css", "text/css") ->
				 $("h", "text/plain") ->
				 $("c", "text/plain");
	}
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
		out = errortext::httpd::html_body %format (errortext::httpd::html_illuri);

		if (parent->eventmask & HTTPD_ERROR)
		{
			string ertxt = errortext::httpd::illuri %format (uri);
			
			parent->eventhandle ($attr("class", "error")->
								 $("ip", s.peer_name) ->
								 $("text", ertxt));
		}
		return 500;
	}
	
	if (uri.strchr ('?') >= 0)
	{
		uri.cropat ('?');
	}
	
	string createdpath; //< String to build up the translated uri.
	string realpath; //< The translated uri with translated pathvolume.
	
	createdpath = "%s%s%s" %format (root, uri[0]=='/'?"":"/", uri);
	realpath = fs.transr (createdpath);

	// If the root path is not a pathvolume, compare it to the final
	// translated path to make sure we're matching with the root.
	// This complete check is redundant.
	if ( (root[0] == '/') && 
	     (!roothasvolume) && 
	     (realpath.strncmp (root, root.strlen()) != 0) )
	{
		outhdr["Content-type"] = "text/html";
		out = errortext::httpd::html_body %format (errortext::httpd::html_illuri);
					
		if (parent->eventmask & HTTPD_ERROR)
		{
			string ertxt;
			ertxt = errortext::httpd::illuri_details %format (uri, root, realpath);

			parent->eventhandle ($attr("class", "error") ->
								 $("ip", s.peer_name) ->
								 $("text", ertxt));
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
			s.puts ("HTTP/1.1 404 NOT FOUND\r\n"
					"Content-type: text/html\r\n");
			
			bytes = parent->sendfile (s, parent->defaultdocument (404));
			env["sentbytes"] = bytes;
			return -404;
		}
		outhdr["Content-type"] = "text/html";
		out = errortext::httpd::html_404;
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

	#define HTTP_F "%a, %e %b %Y %H:%M:%S %Z"

	value vinf = fs.getinfo (realpath);
	timestamp tmodif = vinf["mtime"].uval();
	timestamp tnow = core.time.now ();
	string smodif = tmodif.format (HTTP_F);
	if (inhdr.exists ("If-Modified-Since"))
	{
		if (inhdr["If-Modified-Since"].sval() == smodif)
		{
			s.puts ("HTTP/1.1 304 NOT CHANGED\r\n"
					"Connection: %s\r\n"
					"Content-length: 0\r\n\r\n"
					%format (keepalive ? "keepalive" : "close"));
			
			env["sentbytes"] = 0;
			return -304;
		}
	}
	
	int maxage = (tnow.unixtime() - tmodif.unixtime()) / 2;
	if (maxage < 60) maxage = 60;
	
	timestamp texp = tnow.unixtime () + maxage;
	
	s.puts ("HTTP/1.1 200 OK\r\n"
			"Connection: %s\r\n"
			"Content-type: %s\r\n"
			"Cache-control: max-age=%i\r\n"
			"Date: %s\r\n"
			"Last-Modified: %s\r\n"
			"Expires: %s\r\n"
			"Content-length: %i\r\n\r\n"
			%format (keepalive ? "keepalive" : "close",
					 mimetype,
					 maxage,
					 tnow.format (HTTP_F),
					 smodif,
					 texp.format (HTTP_F),
					 outsz));

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
