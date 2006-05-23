#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/statstring.h>
#include <grace/currency.h>
#include <grace/tcpsocket.h>


class boolcaststestApp : public application
{
public:
		 	 boolcaststestApp (void) :
				application ("grace.testsuite.boolcasts")
			 {
			 }
			~boolcaststestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(boolcaststestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int boolcaststestApp::main (void)
{
	value v;
	statstring ss;
	string str;
	currency cc;
	tcpsocket ts;
	file f;
	
	if (v) FAIL ("value");
	if (ss) FAIL ("statstring");
	if (str) FAIL ("string");
	if (cc) FAIL ("currency");
	if (ts) FAIL ("tcpsocket");
	if (f) FAIL ("file");
	
	return 0;
}

