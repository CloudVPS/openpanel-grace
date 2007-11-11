#ifndef _IPADDRESS_H
#define _IPADDRESS_H 1

#include <grace/valuable.h>
#include <grace/value.h>

class ipaddress : public valuable
{
public:
						 ipaddress (void)
						 {
						 	addr = 0;
						 }
						 ipaddress (unsigned int o)
						 {
						 	addr = o;
						 }
						 ipaddress (int o)
						 {
						 	addr = o;
						 }
						 ipaddress (const string &o)
						 {
						 	addr = str2ip (o);
						 }
						 ipaddress (const char *o)
						 {
						 	addr = str2ip (o);
						 }
						 ipaddress (const ipaddress &o)
						 {
						 	addr = o.addr;
						 }
						 ipaddress (const value &o)
						 {
						 	addr = o.ipval ();
						 }
						~ipaddress (void)
						 {
						 }
						
	ipaddress			&operator= (const ipaddress &o)
						 {
						 	addr = o.addr;
						 }
						 
						 operator unsigned int (void)
						 {
						 	return addr;
						 }
						 
	void				 tovalue (value &into) const
						 {
						 	into.setip (addr);
						 }
	
	ipaddress			 operator+ (ipaddress i) const
						 {
						 	return (ipaddress) (addr+i);
						 }
	ipaddress			 operator- (ipaddress i) const
						 {
						 	return (ipaddress) (addr-i);
						 }
	ipaddress			 operator& (ipaddress o) const
						 {
						 	return (ipaddress) (addr & o.addr);
						 }
	ipaddress			 operator| (ipaddress o) const
						 {
						 	return (ipaddress) (addr | o.addr);
						 }
	
	ipaddress			&operator++ (int i)
						 {
						 	addr++;
						 	return *this;
						 }
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
	
protected:
	unsigned int		 addr;
};

#endif
