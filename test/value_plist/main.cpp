#include <grace/application.h>

class value_plistApp : public application
{
public:
		 	 value_plistApp (void) :
				application ("grace.testsuite.value_plist")
			 {
			 }
			~value_plistApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_plistApp);

int value_plistApp::main (void)
{
	value data;
	value rback;
	data.loadxml ("in.xml");
	data.saveplist ("out.plist");
	rback.loadplist ("out.plist");
	rback.savexml ("out.xml");
	return 0;
}

