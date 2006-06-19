#include <grace/application.h>
#include <grace/filesystem.h>

class value_byinttestApp : public application
{
public:
		 	 value_byinttestApp (void) :
				application ("grace.testsuite.value_byint")
			 {
			 }
			~value_byinttestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_byinttestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int value_byinttestApp::main (void)
{
	value test;
	test[0] = "zero";
	test[1] = "one";
	test[3] = "three";
	test.savexml ("out.xml");
	
	return 0;
}

