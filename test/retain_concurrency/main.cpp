#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/thread.h>
#include <grace/process.h>

lock<int> finishedCount;

class testThread : public thread
{
public:
	testThread (void) : thread ("testThread")
	{
		spawn();
	}
	~testThread (void)
	{
	}
	static value *dowork (void)
	{
		returnclass (value) res retain;
		
		systemprocess proc ((string) "ps axuw");
		proc.run ();
		while (! proc.eof())
		{
			res.newval() = proc.gets();
		}
		proc.serialize();
		return &res;
	}
	void run (void)
	{
		value v;
		
		for (int i=0; i<1024; ++i)
		{
			v = dowork();
		}
		
		exclusivesection (finishedCount)
		{
			finishedCount++;
		}
	}
};

class retain_concurrencytestApp : public application
{
public:
		 	 retain_concurrencytestApp (void) :
				application ("grace.testsuite.retain_concurrency")
			 {
			 }
			~retain_concurrencytestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(retain_concurrencytestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int retain_concurrencytestApp::main (void)
{
	exclusivesection (finishedCount) finishedCount = 0;
	int waitTimeout = 0;

	new testThread;
	new testThread;
	new testThread;
	new testThread;
	new testThread;
	new testThread;
	new testThread;
	new testThread;
	new testThread;
	new testThread;
	new testThread;
	new testThread;
	
	while (true)
	{
		sharedsection (finishedCount)
		{
			if (finishedCount == 12) breaksection return 0;
			if (finishedCount > 0)
			{
				waitTimeout++;
				if (waitTimeout > 16)
				{
					breaksection
					{
						FAIL("timeout");
					}
				}
			}
		}
		sleep (1);
	}
	return 0;
}

