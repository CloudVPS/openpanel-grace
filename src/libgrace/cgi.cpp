// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

// ========================================================================
// cgi.cpp: CGI Application Model Object
// ========================================================================

#include <grace/system.h>
#include <grace/cgi.h>
#include <grace/strutil.h>
#include <grace/defaults.h>

// ========================================================================
// CONSTRUCTOR
// -----------
// Creates a cgi instance as an inherited application object
// ========================================================================
cgi::cgi (const char *appname) : application (appname)
{
	// --------------------------------------------------------------------
	// If this is a GET request, parse the QUERY_STRING environment
	// variable.
	// --------------------------------------------------------------------
	
	if (! env.exists ("DOCUMENT_ROOT"))
	{
		fs.pathvol["docroot"].newval() = fs.pwd();
	}
	else
	{
		string troot;
		
		troot = env["DOCUMENT_ROOT"];
		fs.pathvol["docroot"].newval() = troot;
		fs.pathvol["conf"].newval() = troot;
		troot.strcat ("/");
		troot.strcat (creator);
		fs.pathvol["conf"].newval() = troot;
	}
	
	if (env["REQUEST_METHOD"] == "GET")
	{
		string buf;
		buf = env["QUERY_STRING"];
		
		value pairs;
		value pair;
		string pval;

		pairs = strutil::split (buf, '&');
		foreach (namevalue, pairs)
		{
			string vnam;
			
			pair = strutil::split (namevalue, '=');
			vnam = pair[0];
			
			// FIXME this could be fucked by case matching
			
			if ((vnam=="DOCUMENT_ROOT")||(vnam=="CONTENT_TYPE")||
				(vnam=="REQUEST_METHOD")||(vnam=="REMOTE_ADDR"))
			{
				vnam.strcat ("_");
			}
			
			if (pair.count() == 2)
			{
				env[vnam.str()] = strutil::urldecode (pair[1]);
			}
			else
			{
				env[vnam.str()] = 1;
			}
		}
	}
	
	// --------------------------------------------------------------------
	// Else, if it's a POST request, handle the data transfer
	// --------------------------------------------------------------------
	
	else if (env["REQUEST_METHOD"] == "POST")
	{
		value v;
		value inhdr;
		string buf;
		
		// ----------------------------------------------------------------
		// Read all headers
		// ----------------------------------------------------------------

		v["Content-type"] = env["CONTENT_TYPE"];
		v["Content-length"] = env["CONTENT_LENGTH"];
		
		// ----------------------------------------------------------------
		// Parse the body, if needed
		// ----------------------------------------------------------------
		
		int clen = v["Content-length"];
		
		if (clen > (defaults::lim::cgi::postsize))
		{
			headers["Content-type"] = "text/html";
			buffer.printf (errortext::cgi::errbody, errortext::cgi::size);
			
			sendpage();
			throw cgiEndOfFileException();
		}
		
		if (clen) // There's data to be read
		{
			try
			{
				buf = fin.read (clen);
			}
			catch (...) // End-of-File
			{
				headers["Content-type"] = "text/html";
				buffer.printf (errortext::cgi::errbody, errortext::cgi::eof);
				
				sendpage();
				throw cgiEndOfFileException();
			}

			// Parse the content-type to see how the information is
			// being posted. We currently support multipart/form-data
			// (necessary to allow binary uploads) and the regular
			// url-encoded format. It might make sense to also allow
			// XML formats in cases where we are not serving webpages.
						
			string ctype;
			int tc;
			ctype = v["Content-type"];
			
			tc = ctype.strchr (';');
			if (tc>0) ctype = ctype.left (tc);
			
			// The multipart/form-data content type is normally used
			// when a file is uploaded, although any HTML form can
			// request it from the browser.
			
			if (ctype == "multipart/form-data")
			{
				string parseme;
				value vattrib;
				
				// I'm not entirely proud of this but parsehdr
				// already does everything so nicely
				parseme.printf ("Content-type: %s",
								v["Content-type"].cval());
				vattrib = strutil::parsehdr (parseme);
				
				string bound;
				string bsep;
				string endmark;
				int endpos;
				
				// Find the MIME boundary
				bound = vattrib["Content-type"]("boundary");
				if (! bound.strlen())
				{
					// This is where we complain for improper
					// MIME data
					headers["Content-type"] = "text/html";
					buffer.printf (errortext::cgi::errbody, errortext::cgi::bound);
					sendpage ();
					throw cgiPostFormatException();
				}
				
				buf = buf.mid (bound.strlen()+2);
				endmark.printf ("\r\n--%s--\r\n", bound.str());
				if ((endpos = buf.strstr (endmark)) >= 0)
					buf.crop (endpos);
				
				bsep.printf ("\r\n--%s\r\n", bound.str());
				
				value parts;
				parts = strutil::split (buf, bsep);
				parts.newval() = bsep;
				
				foreach (part,parts)
				{
					value prt;
					
					prt = strutil::parsemime (part.sval());
					if (prt["Content-Disposition"] == "form-data")
					{
						string name;
						string fname;
						name = prt["Content-Disposition"]("name");
						fname = prt["Content-Disposition"]("filename");
						if (! name.strlen()) name = "0";
						
						if (env.exists (name))
						{
							string tname;
							tname.printf ("%s.array", name.str());
							if (! env.exists (tname))
							{
								env[tname].newval() = env[name];
								env[tname].type("array");
							}
							env[tname].newval() = prt[".data"];
						}
						
						env[name] = prt[".data"];
						env[name].setattrib ("content-type",
										     prt["Content-Type"].sval());
						if (fname.strlen())
							env[name].setattrib ("filename", fname);
					}
				}
			}
			
			// XML post
			
			else if (ctype == "application/xml")
			{
				string parseme; // a little hack to retro-parse extra
				value vattrib;  // parameters from the content-type
				string inschema;
				
				parseme.printf ("Content-type: %s",
								v["Content-type"].cval());
				vattrib = strutil::parsehdr (parseme);
				
				// All this to see if we have a schema attribute
				inschema = vattrib["Content-type"]("schema");
				if (inschema.strlen())
				{
				/* FIXME
					if (! schemas.exists (inschema))
					{
						headers["Content-Type"] = "text/html";
						buffer.printf ("<html><body bgcolor=white>"
									   "<h2>Error</h2>\n"
					   				   "Invalid schema"
					   				   "</body></html>\n");
					    sendpage();
					    exit (1);
					}
					
					env.fromxml (buf, schemas[(statstring) inschema]);
				*/
				}
				else
				{
					env.fromxml (buf);
				}
			}
			else // assuming regular HTTP POST content
			{
				value pairs;
				value pair;
				string pval;
				
				pairs = strutil::split (buf, '&');
				foreach (namevalue, pairs)
				{
					string vnam;
					pair = strutil::split (namevalue, '=');
					vnam = pair[0];
					
					// FIXME same problem with case
					
					if ((vnam=="DOCUMENT_ROOT")||(vnam=="CONTENT_TYPE")||
					    (vnam=="REQUEST_METHOD")||(vnam=="REMOTE_ADDR"))
						vnam.strcat ("_");
			
					
					if (pair.count() == 2)
					{
						if (env.exists (vnam.str()))
						{
							string tnam;
							tnam.printf ("%s.array", vnam.str());
							if (! env.exists (tnam))
							{
								env[tnam].newval() = env[vnam];
								env[tnam].type("array");
							}
							env[vnam] = strutil::urldecode (pair[1]);
							env[tnam].newval() = env[vnam];
						}
						else
						{
							env[vnam] = strutil::urldecode (pair[1]);
						}
					}
					else
					{
						env[vnam.str()] = 1;
					}
				}
			}
		} // (nelc) fi
	}

	// --------------------------------------------------------------------
	// Ok, neither POST nor GET. Let's assume <form method=bogus>
	// --------------------------------------------------------------------
	
	else
	{
		headers["Content-Type"] = "text/html";
		buffer.printf ("<html><body bgcolor=white><h2>Error</h2>\n"
					   "This program requires to be run as a CGI"
					   "</body></html>\n");
		
		sendpage();
		exit (1);
	}
}

// ========================================================================
// DESTRUCTOR
// ----------
// A no-brainer, no dynamic structures allocated.
// ========================================================================
cgi::~cgi (void)
{
}

// ========================================================================
// METHOD ::main
// -------------
// Should be overloaded from this base class. Print out an error.
// ========================================================================
int cgi::main (void)
{
	ferr.printf ("unoverloaded cgi::main!\n");
	return 1;
}

void cgi::addschema (const statstring &name, const string &filename)
{
	string fn;
	if (filename.strlen()) fn = filename;
	else fn.printf ("schema:%s", name.str());
	
	/* schemas[name].load (fn); */
}

// ========================================================================
// METHOD ::sendpage
// -----------------
// Takes the content that the overloaded main() method has put into the
// string object 'buffer', calculate the length and print all the headers
// as well as the content.
// ========================================================================
void cgi::sendpage (void)
{
	headers["Content-length"] = (int) buffer.strlen();
	foreach (header, headers)
	{
		fout.printf ("%s: %s\r\n", header.name(), header.cval());
	}
	fout.printf ("\r\n");
	fout.puts (buffer);
}

// ========================================================================
// CONSTRUCTOR
// -----------
// Nothing needs to be done here yet
// ========================================================================
cgitemplate::cgitemplate (void)
{
}

// ========================================================================
// DESTRUCTOR
// ----------
// Nothing needs to be done here yet
// ========================================================================
cgitemplate::~cgitemplate (void)
{
}

// ========================================================================
// METHOD ::load
// -------------
// Attempts to open the file <fnam>, read the contents and split it into
// named sections separated by @section statements.
// ========================================================================
bool cgitemplate::load (const string &fnam)
{
	string tfil;
	
	tfil = fs.load (fnam, filesystem::optional);
	if (! tfil.strlen()) return false;
	
	bool done = false;
	
	int  sstart = 0;
	int  send = 0;
	
	while (! done)
	{
		sstart = tfil.strstr ("@section", send);
		if (sstart >= 0)
		{
			send = tfil.strstr ("@section", sstart+1);
			if (send<0) send = tfil.strlen();
			
			int nl = tfil.strstr ("\n", sstart);
			if (nl>0)
			{
				string sectionname;
				
				sectionname = tfil.mid (sstart + 9, nl - (sstart + 9));
				tmpl[sectionname] = tfil.mid (nl+1, send - (nl+1));
			}
		}
		else done = true;
	}
	
	return true;
}

// ========================================================================
// METHOD ::parse
// --------------
// Takes a specific section of the loaded template file and parses it with
// the environment passed as "env". The parsed page is returned as a
// string object.
// ========================================================================
string *cgitemplate::parse (const string &section, value &env)
{
	returnclass (string) res retain;
	
	if (! tmpl.exists(section))
	{
		string errstr;
		errstr.printf (errortext::cgi::section, section.str());
		
		res.printf (errortext::cgi::errbody, errstr.str());
		return &res;
	}
	
	value split;
	split = strutil::split (tmpl[section], '$');

	int j = 0;

	foreach (e, split)
	{
		if (j & 1)
		{
			res += env[e.cval()];
		}
		else
		{
			res += e.sval();
		}
	}
	
	return &res;
}

// ========================================================================
// CONSTRUCTOR
// -----------
// Creates a cgi instance as an inherited application object
// ========================================================================
rpccgi::rpccgi (const char *appname) : application (appname)
{
	if (env["REQUEST_METHOD"] == "POST")
	{
		value v;
		value inhdr;
		string buf;
		
		// ----------------------------------------------------------------
		// Read all headers
		// ----------------------------------------------------------------
		
		v["Content-length"] = env["CONTENT_LENGTH"];
		v["Content-type"] = env["CONTENT_TYPE"];
		
		// ----------------------------------------------------------------
		// Parse the body, if needed
		// ----------------------------------------------------------------
		
		unsigned int clen = v["Content-length"];
		
		if (clen) // There's data to be read
		{
			try
			{
				buf = fin.read (clen);
				while (buf.strlen() < clen)
				{
					buf.strcat (fin.read (clen - buf.strlen()));
				}
			}
			catch (...) // End-of-File
			{
				headers["Content-type"] = "text/html";
				buffer.printf (errortext::cgi::errbody, errortext::cgi::eof);
				sendpage();
				throw cgiEndOfFileException();
			}
			
			if (v["Content-type"] == "text/w3rpc")
			{
				env.decode (buf);
			}
		} // (nelc) fi
	}

	// --------------------------------------------------------------------
	// Ok, neither POST nor GET. Let's assume <form method=bogus>
	// --------------------------------------------------------------------
	
	else
	{
		headers["Content-Type"] = "text/html";
		buffer.printf (errortext::cgi::errbody, errortext::cgi::nocgi);
		
		sendpage();
		exit (1);
	}
}

// ========================================================================
// METHOD ::sendpage
// -----------------
// Takes the content that the overloaded main() method has put into the
// string object 'buffer', calculate the length and print all the headers
// as well as the content.
// ========================================================================
void rpccgi::sendpage (void)
{
	headers["Content-length"] = (int) buffer.strlen();
	foreach (header, headers)
	{
		fout.printf ("%s: %s\r\n", header.name(), header.cval());
	}
	fout.printf ("\r\n");
	fout.puts (buffer);
}

// ========================================================================
// DESTRUCTOR
// ----------
// A no-brainer, no dynamic structures allocated.
// ========================================================================
rpccgi::~rpccgi (void)
{
}

// ========================================================================
// METHOD ::main
// -------------
// Should be overloaded from this base class. Print out an error.
// ========================================================================
int rpccgi::main (void)
{
	ferr.printf (errortext::cgi::nomain);
	return 1;
}

