#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/fswatch.h>

class fswatchtestApp : public application
{
public:
		 	 fswatchtestApp (void) :
				application ("grace.testsuite.fswatch")
			 {
			 }
			~fswatchtestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(fswatchtestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int fswatchtestApp::main (void)
{
	fs.rm ("outfile");
	fswatch fsw (".");
	
	string tmp = "boe";
	fs.save ("outfile", tmp);
	value v = fsw.listchanges ();
	
	if (! v[fschange::created].exists ("outfile"))
	{
		FAIL("created file not detected");
	}
	
	sleep (1);
	fs.save ("outfile", tmp);
	
	v = fsw.listchanges ();
	
	if (! v[fschange::modified].exists ("outfile"))
	{
		FAIL("modified file not detected");
	}
	
	fs.rm ("outfile");
	
	v = fsw.listchanges ();

	if (! v[fschange::deleted].exists ("outfile"))
	{
		FAIL("deleted file not detected");
	}
	
	return 0;
}

