#include <grace/application.h>
#include <grace/filesystem.h>

class arraybuildertestApp : public application
{
public:
		 	 arraybuildertestApp (void) :
				application ("grace.testsuite.arraybuilder")
			 {
			 }
			~arraybuildertestApp (void)
			 {
			 }

	int		 main (void);
};

$appobject (arraybuildertestApp);
$version (1.2);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

value *getdeferror (void)
{
    return $("code", 42) ->
           $("error", "Storpel failed");
}

int arraybuildertestApp::main (void)
{
	value includeme = $("included", true);
	value v =  $("name", "John Woe")
			-> $("address", "Dark Alley")
			-> $("email", "john@woe.org")
			-> $("url", "http://www.woe.org/tree/")
			-> $("friends", $type("list")
						 -> $attr("ordering","random")
						 -> $("jdoe","John Doe")
			  		     -> $("janed","Jane Doe"))
			-> $("level", 11)
			-> $merge(includeme);
	
	fout.writeln (v["friends"].type());
	v.savexml ("out.xml");
	
	v = getdeferror ();
	v.savexml ("out2.xml");
	
	return 0;
}

