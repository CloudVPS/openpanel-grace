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

statstring *stringdict::get (unsigned int key)
{
	returnclass (statstring) res retain;
	
	if (key > (unsigned int) byid.count()) return &res;
	res = byid[(int) key];
	return &res;
}

unsigned int stringdict::count (void)
{
	return bystring.count();
}

