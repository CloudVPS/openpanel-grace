#include <grace/application.h>
#include <grace/filesystem.h>

class stringtestApp : public application
{
public:
		 	 stringtestApp (void) :
				application ("grace.testsuite.string")
			 {
			 }
			~stringtestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(stringtestApp);

#define TESTSTR "The fat cat jumped over the lazy bitch."
#define XMLSTR "Wow, <b>what big ears</b> you have there granny!"
#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int stringtestApp::main (void)
{
	string tstr = TESTSTR;
	string tstr2 = TESTSTR;
	string stringFromString = tstr;
	statstring statFromChar = TESTSTR;
	statstring statFromString;
	statFromString = tstr;
	value valueFromString;
	valueFromString = tstr;
	value valueFromChar;
	valueFromChar = TESTSTR;
	string emptystr;
	string otherEmptystr;
	
	if (tstr.strlen() != 39) FAIL("strlen");
	if (tstr != TESTSTR) FAIL("operator!= cstr");
	if (tstr != tstr) FAIL("operator!= self");
	if (tstr != tstr2) FAIL("operator!= string");
	if (tstr != statFromChar) FAIL("operator!= statfromchar");
	if (tstr != statFromString) FAIL("operator!= statfromstring");
	if (tstr != valueFromString.sval()) FAIL("operator!= valuefromstring");
	if (! (tstr == TESTSTR)) FAIL("operator== cstr");
	if (! (tstr == tstr)) FAIL("operator== self");
	if (! (tstr == tstr2)) FAIL("operator== string");
	if (! (tstr == statFromChar)) FAIL("operator== statfromchar");
	if (! (tstr == statFromString)) FAIL("operator== statfromstring");
	if (! (tstr == valueFromString.sval())) FAIL("operator== valuefromstring");
	if (! (emptystr == "")) FAIL("operator== empty cstr_empty");
	if (! (emptystr != "test")) FAIL ("operator!= empty cstr");
	if (emptystr) FAIL ("operator bool empty");
	if (! (emptystr == otherEmptystr)) FAIL ("operator== empty empty");
	if (emptystr != otherEmptystr) FAIL ("operator!= empty empty");

	string cutdata;
	
	cutdata = tstr.cutat ("over");
	if (cutdata != "The fat cat jumped ") FAIL("cutdata");
	if (tstr != " the lazy bitch.") FAIL("leftover");
	if (tstr2.strlen() != 39) FAIL("tstr2-copy-on-write booboo");
	if (valueFromString.sval() != tstr2) FAIL("tstr2-match-valuefromstring");
	
	if (! tstr2.globcmp ("The*j?mp*lazy*")) FAIL("globcmp");
	
	tstr = tstr2;
	if (tstr != tstr2) FAIL("copy-back");
	
	tstr = XMLSTR;
	tstr.escapexml();
	tstr.unescapexml();
	if (tstr != XMLSTR) FAIL("xml-escape-unescape");
	
	tstr = tstr.encode64();
	tstr = tstr.decode64();
	if (tstr != XMLSTR) FAIL("base64-encode-decode");
	
	tstr.escape();
	tstr.unescape();
	if (tstr != XMLSTR) FAIL("escape-unescape");
	
	string result;

	#define TESTCUT(teststr,operation,arg,lresult,rresult,error) \
		{ tstr = teststr; result = tstr.operation (arg); \
		  if (result != lresult) FAIL(error); \
		  if (tstr != rresult) FAIL(error); \
		}

	TESTCUT("one.two.three",cutat,'.',"one","two.three","cutat char");
	TESTCUT("one.two.three",cutat,".","one","two.three","cutat cstr");
	TESTCUT("one.two.three",cutatlast,'.',"one.two","three","cutatlast char");
	TESTCUT("one.two.three",cutatlast,".","one.two","three","cutatlast cstr");
	TESTCUT("one.two.three",cutafter,'.',"two.three","one","cutafter char");
	TESTCUT("one.two.three",cutafter,".","two.three","one","cutafter cstr");
	TESTCUT("one.two.three",cutafterlast,'.',"three","one.two","cutafterlast char");
	TESTCUT("one.two.three",cutafterlast,".","three","one.two","cutafterlast cstr");
	TESTCUT("one.two.three",copyuntil,'.',"one","one.two.three","copyuntil char");
	TESTCUT("one.two.three",copyuntil,".","one","one.two.three","copyuntil cstr");
	TESTCUT("one.two.three",copyuntillast,'.',"one.two","one.two.three","copyuntillast char");
	TESTCUT("one.two.three",copyuntillast,".","one.two","one.two.three","copyuntillast cstr");
	TESTCUT("one.two.three",copyafter,'.',"two.three","one.two.three","copyafter char");
	TESTCUT("one.two.three",copyafter,".","two.three","one.two.three","copyafter cstr");
	TESTCUT("one.two.three",copyafterlast,'.',"three","one.two.three","copyafterlast char");
	TESTCUT("one.two.three",copyafterlast,".","three","one.two.three","copyafterlast cstr");
	
	tstr = "AbcdefgHijklklmnopqrstuvwxyz";
	tstr.ctolower ();
	if (tstr != "abcdefghijklklmnopqrstuvwxyz") FAIL("ctolower");
	
	tstr = "wiBBle";
	tstr.capitalize();
	if (tstr != "Wibble") FAIL("capitalize");

	tstr = "2001";
	if (tstr.toint() != 2001) FAIL("toint");
	
	tstr = "abcdef";
	result = tstr;
	
	tstr.pad (10,'-');
	if (tstr != "abcdef----") FAIL("pad");
	if (result != "abcdef") FAIL("pad copy-on-write");
	
	tstr = "testing";
	tstr.insert ("we are ");
	
	if (tstr != "we are testing") FAIL("insert");
	
	tstr = "0123456789";
	tstr.crop (5);
	if (tstr != "01234") FAIL("crop");
	
	tstr = "oh! my!? what the?";
	tstr.replace (" !?",'_');
	if (tstr != "oh__my___what_the_") FAIL("replace");
	
	tstr = "   wtf  ";
	tstr.chomp();
	if (tstr != "wtf")
	{
		fout.printf ("\"%s\"\n", tstr.str());
		FAIL("chomp");
	}
	
	tstr = "AB-CD";
	tstr = tstr.stripchar ('-');
	if(tstr != "ABCD") FAIL("stripchar");
	
	tstr = "AB-CD+EF";
	tstr = tstr.stripchars ("-+");
	if(tstr != "ABCDEF") FAIL("stripchars");
	
	// Test trimming
	tstr = " \n\tHELLOW\nWORLD\n\t";
	tstr = tstr.ltrim ("\n\t ");
	if(tstr != "HELLOW\nWORLD\n\t") FAIL("ltrim");
	
	//Right trim
	tstr = tstr.rtrim ("\n\t ");
	::printf(tstr.cval());
	if(tstr != "HELLOW\nWORLD") FAIL("rtrim");
	
		// Test trimming
	tstr = " \n\tHELLOW\nWORLD\n\t";
	tstr = tstr.trim ("\n\t ");
	if(tstr != "HELLOW\nWORLD") FAIL("trim");

	value rset;
	rset["\""] = "&quot;";
	rset["&"] = "&amp;";
	rset["<"] = "&lt;";
	
	tstr = "Who are \"Kool & The Gang\" again? K&TG < NOTK!";
	tstr.replace (rset);
	
	if (tstr != "Who are &quot;Kool &amp; The Gang&quot; again? "
	            "K&amp;TG &lt; NOTK!") FAIL ("replace");
	            
	rset.clear();
	rset["&quot;"] = "\"";
	rset["&amp;"] = "&";
	rset["&lt;"] = "<";
	tstr.replace (rset);
	
	if (tstr != "Who are \"Kool & The Gang\" again? K&TG < NOTK!")
		FAIL("replace2");
		
	string empty;
	if (empty.strncmp ("wibble", 6) == 0)
		FAIL("bug#40-regression");

	statstring sempty;
	statstring one ("1");
	statstring two ("wibble242");
	statstring three ("/@!5245235");
	
	value v;
	for (int i=0; i<4096; ++i)
	{
		v["test"] = (const string &) sempty;
		v["test"] = (const string &) one;
		v["case"] = (const string &) two;
		v["case"] = (const string &) sempty;
		v["test"] = (const string &) three;
	}
	
	return 0;
}

