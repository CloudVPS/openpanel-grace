#include <grace/application.h>
#include <grace/xmlschema.h>

class value_xmlwrapApp : public application
{
public:
		 	 value_xmlwrapApp (void) :
				application ("grace.testsuite.value_xmlwrap")
			 {
			 }
			~value_xmlwrapApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_xmlwrapApp);

int value_xmlwrapApp::main (void)
{
	xmlschema S ("schema:test.schema.xml");
	value V;
	
	V.loadxml ("in.xml", S);
	V.savexml ("out.xml");
	V.savexml ("out2.xml", value::nocompact, S);
	return 0;
}

