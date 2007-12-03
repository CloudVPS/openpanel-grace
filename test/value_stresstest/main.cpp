#include <grace/application.h>
#include <grace/thread.h>
#include <grace/lock.h>

#include <stdlib.h>

extern void dumpstringref (int);

class gthread : public groupthread
{
public:
			 gthread (class stresstestApp *papp);
			~gthread (void)
			 {
			 }
			
	void	 run (void);
			 
	class stresstestApp *app;
};

class collector : public thread
{
public:
			 collector (class stresstestApp *papp);
			~collector (void)
			 {
			 }
			 
	void	 run (void);
	
	class stresstestApp *app;
	value				 data;
};

class stresstestApp : public application
{
public:
		 	 stresstestApp (void) :
				application ("grace.testsuite.value_stresstest")
			 {
			 	ndone.o = 0;
			 	resultCount = 0;
			 }
			~stresstestApp (void)
			 {
			 }

	int		 main (void);
	
	unsigned int	 resultCount;
	lock<int>		 ndone;
	threadgroup		 threads;
	collector		*collectorThread;
};

APPOBJECT(stresstestApp);

gthread::gthread (stresstestApp *papp)
   : groupthread (papp->threads)
{
   app = papp;
   spawn();
}

void gthread::run (void)
{
	value ev;
	string tid;
	tid.printf ("%i", threadid());
	ev["thread"] = tid;
	
	srand (::time (NULL) ^ threadid());
	
	for (int i=0; i<8; ++i)
	{
		ev["command"] = "start";
		app->collectorThread->sendevent(ev);
		for (int j=0; j<16; ++j)
		{
			int count = i+1;
			int tsleep = rand() & 31;
			
			ev["command"] = "count";
			ev["count"] = count;
			app->collectorThread->sendevent (ev);
			
			if (tsleep == 0) sleep (1);
		}
		ev.rmval ("count");
		ev["command"] = "stop";
		app->collectorThread->sendevent(ev);
		sleep (1);
	}
	app->ndone.lockw();
	app->ndone.o++;
	app->ndone.unlock();
}

collector::collector (stresstestApp *papp)
	: thread ()
{
	app = papp;
	spawn ();
}

void collector::run (void)
{
	value ev;
	statstring tid;
	unsigned int totalCount = 0;
	
	bool shouldRun = true;
	
	while (shouldRun)
	{
		ev = nextevent();
		tid = ev["thread"].sval();
		
		if (ev["command"] == "start")
		{
			if (data.exists (tid)) ferr.printf ("unit still found: %s\n", tid.str());
			data[tid]["count"] = 0;
		}
		else if (ev["command"] == "count")
		{
			data[tid]["count"] = data[tid]["count"].ival() + ev["count"].ival();
		}
		else if (ev["command"] == "stop")
		{
			totalCount += data[tid]["count"].ival();
			data.rmval (tid);
			if (data.exists (tid))
			{
				ferr.printf ("ass!@$#!@$\n");
			}
		}
		else if (ev["command"] == "die")
		{
			fout.printf ("%u\n", totalCount);
			app->resultCount = totalCount;
			shouldRun = false;
		}
	}
}


int stresstestApp::main (void)
{
	bool done = false;
	int nrounds = 0;
	
	collectorThread = new collector (this);
	
	for (int i=0; i<32; ++i)
	{
		new gthread (this);
	}
	while (ndone.o < 32)
	{
		sleep (1);
	}
	value outev;
	outev["command"] = "die";
	collectorThread->sendevent (outev);
	while (! resultCount) sleep (1);
	if (resultCount != 18432)
	{
		ferr.printf ("wrong result, expected 18432\n");
		return 1;
	}
	dumpstringref(1);
	return 0;
}

