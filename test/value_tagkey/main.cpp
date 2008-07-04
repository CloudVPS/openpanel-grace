#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/xmlschema.h>

class value_tagkeytestApp : public application
{
public:
		 	 value_tagkeytestApp (void) :
				application ("grace.testsuite.value_tagkey")
			 {
			 }
			~value_tagkeytestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_tagkeytestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int value_tagkeytestApp::main (void)
{
	xmlschema S ("schema:tagkey.schema.xml");
	fout.writeln ("%s" %format (S.tagkey()));
	value v;
	v.loadxml ("in.xml", S);
	v.savexml ("out.xml");
	return 0;
}

