#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/valuable.h>

class myrecord : public valuable
{
public:
				 myrecord (void)
				 {
				 	name = "";
				 	email = "";
				 	level = 0;
				 }
				~myrecord (void)
				 {
				 }
				 myrecord (const value &v)
				 {
				 	fromvalue (v);
				 }
				 myrecord (value *v)
				 {
				 	fromvalue (v);
				 }
	
	void		 setname (const string &s) { name = s; }
	void		 setemail (const string &s) { email = s; }
	void		 setlevel (int i) { level = i; }
	
	void		 print (file &to)
				 {
				 	to.printf ("User: %s <%s>\n", name.str(), email.str());
				 	to.printf ("Level: %i\n", level);
				 }
	
protected:
	virtual void fromvalue (const value &v)
				 {
				 	name = v["name"];
				 	email = v["email"];
				 	level = v["level"];
				 }
	virtual void tovalue (value &v)
				 {
				 	v["name"] = name;
				 	v["email"] = email;
				 	v["level"] = level;
				 }
	
	string		 name;
	string		 email;
	int			 level;
};

class valuabletestApp : public application
{
public:
		 		 valuabletestApp (void) :
					application ("grace.testsuite.valuable")
				 {
				 }
				~valuabletestApp (void)
				 {
				 }

	int			 main (void);
	myrecord	*makerecord (void);
};

APPOBJECT(valuabletestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

myrecord *valuabletestApp::makerecord (void)
{
	returnclass (myrecord) rec retain;
	rec.setname ("John");
	rec.setemail ("john@doe.org");
	rec.setlevel (15);
	return &rec;
}

int valuabletestApp::main (void)
{
	value v = makerecord ();
	v.savexml ("out.xml");
	
	myrecord nrec;
	nrec = v;
	nrec.print (fout);
	
	value vv;
	vv = nrec;
	vv.savexml ("out2.xml");
	return 0;
}

