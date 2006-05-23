#include <grace/application.h>
#include <grace/filesystem.h>

class value_asciitestApp : public application
{
public:
		 	 value_asciitestApp (void) :
				application ("grace.testsuite.value_ascii")
			 {
			 }
			~value_asciitestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_asciitestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int value_asciitestApp::main (void)
{
	value v;
	v.loadxml ("in.xml");
	v.save ("out.dat");
	
	value w;
	w.load ("out.dat");
	w.savexml ("out.xml");
	return 0;
}

