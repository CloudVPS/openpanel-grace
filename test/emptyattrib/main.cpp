#include <grace/application.h>
#include <grace/filesystem.h>

class emptyattribtestApp : public application
{
public:
		 	 emptyattribtestApp (void) :
				application ("grace.testsuite.emptyattrib")
			 {
			 }
			~emptyattribtestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(emptyattribtestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int emptyattribtestApp::main (void)
{
	value v;
	v.loadxml ("in.xml");
	v.savexml ("out.xml");
	
	return 0;
}

