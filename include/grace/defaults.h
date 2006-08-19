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
 #define sparameter const char *
 #define defaultvalue(foo) = foo
#else
 #define parameter extern
 #define sparameter extern const char *
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
		/// Number of backup-files kept for log-rotation.
		parameter int logrotate defaultvalue (4);

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
	
	/// Settings for retainable memory allocations.
	namespace memory
	{
		/// Determines behavior of the pool allocator. In a given
		/// situation, there should be no more than two allocations per
		/// active thread (a retained return value that picks up another
		/// retained return value). In the default setting, the allocator
		/// will throw an exception if all blocks in a pool are used up.
		/// The default pool size can keep about 200/300 blocks of the
		/// currently implemented retainable objects (typical blocksize
		/// of 24 or 32). If your application uses a massive amount of
		/// active threads and you already determined that the system
		/// does not leak retainables, you should consider changing this
		/// default to false.
		parameter bool leakprotection defaultvalue (true);
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

namespace names
{
	namespace logprio
	{
		sparameter emergency 	defaultvalue ("EMERG");
		sparameter alert     	defaultvalue ("ALERT");
		sparameter critical  	defaultvalue ("CRITC");
		sparameter error     	defaultvalue ("ERROR");
		sparameter warning   	defaultvalue ("WARN ");
		sparameter info      	defaultvalue ("INFO ");
		sparameter application	defaultvalue ("APPL ");
		sparameter debug     	defaultvalue ("DEBUG");
	}
}

namespace errortext
{
	namespace daemon
	{
		sparameter writepid defaultvalue ("%% Could not write pid-file\n");
		sparameter running defaultvalue ("%% Service %s already running\n");
		sparameter nofork defaultvalue ("%% Could not fork\n");
	}
	namespace application
	{
		sparameter nomain defaultvalue ("%% application::main() not overloaded");
	}
	namespace cgi
	{
		sparameter errbody defaultvalue ("<html><body><h2>CGI Error</h2>%s</body></html>\n");
		sparameter size defaultvalue ("Size overflow");
		sparameter eof defaultvalue ("Unexpected end-of-file ");
		sparameter bound defaultvalue ("No boundary provided for multipart");
		sparameter section defaultvalue ("Section '%S' not found");
		sparameter nocgi defaultvalue ("This program must be run as a CGI");
		sparameter nomain defaultvalue ("%% cgi::main() not overloaded");
	}
	namespace configdb
	{
		sparameter noschema defaultvalue ("Schema not found: %s");
		sparameter novalidator defaultvalue ("Validator schema not found: %s");
		sparameter noconf defaultvalue ("Configuration not found: %s");
		sparameter errnew defaultvalue ("Error in new configuration");
	}
	namespace file
	{
		sparameter codecerr defaultvalue ("Codec error: %s");
		sparameter codecinexc defaultvalue ("Codec exception while fetching data");
		sparameter codecflush defaultvalue ("Error flushing codec data");
		sparameter ringexc defaultvalue ("Exception in ringbuffer");
		sparameter bufexc defaultvalue ("Buffer exception");
		sparameter brokenpipe defaultvalue ("Broken pipe");
		sparameter notopen defaultvalue ("File not open");
		sparameter eof defaultvalue ("End of file");
		sparameter rdto_select defaultvalue ("Read timeout (select)");
		sparameter rdto_read defaultvalue ("Read timeout (rread)");
	}
	namespace http
	{
		sparameter invalidurl defaultvalue ("%s is not a valid url");
		sparameter connect_usock defaultvalue ("Could not connect to unix socket '%s'");
		sparameter connect_proxy defaultvalue ("Could not connect to proxy at %s:%i");
		sparameter connect defaultvalue ("Could not connect to %s:%i");
		sparameter chunksz defaultvalue ("Max chunksize exceeded: %i < %i");
		sparameter connbroken defaultvalue ("Connection broken: %s");
		sparameter timeout defaultvalue ("Connected timed out");
		sparameter prebroken defaultvalue ("Premature end of connection");
		sparameter chunk defaultvalue ("Error getting chunked data");
	}
	namespace httpd
	{
		sparameter html_body defaultvalue ("<html><body>%s</body></html>");
		sparameter html_illuri defaultvalue ("<h2>Illegal URI Requested</h2>");
		sparameter html_401 defaultvalue ("<h2>Please Authenticate</h2>");
		sparameter html_404 defaultvalue ("<h2>404 Not Found</h2>Look eslewhere.");
		sparameter html_404_vhost defaultvalue ("<h2>404 Site Not Found</h2>Either this server is misconfigured or the DNS entry for %Z is not pointer the right way.");
		sparameter html_413 defaultvalue ("<h2>POST object-size too large</h2>");
		sparameter html_500_method defaultvalue ("<h2>500 Unknown method</h2>");
		sparameter html_spage defaultvalue ("<h2>Processing Error</h2>Execution error in serverpage::run");
		sparameter illuri defaultvalue ("Illegal URI %S");
		sparameter illuri_details defaultvalue ("Illegal URI %S root=<%s> realpath=<%s>");
		sparameter ftype_base defaultvalue ("Unoverloaded filetypehandler invoked for file '%S'");
		sparameter toolarge defaultvalue ("Entity too large: %i");
		sparameter getbody defaultvalue ("GET request with body data");
		sparameter method defaultvalue ("Unknown method '%S'");
		sparameter authfail defaultvalue ("Authentication failed for user '%S' in realm '%S' <%S>");
		sparameter novhost defaultvalue ("Could not resolve vhost '%S'");
	}
	namespace process
	{
		sparameter nomain defaultvalue ("Unoverloaded process::main()\n");
	}
	namespace smtp
	{
		sparameter connfail defaultvalue ("Connection failed");
		sparameter connclose defaultvalue ("Connection closed");
		sparameter start defaultvalue ("SMTP handshake error: ");
		sparameter helo defaultvalue ("SMTP HELO error: ");
		sparameter mailfrom defaultvalue ("SMTP MAIL FROM error: ");
		sparameter rcptto defaultvalue ("SMTP RCPT TO error: ");
		sparameter data defaultvalue ("SMTP DATA error: ");
		sparameter deliver defaultvalue ("SMTP Delivery error: ");
	}
	namespace smtpd
	{
		sparameter ucommand defaultvalue ("Unrecognized command");
		sparameter pipe defaultvalue ("Broken pipe");
	}
	namespace sock
	{
		sparameter isconn defaultvalue ("Socket already connected");
		sparameter connfail defaultvalue ("Connection failed: %s");
		sparameter create defaultvalue ("Could not create a socket");
		sparameter codec defaultvalue ("Codec error: %s");
		sparameter chandshake defaultvalue ("Codec handshake i/o error");
	}
	namespace validator
	{
		sparameter rule_unknown defaultvalue ("Unknown rule-id");
		sparameter rule_error defaultvalue ("Error matching rule");
		sparameter matchtype defaultvalue ("Unknown match type");
		sparameter matchid defaultvalue ("Error performing match.id rule on a match with no id");
		sparameter optmdep defaultvalue ("Optional object implies a missing mandatory object");
		sparameter mdep defaultvalue ("Mandatory class member object missing");
		sparameter mattrdep defaultvalue ("Mandatory attribute object missing");
		sparameter attr_unknown defaultvalue ("Unknown attribute");
		sparameter nomatch defaultvalue ("Data match failed");
		sparameter noindex defaultvalue ("Expected child with index key");
		sparameter type_unknown defaultvalue ("Unknown type-constraint");
		sparameter wrongtype defaultvalue ("Object-type mismatch");
	}
}

#undef parameter
#undef defaultvalue
#endif
