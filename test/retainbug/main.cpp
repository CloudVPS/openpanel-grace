#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/file.h>

class retainbugtestApp : public application
{
public:
		 	 retainbugtestApp (void) :
				application ("grace.testsuite.retainbug")
			 {
			 }
			~retainbugtestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(retainbugtestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

string *testing (void)
{
	returnclass (string) res retain;
	return &res;
}

int retainbugtestApp::main (void)
{
	value v;
	v = "in.txt";
	
	try
	{
		file f;
		if (1)
		{
			f.openread (v.sval());
			string test;
			
			testing();
			
			test = "abcdefghijkl";
			for (int i=0; i<800; ++i)
			{
				test.strcat (" \t");
				test.trim (" \t");
			}
		}
	}
	catch (memoryLeakException e)
	{
		return 0;
	}
	
	FAIL ("leak not caught\n");
}

