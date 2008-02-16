#include <grace/application.h>
#include <grace/filesystem.h>

class value_splicetestApp : public application
{
public:
		 	 value_splicetestApp (void) :
				application ("grace.testsuite.value_splice")
			 {
			 }
			~value_splicetestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_splicetestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int value_splicetestApp::main (void)
{
	value v = $("zero") ->
			  $("one") ->
			  $("two") ->
			  $("three") ->
			  $("four") ->
			  $("five") ->
			  $("six") ->
			  $("seven") ->
			  $("eight") ->
			  $("nine") ->
			  $("ten");
			  
	value outv = $("3", v.splice (3)) ->
				 $("2,2", v.splice (2,2)) ->
				 $("-3", v.splice (-3)) ->
				 $("1,15", v.splice (1,15)) ->
				 $("23,4", v.splice (23,4));
				 
	fout.writeln (outv.tojson ());
	return 0;
}

