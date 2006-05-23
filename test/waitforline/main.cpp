#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/process.h>

class recvprocess : public process
{
public:
			 recvprocess (void) : process ("receiver", false)
			 {
			 }
			~recvprocess (void)
			 {
			 }
	int		 main (void)
			 {
			 	string buffer;
			 	buffer = fin.gets();
			 	if (buffer != "wibble") return 2;
			 	
			 	if (! fin.waitforline (buffer, 1500, 256))
			 		return 1;
			 	
			 	if (buffer.strncmp ("helloworld", 10) == 0)
			 		return 0;
			 	
			 	return 2;
			 }
};

class waitforlinetestApp : public application
{
public:
		 	 waitforlinetestApp (void) :
				application ("grace.testsuite.waitforline")
			 {
			 }
			~waitforlinetestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(waitforlinetestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int waitforlinetestApp::main (void)
{
	recvprocess proc;
	proc.run();
	while (! proc.running()) sleep (1);
	proc.puts ("wibble\n");
	sleep (1);
	proc.puts ("helloworld\n");
	proc.serialize();
	switch (proc.retval())
	{
		case 0:
			return 0;
		
		case 1:
			FAIL("timeout");
		
		case 2:
			FAIL("wrong buffer");
	}

	FAIL("unknown return value");
}
