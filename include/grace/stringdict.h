#ifndef _STRINGDICT_H
#define _STRINGDICT_H 1

#include <grace/value.h>
#include <grace/array.h>

/// Keeps a collection of text strings, with each unique string getting
/// a unique serial number. Used for keeping compact serialization.
class stringdict
{
public:
						 stringdict (void);
						~stringdict (void);
				
	unsigned int		 get (const statstring &);
	const statstring	&get (unsigned int);
	unsigned int		 count (void);

protected:
	value			 	 bystring;
	array<statstring>	 byid;
};

#endif
