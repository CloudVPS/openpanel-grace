#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/currency.h>
#include <grace/xmlschema.h>

class currencytestApp : public application
{
public:
		 	 currencytestApp (void) :
				application ("grace.testsuite.currency")
			 {
			 }
			~currencytestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(currencytestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int currencytestApp::main (void)
{
	currency base = 137.14;
	currency net;
	currency vat;
	currency monthly;
	xmlschema S ("schema:grace.testsuite.currency.schema.xml");

	vat = base * 0.185;
	net = base + vat;
	monthly = net / 12;
	
	value out;
	value in;
	
	out["base"] = base;
	out["vat"] = vat;
	out["net"] = net;
	out["monthly"] = monthly;
	out.savexml ("out.xml");
	
	in.loadxml ("out.xml");
	in.savexml ("out2.xml", value::nocompact, S);
	in.clear();
	in.loadxml ("out2.xml", S);
	in.type ("dict");
	in.savexml ("out3.xml");
	return 0;
}

