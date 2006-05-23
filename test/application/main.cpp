#include <grace/application.h>
#include <grace/configdb.h>
#include <stdlib.h>
#include <grace/array.h>

class apptestApp : public application
{
public:
		 	 apptestApp (void) :
				application ("grace.testsuite.application"),
				conf (this)
			 {
			 }
			~apptestApp (void)
			 {
			 }

	int		 main (void);
	
	configdb<apptestApp>	conf;
	
	bool	 handleOwner (config::action, keypath &,
						  const value &, const value &);
};

APPOBJECT(apptestApp);

int apptestApp::main (void)
{
	string loaderr;
	array<int> test;
	
	conf.addwatcher ("ownerDetails", &apptestApp::handleOwner);
	if (! conf.load ("grace.testsuite.application", loaderr))
	{
		ferr.printf ("error loading config:\n");
		ferr.writeln (loaderr);
		return 1;
	}
	
	fout.printf ("alpha=%s\n", argv["--alpha"].cval());
	fout.printf ("bravo=%s\n", argv.exists ("--bravo") ? "true" : "false");
	if (argv.exists ("--charlie"))
	{
		fout.printf ("charlie=%s\n", argv["--charlie"].cval());
	}
	for (int i=0; i<argv["*"].count(); ++i)
	{
		fout.printf ("extarg%i=%s\n", i, argv["*"][i].cval());
	}
	return 0;
}

bool apptestApp::handleOwner (config::action act, keypath &kp,
							  const value &nval, const value &oval)
{
	switch (act)
	{
		case config::isvalid:
			if (! nval.exists ("name")) return false;
			if (! nval.exists ("email")) return false;
			return true;
			
		case config::create:
			ferr.printf ("create owner.name <%s>\n", nval["name"].cval());
			ferr.printf ("create owner.email <%s>\n", nval["email"].cval());
			return true;
		
		case config::change:
			ferr.printf ("change owner.name <%s>\n", nval["name"].cval());
			ferr.printf ("change owner.email <%s>\n", nval["email"].cval());
			return true;
	}
	return false;
}
