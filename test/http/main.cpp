#include <grace/application.h>
#include <grace/http.h>
#include <grace/httpd.h>

extern "C" void grace_init (void) { __THREADED = true; }

class httpApp : public application
{
public:
		 	 httpApp (void) :
				application ("grace.testsuite.http")
			 {
			 }
			~httpApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(httpApp);

void __breakme (void) {}

int httpApp::main (void)
{
	value hostdata;
	value pwdata;
	
	hostdata["*"] = "default";
	hostdata["localhost"] = "localhost";

	pwdata["me"] = "password";
	valueauth pwdb (pwdata);
	
	httpd 			srv (4269);
					srv.systempath ("docroot");
	httpdvhost		srv_vhost (srv, hostdata);
	httpdbasicauth	srv_auth (srv, "*/restricted.dat", "realm", pwdb);
	httpdfileshare	srv_fshare (srv, "*", "docroot");
	
	srv.start ();
	sleep (360);
	
	httpsocket hs;
	
	string public_local;
	string public_default;
	string restr_local;
	string restr_default;
	string ctype_text;
	value rhdr;

	public_local = hs.get ("http://localhost:4269/public.dat");
	if (! public_local.strlen ())
	{
		ferr.printf ("FAIL public local\n");
		return 1;
	}
	public_default = hs.get ("http://127.0.0.1:4269/public.dat");
	if (! public_default.strlen ())
	{
		ferr.printf ("FAIL public default\n");
		return 2;
	}
	ctype_text = hs.get ("http://127.0.0.1:4269/contenttypetest.html",rhdr);
	if (rhdr["Content-type"] != "text/html")
	{
		ferr.printf ("FAIL content-type\n");
		return 2;
	}
	
	restr_local = hs.get ("http://localhost:4269/restricted.dat");
	if (hs.status == 200)
	{
		ferr.printf ("FAIL restricted local auth\n");
		return 3;
	}
	
	hs.authentication ("me","password");
	
	for (int i=0; i<10; ++i)
	{
		restr_local = hs.get ("http://localhost:4269/restricted.dat");
		if (! restr_local.strlen ())
		{
			ferr.printf ("FAIL restricted local: %i %s\n", hs.status, hs.error.str());
			restr_local = hs.get ("http://localhost:4269/restricted.dat");
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
		restr_default = hs.get ("http://127.0.0.1:4269/restricted.dat");
		if (! restr_default.strlen ())
		{
			ferr.printf ("FAIL restricted default\n");
			return 5;
		}
	}
	
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

