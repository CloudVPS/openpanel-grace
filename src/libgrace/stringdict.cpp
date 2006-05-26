#include <grace/dictionary.h>
#include <grace/stringdict.h>

stringdict::stringdict (void)
{
}

stringdict::~stringdict (void)
{
}

unsigned int stringdict::get (const statstring &id)
{
	unsigned int cnt;
	if (bystring.exists (id)) return bystring[id];
	byid.add (new statstring (id));
	cnt = byid.count() -1;
	bystring[id] = cnt;
	return cnt;
}

const statstring &stringdict::get (unsigned int key)
{
	static statstring nil;
	
	if (key > byid.count()) return nil;
	return byid[(int) key];
}

unsigned int stringdict::count (void)
{
	return bystring.count();
}

