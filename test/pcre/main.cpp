#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/pcre.h>

class pcretestApp : public application
{
public:
		 	 pcretestApp (void) :
				application ("grace.testsuite.pcre")
			 {
			 }
			~pcretestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(pcretestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int pcretestApp::main (void)
{
	pcregexp RE_A ("^(one|two|three) (one|two|three)$");
	string t = RE_A.replace ("one two","\\2 \\1");
	if (t != "two one") FAIL ("replace backref");
	
	pcregexp RE_B (".*lam.*");
	if (! RE_B.match ("klamboe")) FAIL ("matchpositive");
	if (  RE_B.match ("stamkroeg")) FAIL ("matchnegative");
	
	pcregexp RE_C ("([a-z]*) asshole");
	value v = RE_C.capture ("major asshole");
	if (v[0] != "major") FAIL ("capture");
	
	return 0;
}

