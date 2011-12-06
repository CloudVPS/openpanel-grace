#include <grace/application.h>
#include <grace/http.h>
#include <grace/sslsocket.h>
#include <grace/httpd.h>
#include <grace/strutil.h>

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
	value hostdata = $("*","default")->$("localhost","localhost");
	value public_local;
	httpsd 			srv (4269);
					srv.systempath ("docroot");
	httpdvhost		srv_vhost (srv, hostdata);
	httpdfileshare	srv_fshare (srv, "*", "docroot");

	srv.loadkeyfile("brokencert.pem");
	srv.start ();

	httpssocket hs;

	hs.nocertcheck();
	hs.keepalive(false);

	debug_out( "get https://localhost:4269/public.dat\n");

	public_local = hs.get ("https://localhost:4269/public.dat");
	if (! public_local.strlen ())
	{
		ferr.printf ("FAIL public local\n");
		ferr.printf ( hs.error );
		ferr.printf ( ".\n" );
		return 2;
	}

	fs.save ("localhost", public_local);

	ferr.printf ("Shutting down...\n");
	srv.shutdown();
	ferr.printf ("Shutdown completed...\n");
	return 0;
}
