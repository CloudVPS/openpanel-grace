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
	
	// Expose regression of a bug with string;:bingetvint on a string with
	// inner offset.
	string cropme = "abcde";
	value encodeme = $("hello","world") -> $("answer",42);
	cropme.strcat (encodeme.toshox());
	cropme = cropme.mid (5);
	value decoded;
	decoded.fromshox (cropme);
	if (decoded["hello"] != "world") FAIL ("fromshox with offset");
	if (decoded["answer"] != 42) FAIL ("fromshox with offset (int)");
	
	return 0;
}

