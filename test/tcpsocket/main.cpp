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
	int      testconnect( tcplistener& ls, const ipaddress& addr, string tag );
};

APPOBJECT(tcpsocketApp);


int tcpsocketApp::testconnect( tcplistener& ls, const ipaddress& addr, string tag )
{
    tcpsocket outs;
	tcpsocket ins;
	string buffer;
	
	if (! outs.connect (addr, 8124))
	{
		ferr.printf ("%s fail: outs.connect\n", tag.str());
		return 1;
	}
	
	ins = ls.accept ();
	if (ins.peer_name != addr)
	{
		ferr.printf ("%s fail: peer_name <%s>\n", tag.str(), ins.peer_name.str());
		return 1;
	}
	
	outs.printf ("testing one two three\nfour five six\n");
	buffer = ins.gets();
	if (buffer != "testing one two three")
	{
		ferr.printf ("%s fail: readline 1\n", tag.str());
		sleep (3);
		return 1;
	}
	buffer = ins.gets();
	if (buffer != "four five six")
	{
		ferr.printf ("%s fail: readline 2\n", tag.str());
		sleep (5);
		return 1;
	}
	outs.close();
	ins.close();
}

int tcpsocketApp::main (void)
{
	tcplistener ls (8124);
	
	int a = testconnect( ls, "127.0.0.1", "IPv4" );
	if (a) return a;
	
	a = testconnect( ls, "::1", "IPv6" );
	if (a) return a;
	
	return 0;
}

