#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/thread.h>
#include <grace/system.h>

class producer : public thread
{
public:
				 producer (void) : thread ("producer")
				 {
				 	spawn ();
				 }
			 
				 ~producer (void) {}
			 
	void		 run (void);
	value		*get (void);
	
protected:
	lock<value>	 db;
};

class value_threadcopytestApp : public application
{
public:
		 	 value_threadcopytestApp (void) :
				application ("grace.testsuite.value_threadcopy")
			 {
			 }
			~value_threadcopytestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(value_threadcopytestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int value_threadcopytestApp::main (void)
{
	producer P;
	producer Q;
	producer R;
	value v;
	
	__musleep (100);
	
	for (int i=0; i<500; ++i)
	{
		if ((i % 100) == 0) fout.writeln ("%i" %format (i));
		value v = P.get();
		v = Q.get();
		v = R.get();
		v.savexml ("v.xml");
		__musleep (rand() & 25);
	}
	return 0;
}

value *producer::get (void)
{
	returnclass (value) res retain;
	sharedsection (db)
	{
		res = db;
	}
	return &res;
}

void producer::run (void)
{
	while (true)
	{
		exclusivesection (db)
		{
			db.clear ();
			for (int i=0; i<64; ++i)
			{
				statstring key = strutil::uuid ();
				string value = strutil::uuid ();
				
				db[key] = value;
			}
		}
		core.sh ("/usr/bin/true");
	}
}
