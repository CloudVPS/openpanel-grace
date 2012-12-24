#include <grace/application.h>
#include <grace/http.h>
#include <grace/sslsocket.h>
#include <grace/httpd.h>
#include <grace/strutil.h>

extern "C" void grace_init (void) { __THREADED = true; }

class httpsCurlApp : public application
{
public:
		 	 httpsCurlApp (void) :
				application ("grace.testsuite.https")
			 {
			 }
			~httpsCurlApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(httpsCurlApp);

void __breakme (void) {}

#define debug_out if(1) ::printf

int httpsCurlApp::main (void)
{
	httpsd srv (14265);
	srv.systempath ("docroot");
	srv.loadkeyfile("cert.pem");

	httpdfileshare	srv_fshare (srv, "*", "docroot");
	
	srv.start ();

	ferr.writeln ("Service started");

	sleep(5);

	ferr.printf ("Shutting down...\n");	
	srv.shutdown();
	ferr.printf ("Shutdown completed...\n");
	return 0;
}

