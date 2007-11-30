// ========================================================================
// application.h: Base class that defines an application. It takes care
//                of commandline arguments, paths and resource files.
//                Applications override the ::main method.
//
// Copyright (C) 2004 Pim van Riezen <pi@madscience.nl>
//                    Madscience Labs, Rotterdam
// ========================================================================

#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <grace/value.h>
#include <grace/file.h>

/// This base class models an application.
/// Provides housekeeping tasks for an application, including:
///   - Command line argument parsing
///   - Getting the unix environment variables into a tasty value object.
///   - Loading the standard resource file
///   - Handling compound paths (alias-paths).
/// The application class uses the global filesystem object 'fs' and its
/// alias-path mechanism to look for certain files. If these are not
/// explicitly provided by the environment, a couple will be explicitly
/// set.
///   - \b app: Points to the root of the bundle directory for the application.
///     If the application was called by its absolute path, this will be used.
///     Otherwise, $PATH will be used to locate the bundle directory.
///   - \b rsrc: Points at least to app:Contents/Resources/
///   - \b schema: Points at least to app:Contents/Schemas/
///   - \b home: Points to the user's home directory.
///   - \b conf: Points to one of the following:
///     - \e Mac OS X layout: 
///       - home:Library/Preferences/tld.domain.applicationid/
///       - app:Contents/Configuration Defaults/
///       - /Library/Preferences/tld.domain.applicationid/
///     - \e Unix layout:
///       - home:.conf/tld.domain.applicationid/
///       - app:Contents/Configuration Defaults/
///       - /etc/conf/tld.domain.applicationid/
///
/// On startup, the file \e rsrc:grace.runoptions.xml is loaded into the opt
/// value and \e rsrc:resources.xml is loaded into the \b rsrc value object.
class application
{
public:
					 /// Default constructor.
					 /// Loads resources, initializes.
					 ///
					 /// \param appid the application-id [tld.domain.foo.app]
					 application (const string &appid);
	virtual			~application (void);

					 /// Second stage initialization.
					 /// Parses commandline arguments according to the
					 /// rules found in the "rsrc:grace.runoptions.xml" file.
					 /// Arguments and flags defined in this rule file
					 /// are filed with their proper (long format) key
					 /// in the argv[] value object inside the application
					 /// object. Unaccounted or extra arguments are stored
					 /// as unkeyed items inside argv["*"][].
					 ///
					 /// \param argc argument count
					 /// \param argv argument array
					 ///
					 /// The layout of the a runoptions file is as follows:
					 /// \verbinclude application_doc_runoptions.xml
					 ///
					 /// Options with a 'short' commandline flag as their
					 /// key, as a rule, only contain a reference to their
					 /// long counterpart.
	void			 init (int argc, char **argv);
	
					 /// The main loop.
					 /// Represents the application's main loop, a concrete
					 /// subclass should implement the actual application's
					 /// main loop here.
					 /// \return Application return value after it finished.
	virtual int		 main (void);

	value			 argv; ///< Dictionary of parsed command line arguments
	value			 env;  ///< The UNIX environment
	value			 rsrc; ///< The resource file
	value			 opt;  ///< The runoptions file
	string			 creator; ///< The application id
	
	file			 fin; ///< The standard input channel
	file			 fout; ///< The standard output channel
	file			 ferr; ///< The standard error channel
};

/// Defines the app() call to create an instance of your application class.
/// If your application class is called myApp, then embed an
/// APPOBJECT(myApp) into your main.cpp file.
#define APPOBJECT(foo) extern "C" foo *app (void) { static foo *a = new foo; return a; }
#define MAIN (* (app()))

#define $appid(theid) const char *__GRACE_APPID = #theid
#define $version(theversion) const char *__GRACE_APPVERSION = #theversion
#define $appobject(classname) extern "C" classname *app (void) \
	{ \
		static classname *a = new classname; \
		return a; \
	}


			

#endif
