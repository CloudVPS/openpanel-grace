#include <grace/application.h>
#include <grace/filesystem.h>

class value_consttestApp : public application
{
public:
		 	 value_consttestApp (void) :
				application ("grace.testsuite.value_const")
			 {
			 }
			~value_consttestApp (void)
			 {
			 }

	int		 test (const value &v)
			 {
			 	fs.save ("out.xml", v[9].toxml());
			 }
	int		 main (void);
};

APPOBJECT(value_consttestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int value_consttestApp::main (void)
{
	value v;
	v.newval();
	v.newval()["test"] = "testing";
	test (v);
	
	return 0;
}

