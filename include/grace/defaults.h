// ========================================================================
// defaults.h: Collection of arbitrary choices for limits and defaults
//
// (C) Copyright 2005 Pim van Riezen <pi@madscience.nl>
//                    Madscience Labs, Rotterdam 
// ========================================================================

#ifndef _DEFAULTS_H
#define _DEFAULTS_H 1

// The defaults.cpp file includes this file with an extra #define, turning
// the 'extern' declarations of the header into actual initializations.
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
			/// Default size of an i/o read buffer.
			parameter int readbuf defaultvalue (8 KB);
			
			/// Default size of an i/o write buffer.
			parameter int writebuf defaultvalue (8 KB);
			
			/// Default size of the ringbuffer for
			/// reading lines.
			parameter int ringbuffer defaultvalue (16 KB);
		}
	}
	
	/// External data limits
	namespace lim
	{
		/// Maximum post size handled by CGI objects.
		namespace cgi
		{
			/// Maximum post size.
			parameter int postsize defaultvalue (8 MB);
		}
		
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
		namespace mainthread
		{
			/// Number of seconds to idle between event polling.
			parameter int idle defaultvalue (5);
		}
		
		/// Keepalive behaviour.
		namespace keepalive
		{
			/// The keepalive trigger. If the maximum number of threads
			/// divided by this parameter exceeds the number of active
			/// threads, keepalive should not be honored.
			parameter int trigger defaultvalue (2);
		}
		
		/// Worker thread deallocation strategy.
		namespace wkthread
		{
			/// Number of rounds to keep a thread alone.
			parameter int minrounds defaultvalue (5);
			
			/// Minimum headroom space before we should consider
			/// trimming threads.
			parameter int minoverhead defaultvalue (2);
		}
	}
	
	/// TCP listening options.
	namespace tcplistener
	{
		/// Maximum backlog.
		parameter int backlog defaultvalue (32);
	}	
	
	/// Tuning settings for the smtpd.
	namespace smtpd
	{
		/// Behaviour of the main thread.
		namespace mainthread
		{
			/// Number of seconds to idle between event polling.
			parameter int idle defaultvalue (5);
		}
		
		/// Worker thread deallocation strategy.
		namespace wkthread
		{
			/// Number of rounds to keep a thread alone.
			parameter int minrounds defaultvalue (5);

			/// Minimum headroom space before we should consider
			/// trimming threads.
			parameter int minoverhead defaultvalue (2);
		}
	}
}

#undef parameter
#undef defaultvalue
#endif
