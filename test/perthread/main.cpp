#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/thread.h>
#include <grace/perthread.h>

perthread<value> DB;
int outputOne;
int outputTwo;
int outputThree;
conditional threadStopped;

class testThread : public thread
{
public:
			 testThread (int *pout, int pcnt) { out = pout; cnt = pcnt; spawn(); }
			 ~testThread (void) { }
			 
	void	 run (void)
			 {
			 	::printf ("counting till %i\n", cnt);
			 	for (int i=0; i<cnt; ++i)
			 	{
				 	DB.get() = DB.get().ival() + 1;
				 }
				 *out = DB.get();
				 threadStopped.signal ();
			 }

protected:
	int		*out;
	int		 cnt;
};

class perthreadtestApp : public application
{
public:
		 	 perthreadtestApp (void) :
				application ("grace.testsuite.perthread")
			 {
			 }
			~perthreadtestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(perthreadtestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int perthreadtestApp::main (void)
{
	testThread one (&outputOne, 18321);
	testThread two (&outputTwo, 33510);
	testThread three (&outputThree, 18495);
	
	threadStopped.wait ();
	::printf ("%i %i %i\n", outputOne, outputTwo, outputThree);
	threadStopped.wait ();
	::printf ("%i %i %i\n", outputOne, outputTwo, outputThree);
	threadStopped.wait ();
	::printf ("%i %i %i\n", outputOne, outputTwo, outputThree);
	
	value out;
	out.newval() = outputOne;
	out.newval() = outputTwo;
	out.newval() = outputThree;
	
	out.savexml ("out.xml");
	return 0;
}

