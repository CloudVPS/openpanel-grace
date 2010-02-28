#include <grace/application.h>
#include <grace/filesystem.h>
#include <dbfile/gdbmfile.h>

class gdbmtestApp : public application
{
public:
		 	 gdbmtestApp (void) :
				application ("grace.testsuite.gdbm")
			 {
			 }
			~gdbmtestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(gdbmtestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int gdbmtestApp::main (void)
{
	gdbmfile dbf;
	dbf.setencoding (dbfile::shox);
	if (! dbf.open ("mydb")) FAIL ("could not open db");
	
	dbf.db["pi"]["class"] = "weenie";
	dbf.db["pi"]["email"] = "pi@test.panelsix.com";
	dbf.db["peter"]["class"] = "wanker";
	dbf.db["peter"]["email"] = "peter@test.panelsix.com";
	dbf.db["jeroen"]["class"] = "wimp";
	dbf.db["jeroen"]["email"] = "jeroen@test.panelsix.com";
	dbf.db["lennard"]["class"] = "wuss";
	dbf.db["lennard"]["email"] = "lennard@test.panelsix.com";
	dbf.commit ();
	dbf.close();
	
	gdbmfile X;
	X.setencoding (dbfile::shox);
	
	try
	{
		X.db["peter"]["class"];
		FAIL("no exception");
	}
	catch (dbfileNotOpenException e)
	{
	}
	file f;
	f.openwrite ("out.dat");
	
	if (! X.open ("mydb")) FAIL ("could not re-open db");

	if (X.db["peter"]["class"].sval() != "wanker") FAIL ("readclass");

	foreach (node, X.db)
	{
		f.printf ("%s (%s): %s\n", node.id().str(),
				  node["class"].cval(), node["email"].cval());
	}
	X.close ();
	f.close ();
	
	return 0;
}

