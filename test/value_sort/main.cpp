#include <grace/application.h>

class value_sortApp : public application
{
public:
		 	 value_sortApp (void) :
				application ("grace.testsuite.value_sort")
			 {
			 }
			~value_sortApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_sortApp);

int value_sortApp::main (void)
{
	value data;
	value rback;
	data.loadxml ("in.xml");
	
	data.sort (naturalSort, "title");
	data.savexml ("natural.xml");
	
	data.sort (recordSort, "title");
	data.savexml ("record.xml");
	
	data = $("sheen",1) ->
		   $("apple",2) ->
		   $("thirsty", 3) ->
		   $("bastard", 4);
	
	data.sort (naturalLabelSort);
	data.rmval ("apple");
	
	if (data.count() == 4)
	{
		ferr.writeln ("ERR: count");
		return 1;
	}
	
	if (data.exists ("apple"))
	{
		ferr.writeln ("ERR: rmval");
		return 1;
	}
	
	return 0;
}

