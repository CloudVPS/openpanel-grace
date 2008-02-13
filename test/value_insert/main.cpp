#include <grace/application.h>
#include <grace/filesystem.h>

class value_inserttestApp : public application
{
public:
		 	 value_inserttestApp (void) :
				application ("grace.testsuite.value_insert")
			 {
			 }
			~value_inserttestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_inserttestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int value_inserttestApp::main (void)
{
	value test;
	test[0] = "cruel";
	test[1] = "world";
	test.insertval() = "goodbye";
	test.savexml ("out.xml");
	
	return 0;
}

