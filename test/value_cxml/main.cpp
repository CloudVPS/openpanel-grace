#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/xmlschema.h>

class value_cxmltestApp : public application
{
public:
		 	 value_cxmltestApp (void) :
				application ("grace.testsuite.value_cxml")
			 {
			 }
			~value_cxmltestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_cxmltestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int value_cxmltestApp::main (void)
{
	xmlschema S ("schema:nl.madscience.test.cxml.schema.xml");
	value V;
	value VV;
	string cxml;
	
	V.loadxml ("in.xml", S);
	cxml = V.tocxml (S);
	fs.save ("out.cxml", cxml);
	VV.fromcxml (cxml, S);
	VV.savexml ("out.xml", value::nocompact, S);
	
	return 0;
}

