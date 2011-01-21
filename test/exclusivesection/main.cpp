#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/lock.h>
#include <grace/thread.h>

lock<int> A;
lock<int> B;
bool failed = false;

class threadone : public thread
{
public:
				 threadone (void) : thread ("threadone")
				 {
					spawn ();
				 }
			
				~threadone (void) {}
			
	void		 run (void)
				 {
				 	for (int i=0; i<10; ++i)
				 	{
						exclusivesection (A)
						{
							A = 1;
							exclusivesection (B)
							{
								B = 1;
								sleep (1);
							}
							A = 2;
						}
					}
				 	shutcond.broadcast();
				 }

	conditional	 shutcond;
};

class threadtwo : public thread
{
public:
				 threadtwo (void) : thread ("threadtwo")
				 {
					spawn ();
				 }
			
				~threadtwo (void) {}
			
	void		 run (void)
			 	 {
			 		sleep (3);
			 		for (int i=0; i<10; ++i)
			 		{
						exclusivesection (A)
						{
							if (A != 2) failed = true;
						}
					}
			 		shutcond.broadcast();
				 }

	conditional	 shutcond;
};

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
	A.o = 0; B.o = 0;
	
	dolocked (42);
	if (! lval.trylockw (0)) FAIL("outer lock not released");
	if (! second.trylockw (0)) FAIL("inner lock not released");
	
	threadone *one = new threadone;
	threadtwo *two = new threadtwo;
	
	one->shutcond.wait();
	two->shutcond.wait();
	
	if (failed) FAIL("nested sections");
	
	return 0;
}

