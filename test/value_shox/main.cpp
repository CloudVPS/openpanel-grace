#include <grace/application.h>
#include <grace/filesystem.h>

class value_shoxtestApp : public application
{
public:
		 	 value_shoxtestApp (void) :
				application ("grace.testsuite.value_shox")
			 {
			 }
			~value_shoxtestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_shoxtestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int value_shoxtestApp::main (void)
{
	value v;
	v["test"] = 42;
	v["list"].newval() = "foo";
	v["list"].newval() = "bar";
	v["list"].newval() = "baz";
	v["abool"] = true;
	v["afloat"] = 1.337;
	
	string out;
	v.saveshox ("out.shox");
	
	value vv;
	
	vv.loadshox ("out.shox");
	
	fout.printf ("loaded\n");
	//sleep (5);
	vv.savexml ("out.xml");
	vv["list"].savexml ("out2.xml");
	
	return 0;
}

