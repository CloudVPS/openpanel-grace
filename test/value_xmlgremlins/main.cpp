#include <grace/application.h>
#include <grace/filesystem.h>

class value_xmlgremlinstestApp : public application
{
public:
		 	 value_xmlgremlinstestApp (void) :
				application ("grace.testsuite.value_xmlgremlins")
			 {
			 }
			~value_xmlgremlinstestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_xmlgremlinstestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int value_xmlgremlinstestApp::main (void)
{
	value myval;
	
	myval.loadxml ("in1.xml");
	myval.loadxml ("in2.xml");
	myval.loadxml ("in3.xml");
	myval.loadxml ("in4.xml");
	myval.savexml ("out.xml", value::compact);
	
	return 0;
}

