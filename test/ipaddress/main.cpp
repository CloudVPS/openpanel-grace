#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/netdb.h>

class ipaddresstestApp : public application
{
public:
		 	 ipaddresstestApp (void) :
				application ("grace.testsuite.ipaddress")
			 {
			 }
			~ipaddresstestApp (void)
			 {
			 }
			 
	void	 test (const value &v)
			 {
			 	fout.writeln ("%s" %format (v));
			 }

	int		 main (void);
};

APPOBJECT(ipaddresstestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int ipaddresstestApp::main (void)
{
	ipaddress addr = netdb::resolve ("localhost");
	test (netdb::resolve ("localhost"));
	
	fout.writeln ("%P" %format (netdb::resolve ("localhost")));
	
	value foo = $("address", netdb::resolve ("localhost"));
	foo.savexml ("out.xml");
	
	value x = "172.17.2.255";
	ipaddress fromval = x;
	string backval = "%P" %format (fromval);
	if (backval != "172.17.2.255") FAIL ("fromvalue");
	
	value v;
	v.loadxml ("out.xml");
	if (addr != v) FAIL ("readabck compare");
	if (v != addr) FAIL ("readback compare flip");
	
	return 0;
}

