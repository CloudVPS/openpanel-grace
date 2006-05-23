#include <grace/application.h>
#include <grace/tcpsocket.h>

class tcpsocketApp : public application
{
public:
		 	 tcpsocketApp (void) :
				application ("grace.testsuite.tcpsocket")
			 {
			 }
			~tcpsocketApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(tcpsocketApp);

int tcpsocketApp::main (void)
{
	tcplistener ls (8124);
	tcpsocket outs;
	tcpsocket ins;
	string buffer;
	
	if (! outs.connect ("127.0.0.1", 8124))
	{
		ferr.printf ("fail: outs.connect\n");
		return 1;
	}
	
	ins = ls.accept ();
	if (ins.peer_name != "127.0.0.1")
	{
		ferr.printf ("fail: peer_name <%s>\n", ins.peer_name.str());
		return 1;
	}
	
	outs.printf ("testing one two three\nfour five six\n");
	buffer = ins.gets();
	if (buffer != "testing one two three")
	{
		ferr.printf ("fail: readline 1\n");
		sleep (3);
		return 1;
	}
	buffer = ins.gets();
	if (buffer != "four five six")
	{
		ferr.printf ("fail: readline 2\n");
		sleep (5);
		return 1;
	}
	outs.close();
	ins.close();
	return 0;
}

