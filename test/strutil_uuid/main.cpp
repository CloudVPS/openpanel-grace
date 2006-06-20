#include <grace/application.h>
#include <grace/filesystem.h>

class strutil_uuidtestApp : public application
{
public:
		 	 strutil_uuidtestApp (void) :
				application ("grace.testsuite.strutil_uuid")
			 {
			 }
			~strutil_uuidtestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(strutil_uuidtestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int strutil_uuidtestApp::main (void)
{
	string uuid;

	uuid = strutil::uuid();
	fout.printf("%s\n", uuid.str());
	if(!uuid.validate("0123456789abcdef-"))
		FAIL("invalid characters in UUID");
	
	return 0;
}

