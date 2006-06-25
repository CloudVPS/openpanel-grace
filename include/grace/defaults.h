// ========================================================================
// defaults.h: Collection of arbitrary choices for limits and defaults
//
// (C) Copyright 2005 Pim van Riezen <pi@madscience.nl>
//                    Madscience Labs, Rotterdam 
// ========================================================================

#ifndef _DEFAULTS_H
#define _DEFAULTS_H 1

#ifdef _DEFAULTS_CPP
 #define parameter 
 #define defaultvalue(foo) = foo
#else
 #define parameter extern
 #define defaultvalue(foo) 
#endif

#define KB *1024
#define MB *(1024*1024)
#define GB *(1024*1024*1024)

/// Arbitrary settings that need some choice of default.
namespace defaults
{
	/// Internal size parameters
	namespace sz
	{
		/// Default maximum size of a file::log logfile.
		parameter int logfile defaultvalue (2 MB);
		
		/// Buffer settings for objects derived from file.
		namespace file
		{
			parameter int readbuf defaultvalue (8 KB);
			parameter int writebuf defaultvalue (8 KB);
		}
	}
	
	/// External data limits
	namespace lim
	{
		/// Maximum post size handled by CGI objects.
		namespace cgi { parameter int postsize defaultvalue (8 MB); }
		
		/// Data limits for httpd
		namespace httpd
		{
			/// Maximum post size
			parameter int postsize defaultvalue (2 MB);
			
			/// Maximum size of a chunked encoding block.
			parameter int chunksize defaultvalue (512 KB);
		}
	}
}

/// Tunable parameters
namespace tune
{
	/// Tuning settings for httpd.
	namespace httpd
	{
		/// Behaviour of the main thread.
		namespace mainthread { parameter int idle defaultvalue (5); }
		
		/// Keepalive behaviour.
		namespace keepalive { parameter int trigger defaultvalue (2); }
		
		/// Worker thread deallocation strategy.
		namespace wkthread
		{
			parameter int minrounds defaultvalue (5);
			parameter int minoverhead defaultvalue (2);
		}
	}
	
	/// TCP listening options.
	namespace tcplistener { parameter int backlog defaultvalue (32); }	
	
	/// Tuning settings for the smtpd.
	namespace smtpd
	{
		/// Behaviour of the main thread.
		namespace mainthread { parameter int idle defaultvalue (5); }
		
		/// Worker thread deallocation strategy.
		namespace wkthread
		{
			parameter int minrounds defaultvalue (5);
			parameter int minoverhead defaultvalue (2);
		}
	}
}

#undef parameter
#undef defaultvalue
#endif
