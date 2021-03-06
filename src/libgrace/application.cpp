// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

// ========================================================================
// application.cpp: Application Model Object
// ========================================================================

#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/xmlschema.h>
#include <grace/defaults.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

extern char **environ;

file fin;
file fout;
file ferr;

// ==========================================================================
// CONSTRUCTOR
// -----------
// Register the creator type, pick up the environment and open the
// application's resource database.
// ==========================================================================
application::application (const string &creator_id)
{
	string path;
	
	// Set the creator
	creator = creator_id;
	
	// Copy the environment
	
	string tr, tl;
	
	for (int i=0; environ[i] != NULL; ++i)
	{
		tr = environ[i];
		tl = tr.cutat ('=');
		if (tl.strstr ("_PATH_") != 0)
		{
			env[tl] = tr;
		}
	}
	
	fin.openread (0);
	fout.openwrite (1);
	ferr.openwrite (2);
}

// ==========================================================================
// DESTRUCTOR
// ----------
// Nothing to do here (yet)
// ==========================================================================
application::~application (void)
{
	try
	{
		fin.close ();
		//fout.close ();
		ferr.close ();
	}
	catch (...)
	{
	}
}

// ==========================================================================
// METHOD init
// -----------
// This is normally invoked from the APPOBJECT() macro, passing the argument 
// array from main() to the application object.
//
// The opt[] value database is consulted to transport options specified in
// the derived constructor to the argv[] value database.
// ==========================================================================
void application::init (int Argc, char *Argv[])
{
	int crsr;
	string tmp;
	string appnam;
	string appath;
	string attr;
	string temppath;
	
	argv[0] = Argv[0];
	appnam = argv[0];

	if (appnam.strchr ('/') >= 0)
	{
		if ((appnam[0] == '.')&&(appnam[1] == '/'))
		{
			appnam = appnam.mid (2);
			appath = fs.pwdize (appnam);
		}
		else
		{
			appath = appnam;
		}
	}
	else
	{
		tmp = "tools:%s" %format (appnam);
		appath = fs.transr (tmp);
	}

	if (fs.exists (appath))
	{
		bool maybelink = true;
		struct stat mst;
		char linkbuffer[512];
		
		while (maybelink)
		{
			if (lstat (appath.str(), &mst) <0) break;
			if ( (mst.st_mode & S_IFMT) == S_IFLNK)
			{
				int t;
				t = readlink (appath.str(), linkbuffer, 511);
				
				if (t>0)
				{
					linkbuffer[t] = linkbuffer[511] = 0;
				}
				else
				{
					linkbuffer[0] = 0;
				}
				
				if (linkbuffer[0] == '/')
				{
					appath = linkbuffer;
				}
				else
				{
					appath = appath.cutatlast ('/');
					appath.strcat ('/');
					appath.strcat (linkbuffer);
				}
			}
			else maybelink = false;
		}
	}
	
	if (appnam.strchr ('/') >= 0)
		delete appnam.cutatlast ("/");

	if (appath.strstr ("/Contents/") >= 0)
	{
		delete appath.cutafter ("/Contents/");
	}
	else
	{
		appath += ".app";
	}
	
	if (! fs.exists (appath))
	{
		appath = "apps:";
		appath += appnam;
		
		if (appath.strstr (".app") < 0)
			appath += ".app";
			
		appath = fs.pwdize (appath);
	}
	if (! fs.exists (appath))
	{
		appath = "tools:";
		appath += appnam;
		
		if (appath.strstr (".app") < 0)
			appath += ".app";
			
		appath = fs.pwdize (appath);
	}
	
	attr = fs.getresource (appath, "mime");
	if (attr.strlen()) env.setattrib ("mime", attr);
	
	attr = fs.getresource (appath, "apid");
	if (attr.strlen()) env.setattrib ("apid", attr);
	
	fs.pathvol["app"].newval() = appath;
	
	temppath = appath;
	temppath.strcat ("/Contents/Resources");
	temppath = fs.transr (temppath);
	fs.pathvol["rsrc"].newval() = temppath;

	temppath = appath;
	temppath.strcat ("/Contents/Templates");
	temppath = fs.transr (temppath);
	fs.pathvol["tmpl"].newval() = temppath;

	temppath = appath;
	temppath.strcat ("/Contents/Tools");
	temppath = fs.transr (temppath);
	fs.pathvol["tools"].newval() = temppath;

	temppath.crop (0);
	temppath = appath;
	temppath.strcat ("/Contents/Schemas");
	temppath = fs.transr (temppath);
	fs.pathvol["schema"].newval() = temppath;
	
	temppath = appath;
	temppath.strcat ("/Contents/Configuration Defaults");
	temppath = fs.transr (temppath);
	fs.pathvol["defaults"].newval() = temppath;
	fs.pathvol["conf"].newval() = temppath;
	fs.pathvol["conf"][-1].setattrib ("readonly", true);

	if (fs.exists ("/Library/Preferences"))
	{
		temppath = "/Library/Preferences/%s" %format (creator);
		fs.pathvol["conf"].newval() = temppath;
	}
	else if (fs.exists ("/etc/conf"))
	{
		temppath = "/etc/conf/%s" %format (creator);
		fs.pathvol["conf"].newval() = temppath;
	}
	
	if (fs.exists ("home:Library/Preferences"))
	{
		temppath = "%s/Library/Preferences/%s" %format (env["HOME"], creator);
		fs.pathvol["conf"].newval() = temppath;
	}
	else if (fs.exists ("home:.conf"))
	{
		temppath = "%s/.conf/%s" %format (env["HOME"], creator);
		fs.pathvol["conf"].newval() = temppath;
	}

	fs.pathvol["etc"].newval() = "/etc";
	if (fs.exists ("home:etc"))
	{
		fs.pathvol["etc"].newval() = "%s/etc" %format (env["HOME"]);
	}
	else if (fs.exists ("home:.etc"))
	{
		fs.pathvol["etc"].newval() = "%s/.etc" %format (env["HOME"]);
	}

	string appnamLower = appnam;
	appnamLower.ctolower();
		
	if (fs.exists ("rsrc:grace.runoptions.xml"))
	{
		opt.loadxml ("rsrc:grace.runoptions.xml", xmlschema::runopt());
	}
	
	if (fs.exists ("rsrc:resources.xml"))
	{
		rsrc.loadxml ("rsrc:resources.xml");
	}
	
	crsr = 1;

	// Iterate over the commandline arguments
	while (crsr < Argc)
	{
		string arg;
		arg = Argv[crsr];
		
		// short option (-f -o -o)
		if ((arg.strlen()>2) && (arg[0] == '-') && (arg[1] != '-'))
		{
			value &optarg = opt[arg];
			statstring longname = optarg["long"].sval();
			value &longarg = opt[longname];
			// should it have arguments (and more than 1)?
			if (longarg["argc"] > 1)
			{
				++crsr;
				for (int i=0; i < (int) longarg["argc"]; ++i)
				{
					if (crsr < Argc)
					{
						argv[longname][i] = Argv[crsr];
						++crsr;
					}
				}
			}
			else
			{
				// ok just 1?
				if (longarg["argc"] == 1)
				{
					++crsr;
					if (crsr < Argc)
					{
						argv[longname] = Argv[crsr];
						if (longarg["hide"] == true)
						{
							Argv[crsr][0] = 0;
						}
					}
				}
				else argv[longname] = 1;
			}
		}
		
		// ok perhaps a long option?
		else if ((arg.strlen()>2) && (arg[0] == '-') && (arg[1] == '-'))
		{
			// adapt if it has the form --foo="bar"
			if ((arg.strchr ('=')) >= 0)
			{
				value tv;
				
				tv = strutil::splitquoted (arg, '=');
				argv[tv[0].sval()] = tv[1].sval();
			}
			else if (opt.exists (arg))
			{
				if (opt[arg]["argc"] > 1)
				{
					++crsr;
					for (int i=0; i< opt[arg]["argc"].ival(); ++i)
					{
						if (crsr < Argc)
						{
							argv[arg][i] = Argv[crsr];
							if (opt[arg]["hide"] == true)
							{
								Argv[crsr][0] = 0;
							}
							++crsr;
						}
					}
				}
				else
				{
					if (opt[arg]["argc"] == 1)
					{
						++crsr;
						if (crsr < Argc)
						{
							argv[arg] = Argv[crsr];
							if (opt[arg]["hide"] == true)
							{
								Argv[crsr][0] = 0;
							}
						}
					}
					else argv[arg] = 1;
				}
			}
			else
			{
				// builtin helper
				if (arg == "--help")
				{
					value shortcuts;
					
					foreach (option,opt)
					{
						string name;
						name = option.name();
						if (name[1] != '-')
						{
							shortcuts[option["long"].sval()] = name;
						}
						else
						{
							ferr.puts ("%s %s " %format (shortcuts[name],name));
							
							if ((option["argc"] == 0) ||
								(! option["default"].sval().strlen()))
							{
								//ferr.printf ("");
							}
							else
							{
								ferr.puts ("[%s]" %format (option["default"]));
							}
							if (option["help"].sval().strlen())
							{
								ferr.puts (": %s" %format (option["help"]));
							}
							ferr.puts ("\n");
						}
					}
					exit (1);
				}
				else
				{
					argv["*"].newval() = arg;
				}
			}
		}
		else
		{
			argv["*"].newval() = arg;
		}
		++crsr;
	}
	
	// Fill in default values where none have been provided
	foreach (o, opt)
	{
		if ( (o.exists ("default")) &&
		     (! argv.exists (o.name())) )
		{
			argv[o.name()] = o["default"].sval();
		}
	}
}

// ==========================================================================
// METHOD main
// -----------
// The inheriting class should do something useful with this
// virtual.
// ==========================================================================
int application::main (void)
{
	ferr.printf (errortext::application::nomain);
	return 1;
}


// A way to get a pointer to the global application object.
extern "C" application *app(void);

typedef void (*initfuncptr)(void);

// ==========================================================================
// FUNCTION main
// -------------
// Uses the global 'app' as defined by the APPOBJECT() macro to
// kickstart an application.
// ==========================================================================

#include <dlfcn.h>

extern memory::pool *__retain_ptr;

int main (int argc, char *argv[])
{
	__THREADED = true;
	int returnv;
	initfuncptr inithook = NULL;
	void *dlh = NULL;
	int sdf;
	
	dlh = dlopen (NULL, RTLD_LAZY);
	if (dlh)
	{
		inithook = (initfuncptr) dlsym (dlh, "grace_init");
		if (inithook) (*inithook)();
	}
	
	application *a = app();
	try
	{
		a->init (argc, argv);
	}
	catch (exception e)
	{
		return -1;
	}
	returnv = a->main ();
	delete a;
	if (__retain_ptr) __retain_ptr->exit ();
	return returnv;
}
