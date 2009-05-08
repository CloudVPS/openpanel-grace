#include <grace/application.h>
#include <grace/filesystem.h>

class fileforeachtestApp : public application
{
public:
		 	 fileforeachtestApp (void) :
				application ("grace.testsuite.fileforeach")
			 {
			 }
			~fileforeachtestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(fileforeachtestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int fileforeachtestApp::main (void)
{
	file f;
	value v;
	f.openread ("in");
	foreach (line, f) v.newval() = line;
	
	v.savexml ("out.xml");
	return 0;
}

