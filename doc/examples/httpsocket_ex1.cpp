#include <grace/application.h>
#include <grace/http.h>
#include <grace/str.h>

class leechApp : public application
{
public:
			 leechApp (void) : application ("tld.example.app.leech")
			 {
			 }
			~leechApp (void)
			 {
			 }
			 
	int		 main (void);
};

APPOBJECT(leechApp);

int leechApp::main (void)
{
	httpsocket hs;
	string returnData;
	string url;
	
	url = argv["*"][0];
	if (! url.strlen)
	{
		ferr.printf ("Usage: leech <url>\n"); return 1;
	}
	
	returnData = hs.get (url);
	fout.puts (returnData);
	return 0;
}
