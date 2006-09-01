#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/value.h>

class value_xmlbadformtestApp : public application
{
public:
		 	 value_xmlbadformtestApp (void) :
				application ("grace.testsuite.value_xmlbadform")
			 {
			 }
			~value_xmlbadformtestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_xmlbadformtestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int value_xmlbadformtestApp::main (void)
{
	string err;
	value v;
	
	if (v.loadxml ("in.xml", err))
		FAIL ("bad-ass xml triggered no error");
	
	err.strcat ("\n");
	fs.save ("out.txt", err);
	return 0;
}

