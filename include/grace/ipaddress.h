#ifndef _IPADDRESS_H
#define _IPADDRESS_H 1

#include <grace/value.h>

/// Representation of an IPv4 address.
/// This class is meant to be passed-by-value. It only contains
/// a single unsigned int and mainly acts as a convenient type-
/// indicator when handling with ip-addresses in the context of
/// value objects, for different encoding standards where there
/// needs to be a choice between writing an integer (shox, SQL)
/// or a string (XML). A plain unsigned int doesn't have those
/// needs and it is hard to dinstinguish the two if an ipaddress
/// where just a typedef.
/// The constructors and operators implemented should make this
/// a drop-in replacement for wielding sockaddr_in or unsigned
/// ints.
class ipaddress
{
public:
						 /// Default constructor. Sets the value
						 /// to 0.0.0.0
						 ipaddress (void)
						 {
						 	addr = 0;
						 }
						 
						 /// Constructor from unsigned int.
						 ipaddress (unsigned int o)
						 {
						 	addr = o;
						 }
						 
						 /// Constructor from a plain int (ugly).
						 ipaddress (int o)
						 {
						 	addr = o;
						 }
						 
						 /// Constructor from a string. 
						 ipaddress (const string &o)
						 {
						 	addr = str2ip (o);
						 }
						 
						 /// Constructor from a c-string. Needed
						 /// because gcc can't figure out whether
						 /// to cast it to value or string otherwise.
						 /// Where's #pragma this_one_bitch when you
						 /// need it?.
						 ipaddress (const char *o)
						 {
						 	addr = str2ip (o);
						 }
						 
						 /// Copy-constructor.
						 ipaddress (const ipaddress &o)
						 {
						 	addr = o.addr;
						 }
						 
						 /// Constructor from a value.
						 /// Just calls ipval().
						 ipaddress (const value &o)
						 {
						 	addr = o.ipval ();
						 }
						 
						 /// Boring destructor.
						~ipaddress (void)
						 {
						 }
						
						 /// Assignment operator.
						 /// \param o The original.
	ipaddress			&operator= (const ipaddress &o)
						 {
						 	addr = o.addr;
						 	return *this;
						 }
						 
	ipaddress			&operator= (const value &v)
						 {
						 	addr = v.ipval();
						 	return *this;
						 }
						 
	ipaddress			&operator= (int i)
						 {
						 	addr = i;
						 	return *this;
						 }
						 
	ipaddress			&operator= (const char *c)
						 {
						 	addr = str2ip (c);
						 	return *this;
						 }
						 
						 /// Cast-o-matic operator to unsigned int.
						 operator unsigned int (void) const
						 {
						 	return addr;
						 }
						 
						 /// Increment operator.
						 /// \todo Implement prefix/postfix properly.
	ipaddress			&operator++ (int i)
						 {
						 	addr++;
						 	return *this;
						 }

						 /// Decrement operator.
						 /// \todo Implement prefix/postfix properly.
	ipaddress			&operator-- (int i)
						 {
						 	addr--;
						 	return *this;
						 }

	ipaddress			&operator+= (const ipaddress &o)
						 {
						 	addr += o.addr;
						 	return *this;
						 }

	ipaddress			&operator-= (const ipaddress &o)
						 {
						 	addr -= o.addr;
						 	return *this;
						 }

	ipaddress			&operator&= (const ipaddress &o)
						 {
						 	addr &= o.addr;
						 	return *this;
						 }

	ipaddress			&operator|= (const ipaddress &o)
						 {
						 	addr |= o.addr;
						 	return *this;
						 }
						 
						 #define INTOP(thetype) \
						 	bool operator== (thetype o) const \
						 	{ \
						 		return o == (thetype) addr; \
						 	} \
						 	bool operator!= (thetype o) const \
						 	{ \
						 		return o != (thetype) addr; \
						 	}
						 
						 INTOP (int)
						 INTOP (unsigned int)
						 
						 
	bool				 operator== (const value &o) const
						 {
						 	return o.ipval() == addr;
						 }
						 
	bool				 operator!= (const value &o) const
						 {
						 	return o.ipval() != addr;
						 }
						 
	bool				 operator== (const ipaddress &o) const
						 {
						 	return addr == o.addr;
						 }
	
	bool				 operator!= (const ipaddress &o) const
						 {
						 	return addr != o.addr;
						 }
						 
protected:
	unsigned int		 addr; ///< The IPv4 address in host format.
};

#endif
