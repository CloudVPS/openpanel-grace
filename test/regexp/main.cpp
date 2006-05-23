#include <grace/application.h>
#include <grace/filesystem.h>

class regexpApp : public application
{
public:
		 	 regexpApp (void) :
				application ("grace.testsuite.regexp")
			 {
			 }
			~regexpApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(regexpApp);

int regexpApp::main (void)
{
	string tstr;
	
	regexpression re ("^[[:digit:]]{4}[[:alpha:]]{2}");

	if (! re.eval ("3038XX"))
	{
		ferr.printf ("fail1\n"); return 1;
	}
	if (re.eval ("173YA"))
	{
		ferr.printf ("fail2\n"); return 1;
	}
	if (re.eval ("This is a test"))
	{
		ferr.printf ("fail3\n"); return 1;
	}
	
	tstr = "This is a dumb test";
	tstr = strutil::regexp (tstr, "s/dumb/cool/");
	if (tstr != "This is a cool test")
	{
		ferr.printf ("fail4\n");
	}
	return 0;
}

