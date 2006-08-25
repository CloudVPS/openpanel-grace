#include <grace/dictionary.h>
#include <grace/stringdict.h>

// ========================================================================
// CONSTRUCTOR stringdict
// ========================================================================
stringdict::stringdict (void)
{
}

// ========================================================================
// DESTRUCTOR stringdict
// ========================================================================
stringdict::~stringdict (void)
{
}

// ========================================================================
// METHOD ::get
// ========================================================================
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

// ========================================================================
// METHOD ::count
// ========================================================================
unsigned int stringdict::count (void)
{
	return bystring.count();
}

