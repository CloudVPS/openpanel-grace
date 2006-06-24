// ========================================================================
// currency.cpp: Keyed generic data storage class
//
// (C) Copyright 2006 Pim van Riezen <pi@madscience.nl>
//                    Madscience Labs, Rotterdam 
// ========================================================================

//      012345678 TAB-O-METER should measure 4
//      ^	^

#include <grace/currency.h>
#include <grace/value.h>

#include <math.h>

// ========================================================================
// CONSTRUCTOR
// -----------
// Initializes internal value to 0.000
// ========================================================================
currency::currency (void)
{
	val = 0;
}

// ========================================================================
// COPY-CONSTRUCTOR (double)
// ========================================================================
currency::currency (double orig)
{
	val = (long long) (orig * 1000.0);
}

// ========================================================================
// COPY-CONSTRUCTOR (currency)
// ========================================================================
currency::currency (const currency &orig)
{
	val = orig.val;
}

// ========================================================================
// COPY-CONSTRUCTOR (ptr to currency)
// ========================================================================
currency::currency (currency *orig) : retainable (orig)
{
}

// ========================================================================
// COPY-CONSTRUCTOR (64 bit word assumed to be currency)
// ========================================================================
currency::currency (long long orig)
{
	val = orig;
}

// ========================================================================
// COPY-CONSTRUCTOR (value object)
// ========================================================================
currency::currency (const class value &orig)
{
	val = orig.getcurrency();
}

// ========================================================================
// COPY-CONSTRUCTOR (ptr to value object)
// ========================================================================
currency::currency (class value *orig)
{
	val = orig->getcurrency();
	delete orig;
}

// ========================================================================
// DESTRUCTOR
// ========================================================================
currency::~currency (void)
{
}

// ========================================================================
// ASSIGNMENT OPERATORS
// ========================================================================
currency &currency::operator= (long long orig)
{
	val = orig;
	return *this;
}

currency &currency::operator= (const currency &orig)
{
	val = orig.val;
	return *this;
}

currency &currency::operator= (currency *orig)
{
	retainvalue (orig);
	return *this;
}

currency &currency::operator= (double orig)
{
	val = (long long) (orig * 1000.0);
	return *this;
}

currency &currency::operator= (const class value &orig)
{
	val = orig.getcurrency();
	return *this;
}

currency &currency::operator= (class value *orig)
{
	val = orig->getcurrency();
	delete orig;
	return *this;
}

// ========================================================================
// PLUS OPERATORS
// ========================================================================
long long currency::operator+ (long long with) const
{
	return (val + with);
}

long long currency::operator+ (const currency &with) const
{
	return (val + with.val);
}

// ========================================================================
// MINUS OPERATORS
// ========================================================================
long long currency::operator- (long long with) const
{
	return (val - with);
}

long long currency::operator- (const currency &with) const
{
	return (val - with.val);
}

// ========================================================================
// INCREMENT OPERATORS
// ========================================================================
currency &currency::operator+= (long long with)
{
	val += with;
	return *this;
}

currency &currency::operator+= (const currency &with)
{
	val += with.val;
	return *this;
}

// ========================================================================
// DECREMENT OPERATORS
// ========================================================================
currency &currency::operator-= (long long with)
{
	val -= with;
	return *this;
}

currency &currency::operator-= (const currency &with)
{
	val -= with.val;
	return *this;
}

// ========================================================================
// MULTIPLICATION OPERATORS
// ========================================================================
currency &currency::operator*= (int with)
{
	long long mfact = with;
	val *= mfact;
	return *this;
}

currency &currency::operator*= (double with)
{
	long long intfactor;
	double dintfactor,fraction;
	long long fractpart;
	
	intfactor = llrint (with);
	dintfactor = intfactor;
	fraction = with - dintfactor;
	
	fractpart = llrint (((double)val )* fraction);
	val *= intfactor;
	val += fractpart;
	return *this;
}

long long currency::operator* (int with) const
{
	long long mfact = with;
	return (val * mfact);
}

long long currency::operator* (double with) const
{
	long long intfactor;
	double dintfactor,fraction;
	long long fractpart;
	
	intfactor = llrint (with);
	dintfactor = intfactor;
	fraction = with - dintfactor;

	fractpart = (long long) ((double)val * fraction);
	
	return ( (val * intfactor) + fractpart );
}

// ========================================================================
// DIVISION OPERATORS
// ========================================================================
long long currency::operator/ (int with) const
{
	long long mfact = with;
	return (val / mfact);
}

long long currency::operator/ (double with) const
{
	double dval;
	long long result;
	dval = (double) val;
	result = llrint (dval / with);
	return result;
}

// ========================================================================
// TEST OPERATORS
// ========================================================================
bool currency::operator== (const currency &other) const
{
	return (val == other.val);
}

bool currency::operator== (long long other) const
{
	return (val == other);
}

bool currency::operator!= (const currency &other) const
{
	return (val != other.val);
}

bool currency::operator!= (long long other) const
{
	return (val != other);
}

bool currency::operator< (const currency &other) const
{
	return (val < other.val);
}

bool currency::operator< (long long other) const
{
	return (val < other);
}

bool currency::operator<= (const currency &other) const
{
	return (val <= other.val);
}

bool currency::operator<= (long long other) const
{
	return (val <= other);
}

bool currency::operator> (const currency &other) const
{
	return (val > other.val);
}

bool currency::operator> (long long other) const
{
	return (val > other);
}

bool currency::operator>= (const currency &other) const
{
	return (val >= other.val);
}

bool currency::operator>= (long long other) const
{
	return (val >= other);
}

// ========================================================================
// METHOD ::str
// ------------
// Rounds the fixed-point three decimals number kept in val to the nearest
// two decimal value and returns the string representation in the format
// "42.00".
// ========================================================================
const char *currency::str (void) const
{
	int cents;
	long long absval;
	
	absval = (val<0) ? -val : val;
	cents = ((absval+5LL) % 1000) / 10;
	absval = (absval+5LL)/1000LL;
	if (val<0) absval = -absval;
	::sprintf ((char *) presentation, "%lli.%02i", absval, cents);
	return (const char *) presentation;
}

// ========================================================================
// FUNCTION printcurrency
// ----------------------
// Utility function to print a two-decimal rounded version of a fixed point
// currency value into a string object.
// ========================================================================
void printcurrency (string &into, long long val)
{
	int cents;
	long long absval;
	
	absval = (val<0) ? -val : val;
	cents = ((absval+5LL) % 1000) / 10;
	absval = (absval+5LL)/1000LL;
	if (val<0) absval = -absval;

	into.printf ("%L.%02i", absval, cents);
}

// ========================================================================
// FUNCTION parsecurrency
// ----------------------
// Utility function to convert a string representation of a two decimal
// value into a fixed point currency value.
// ========================================================================
long long parsecurrency (const string &from)
{
	const char *crsr;
	long long base;
	long long fract;
	char decimal[2];
	
	crsr = from.str();
	while ((*crsr)!='.')
	{
		if (! (*crsr))
		{
			return 1000LL * strtoll (from.str(),NULL,10);
		}
		if (! (::isdigit(*crsr)||::isspace(*crsr)))
			return 0LL;
		++crsr;
	}
	++crsr;
	
	base = strtoll (from,NULL,10);
	
	decimal[0] = '0';
	decimal[1] = '0';
	
	if (*crsr)
	{
		decimal[0] = *crsr;
		++crsr;
		if (*crsr) decimal[1] = *crsr;
	}
	
	fract = 10LL * strtoll (decimal, NULL, 10);
	return (1000LL * base) + fract;
}

