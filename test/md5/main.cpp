#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/md5.h>

class md5testApp : public application
{
public:
		 	 md5testApp (void) :
				application ("grace.testsuite.md5")
			 {
			 }
			~md5testApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(md5testApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int md5testApp::main (void)
{
	md5checksum md5;
	md5.append ("This is a test, thanks for listening to our input.");
	string out = md5.hex();
	fs.save ("out1.md5", out);

	md5.init();
	md5.append ("Lorem ipsum dolor sit amet, consectetuer adipiscing elit. "
				"Nam in nibh nec nisi interdum porttitor. Phasellus porta. "
				"Nunc at felis. In hac habitasse platea dictumst. Nam euismod "
				"orci quis posuere.");
	out = md5.hex();
	fs.save ("out2.md5", out);
	
	string mysalt = "CDjtBsOC";
	string mypw = "Iajtl0c";
	out = md5.md5pw (mypw.cval(), mysalt.cval());
	fout.writeln (out);
	
	return 0;
}

