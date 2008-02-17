#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/system.h>
#include <grace/timestamp.h>

class timestamptestApp : public application
{
public:
		 	 timestamptestApp (void) :
				application ("grace.testsuite.timestamp")
			 {
			 }
			~timestamptestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(timestamptestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int timestamptestApp::main (void)
{
	timestamp tsa, tsb, tsresult;

	tsa = kernel.time.now();		//	set timestamp from a current: time_t
	// sleep(1);	
	tsb = kernel.time.unow();		//  set a timestamp from a current: timeval
	// tsb-=tsa;

	fout.printf ("A: %s\n", tsa.format("%s").cval());
	fout.printf ("B: %s\n", tsb.format("%s").cval());
	fout.printf ("A: usecs: %U #\n", tsa.getusec());
	fout.printf ("B: usecs: %U #\n", tsb.getusec());
	
	// test sub
	tsresult = tsb - tsa;
	::printf ("Operator - : result : %llu \n", tsresult.getusec());
	if(tsresult.getusec() == 0)
	{
		// try it again, there is this 1:1000000 chance that
		// this was just a fluke and we managed to measure tsb
		// at exactly 0usec past the second.
		tsa = kernel.time.now();
		tsb = kernel.time.unow();
		tsresult = tsb - tsa;
		if (tsresult.getusec() == 0)
		{
			fout.printf ("Failed operator -\n");
			return 1;
		}
	}
	
	// Test comparisons
	if ((tsa == tsa) && (tsa != tsb)) {
		fout.printf ("Comparison == \tok\n");
		fout.printf ("Comparison != \tok\n");
	} else {
		fout.printf ("Comparison == \tFAILED!\n");
		fout.printf ("Comparison != \tFAILED!\n");
		return 1;
	}
	
	// Test comparisons
	if (tsa < tsb) {
		fout.printf ("Comparison < \tok\n");
	} else {
		fout.printf ("Comparison < \tFAILED!\n");
		return 1;
	}

	// Test comparisons
	if (tsa <= tsb) {
		fout.printf ("Comparison <= \tok\n");
	} else {
		fout.printf ("Comparison <= \tFAILED!\n");
		return 1;
	}
	
	// Test comparisons
	if (! (tsa >= tsb)) {
		fout.printf ("Comparison >= \tok\n");
	} else {
		fout.printf ("Comparison >= \tFAILED!\n");
		return 1;
	}

	// Test comparisons
	if (! (tsa > tsb)) {
		fout.printf ("Comparison > \tok\n");
	} else {
		fout.printf ("Comparison > \tFAILED!\n");
		return 1;
	}


	// test sleepuntil function
	time_t testt = kernel.time.now();
	testt += 12; // Add 12 seconds for testing
	timestamp destTime(testt);
	
	fout.printf("%S\n", destTime.ctime().cval());

	fout.printf ("*** Sleeping approx 12 seconds ****\n");
	kernel.time.sleepuntil (destTime);
	fout.printf ("*** Ending sleepuntil()        ****\n*\n");
	fout.printf ("* If this was not close to 12 seconds, your\n");
	fout.printf ("* library sucks! \n*\n\n");
	
	timestamp parsed1, parsed2;
	parsed1.iso ("2007-12-24T13:39:15");
	parsed2.iso ("2008-12-16T11:15:15");
	fout.writeln ("delta: %i" %format (parsed1.delta (parsed2, days)));
	
	value out = $("parsed1", parsed1) ->
				$("parsed2", parsed2);
				
	fout.writeln (out.toxml());
	
	value in;
	in.fromxml (out.toxml());
	
	timestamp pback = in["parsed1"];
	fout.writeln (pback.ctime());
	in.savexml ("out.xml");
	
	return 0;
}

