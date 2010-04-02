#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/file.h>
#include <grace/defaults.h>

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

bool LEAKHANDLED;

void leakhandler (void)
{
	LEAKHANDLED = true;
}

int retainbugtestApp::main (void)
{
	LEAKHANDLED = false;
	value v;
	v = "in.txt";
	
	defaults::memory::leakprotection = false;
	defaults::memory::leakcallback = leakhandler;
	
	file f;
	for (int i=0; (i<1024) && (! LEAKHANDLED); ++i)
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

	if (! LEAKHANDLED) FAIL ("leak not caught\n");
	return 0;
}

