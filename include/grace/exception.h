#ifndef _EXCEPTION_H
#define _EXCEPTION_H 1

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
