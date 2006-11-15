#ifndef _EXCEPTION_H
#define _EXCEPTION_H 1

#include <string.h>
#include <stdlib.h>

/// Base class for exceptions.
/// Create your own derivations using the THROWS_EXCEPTION macro,
/// which takes 3 arguments:
/// - The classname for your exception
/// - Its 'id' (In Grace we use genhash to get a hash of the class name)
/// - Its default textual error message.
/// So, for instance, if you want to define an exception in your
/// nuclear reactor control system from someone trying to open up the
/// wrong valve, you could set something up like:
/// THROWS_EXCEPTION (coreWouldExplodeException, 0x394d42c3, "Explosion hazard");
class exception
{
public:
	exception (int c, const char *d)
	{
		code = c;
		description = ::strdup (d);
	}
	
	~exception (void)
	{
		free (description);
	}
	
	/// The exception code. Useful for handling a specific class of
	/// exception. Exceptions defined through the THROWS_EXCEPTION macro
	/// have a static method getcode() so you can do things like:
	/// \verbinclude exception_ex.cpp
	unsigned int code;
	
	/// A textual description of the exception thrown. An exception class
	/// through THROWS_EXCEPTION points this at a default string, but
	/// its constructor allows for a custom string. So you can do stuff
	/// like this:
	/// \verbinclude exception_ex2.cpp
	char *description;
};

/// Macro for defining a custom exception class.
#define THROWS_EXCEPTION(cls,cod,des) \
	class cls : public exception \
	{ \
	public: \
		cls (const char *o) : exception (cod, o) { } \
		cls (void) : exception (cod, des) { } \
		static unsigned int getcode (void) { return cod; } \
	};

#endif
