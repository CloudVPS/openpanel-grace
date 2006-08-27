#ifndef _EXCEPTION_H
#define _EXCEPTION_H 1

/// Base class for exceptions.
/// Create your own derivations using the THROWS_EXCEPTION macro,
/// which takes 3 arguments:
/// - The classname for your exception
/// - Its 'id' (In Grace we use grash to derive a hash of the class name)
/// - Its default textual error message.
/// So, for instance, if you want to define an exception in your
/// nuclear reactor control system from someone trying to open up the
/// wrong valve, you could set something up like:
/// THROWS_EXCEPTION (coreWouldExcplodeException, 0x394d42c3, "Explosion hazard");
class exception
{
public:
	exception (int c, const char *d) { code = c; description = d; }
	unsigned int code;
	const char *description;
};

#define THROWS_EXCEPTION(cls,cod,des) \
	class cls : public exception \
	{ \
	public: \
		cls (const char *o) : exception (cod, o) { } \
		cls (void) : exception (cod, des) { } \
		static unsigned int getcode (void) { return cod; } \
	};

#endif
