#include <grace/application.h>
#include <grace/filesystem.h>

class value_arraymergetestApp : public application
{
public:
		 	 value_arraymergetestApp (void) :
				application ("grace.testsuite.value_arraymerge")
			 {
			 }
			~value_arraymergetestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_arraymergetestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int value_arraymergetestApp::main (void)
{
	value arrayone = $("one")->$("two")->$("three");
	value emptyarray;
	value arraytwo = $("four")->$("five");
	arrayone << emptyarray;
	arrayone << arraytwo;
	arrayone.savexml ("out.xml");
	return 0;
}
