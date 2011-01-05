#include <grace/application.h>
#include <grace/http.h>
#include <grace/sslsocket.h>
#include <grace/httpd.h>

extern "C" void grace_init (void) { __THREADED = true; }

class httpsApp : public application
{
public:
		 	 httpsApp (void) :
				application ("grace.testsuite.https")
			 {
			 }
			~httpsApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(httpsApp);

void __breakme (void) {}

#define debug_out if(1) ::printf

int httpsApp::main (void)
{
	value hostdata;
	value pwdata;
	
	hostdata["*"] = "default";
	hostdata["localhost"] = "localhost";

	pwdata["me"] = "password";
	valueauth pwdb (pwdata);
	
	httpsd 			srv (4269);
					srv.systempath ("docroot");
	httpdvhost		srv_vhost (srv, hostdata);
	httpdbasicauth	srv_auth (srv, "*/restricted.dat", "realm", pwdb);
	httpdfileshare	srv_fshare (srv, "*", "docroot");
	
	srv.loadkeyfile("cert.pem");
	
	srv.start ();
	
	httpssocket hs;
	hs.nocertcheck();
	hs.keepalive(true);
	
	string public_local;
	string public_default;
	string restr_local;
	string restr_default;

	debug_out( "get https://localhost:4269/public.dat\n");

	public_local = hs.get ("https://localhost:4269/public.dat");
	if (! public_local.strlen ())
	{
		ferr.printf ("FAIL public local\n");
		ferr.printf ( hs.error );
		ferr.printf ( ".\n" );
		return 1;
	}

	debug_out( "get https://127.0.0.1:4269/public.dat\n");
	public_default = hs.get ("https://127.0.0.1:4269/public.dat");
	if (! public_default.strlen ())
	{
		ferr.printf ("FAIL public default\n");
		return 2;
	}

	debug_out( "get https://localhost:4269/restricted.dat\n");
	restr_local = hs.get ("https://localhost:4269/restricted.dat");
	if (hs.status == 200)
	{
		ferr.printf ("FAIL restricted local auth\n");
		return 3;
	}

	hs.authentication ("me","password");
	hs.keepalive (true);
	
	for (int i=0; i<10; ++i)
	{
		debug_out( "With passwd: %d\n", i );
		
		restr_local = hs.get ("https://localhost:4269/restricted.dat");
		if (! restr_local.strlen ())
		{
			ferr.printf ("FAIL restricted local: %i %s\n", hs.status, hs.error.str());
			restr_local = hs.get ("https://localhost:4269/restricted.dat");
			if (! restr_local.strlen ())
			{
				ferr.printf ("FAIL restricted local: %i\n", hs.status);
				__breakme ();
				return 4;
			}
			else
			{
				ferr.printf ("FAIL: restricted local once\n");
				return 9;
			}
		}
		restr_default = hs.get ("https://127.0.0.1:4269/restricted.dat");
		if (! restr_default.strlen ())
		{
			ferr.printf ("FAIL restricted default\n");
			return 5;
		}
	}
	
	debug_out( "Done...\n" );
	
	hs.sock().close();
	
	fs.save ("default.public", public_default);
	fs.save ("localhost.public", public_local);
	fs.save ("default.restricted", restr_default);
	fs.save ("localhost.restricted", restr_local);

	ferr.printf ("Shutting down...\n");	
	srv.shutdown();
	ferr.printf ("Shutdown completed...\n");
	return 0;
}

