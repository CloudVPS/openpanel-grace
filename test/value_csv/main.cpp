#include <grace/application.h>

class value_csvApp : public application
{
public:
		 	 value_csvApp (void) :
				application ("grace.testsuite.value_csv")
			 {
			 }
			~value_csvApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_csvApp);

int value_csvApp::main (void)
{
	value foo;
	foo.loadcsv ("in.csv",true,"id");
	foo.savexml ("out.xml");
	if (! foo.savecsv ("out.csv"))
	{
		ferr.printf ("Error saving\n");
		return 1;
	}
	return 0;
}
