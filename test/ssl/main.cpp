#include <grace/application.h>
#include <grace/tcpsocket.h>
#include <grace/sslsocket.h>

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
	sslsocket outs;
	try
	{
		string buffer;
		
		if (! outs.connect ("www.openprovider.nl", 443))
		{
			ferr.printf ("fail: outs.connect\n");
			ferr.printf ("err: %s\n", outs.error().str());
			return 1;
		}
		
		outs.printf ("GET / HTTP/1.0\r\nHost: www.openprovider.nl\r\n\r\n");
		while (! outs.eof())
		{
			buffer = outs.gets();
			fout.writeln (buffer);
		}
		
		outs.close();
	}
	catch (...)
	{
		ferr.printf ("exception: %s\n", outs.error().str());
		ferr.printf ("codec-exception: %s\n", outs.codecerror().str());
		return 1;
	}
	
	return 0;
}

