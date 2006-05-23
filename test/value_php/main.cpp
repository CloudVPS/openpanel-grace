#include <grace/application.h>
#include <grace/filesystem.h>

class value_phpApp : public application
{
public:
		 	 value_phpApp (void) :
				application ("grace.testsuite.value_php")
			 {
			 }
			~value_phpApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_phpApp);

int value_phpApp::main (void)
{
	value data;
	string sdat;
	value rback;
	data.loadxml ("in.xml");
	sdat = data.phpserialize(true);
	fs.save ("out.phpdat", sdat);
	rback.phpdeserialize (sdat);
	rback.savexml ("out.xml");
	return 0;
}

