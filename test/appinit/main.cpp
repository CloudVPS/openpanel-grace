#include <grace/application.h>
#include <grace/filesystem.h>

class appinittestApp : public application
{
public:
		 	 appinittestApp (void) :
				application ("grace.testsuite.appinit")
			 {
			 }
			~appinittestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(appinittestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int myglobal;

extern "C" void grace_init (void)
{
	myglobal = 42;
}

int appinittestApp::main (void)
{
	if (myglobal != 42)
	{
		FAIL("global not set");
	}
	return 0;
}

