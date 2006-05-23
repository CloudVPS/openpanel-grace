#include <grace/application.h>
#include <grace/xmlschema.h>
#include <grace/validator.h>

class value_xmlwrapApp : public application
{
public:
		 	 value_xmlwrapApp (void) :
				application ("grace.testsuite.value_xmlwrap")
			 {
			 }
			~value_xmlwrapApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_xmlwrapApp);

int value_xmlwrapApp::main (void)
{
	xmlschema S ("schema:test.schema.xml");
	validator V ("schema:test.validator.xml");
	value v;
	string error;
	
	fout.writeln ("in.xml");
	v.loadxml ("in.xml", S);
	if (! V.check (v, error))
	{
		ferr.puts (error);
		return 1;
	}
	
	fout.writeln ("in2.xml");
	v.loadxml ("in2.xml", S);
	if (V.check (v, error))
	{
		ferr.writeln ("Validation error 1 not caught\n");
		return 1;
	}
	
	fout.writeln (error);
	error.crop (0);
	
	fout.writeln ("in3.xml");
	v.loadxml ("in3.xml", S);
	if (V.check (v, error))
	{
		ferr.writeln ("Validation error 2 not caught\n");
		return 1;
	}
	
	fout.writeln (error);
	error.crop (0);

	fout.writeln ("in4.xml");
	v.loadxml ("in4.xml", S);
	if (V.check (v, error))
	{
		ferr.writeln ("Validation error 3 not caught\n");
		return 1;
	}
	
	fout.writeln (error);
	error.crop (0);

	fout.writeln ("in5.xml");
	v.loadxml ("in5.xml", S);
	if (V.check (v, error))
	{
		ferr.writeln ("Validation error 4 not caught\n");
		return 1;
	}
	fout.writeln (error);
	error.crop (0);

	fout.writeln ("in6.xml");
	v.loadxml ("in6.xml", S);
	if (! V.check (v, error))
	{
		ferr.writeln (error);
		return 1;
	}

	return 0;
}

