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
	string res;
	
	const char *shorttext = "Hello, world\n";
	const char *midtext = "This is a longer string that exceeds "
						  "the 63 byte boundary where a vstring "
						  "and a pascal string become different "
						  "entities.";
	
	offs = out.binputvstr (offs, shorttext);
	
	res.crop();
	offs = 0;
	offs = out.bingetvstr (offs, res);
	if (! offs) FAIL("failure for bingetvstr");
	if (res != shorttext) FAIL("compare vstr(1)");
	
	out.crop();
	offs = 0;
	offs = out.binputvstr (offs, midtext);
	
	res.crop();
	offs = 0;
	offs = out.bingetvstr (offs, res);
	if (! offs) FAIL("failure for bingetvstr");
	if (res != midtext) FAIL("compare vstr(2)");
	
	return 0;
}

