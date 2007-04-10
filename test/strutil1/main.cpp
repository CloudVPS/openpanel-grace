#include <grace/application.h>
#include <grace/filesystem.h>

class strutil1App : public application
{
public:
		 	 strutil1App (void) :
				application ("grace.testsuite.strutil1")
			 {
			 }
			~strutil1App (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(strutil1App);

#define FAIL(bar) { ferr.printf (bar); return 1; }

int strutil1App::main (void)
{
	value v;
	string encoded;
	value decoded;
	
	v.loadxml ("in.xml");
	encoded = strutil::httpurlencode (v);
	fs.save ("out.dat", encoded);
	decoded = strutil::httpurldecode (encoded);
	decoded.savexml ("out.xml", value::nocompact);
	
	string splitme;
	splitme = "one two three four";
	
	v = strutil::splitspace (splitme);
	if (v[0] != "one") FAIL("splitspace");
	if (v[1] != "two") FAIL("splitspace");
	if (v[3] != "four") FAIL("splitspace");
	
	splitme = "one,two,three,four";
	v = strutil::split (splitme, ',');
	if (v[0] != "one") FAIL("split char");
	if (v[1] != "two") FAIL("split char");
	if (v[3] != "four") FAIL("split char");

	v = strutil::split (splitme, ",");
	if (v[0] != "one") FAIL("split str");
	if (v[1] != "two") FAIL("split str");
	if (v[3] != "four") FAIL("split str");
	
	splitme = "one,\"two,three\",four,five";
	v = strutil::splitquoted (splitme, ',');
	if (v[0] != "one") FAIL("splitquoted");
	if (v[1] != "two,three") FAIL("splitquoted");
	if (v[3] != "five") FAIL("splitquoted");
	
	string encodeme;
	string res;
	
	encodeme = "test \"this\" dude";
	res = strutil::encodecsv (encodeme);
	if (res != "test \"\"this\"\" dude") FAIL("encodecsv");

	splitme = "1,\"two \"\"little\"\" chickens\",\"three\"";
	v = strutil::splitcsv (splitme);
	if (v[0] != 1) FAIL("splitcsv");
	if (v[1] != "two \"little\" chickens") FAIL("splitcsv");
	if (v[2] != "three") FAIL("splitcsv");
	
	string hdr;
	hdr = "Content-type: application/x-foobar; lame=\"yes\"";
	v = strutil::parsehdr (hdr);
	if (! v.exists ("Content-type")) FAIL("parsehdr");
	if (v["Content-type"]("lame") != "yes") FAIL("parsehdr");
	if (v["Content-type"] != "application/x-foobar") FAIL("parsehdr");

	splitme = "one\r\ntwo\nthree\r\nfour\r\n";
	v = strutil::splitlines (splitme);

	if (v[0] != "one") FAIL("splitlines");
	if (v[1] != "two\nthree") FAIL("splitlines");
	if (v[2] != "four") FAIL("splitlines");
	
	if (v.count() != 3) FAIL("splitlines");
	
	splitme = "one\r\ntwo";
	v = strutil::splitlines (splitme);
	if (v.count() != 2) FAIL("splitlines unterminated");

	string formdata;
	formdata = "hey <you> fool!\nwhat is this?\n";
	res = strutil::htmlize (formdata);
	
	if (res != "hey &lt;you&gt; fool!<br>\nwhat is this?<br>\n")
	{
		fout.writeln (res);
		FAIL("htmlize");
	}
	
	string wrapme;
	
	wrapme = "There are three infallible ways of pleasing an author, and "
			 "the three form a rising scale of compilement: 1, to tell "
			 "him you have read one of his books; 2, to tell him you "
			 "have read all of his books; 3, to ask him to let you read "
			 "the manuscript of his forthcoming book. No. 1 admits you "
			 "to his respect; No. 2 admits you to his admiration; No. 3 "
			 "carries you clear into his heart.";
	
	res = strutil::wrap (wrapme, 72);
	fs.save ("wrapped.txt", res);
	
	v.clear();
	v["one"] = "1";
	v["two"] = 2;
	v["three"] = "three";
	
	string parseme = "$one$, $two$, $three$!";
	
	res = strutil::valueparse (parseme, v);
	
	if (res != "1, 2, three!") FAIL("valueparse");
	
	v.clear();
	
	string nvs = "name=John; address = \"1 Brick Road\"; type = upper class;"
				 "answer=42;test=\"1;2;3\"";
	
	v = strutil::parsenv (nvs);
	
	if (v["name"] != "John") FAIL("parsenv1");
	if (v["address"] != "1 Brick Road") FAIL("parsenv2");
	if (v["type"] != "upper class") FAIL("parsenv3");
	if (v["answer"] != "42") FAIL("parsenv4");
	if (v["test"] != "1;2;3") FAIL("parsenv5");
	
	return 0;
}
