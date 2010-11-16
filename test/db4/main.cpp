#include <grace/application.h>
#include <grace/filesystem.h>
#include <dbfile/db4file.h>

class db4testApp : public application
{
public:
		 	 db4testApp (void) :
				application ("grace.testsuite.db4")
			 {
			 }
			~db4testApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(db4testApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int db4testApp::main (void)
{
	db4file dbf;
	dbf.setencoding (dbfile::shox);
	if (! dbf.open ("mydb")) FAIL ("could not open db");
	
	dbf.db["pi"]["class"] = "piclass";
	dbf.db["pi"]["email"] = "pi@test.openpanel.com";
	dbf.db["peter"]["class"] = "peclass";
	dbf.db["peter"]["email"] = "peter@test.openpanel.com";
	dbf.db["jeroen"]["class"] = "jeclass";
	dbf.db["jeroen"]["email"] = "jeroen@test.openpanel.com";
	dbf.db["lennard"]["class"] = "leclass";
	dbf.db["lennard"]["email"] = "lennard@test.openpanel.com";
	dbf.commit ();
	dbf.close();
	
	db4file X;
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

	if (X.db["peter"]["class"].sval() != "peclass") FAIL ("readclass");
	
	foreach (node, X.db)
	{
		f.printf ("%s (%s): %s\n", node.id().str(),
				  node["class"].cval(), node["email"].cval());
	}
	X.close ();
	f.close ();
	
	return 0;
}

