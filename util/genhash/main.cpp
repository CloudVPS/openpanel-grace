#include "genhash.h"
#include <grace/checksum.h>
#include <grace/exception.h>

APPOBJECT(genhashApp);

//  =========================================================================
/// Main method.
//  =========================================================================
int genhashApp::main (void)
{
	int i;
	
	if (argv["*"].count())
	{
		for (i=0; i<argv["*"].count(); ++i)
		{
			printhash (argv["--hash-type"], argv["*"][i]);
		}
	}
	else
	{
		string buffer;
		try
		{
			while (! fin.eof())
			{
				string line = fin.read (4096);
				if (! line.strlen()) break;
				buffer.strcat (line);
			}
		}
		catch (exception e)
		{
			ferr.printf ("Exception caught: %s\n", e.description);
		}
		
		printhash (argv["--hash-type"], buffer);
	}
	return 0;
}

void genhashApp::printhash (const string &tp, const string &dt)
{
	caseselector (tp)
	{
		incaseof ("grace") :
			fout.printf ("0x%08x\n", checksum (dt));
			break;
		
		incaseof ("grace64") :
			fout.printf ("0x%016llx\n", checksum64(dt));
			break;
			
		incaseof ("djb64") :
			fout.printf ("0x%016llx\n", djbhash64(dt));
			break;
			
		defaultcase :
			ferr.printf ("Unknown hash-type\n");
			exit (1);
			
	}
}
