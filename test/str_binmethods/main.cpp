#include <grace/application.h>
#include <grace/filesystem.h>

class str_binmethodstestApp : public application
{
public:
		 	 str_binmethodstestApp (void) :
				application ("grace.testsuite.str_binmethods")
			 {
			 }
			~str_binmethodstestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(str_binmethodstestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int str_binmethodstestApp::main (void)
{
	size_t offs = 0;
	string out;
	
	offs = out.binputvstr (offs, "Hello, world\n");
	fout.printf ("%S\n", out.str());
	
	out.crop();
	offs = 0;
	offs = out.binputvstr (offs, "This is a longer string that exceeds "
								 "the 63 byte boundary where a vstring "
								 "and a pascal string become different "
								 "entities.");
	fout.printf ("%S\n", out.str());
	
	string res;
	offs = 0;
	offs = out.bingetvstr (offs, res);
	if (! offs) FAIL("failure for bingetvstr");
	
	fout.writeln (res);
	
	return 0;
}

