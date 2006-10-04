// ========================================================================
// application.cpp: Application Model Object
//
// (C) Copyright 2004 Pim van Riezen <pi@madscience.nl>
//                    Madscience Labs, Rotterdam 
// ========================================================================

#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/xmlschema.h>
#include <grace/defaults.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

extern char **environ;

// ==========================================================================
// CONSTRUCTOR
// -----------
// Register the creator type, pick up the environment and open the
// application's resource database.
// ==========================================================================
application::application (const char *creator_id)
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
		tmp.printf ("tools:%s", appnam.str());
		appath = fs.transr (tmp);
	}

	if (fs.exists (appath))
	{
		bool maybelink = true;
		struct stat mst;
		char linkbuffer[512];
		
		while (maybelink)
		{
			lstat (appath.str(), &mst);
			if ( (mst.st_mode & S_IFMT) == S_IFLNK)
			{
				int t;
				t = readlink (appath.str(), linkbuffer, 511);
				linkbuffer[t] = linkbuffer[511] = 0;
				
				if (linkbuffer[0] == '/')
				{
					appath = linkbuffer;
				}
				else
				{
					appath = appath.cutatlast ('/');
					appath.printf ("/%s", linkbuffer);
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
	temppath.printf ("/Contents/Resources");
	temppath = fs.transr (temppath);
	fs.pathvol["rsrc"].newval() = temppath;

	temppath = appath;
	temppath.printf ("/Contents/Tools");
	temppath = fs.transr (temppath);
	fs.pathvol["tools"].newval() = temppath;

	temppath.crop (0);
	temppath = appath;
	temppath.printf ("/Contents/Schemas");
	temppath = fs.transr (temppath);
	fs.pathvol["schema"].newval() = temppath;
	
	temppath = appath;
	temppath.printf ("/Contents/Configuration Defaults");
	temppath = fs.transr (temppath);
	fs.pathvol["defaults"].newval() = temppath;
	fs.pathvol["conf"].newval() = temppath;
	fs.pathvol["conf"][-1].setattrib ("readonly", true);

	if (fs.exists ("/Library/Preferences"))
	{
		temppath = "/Library/Preferences";
		temppath.printf ("/%s", creator.str());
		fs.pathvol["conf"].newval() = temppath;
	}
	else if (fs.exists ("/etc/conf"))
	{
		temppath = "/etc";
		temppath.printf ("/conf/%s", creator.str());
		fs.pathvol["conf"].newval() = temppath;
	}
	
	if (fs.exists ("home:Library/Preferences"))
	{
		temppath = env["HOME"];
		temppath.printf ("/Library/Preferences/%s", creator.str());
		fs.pathvol["conf"].newval() = temppath;
	}
	else if (fs.exists ("home:.conf"))
	{
		temppath = env["HOME"];
		temppath.printf ("/.conf/%s", creator.str());
		fs.pathvol["conf"].newval() = temppath;
	}

	string appnamLower = appnam;
	appnamLower.ctolower();
		
	if (fs.exists ("rsrc:grace.runoptions.xml"))
	{
		xmlschema runOptionsSchema (XMLRunOptionsSchemaType);
		opt.loadxml ("rsrc:grace.runoptions.xml", runOptionsSchema);
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
		if ((arg[0] == '-') && (arg[1] != '-'))
		{
			value optarg = opt[arg];
			string longname = opt[arg]["long"].sval();
			value longarg = opt[opt[arg]["long"].sval()];
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
		else if ((arg[0] == '-') && (arg[1] == '-'))
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
							ferr.printf ("%s %s ", shortcuts[name].cval(),
														name.str());
							
							if ((option["argc"] == 0) ||
								(! option["default"].sval().strlen()))
							{
								//ferr.printf ("");
							}
							else
							{
								ferr.printf
									("[%s]", option["default"].cval());
							}
							if (option["help"].sval().strlen())
							{
								ferr.printf
									(": %s", option["help"].cval());
							}
							ferr.printf ("\n");
						}
					}
					exit (1);
				}
			}
			else
			{
				argv["*"].newval() = arg;
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

int main (int argc, char *argv[])
{
	initfuncptr inithook = NULL;
	void *dlh = NULL;
	
	dlh = dlopen (NULL, RTLD_LAZY);
	if (dlh)
	{
		inithook = (initfuncptr) dlsym (dlh, "grace_init");
		if (inithook) (*inithook)();
	}
	
	app()->init (argc, argv);
	return app()->main ();
}
