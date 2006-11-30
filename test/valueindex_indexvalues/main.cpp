#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/valueindex.h>

class valueindex_indexvaluestestApp : public application
{
public:
		 	 valueindex_indexvaluestestApp (void) :
				application ("grace.testsuite.valueindex_indexvalues")
			 {
			 }
			~valueindex_indexvaluestestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(valueindex_indexvaluestestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int valueindex_indexvaluestestApp::main (void)
{
    value a;
    valueindex b;

    a["a"]="abc";
    a["e"]="efg";

    b.indexvalues(a);
            
    fout.printf("%d %d %d\n", b.exists("a"), b.exists("abc"), b.exists("qqq"));

	return 0;
}

