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
	lock<int>    second;
	void		 dolocked (int i);
	int			 main (void);
};

APPOBJECT(exclusivesectiontestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

void exclusivesectiontestApp::dolocked (int i)
{
	exclusivesection (lval)
	{
		exclusivesection (second)
		{
			lval = i;
			return;
		}
	}
}

int exclusivesectiontestApp::main (void)
{
	dolocked (42);
	if (! lval.trylockw (0)) FAIL("outer lock not released");
	if (! second.trylockw (0)) FAIL("inner lock not released");
	return 0;
}

