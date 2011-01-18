#include <grace/application.h>
#include <grace/filesystem.h>

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int main (void)
{
	value vv;
/*
	vv.frommsgpack ( fs.load("int.msgpack") );
	vv.savexml ("int.xml");

	vv.frommsgpack ( fs.load("string.msgpack") );
	vv.savexml ("string.xml");

	vv.frommsgpack ( fs.load("array.msgpack") );
	vv.savexml ("array.xml");
*/
	vv.frommsgpack ( fs.load("in.msgpack") );
	vv.savexml ("out.xml");

	return 0;
}

