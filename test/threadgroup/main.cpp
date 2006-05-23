#include <grace/application.h>
#include <grace/thread.h>
#include <grace/lock.h>

#include <stdlib.h>

class gthread : public groupthread
{
public:
			 gthread (class threadgroupApp *papp);
			~gthread (void)
			 {
			 }
			
	void	 run (void);
			 
	class threadgroupApp *app;
};



class threadgroupApp : public application
{
public:
		 	 threadgroupApp (void) :
				application ("grace.testsuite.threadgroup")
			 {
			 	counter.o = 0;
			 	ndone.o = 0;
			 }
			~threadgroupApp (void)
			 {
			 }

	int		 main (void);
	
	threadgroup		threads;
	lock<int>		counter;
	lock<int>		ndone;
};

APPOBJECT(threadgroupApp);

gthread::gthread (threadgroupApp *papp)
   : groupthread (papp->threads)
{
   app = papp;
   spawn();
}

void gthread::run (void)
{
	sleep (1);
	app->counter.lockw();
	app->counter.o++;
	app->counter.unlock();
	sleep (rand() % 15);
	app->counter.lockw();
	app->counter.o--;
	app->counter.unlock();
	app->ndone.lockw();
	app->ndone.o++;
	app->ndone.unlock();
}


int threadgroupApp::main (void)
{
	bool done = false;
	int nrounds = 0;
	for (int i=0; i<256; ++i)
	{
		new gthread (this);
	}
	while (! done)
	{
		int c;
		int d;
		
		counter.lockr();
		c = counter.o;
		counter.unlock();
		
		ndone.lockr();
		d = ndone.o;
		ndone.unlock();
		
		fout.printf ("count=%i done=%i\n", c, d);
		if (d == 256) done = true;
		else
		{
			++nrounds;
			if (nrounds >120)
			{
				fout.printf ("timeout condition\n");
				return 1;
			}
			sleep (1);
		}
	}
	
	int ecount = 1;
	counter.lockr();
	ecount = counter.o;
	counter.unlock();
	if (ecount != 0)
	{
		fout.printf ("count not back to 0\n");
		return 1;
	}
	
	return 0;
}

