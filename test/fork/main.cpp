#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/process.h>
#include <grace/thread.h>

void alarmhandler (int sig)
{
	_exit (1);
}

class forktestApp : public application
{
public:
		 		 forktestApp (void) :
					application ("grace.testsuite.fork")
				 {
				 	counter.o = 0;
				 }
				~forktestApp (void)
				 {
				 }

	threadgroup	 forkers;
	int			 main (void);
	lock<int>	 counter;
};

class forker : public groupthread
{
public:
				 forker (forktestApp *a);
				~forker (void);
				
	void		 run (void);

protected:
	forktestApp	&app;
};

APPOBJECT(forktestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int forktestApp::main (void)
{
	for (int i=0; i<4; ++i)
	{
		new forker (this);
	}
	
	while (true)
	{
		sleep (3);
		int i;
		sharedsection (counter) i = counter;
		if (i>=4000) break;
		ferr.printf ("%i\n", i);
	}
	
	return 0;
}

forker::forker (forktestApp *a) : groupthread (a->forkers), app (*a)
{
	spawn ();
}

forker::~forker (void)
{
}

void forker::run (void)
{
	for (int i=0; i<1000; ++i)
	{
		systemprocess proc ((string)"id", false);
		proc.run ();
		string line = proc.gets ();
		proc.close ();
		proc.serialize ();
		
		lock<int> &cnt = app.counter;
		exclusivesection (cnt)
		{
			cnt++;
		}
	}
}

