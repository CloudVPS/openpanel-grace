#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/xmlschema.h>

class value_xmlbase64testApp : public application
{
public:
		 	 value_xmlbase64testApp (void) :
				application ("grace.testsuite.value_xmlbase64")
			 {
			 }
			~value_xmlbase64testApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_xmlbase64testApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int value_xmlbase64testApp::main (void)
{
	string gifImage;
	value outval;
	value inval;
	xmlschema sch("schema:test.schema.xml");
	
	gifImage = fs.load ("in.gif");
	if (! gifImage.strlen())
	{
		FAIL("could not load image");
	}
	
	outval["image"] = gifImage;
	outval["description"] = "An asshat";
	outval.savexml ("out.xml", value::nocompact, sch);
	
	inval.loadxml ("out.xml", sch);
	gifImage.crop ();
	if (gifImage.strlen())
	{
		FAIL("error cropping string");
	}
	gifImage = inval["image"];
	fs.save ("out.gif", gifImage);
	
	return 0;
}

