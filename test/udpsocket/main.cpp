#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/udpsocket.h>

class udpsockettestApp : public application
{
public:
		 	 udpsockettestApp (void) :
				application ("grace.testsuite.udpsocket")
			 {
			 }
			~udpsockettestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(udpsockettestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int udpsockettestApp::main (void)
{
	udpsocket s1, s2;
	ipaddress a;
	if (! s2.bind (3314))
	{
		FAIL ("bind");
	}
	
	s1.sendto ("127.0.0.1", 3314, "hello, world");
	string in = s2.receive (a, 1000);
	if (in != "hello, world")
	{
		fout.writeln ("%P" %format (in));
		FAIL ("mismatch");
	}
	
	if (a != (ipaddress) "127.0.0.1")
	{
		fout.writeln ("%P" %format (a));
		FAIL ("recvip");
	}
	
	return 0;
}

