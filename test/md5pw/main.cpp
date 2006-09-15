#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/md5.h>

class md5pwtestApp : public application
{
public:
		 	 md5pwtestApp (void) :
				application ("grace.testsuite.md5pw")
			 {
			 }
			~md5pwtestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(md5pwtestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int md5pwtestApp::main (void)
{
	md5checksum md5;
    string in, out;

	in="foobarbaz";
	out = md5.md5pw(in, "babylon");
	fs.save("out.dat", out);
	return 0;
}

