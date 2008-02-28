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
	
	pcregexp RE_D ("^exact$");
	if (RE_D.match ("  exact")) FAIL ("exact1");
	if (RE_D.match ("exact  ")) FAIL ("exact2");
	if (! RE_D.match ("exact")) FAIL ("exact3");
	
	pcregexp RE_E ("^[[:space:]]*[[:alnum:]]+[[:space:]]*$");
	if (RE_E.match ("$toot ")) FAIL ("classmatch1");
	if (! RE_E.match ("  theword5 ")) FAIL ("classmatch2");
	if (RE_E.match ("  test two  ")) FAIL ("classmatch3");

	pcregexp RE_F ("wimp\\$");
	if (RE_F.match ("wimp")) FAIL ("escape1");
	if (! RE_F.match (" oh wimp$ guy!")) FAIL ("escape2");
	
	return 0;
}

