#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/process.h>

class nonewlineApp : public application
{
public:
		 	 nonewlineApp (void) :
				application ("grace.testsuite.nonewline")
			 {
			 }
			~nonewlineApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(nonewlineApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int nonewlineApp::main (void)
{
	string s = fin.gets ();
	
	if(s.strlen())
	{
		return 0;
	}
	else
	{
		FAIL("got empty line")
	}
}
