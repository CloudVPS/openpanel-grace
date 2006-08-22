#include <grace/application.h>
#include <grace/filesystem.h>

class value_xmlescapetestApp : public application
{
public:
		 	 value_xmlescapetestApp (void) :
				application ("grace.testsuite.value_xmlescape")
			 {
			 }
			~value_xmlescapetestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_xmlescapetestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int value_xmlescapetestApp::main (void)
{
	value tst;
	tst.loadxml ("in.xml");
	tst["Nasty backslash\\"] = "some data";
	tst["John \"Quotes\" Quoter"] = "a dork";
	tst.savexml ("out.xml");
	tst.loadxml ("out.xml");
	tst.savexml ("out.xml");
	return 0;
}

