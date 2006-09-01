#include "svalidate.h"
#include <grace/application.h>
#include <grace/validator.h>
#include <grace/xmlschema.h>

APPOBJECT(svalidateApp);

int svalidateApp::main (void)
{
	validator V ("rsrc:grace.root.validator.xml");
	validator VV ("rsrc:grace.validator.validator.xml");
	xmlschema S (XMLRootSchemaType);
	xmlschema SV (XMLValidatorSchemaType);
	value     d;
	string	  err;
	string	  fn;
	bool verbose = false;
	
	if (argv.exists ("--verbose")) verbose = true;
	
	if (! argv["*"].count())
	{
		ferr.printf ("Usage: svalidate <schema.xml>\n");
		return 1;
	}
	
	fn = argv["*"][0];
	if (fn.strstr (".validator.xml") >= 0)
	{
		d.loadxml (fn, SV);
		if (verbose) ferr.printf ("Validating schema...");
		
		if (! VV.check (d, err))
		{
			if (verbose) ferr.printf (" failed\n");
			ferr.writeln (err);
			return 1;
		}
		if (verbose) ferr.printf (" ok\n");
		fout.printf ("Schema OK\n");
		return 0;
	}
	else
	{
		d.loadxml (argv["*"][0].sval(), S);
	
		if (verbose) ferr.printf ("Validating schema...");
			
		if (! V.check (d, err))
		{
			if (verbose) ferr.printf (" failed\n");
			ferr.writeln (err);
			return 1;
		}
	}
	
	if (verbose) ferr.printf (" ok\n");
	
	foreach (cl, d)
	{
		if (cl.type() == "xml.class")
		{
			if (verbose) ferr.printf ("Checking %s...", cl.name());
			if (cl.exists ("xml.proplist"))
			{
				foreach (member, cl["xml.proplist"])
				{
					if (! member.attribexists ("type"))
					{
						if (! d.exists (member.id()))
						{
							if (verbose) ferr.printf (" failed\n");
							ferr.printf ("No definition for class <%s> found\n",
										 member.name());
							return 1;
						}
					}
				}
			}
			if (verbose) ferr.printf (" ok\n");
		}
	}
	
	fout.printf ("Schema OK\n");
	return 0;
}

