#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/xmlschema.h>

class value_xmlimpliedtestApp : public application
{
public:
		 	 value_xmlimpliedtestApp (void) :
				application ("grace.testsuite.value_xmlimplied")
			 {
			 }
			~value_xmlimpliedtestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_xmlimpliedtestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int value_xmlimpliedtestApp::main (void)
{
	xmlschema S ("schema:test.schema.xml");
	value v;
	
	v["someString"] = "Hello, world.";
	v["someInteger"] = 42;
	v["someCollection"]["someString"] = "Gobble";
	v["someCollection"]["subCollection"]["someInteger"] = 23;
	v.savexml ("out.xml", value::nocompact, S);
	
	value vv;
	vv.loadxml ("out.xml", S);
	vv.savexml ("out2.xml");
	
	return 0;
}

