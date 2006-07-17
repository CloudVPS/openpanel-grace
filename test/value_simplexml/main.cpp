#include <grace/application.h>
#include <grace/filesystem.h>

class value_simplexmltestApp : public application
{
public:
		 	 value_simplexmltestApp (void) :
				application ("grace.testsuite.value_simplexml")
			 {
			 }
			~value_simplexmltestApp (void)
			 {
			 }

	int		 main (void);
	value	*test (void);
};

APPOBJECT(value_simplexmltestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int value_simplexmltestApp::main (void)
{
	value r = test();
	r.savexml ("out.xml");
	return 0;
}

value *value_simplexmltestApp::test (void)
{
	returnclass (value) res retain;
	
	res["test"] = "wibble";
	return &res;
}
