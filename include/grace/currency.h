// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

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
					 /// Constructor.
					 currency (void);
					 
					 /// Copy constructor.
					 currency (const currency &);
					 
					 /// Copy constructor for retained currency.
					 currency (currency *);
					 
					 /// Copy constructor for currency cast as 64
					 /// bits fixed point decimal.
					 currency (long long);
					 
					 /// Copy constructor from a value.
					 currency (const class value &);
					 
					 /// Copy constructor from a retained value.
					 currency (class value *);
					 
					 /// Copy constructor from a double precision float.
					 currency (double);
					 
					 /// Destructor.
					~currency (void);
	
					 //@{
					 /// Assignment.
	currency		&operator= (long long); 
	currency		&operator= (const currency &);
	currency		&operator= (currency *);
	currency		&operator= (const class value &);
	currency		&operator= (class value *);
	currency		&operator= (double);
					 //@}
	
					 //@{
					 /// Mathematical operator overload.
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
	currency		&operator*= (const currency &);
	currency		&operator*= (long long);
	
	currency		&operator/= (int);
	currency		&operator/= (double);
	currency		&operator/= (const currency &);
	currency		&operator/= (long long);
	
	long long		 operator* (int) const;
	long long		 operator* (double) const;
	long long		 operator* (const currency &) const;
	long long		 operator* (long long) const;
	
	long long		 operator/ (int) const;
	long long		 operator/ (double) const;
	long long		 operator/ (const currency &) const;
	long long		 operator/ (long long) const;
					 //@}
	
					 /// Cast to 64 bits fixed point with 3 decimals.
					 operator long long (void) { return val; }
	
					 //@{
					 /// Comparison operator overload.
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
					 //@}

					 /// Convert to string. Prints with two
					 /// digits behind the decimal point, rounded
					 /// up from .005.
	const char		*str (void) const;
	
					 /// Alias to currency::str()
	const char		*cval (void) const { return str(); };

					 /// Cast to 64 bits fixed point with 3 decimals.
	long long		 value (void) const { return val; };
	
	virtual void	 init (bool first);

protected:
	long long		 val; ///< Internal fixed point representation.
	char			 presentation[24]; ///< Printed version.
};

#endif
