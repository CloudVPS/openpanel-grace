#include <grace/application.h>
#include <grace/filesystem.h>

class value_jsontestApp : public application
{
public:
		 	 value_jsontestApp (void) :
				application ("grace.testsuite.value_json")
			 {
			 }
			~value_jsontestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_jsontestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int value_jsontestApp::main (void)
{
	value v = $("test", 42) ->
			  $("shoutouts",
			  		$("brothers") ->
			  		$("sisters") ->
			  		$("nephews")) ->
			  $("scores",
			  		$("brothers", 42) ->
			  		$("grok", 16)) ->
			  $("testresults",
			  		$(true) ->
			  		$(false) ->
			  		$(true));
	
	string out = v.tojson ();
	fs.save ("out.json", out);
	
	value vv;
	vv.fromjson (out);
	vv.savexml ("out.xml");
	
	value vvv;
	vvv.fromjson ("[1,2,3,4]");
	if (vvv.count() != 4) FAIL("int array");
	
	return 0;
}

