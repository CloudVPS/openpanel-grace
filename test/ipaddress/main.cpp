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

#define FAIL(foo) { ferr.printf ("TEST FAILED:" foo "\n"); return 1; }

int ipaddresstestApp::main (void)
{
	
	{
		ipaddress zero;
		if (zero) FAIL("bool cast");
	}
	
	{
		ipaddress v4 = "172.17.2.255";
		if (!v4) FAIL("bool cast (inv)");
		
		if (!v4.isv4()) FAIL("v4");
		
		ipaddress v4_2 = "::FFFF:172.17.2.255";
		if (!v4_2) FAIL("bool cast (inv,ipv6 format)");
		if (!v4_2.isv4()) FAIL("v4 (ipv6 format)");
		
		if( v4 != v4_2 ) FAIL("v4 eq v6 format");
	}
	
	{
		ipaddress v6 = "cdea:8712:5ef5:36fd:a91:2ba2:b09e:9321";
		if (!v6) FAIL("bool cast (inv)");
		
		if (v6.isv4()) FAIL("!v4");
	}
	
	{
		value v = ipaddress("127.0.0.1");
		
		if( v != "127.0.0.1" ) FAIL("value conversion 1");
		if( v != ipaddress("127.0.0.1") ) FAIL("value conversion 2");
		
		v = "127.0.0.1";
		if( v != ipaddress("127.0.0.1") ) FAIL("value conversion 3");

		if( v.ipval() != ipaddress("127.0.0.1") ) FAIL("value conversion 4")		
	}
	
	{
		value v = ipaddress("::1");
		if( v != "::1" ) FAIL("ipv6 value conversion 1");
		if( v != ipaddress("::1") ) FAIL("ipv6 value conversion 2");
		
		v = "::1";
		if( v != ipaddress("::1") ) FAIL("ipv6 value conversion 3");

		if( v.ipval() != ipaddress("::1") ) FAIL("ipv6 value conversion 4")		
	}

	{
		ipaddress a("10.0.90.12");
		ipaddress mask("255.255.255.0");
		
		if( (a & mask ) != ipaddress("10.0.90.0") ) FAIL("Netmask 1");
		if( (a | mask ) != ipaddress("255.255.255.12") ) FAIL("Netmask 2");
		
		a &= mask;
		if( a != ipaddress("10.0.90.0") ) FAIL("Netmask 3");
		
		a |= ipaddress("0.0.0.12");
		if( a != ipaddress("10.0.90.12") ) FAIL("Netmask 4");
	}

	{
		ipaddress a("10.0.90.12");
		
		a[15] = 1;
		if( a != ipaddress("10.0.90.1") ) FAIL("Array access 1");

		a[-1] = 0;
		if( a != ipaddress("10.0.90.0") ) FAIL("Array access 2");
	}


	
	ipaddress addr = netdb::resolve ("localhost");
	test (netdb::resolve ("localhost"));
	
	if( !addr ) FAIL("lookup localhost")
	
	
	fout.writeln ("%P" %format (netdb::resolve ("localhost")));
	
	value foo = $("address", netdb::resolve ("localhost"));
	foo.savexml ("out.xml");
	
	value v;
	v.loadxml ("out.xml");
	v.savexml ("out2.xml");

	if (addr != v) FAIL ("readabck compare");
	if (v != addr) FAIL ("readback compare flip");

	value x = "172.17.2.255";
	ipaddress fromval = x;
	string backval = "%P" %format (fromval);
	if (backval != "172.17.2.255") FAIL ("fromvalue");
	
	v.clear ();
	v.newval() = ipaddress ("1.2.3.4");
	v.newval() = ipaddress ("cdea:8712:5ef5:36fd:a91:2ba2:b09e:9321");
	v.savexml ("addrlist.xml");
	string s = v.tojson();
	
	value vv;
	vv.loadxml ("addrlist.xml");

	if (vv[0] != v[0]) FAIL ("readback1-compare1");
	if (vv[1] != v[1]) FAIL ("readback1-compare2");
	
	fs.rm ("addrlist.xml");
	
	vv.fromjson (s);

	if (vv[0] != v[0]) FAIL ("readback2-compare1");
	if (vv[1] != v[1]) FAIL ("readback2-compare2");
	
	s = v.toshox();
	vv.fromshox (s);
	
	if (vv[0] != v[0]) FAIL ("readback3-compare1");
	if (vv[1] != v[1]) FAIL ("readback3-compare2");
	
	return 0;
}

