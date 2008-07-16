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
	s2.bind (1973);
	s1.sendto ("127.0.0.1", 1973, "hello, world");
	string in = s2.receive ();
	if (in != "hello, world")
	{
		FAIL ("mismatch");
	}
	return 0;
}

