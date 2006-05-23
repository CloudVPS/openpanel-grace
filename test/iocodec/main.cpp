#include <grace/application.h>
#include <grace/filesystem.h>

class iocodecApp : public application
{
public:
		 	 iocodecApp (void) :
				application ("grace.testsuite.iocodec")
			 {
			 }
			~iocodecApp (void)
			 {
			 }

	int		 main (void);
};

class leetxorcodec : public iocodec
{
public:
			 leetxorcodec (unsigned char pkey)
			 {
			 	key = pkey;
			 }
			~leetxorcodec (void)
			 {
			 }

	bool	 setup (void) {return true; }			 
	
	bool	 addinput (const char *dt, size_t sz)
			 {
			 	if (rbuf.room() <= sz) return false;
			 	
			 	char *tbuf = new char[sz+1];
			 	for (int i=0; i<sz; ++i)
			 	{
			 		tbuf[i] = dt[i] ^ key;
			 	}
			 	rbuf.add (tbuf, sz);
			 	delete[] tbuf;
			 	return true;
			 }
	bool	 addoutput (const char *dt, size_t sz)
			 {
			 	if (wbuf.room() <= sz) return false;
			 	
			 	char *tbuf = new char[sz+1];
			 	for (int i=0; i<sz; ++i)
			 	{
			 		tbuf[i] = dt[i] ^ key;
			 	}
			 	wbuf.add (tbuf, sz);
			 	delete[] tbuf;
			 	return true;
			 }
	void	 addclose (void)
			 {
			 }
			 
	void	 fetchinput (ringbuffer &into)
			 {
			 	size_t amt;
			 	amt = rbuf.backlog();
			 	if (amt > into.room()) amt = into.room();
			 	if (!amt) return;
			 	
			 	string tstr;
			 	tstr = rbuf.read (amt);
			 	into.add (tstr.str(), tstr.strlen());
			 }
	void	 peekoutput (string &into)
			 {
			 	into = wbuf.peek (wbuf.backlog());
			 }
	void	 doneoutput (unsigned int sz)
			 {
			 	wbuf.advance (sz);
			 }
	bool	 canoutput (unsigned int sz)
			 {
			 	return (wbuf.room() >= sz);
			 }
			 
	
protected:
	ringbuffer		 rbuf;
	ringbuffer		 wbuf;
	unsigned char	 key;
};

APPOBJECT(iocodecApp);

int iocodecApp::main (void)
{
	file f;
	string str;
	
	f.codec = new leetxorcodec (42);
	if (! f.openwrite ("out1.txt"))
	{
		ferr.printf ("couldn't open output file\n");
		return 1;
	}
	f.printf ("This is a test, please disregard\n");
	f.printf ("Have a nice day\n");
	f.close();
	
	if (! f.openread ("out1.txt"))
	{
		ferr.printf ("couldn't open file back for reading\n");
		return 1;
	}
	
	str = f.read (4096);
	f.close();
	
	fs.save ("out2.txt", str);
	return 0;
}

