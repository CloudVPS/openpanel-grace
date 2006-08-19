#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/value.h>

class appathtestApp : public application
{
public:
		 	 appathtestApp (void) :
				application ("grace.testsuite.appath")
			 {
			 }
			~appathtestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(appathtestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int appathtestApp::main (void)
{
	value v;
	v.loadxml ("rsrc:test.rsrc.xml");
	if (! v.count()) FAIL("misspuss");
	return 0;
}

