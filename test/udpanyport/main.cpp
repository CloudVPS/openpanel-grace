#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/udpsocket.h>

class udpanyporttestApp : public application
{
public:
		 	 udpanyporttestApp (void) :
				application ("grace.testsuite.udpanyport")
			 {
			 }
			~udpanyporttestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(udpanyporttestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int udpanyporttestApp::main (void)
{
	udpsocket u;
	u.bind (0);
	int port = u.getport();
	if (port == 0) FAIL ("getport");
	fout.writeln ("%i" %format (port));
	return 0;
}

