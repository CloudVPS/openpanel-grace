#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/session.h>

class sessiontestApp : public application
{
public:
		 	 sessiontestApp (void) :
				application ("grace.testsuite.session")
			 {
			 }
			~sessiontestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(sessiontestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int sessiontestApp::main (void)
{
	sessionlist sdb;
	
	statstring sess1, sess2;
	value parm;
	
	parm["username"] = "john";
	sess1 = sdb.create (parm);
	parm["username"] = "steve";
	sess2 = sdb.create (parm);
	
	sleep (2);
	
	value sdat;
	sdat = sdb.get (sess1);
	sdat["didstuff"] = true;
	sdb.set (sess1, sdat);
	
	sleep (2);
	sdb.expire (2);
	
	if (sdb.exists (sess2))
		FAIL("session not expired");
	
	if (! sdb.exists (sess1))
		FAIL("active session expired");
	
	return 0;
}

