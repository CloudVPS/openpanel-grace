#include <grace/application.h>
#include <grace/thread.h>
#include <grace/lock.h>

#include <stdlib.h>

class stresstestApp : public application
{
public:
		 	 stresstestApp (void) :
				application ("grace.testsuite.value_stresstest2")
			 {
			 }
			~stresstestApp (void)
			 {
			 }

	int		 main (void);
	void	 mkrand (string &);
};

APPOBJECT(stresstestApp);

int stresstestApp::main (void)
{
	string rndstring;
	int rndnum;
	value left;
	value right;
	int i,j;
	
	for (j=0; j<8; ++j)
	{
		ferr.printf (".");
		for (i=0; i<16; ++i)
		{
			mkrand (rndstring);
			rndnum = rand();
			left[rndstring] = rndnum;
		}
		
		mkrand (rndstring);
		if (right) left[rndstring] = right;
	
		for (i=0; i<16; ++i)
		{
			mkrand (rndstring);
			rndnum = rand();
			right[rndstring] = rndnum;
		}
		
		mkrand (rndstring);
		right[rndstring] = left;
	}
	
	left.savexml ("out1.xml");
	left.clear();
	right.clear();
	left.savexml ("out.xml");
	return 0;
}

void stresstestApp::mkrand (string &into)
{
	into.crop (0);
	for (int i=0; i< (rand()&31); ++i)
	{
		into.strcat ((char)('a'+rand()%26));
	}
}
