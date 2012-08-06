#include <grace/application.h>
#include <grace/filesystem.h>

class value_emptystrtestApp : public application
{
public:
		 	 value_emptystrtestApp (void) :
				application ("grace.testsuite.value_emptystr")
			 {
			 }
			~value_emptystrtestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_emptystrtestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int value_emptystrtestApp::main (void)
{
	value a = $("lockd",$("host","waldorf"));
	value b;
	b.loadxml ("in.xml");
	
	if (a["lockd"]["host"] != b["lockd"]["host"]) return 0;
	FAIL("empty-cmp");
	return 0;
}

