#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/lock.h>

class exclusivesectiontestApp : public application
{
public:
		 		 exclusivesectiontestApp (void) :
					application ("grace.testsuite.exclusivesection")
				 {
				 }
				~exclusivesectiontestApp (void)
				 {
				 }

	lock<int>	 lval;
	void		 dolocked (int i);
	int			 main (void);
};

APPOBJECT(exclusivesectiontestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

void exclusivesectiontestApp::dolocked (int i)
{
	exclusivesection (lval)
	{
		lval = i;
		return;
	}
}

int exclusivesectiontestApp::main (void)
{
	dolocked (42);
	if (! lval.trylockw (0)) FAIL("lock not released");
	return 0;
}

