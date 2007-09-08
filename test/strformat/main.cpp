#include <grace/application.h>
#include <grace/filesystem.h>

class strformattestApp : public application
{
public:
		 	 strformattestApp (void) :
				application ("grace.testsuite.strformat")
			 {
			 }
			~strformattestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(strformattestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int strformattestApp::main (void)
{
	string test;
	string city = "Rotterdam";
	
	test = "Hello, %s. How are things in %s?" %format ("Pim", city);
	fout.writeln (test);
	
	test = "Formatting an integer like '%s' as a string" %format (42);
	fout.writeln (test);
	
	test = "A string as an integer: %i" %format ("wibble");
	fout.writeln (test);
	
	test = "XML-encoding: %Z" %format ("<boo>yadda</boo>");
	fout.writeln (test);
	
	value tree;
	tree["one"] = 1;
	tree["two"] = 2;
	tree["3"] = "three";
	
	test = "XML-serialization: %!" %format (tree);
	fout.writeln (test);
	
	test = "XML-serialization of a string: %!" %format ("boo");
	fout.writeln (test);
	
	foreach (node, tree)
	{
		fout.writeln ("%s = '%s'" %format (node.id(), node));
	}
	
	fout.writeln ("Or even funnier:");
	
	fout.writeln ("<%s>" %format (tree.type()));
	foreach (node, tree)
	{
		fout.writeln ("  <%s id=\"%Z\">%Z</%{0}s>" %format (node.type(),
					  					node.id(), node));
	}
	fout.writeln ("</%s>" %format (tree.type()));
	
	value rec;
	rec["firstName"] = "John";
	rec["lastName"] = "Doe";
	rec["email"] = "johndoe@example.net";
	
	fout.writeln ("Name: %[firstName]s %[lastName]s <%[email]s>" %format (rec));
	fout.writeln ("%[email]7s" %format (rec));
	
	value recs;
	recs["pi@madscience.nl"]["name"] = "Pim van Riezen";
	recs[-1]["group"] = "Personal";
	recs["johndoe@example.net"]["name"] = "Jonathan Doe";
	recs[-1]["group"] = "Work";
	recs["janedoe@example.net"]["name"] = "Jane Doe";
	recs[-1]["group"] = "Work";
	recs["president@whitehouse.gov"]["name"] = "Miserable Failure";
	recs[-1]["group"] = "Official";
	
	foreach (r, recs)
	{
		fout.writeln ("%{1}26s %[name]30s %[group]s" %format (r, r.id()));
	}
	
	fout.writeln ("Longhex: %016X" %format (-1548172834131337LL));
	fout.writeln ("Longhex-from-int: %016X" %format (-3));
	
	value dd = -1548172834131337LL;
	fout.writeln ("Longobject: %!" %format (dd));
	
	return 0;
}

