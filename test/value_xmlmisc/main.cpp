#include <grace/application.h>
#include <grace/xmlschema.h>

class value_xmlmiscApp : public application
{
public:
		 	 value_xmlmiscApp (void) :
				application ("grace.testsuite.value_xmlmisc")
			 {
			 }
			~value_xmlmiscApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_xmlmiscApp);

int value_xmlmiscApp::main (void)
{
	xmlschema S ("schema:nl.madscience.test.xmlmisc.schema.xml");
	value data;
	value rback;
	data.loadxml ("in.xml", S);
	data.savexml ("out.xml", value::nocompact, S);
	data.savexml ("out2.xml");
	rback.loadxml ("out2.xml");
	rback.savexml ("out3.xml", value::nocompact, S);
	return 0;
}

