// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _TRIPLET_H
#define _TRIPLET_H 1

#include <grace/str.h>
#include <grace/value.h>

#include <string.h>
#include <ctype.h>

typedef unsigned short triplet;
typedef char[3] ctriplet;

class _triplet_static_array
{
public:
					_triplet_array (void)
					{
						_array[' '] = 0;
						_array['a'] = 1;
						_array['b'] = 2;
						_array['c'] = 3;
						_array['d'] = 4;
						_array['e'] = 5;
						_array['f'] = 6;
						_array['g'] = 7;
						_array['h'] = 8;
						_array['i'] = 9;
						_array['j'] = 10;
						_array['k'] = 11;
						_array['l'] = 12;
						_array['m'] = 13;
						_array['n'] = 14;
						_array['o'] = 15;
						_array['p'] = 16;
						_array['r'] = 17;
						_array['s'] = 18;
						_array['t'] = 19;
						_array['u'] = 20;
						_array['v'] = 21;
						_array['x'] = 22;
						_array['1'] = 23;
						_array['2'] = 24;
						_array['3'] = 25;
						_array['4'] = 26;
						_array['5'] = 27;
						_array['6'] = 28;
						_array['7'] = 29;
						_array['8'] = 30;
						_array['9'] = 31;
						
						::strcpy (_reverse, " abcdefghijklmnoprstuvx123456789");
					}
					~_triplet_array (void) {}
	char			 _array[128];
	char			 _reverse[32];
};

static class _triplet_static_array _TARRAY;					
					
class tstring
{
public:
					 tstring (void)
					 {
						 _array_init ();
					 }
					 tstring (const tstring &str)
					 {
						 _array_init ();
						 timport (str); 
					 }
					 tstring (tstring *str) 
					 {
						 _array_init ();
						 timport (str); 
					 }
					 tstring (const string &str)
					 {
						 _array_init ();
						 timport (str); 
					 }
					 tstring (string *str)
					 {
						 _array_init ();
						 timport (str); 
					 }
			 
	inline tstring	&operator= (const string &str)
					{
						timport (str);
						return *this;
					}
	inline tstring	&operator= (string *str)
					{
						timport (str);
						return *this;
					}
	triplet			 operator[] (int idx);
	
protected:
	void			 timport (const string &);
	void			 timport (string *str)
					 {
						 timport (*str);
						 delete str;
					 }

	inline triplet	 tencode (ctriplet ct)
					 {
						 triplet t;
						 
						 t  = _TARRAY._array[ct[0]] << 10;
						 t |= _TARRAY._array[ct[1]] << 5;
						 t |= _TARRAY._array[ct[2]];
						 
						 return t;
					 }
	
	inline ctriplet	 tdecode (triplet t)
					 {
						 ctriplet ct;
						 
						 // 0111 1100 0000 0000 7C00
						 // 0000 0011 1110 0000 02E0
						 // 0000 0000 0001 1111 001F
						 
						 ct[0] = _TARRAY._reverse[(t & 0x7c00) >> 10];
						 ct[1] = _TARRAY._reverse[(t & 0x02e0) >> 5];
						 ct[2] = _TARRAY._reverse[(t & 0x001f)];
						 return ct;
					 }

private:
	int				 _size;
	int				 _count;
	unsigned int	*_array;

    void			 _array_add (triplet);
	void			 _array_init (void)
					 {
						 _size = 0;
						 _count = 0;
						 _array = NULL;
					 }
};


typedef unsigned int triplet_refid;
typedef unsigned short triplet_weight;

// ========================================================================
// refid_array
// -----------
// This is a flexible array of refids, with merging methods.
// ========================================================================

class refid_array
{
public:
					 refid_array (void);
					 refid_array (refid_array &);
					 refid_array (refid_array *);
					~refid_array (void);
	
	void			 init (void)
					 {
						 _count = 0;
						 _size = 0;
						 _array_id = NULL;
						 _array_weight = NULL;
					 }

	void			 merge_and (refid_array &r);
	void			 merge_combine (refid_array &r);

	void			 add (triplet_refid r, triplet_weight w);
	void			 remove (int);
	int				 find (triplet_refid r);

	refid_array		&operator= (refid_array &r);
	refid_array		&operator= (refid_array *r);

	triplet_refid	 operator[] (int);
	triplet_weight	 weight (int);
	void			 weight (int, triplet_weight);
	int				 count (void)
					 {
						 return _count;
					 }
						 
protected:
	int				 _count;
	int				 _size;
	triplet_refid   *_array_id;
	triplet_weight	*_array_weight;

	void			 _grow (void);
};

class triplet_refdb
{
public:
						 triplet_refdb (void);
						~triplet_refdb (void) {};

	void				 addref (tstring &s, triplet_refid r);
	void				 addref (triplet, triplet_refid);
							
	refid_array			*query_and (tstring &ts);
	refid_array			*query_combine (tstring &ts);
							
	inline refid_array	&operator[] (triplet t)
						 {
							 return _array[t & 32767];
						 }

protected:
	refid_array		 	_array[32768];
};


#endif
