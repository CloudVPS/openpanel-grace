#include <grace/application.h>
#include <grace/value.h>
#include <grace/xmlschema.h>

class value_iniApp : public application
{
public:
		 	 value_iniApp (void) :
				application ("grace.testsuite.value_ini")
			 {
			 }
			~value_iniApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_iniApp);

int value_iniApp::main (void)
{
	xmlschema inischema ("schema:grace.testsuite.value_ini.schema.xml");
	value data;
	
	if (! data.loadinitree ("input.ini"))
	{
		ferr.printf ("Error loading input.ini");
		return 1;
	}
	data.type ("valueini");
	data.savexml ("output.xml", value::nocompact, inischema);
	return 0;
}

