#include <grace/application.h>
#include <grace/xmlschema.h>

class value_xmlrpcApp : public application
{
public:
		 	 value_xmlrpcApp (void) :
				application ("grace.testsuite.value_xmlrpc")
			 {
			 }
			~value_xmlrpcApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_xmlrpcApp);

int value_xmlrpcApp::main (void)
{
	xmlschema xmlrpcschema ("schema:org.xmlrpc.schema.xml");
	value data;
	value rback;
	data.loadxml ("in.xml");
	data.savexml ("out.xmlrpc", value::nocompact, xmlrpcschema);
	rback.loadxml ("out.xmlrpc", xmlrpcschema);
	rback.savexml ("out.xml");
	return 0;
}

