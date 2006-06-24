#include <grace/str.h>
#include <grace/retain.h>

void printcurrency (string &, long long);
long long parsecurrency (const string &);

#ifndef _CURRENCY_H
#define _CURRENCY_H 1

/// Class representing a numbered amount with a fixed decimal point.
/// Internally, the amount is stored with three decimal places of
/// accuracy and uses this for calculations. When converting the
/// currency for display, however, it is rounded to two decimal
/// places.
class currency : public memory::retainable
{
public:
					 currency (void);
					 currency (const currency &);
					 currency (currency *);
					 currency (long long);
					 currency (const class value &);
					 currency (class value *);
					 currency (double);
					~currency (void);
			
	currency		&operator= (long long);
	currency		&operator= (const currency &);
	currency		&operator= (currency *);
	currency		&operator= (const class value &);
	currency		&operator= (class value *);
	currency		&operator= (double);
	
	long long		 operator+ (long long) const;
	long long		 operator+ (const currency &) const;
	
	long long		 operator- (long long) const;
	long long		 operator- (const currency &) const;
	
	currency		&operator+= (long long);
	currency		&operator+= (const currency &);
	
	currency		&operator-= (long long);
	currency		&operator-= (const currency &);
	
	currency		&operator*= (int);
	currency		&operator*= (double);
	currency		&operator/= (int);
	currency		&operator/= (double);
	
	long long		 operator* (int) const;
	long long		 operator* (double) const;
	long long		 operator/ (int) const;
	long long		 operator/ (double) const;
	
					 operator long long (void) { return val; }
	
	bool			 operator== (const currency &) const;
	bool			 operator== (long long) const;
	bool			 operator!= (const currency &) const;
	bool			 operator!= (long long) const;
	bool			 operator<  (const currency &) const;
	bool			 operator<  (long long) const;
	bool			 operator<= (const currency &) const;
	bool			 operator<= (long long) const;
	bool			 operator>  (const currency &) const;
	bool			 operator>  (long long) const;
	bool			 operator>= (const currency &) const;
	bool			 operator>= (long long) const;

	const char		*str (void) const;
	const char		*cval (void) const { return str(); };

	long long		 value (void) const { return val; };

protected:
	long long		 val;
	char			 presentation[24];
};

#endif
