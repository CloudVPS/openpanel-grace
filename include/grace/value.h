#ifndef _VALUE_H
#define _VALUE_H 1

#include <grace/statstring.h>
#include <grace/str.h>
#include <grace/file.h>
#include <grace/visitor.h>
#include <grace/currency.h>

#include <grace/checksum.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

/// Union for all number types embedded in a value object.
typedef union dtypes
{
	int					ival;
	double				dval;
	unsigned int		uval;
	long long			lval;
	unsigned long long	ulval;
} dtype;

extern statstring t_char;
extern statstring t_uchar;
extern statstring t_short;
extern statstring t_ushort;
extern statstring t_int;
extern statstring t_unsigned;
extern statstring t_bool;
extern statstring t_bool_true;
extern statstring t_bool_false;
extern statstring t_double;
extern statstring t_string;
extern statstring t_ipaddr;
extern statstring t_unset;
extern statstring t_long;
extern statstring t_ulong;
extern statstring t_array;
extern statstring t_dict;
extern statstring t_date;
extern statstring t_currency;

typedef statstring dtenum;

enum creatorlabel {
	valueWithKey,
	valueWithType,
};

/// List of intrinsic variable types.
/// Any vaue object has one of these types, regardless of what it's
/// registered type/class is.
enum itypes {
  i_unset, ///< Empty value.
  i_int, ///< Signed 32 bits integer.
  i_unsigned, ///< Unsined 32 bits integer.
  i_double, ///< Double precision floating point.
  i_long, ///< Signed 64 bits integer.
  i_ulong, ///< Unsigned 64 bits integer.
  i_bool, ///< Boolean.
  i_string, ///< String data.
  i_ipaddr, ///< IPv4 address
  i_date, ///< date/time stamp.
  i_currency ///< fixed point currency
};

enum valueException {
	EX_VALUE_FILE_NOTFOUND	= 0xc1eb726d, ///< Attempted to load a non-existing file.
	EX_VALUE_ERR_PARSE 		= 0x9712c89b ///< Error parsing data.
};


typedef bool (*sortmethod) (value *, value *, const string &);

/// Sort method for sorting by key.
bool labelSort (value *, value *, const string &);

/// Sort method for sorting by immediate value.
bool valueSort (value *, value *, const string &);

/// Sort method for sorting by value of a child node.
bool recordSort (value *, value *, const string &);

/// Natural sort for immediate value.
/// Attempts a more or less natural sorting order, ignoring certain
/// words (like 'the', 'de', 'le', 'la', etc). This sort method is
/// also aware of roman numerals.
bool naturalSort (value *, value *, const string &);

/// Natural sort for child value or key. If the optional key string
/// is set, this function sorts by the value of a child node. If it
/// is left empty, the node's key is used for sorting.
bool naturalLabelSort (value *, value *, const string &);

/// Generic storage for hierarchical data.
/// A value object can either contain direct data (either an integer, a
/// string or some other intrinsic type) or a mixed array of child values,
/// where some or all array members may also have a string key.
/// The class is designed to make it easy to access its data. This means
/// a number of overloaded operators. Typical code is very compact and
/// easy to read, though:
///
/// \verbinclude value_ex1.cpp
///
/// The data created in this example would encode to XML like this:
///
/// \verbinclude value_ex1.xml
class value
{
friend class visitor<value>;
friend class validator;
public:
					 /// Constructor.
					 value (void);
					 
					 /// Constructor with integer key argument.
					 value (creatorlabel, unsigned int);
					 
					 /// Constructor with string key or type argument.
					 value (creatorlabel, const char *);
					 
					 /// Constructor with string key or type argument.
					 value (creatorlabel, const char *, unsigned int);
					 
					 /// Constructor with string key or type argument.
					 value (creatorlabel, const string &);
					 
					 /// Copy-constructor.
					 value (value &);
					 
					 /// Copy-constructor (deletes original).
					 value (value *);
					 
					 /// Copy-constructor (from string).
					 value (const char *);
					 
					 /// Destructor.
					~value (void);
		
	enum			 xmlargs {
						compact = true,
						nocompact = false
					 };

	static bool		 isbuiltin (const statstring &type);
	
	const value		&operator[] (int i) const;
	value			&operator[] (int i);
	inline const value	&operator[] (const char *str) const
	{
		value *v;
		v = findchild (str);
		if (!v) return *this;
		return *v;
	}
	inline value	&operator[] (const char *str)
	{
		if (_type == t_unset)
			_type = t_dict;
			
		value *v = findchild (str);
		return *v;
	}
	inline const value	&operator[] (const string &str) const
	{
		value *v;
		v = findchild (str.str());
		if (!v) return *this;
		return *v;
	}
	inline value	&operator[] (const string &str)
	{
		if (_type == t_unset)
			_type = t_dict;
			
		value *v = findchild (str.str());
		return *v;
	}
	
	inline const value	&operator[] (const statstring &str) const
	{
		value *v;
		v = findchild ((unsigned int) str.key(),
					   (const char *) str.str());
		if (!v) return *this;
		return *v;
	}
	inline value	&operator[] (const statstring &str)
	{
		if (_type == t_unset)
			_type = t_dict;
			
		value *v = findchild ((unsigned int) str.key(),
							  (const char *) str.str());
		return *v;
	}
	
	inline value	&operator[] (const value &va)
	{
		value *v;
		if (va.type() == t_int)
		{
			if (_type == t_unset)
				_type = t_array;
			
			v = getposition ((unsigned int) va.ival());
		}
		else
		{
			if (_type == t_unset)
				_type = t_string;
				
			v = findchild (va.sval());
		}
		return *v;
	}

	inline value	&operator[] (const value &va) const
	{
		value *v;
		if (va.type() == t_int)
		{			
			v = getposition ((unsigned int) va.ival());
		}
		else
		{
			v = findchild (va.sval());
		}
		return *v;
	}
	
	/// Access to object's attributes.
	inline const value &operator() (const statstring &ki) const
	{
		if (! attrib) return *this;
		return (*attrib)[ki];
	}

	/// Access to object's attributes.
	/// Although this operator casts to a value object, attributes
	/// should not have children of their own or XML encoding will
	/// not be possible.
	/// \param ki Key for the attribute.
	inline value	   &operator() (const statstring &ki)
	{
		if (! attrib) attrib = new value;
		return (*attrib)[ki];
	}
	
	/// Directly set a keyed attribute.
	/// \param ki Attribute's key.
	/// \param val Attribute's value.
	inline void setattrib (const statstring &ki, const string &val)
	{
		if (! attrib) attrib = new value;
		(*attrib)[ki] = val;
	}

	/// Directly set a keyed attribute.
	/// \param ki Attribute's key.
	/// \param val Attribute's value.
	inline void setattrib (const statstring &ki, int val)
	{
		if (! attrib) attrib = new value;
		(*attrib)[ki] = val;
	}

	/// Directly set a keyed attribute.
	/// \param ki Attribute's key.
	/// \param val Attribute's value.
	inline void setattrib (const statstring &ki, const char *val)
	{
		if (! attrib) attrib = new value;
		(*attrib)[ki] = val;
	}

	/// Directly set a keyed attribute.
	/// \param ki Attribute's key.
	/// \param val Attribute's value.
	inline void setattrib (const statstring &ki, bool val)
	{
		if (! attrib) attrib = new value;
		(*attrib)[ki] = val;
	}

			 /// Set as an IPv4 address.		
	value	&setip (unsigned int);
	
	value	&operator= (const value &v);
	value	&operator= (value *v);

	inline			 operator bool (void) const
	{
		return bval();
	}
	inline			 operator long long (void) const
	{
		return lval();
	}
	inline			 operator unsigned long long (void) const
	{
		return ulval();
	}
	inline			 operator int (void) const
	{
		return ival();
	}
	inline			 operator unsigned int (void) const
	{
		return uval();
	}
	inline			 operator const string & (void) const
	{
		return sval();
	}
	inline			 operator const char * (void) const
	{
		return cval();
	}
	inline			 operator double (void) const
	{
		return dval();
	}
	inline value	&operator= (bool bval)
	{
		t.ival = bval ? 1 : 0;
		itype = i_bool;
		if (_type == t_unset) _type = t_bool;
		return *this;
	}
	inline value	&operator= (long long dval)
	{
		t.lval = dval;
		itype = i_long;
		if (_type == t_unset) _type = t_long;
		return *this;
	}
	inline value	&operator= (const class currency &c)
	{
		assign (c);
		return *this;
	}
	inline value	&operator= (class currency &c)
	{
		assign (c);
		return *this;
	}
	void			 assign (const class currency &);
	void			 assign (class currency *);
	
/*	inline value	&operator= (time_t ti)
	{
		t.uval = ti;
		itype = i_date;
		if (_type == t_unset) _type = t_date;
	} */
	inline value	&operator= (unsigned long long val)
	{
		t.ulval = val;
		itype = i_ulong;
		if (_type == t_unset) _type = t_ulong;
		return *this;
	}
	inline value	&operator= (const char *str)
	{
		s = str;
		itype = i_string;
		if (_type == t_unset) _type = t_string;
		return *this;
	}
	inline value	&operator= (const string &str)
	{
		s = str;
		itype = i_string;
		if (_type == t_unset) _type= t_string;
		return *this;
	}
	inline value	&operator= (const statstring &str)
	{
		s = str.sval();
		itype = i_string;
		if (_type == t_unset) _type= t_string;
		return *this;
	}
	inline void		 setcurrency (long long cnew)
	{
		t.lval = cnew;
		itype = i_currency;
		if (_type == t_unset) _type = t_currency;
	}
	inline long long getcurrency (void) const
	{
		switch (itype)
		{
			case i_currency:
				return t.lval;
			
			case i_double:
				return (unsigned long long) (t.dval * 1000.0);
			
			case i_string:
				return parsecurrency (s);
	
			case i_int:
				return (t.ival * 1000LL);
			
			case i_unsigned:
				return ((long long)t.uval) * 1000LL;
			
			case i_long:
			case i_ulong:
				return t.lval * 1000LL;
			
			default:
				return 0LL;
		}
		return 0LL; // unreachable
	}
	inline value	&operator= (string *str)
	{
		// By assigning s to str, it will get pointer-nuked, plz not
		// to nuke it again ktxfreind!
		s = str;
		itype = i_string;
		if (_type == t_unset) _type = t_string;
		return *this;
	}
	value	&operator= (int i)
	{
		t.ival = i;
		itype = i_int;
		if (_type == t_unset) _type  = t_int;
		return *this;
		
	}
	inline value	&operator= (unsigned int i)
	{
		t.uval = i;
		itype = i_unsigned;
		if (_type == t_unset) _type = t_unsigned;
		return *this;
	}
	inline value	&operator= (double d)
	{
		t.dval = d;
		itype = i_double;
		if (_type == t_unset) _type  = t_double;
		return *this;
	}
	inline value	&operator<< (value *v)
	{
		for (int i=0; i<(*v).count(); ++i)
		{
			(*this)[(*v)[i].name()] = (*v)[i];
		}
		delete v;
		return *this;
	}
	inline value	&operator<< (value &v)
	{
		for (int i=0; i<v.count(); ++i)
		{
			(*this)[v[i].name()] = v[i];
		}
		return *this;
	}
	inline int		 strncmp (const string &st, int sz = 0) const
	{
		return sval().strncmp (st, sz);
	}
	inline int		 strcasecmp (const string &st) const
	{
		return sval().strcasecmp (st);
	}
	inline int		 strncasecmp (const string &st, int sz = 0) const
	{
		return sval().strncasecmp (st, sz);
	}
	
						 /// Cast as string object.
	const string		&sval (void) const;

						 /// Cast as string object.
	const string		&sval (void);

						 /// Cast as c-string.
	const char			*cval (void) const; 
	
						 /// Alias for cval().
	inline const char	*str (void) const
						 {
						 	return cval();
						 }

						 /// Cast as c-string.
	const char			*cval (void);

						 /// Cast as unsigned int.
	unsigned int		 uval (void) const;
	
						 /// Cast as integer.
	int					 ival (void) const;
	
						 /// Cast as double precision float.
	double				 dval (void) const;
	
						 /// Cast as 64 bits signed integer.
	long long			 lval (void) const;

						 /// Cast as 64 bits unsigned integer.
	unsigned long long	 ulval (void) const;
	
						 /// Cast as 32 bits IPv4 address.
	unsigned int		 ipval (void);

						 /// Cast as 32 bits IPv4 address.
	unsigned int		 ipval (void) const;

						 /// Cast as boolean.
	bool				 bval (void) const; 

	/// Return registered type.
	/// This can either be a statstring reflection of an intrinsic type
	/// like "string" or "integer", but it can also be a custom 'class.
	inline const statstring	 &type (void) const
	{
		return _type;
	}
	
	/// Set registered type string.
	/// \param t The registered type (i.e. "string" or "myclass").
	inline void		 type (const dtenum &t)
	{
		_type = t;
	}
	
	/// Return total child count.
	inline int		 count (void) const
	{
		return arraysz;
	}
	
	/// Return count of numbered array members.
	/// Only child nodes that have no key are counted.
	inline int		 arraysize (void) const
	{
		return ucount;
	}
	
	/// Add unkeyed string child.
	inline value &operator+= (const string &str)
	{
		newval() = str;
		return (*this);
	}
	
	/// Add keyed value as a child.
	inline value &operator+= (value &v)
	{
		(*this)[v.name()] = v;
		return (*this);
	}
	
	inline bool operator< (const value &other) const
	{
		switch (other.itype)
		{
			case i_string:
				return (::strcmp (cval(), other.cval()) <0);
			case i_int:
				return (ival() < other.ival());
			case i_unsigned:
			case i_date:
				return (uval() < other.uval());
			case i_currency:
			case i_long:
				return (lval() < other.lval());
			case i_ulong:
				return (ulval() < other.ulval());
			
			default:
				return (dval() < other.dval());
		}
	}
	inline bool operator< (const value &other)
	{
		switch (other.itype)
		{
			case i_string:
				return (::strcmp (cval(), other.cval()) <0);
			case i_int:
				return (ival() < other.ival());
			case i_unsigned:
			case i_date:
				return (uval() < other.uval());
			case i_currency:
			case i_long:
				return (lval() < other.lval());
			case i_ulong:
				return (ulval() < other.ulval());
			
			default:
				return (dval() < other.dval());
		}
	}
	inline bool operator> (const value &other) const
	{
		switch (other.itype)
		{
			case i_string:
				return (::strcmp (cval(), other.cval()) >0);
			case i_int:
				return (ival() > other.ival());
			case i_unsigned:
			case i_date:
				return (uval() > other.uval());
			case i_currency:
			case i_long:
				return (lval() > other.lval());
			case i_ulong:
				return (ulval() > other.ulval());
			
			default:
				return (dval() > other.dval());
		}
	}
	inline bool operator> (const value &other)
	{
		switch (other.itype)
		{
			case i_string:
				return (::strcmp (cval(), other.cval()) >0);
			case i_int:
				return (ival() > other.ival());
			case i_unsigned:
			case i_date:
				return (uval() > other.uval());
			case i_currency:
			case i_long:
				return (lval() > other.lval());
			case i_ulong:
				return (ulval() > other.ulval());
			
			default:
				return (dval() > other.dval());
		}
	}
	inline bool operator<= (const value &other)
	{
		switch (other.itype)
		{
			case i_string:
				return (::strcmp (cval(), other.cval()) <=0);
			case i_int:
				return (ival() <= other.ival());
			case i_unsigned:
			case i_date:
				return (uval() <= other.uval());
			case i_currency:
			case i_long:
				return (t.lval <= other.t.lval);
			case i_ulong:
				return (ulval() <= other.ulval());
			
			default:
				return (dval() <= other.dval());
		}
	}
	inline bool operator<= (const value &other) const
	{
		switch (other.itype)
		{
			case i_string:
				return (::strcmp (cval(), other.cval()) <=0);
			case i_int:
				return (ival() <= other.ival());
			case i_unsigned:
			case i_date:
				return (uval() <= other.uval());
			case i_currency:
			case i_long:
				return (t.lval <= other.t.lval);
			case i_ulong:
				return (ulval() <= other.ulval());
			
			default:
				return (dval() <= other.dval());
		}
	}
	inline bool operator>= (const value &other) const
	{
		switch (other.itype)
		{
			case i_string:
				return (::strcmp (cval(), other.cval()) >=0);
			case i_int:
				return (ival() >= other.ival());
			case i_unsigned:
			case i_date:
				return (uval() >= other.uval());
			case i_currency:
			case i_long:
				return (t.lval >= other.t.lval);
			case i_ulong:
				return (ulval() >= other.ulval());
			
			default:
				return (dval() >= other.dval());
		}
	}
	inline bool operator>= (const value &other)
	{
		switch (other.itype)
		{
			case i_string:
				return (::strcmp (cval(), other.cval()) >=0);
			case i_int:
				return (ival() >= other.ival());
			case i_unsigned:
			case i_date:
				return (uval() >= other.uval());
			case i_currency:
			case i_long:
				return (t.lval >= other.t.lval);
			case i_ulong:
				return (ulval() >= other.ulval());
			
			default:
				return (dval() >= other.dval());
		}
	}
	inline bool operator== (const value &other) const
	{
		switch (other.itype)
		{
			case i_string:
				return (::strcmp (cval(), other.cval()) ==0);
			case i_int:
				return (ival() == other.ival());
			case i_unsigned:
			case i_date:
				return (uval() == other.uval());
			case i_currency:
			case i_long:
				return (t.lval == other.t.lval);
			case i_ulong:
				return (ulval() == other.ulval());
			
			default:
				return (dval() == other.dval());
		}
	}
	inline bool operator== (const value &other)
	{
		switch (other.itype)
		{
			case i_string:
				return (::strcmp (cval(), other.cval()) ==0);
			case i_int:
				return (ival() == other.ival());
			case i_unsigned:
			case i_date:
				return (uval() == other.uval());
			case i_currency:
			case i_long:
				return (t.lval == other.t.lval);
			case i_ulong:
				return (ulval() == other.ulval());
			
			default:
				return (dval() == other.dval());
		}
	}
	inline bool operator!= (const value &other) const
	{
		switch (other.itype)
		{
			case i_string:
				return (::strcmp (cval(), other.cval()) !=0);
			case i_int:
				return (ival() != other.ival());
			case i_unsigned:
			case i_date:
				return (uval() != other.uval());
			case i_currency:
			case i_long:
				return (t.lval != other.t.lval);
			case i_ulong:
				return (ulval() != other.ulval());
			
			default:
				return (dval() != other.dval());
		}
	}
	inline bool operator!= (const value &other)
	{
		switch (other.itype)
		{
			case i_string:
				return (::strcmp (cval(), other.cval()) !=0);
			case i_int:
				return (ival() != other.ival());
			case i_unsigned:
			case i_date:
				return (uval() != other.uval());
			case i_currency:
			case i_long:
				return (t.lval != other.t.lval);
			case i_ulong:
				return (ulval() != other.ulval());
			
			default:
				return (dval() != other.dval());
		}
	}
	inline bool operator== (const char *other) const
	{
		return (sval().strcmp (other) == 0);
	}
	
	#define DEFOPERATORS(otype,ovalue) \
		inline bool operator== (otype other) const \
		{ \
			return (ovalue == other); \
		} \
		inline bool operator== (otype other) \
		{ \
			return (ovalue == other); \
		} \
		inline bool operator!= (otype other) const \
		{ \
			return (ovalue != other); \
		} \
		inline bool operator!= (otype other) \
		{ \
			return (ovalue != other); \
		} \
		inline bool operator< (otype other) const \
		{ \
			return (ovalue < other); \
		} \
		inline bool operator< (otype other) \
		{ \
			return (ovalue < other); \
		} \
		inline bool operator<= (otype other) const \
		{ \
			return (ovalue <= other); \
		} \
		inline bool operator<= (otype other) \
		{ \
			return (ovalue <= other); \
		} \
		inline bool operator> (otype other) const \
		{ \
			return (ovalue > other); \
		} \
		inline bool operator> (otype other) \
		{ \
			return (ovalue > other); \
		} \
		inline bool operator>= (otype other) const \
		{ \
			return (ovalue != other); \
		} \
		inline bool operator>= (otype other) \
		{ \
			return (ovalue != other); \
		}
	
	DEFOPERATORS(double,dval())
	DEFOPERATORS(int,ival())
	DEFOPERATORS(unsigned int,uval())
	DEFOPERATORS(long long,lval())
	DEFOPERATORS(unsigned long long,ulval())
	
	inline bool operator!= (const string &other) const
	{
		return (sval().strcmp (other));
	}
	inline bool operator!= (const statstring &other) const
	{
		return (sval().strcmp (other.sval()));
	}
	inline bool operator!= (const char *other) const
	{
		return (sval().strcmp (other));
	}
	inline bool operator!= (const char *other)
	{
		return (sval().strcmp (other));
	}
	inline bool operator== (bool other) const
	{
		return (bval() == other);
	}
	inline bool operator== (bool other)
	{
		return (bval() == other);
	}
	bool operator== (const char *s)
	{
		return (sval().strcmp (s) == 0);
	}
	bool operator== (const statstring &s) const
	{
		return (sval() == s.sval());
	}
	bool operator== (const statstring &s)
	{
		return (sval() == s.sval());
	}
	bool operator!= (const statstring &s)
	{
		return (sval() != s.sval());
	}
	inline bool operator!= (bool other) const
	{
		return (bval() != other);
	}
	
					 /// Wipe out value and children.
	void 			 clear (void);
	
					 /// Return reference to a new unkeyed child.
					 /// \param typ Registered type of the new child.
	value			&newval (dtenum typ=t_unset);
	
					 /// Remove child node at index position.
	void			 rmindex (int);
	
					 /// Remove any child.
					 /// \param k Key value of the child to remove.
					 /// \param ks String key value of the child.
					 /// \param idx Array index of value to remove (-1 to use key).
	void			 rmval (unsigned int k, const char *ks, int idx=-1);
	
					 /// Remove child with specific integer key.
	void			 rmval (unsigned int);
	
					 /// Remove child with string key.
	void			 rmval (const char *);

					 /// Remove child with string key.
	void			 rmval (const value &);

					 /// Remove child with string key.
	void			 rmval (const statstring &);

					 /// Return last array member.
	value			&last (void);
	
					 /// Cast key as a c-string.
	const char		*name (void) const;
	
					 /// Cast key as a statstring.
	const statstring &label (void) const;

					 /// Cast key as a statstring.
	const statstring &id (void) const;

					 /// Load data in plain ASCII format.
					 /// Does not support attributes.
					 /// \throw value::exception Loading/parsing exception
	void			 load (const char *);
	
					 /// Load ASCII data from an already open file.
					 /// \throw value::exception Loading/parsing exception
	void			 load (const char *, class file &);
	
					 /// Save as ASCII (sans attributes)
	void			 save (const string &, bool compact=false) const;

					 /// Save as ASCII to an open file (sans attributes)
	void			 save (class file &, bool compact=false) const;
	
					 /// Encode to ASCII.
	string			*encode (bool compact=false) const;
	
					 /// Decode from ASCII.
	void			 decode (string &);

					 /// Load from an ini-file.
	bool			 loadini (const string &fn);
	
					 /// Load from an ini-file with hierarchical sections.
	bool			 loadinitree (const string &fn);

	// csv format import/export
					 /// Convert to CSV format.
					 /// \param withHeaders If true, add a header row.
					 /// \param indexName Column name for the index field.
	string			*tocsv (bool withHeaders=true, const char *indexName="id");
	
					 /// Save in CSV format.
					 /// \param fn Filename to save.
					 /// \param withHeaders If true, add a header row.
					 /// \param indexName Column name for the index field.
	bool			 savecsv (const string &fn, bool withHeaders=true,
							  const char *indexName="id");

					 /// Load from CSV format.
					 /// \param fn File name.
					 /// \param withHeaders True if the file has a header row.
					 /// \param key Label of the index row (empty for index on first row).
	bool			 loadcsv (const string &fn, bool withHeaders=true,
							  const string &key="");

					 /// Convert from CSV format.
					 /// \param csvdata String containing the CSV data as text.
					 /// \param withHeaders True if the file has a header row.
					 /// \param key Label of the index row (empty for index on first row).
	bool			 fromcsv (const string &csvData, bool withHeaders=true,
							  const string &key="");

					 /// Deserialize PHP data.
					 /// \param phpdata A serialized PHP array.
	void			 phpdeserialize (const string &);
	
					 /// Serialize PHP data.
					 /// Discards attribute data, really prefers everything
					 /// to have a key and only really knows about ints
					 /// and strings.
					 /// \return A new string object.
	string			*phpserialize (bool withattr = false);
					 
					 /// Save in XML format.
					 /// \param fn File name to save.
					 /// \param compact Set to value::compact or value::nocompact.
					 /// \param schema XML schema to apply, NULL for none.
	void			 savexml (const char *fn, bool compact=false,
							  class xmlschema *schema=NULL) const;

					 /// Save in XML format.
					 /// \param fn File name to save.
					 /// \param compact Set to value::compact or value::nocompact.
					 /// \param schema XML schema to apply.
	void			 savexml (const char *, bool, class xmlschema &) const;
	
					 /// Convert to string containing XML data.
					 /// \param compact Set to value::compact or value::nocompact.
					 /// \param schema XML schema to apply, NULL for none.
	string			*toxml (bool compact=false, class xmlschema *s=NULL) const;

					 /// Convert to string containing XML data.
					 /// \param compact Set to value::compact or value::nocompact.
					 /// \param schema XML schema to apply.
	string			*toxml (bool compact, class xmlschema &) const;
	
					 /// Convert from string with XML data.
					 /// \param d The XML text data.
					 /// \param s The schema to use for parsing. NULL for none.
	void			 fromxml (const string &d, class xmlschema *s = NULL);

					 /// Convert from string with XML data.
					 /// \param d The XML text data.
					 /// \param s The schema to use for parsing.
	void			 fromxml (const string &, class xmlschema &);
	
					 /// Load from an XML file.
					 /// \param path The file name.
					 /// \param s The schema to be used for parsing.
	void			 loadxml (const string &path, class xmlschema &s);

					 /// Load from an XML file.
					 /// \param path The file name.
					 /// \param s The schema to be used for parsing. NULL for none.
	void			 loadxml (const string &path, class xmlschema *s = NULL);
	 
					 /// Convert from CXML.
					 /// Uses a binary storage format comparable to the
					 /// Apple/NeXT serialized plist. Requires a schema
					 /// with CXML elements.
					 /// \param dat The binary data.
					 /// \param s The schema to use.
	void			 fromcxml (string &dat, class xmlschema &s);
	
					 /// Convert to CXML.
					 /// Creates a string with a binary dump of the
					 /// object and its children. Requires a schema with
					 /// CXML elements.
					 /// \param s The schema to use.
	string			*tocxml (class xmlschema &s);

	// Apple plist format import/export
					 /// Save in Apple's plist format.
					 /// Requires the com.apple.plist.schema.xml schema.
					 /// \param fn File name.
					 /// \param compact Either value::compact or value::nocompact.
	void			 saveplist (const char *fn, bool compact=false);
	
					 /// Convert to Apple's plist format.
					 /// Requires the com.apple.plist.schema.xml schema.
					 /// \param compact Either value::compact or value::nocompact.
	string			*toplist (bool compact=false);

					 /// Convert from Apple's plist format.
					 /// Requires the com.apple.plist.schema.xml schema.
					 /// \param dat The text data.
	void			 fromplist (const string &dat);

					 /// Load from Apple's plist format.
					 /// Requires the com.apple.plist.schema.xml schema.
					 /// \param fn File name to load.
	void			 loadplist (const string &fn);

					 /// Returns true if a child key exists.
	bool			 exists (const char *) const;

					 /// Returns true if a child key exists.
	bool			 exists (unsigned int, const char *) const;

					 /// Returns true if a child key exists.
	bool			 exists (const string &) const;

					 /// Returns true if a child key exists.
	bool			 exists (const statstring &) const;

					 /// Returns true if the object has attributes.
	bool			 haveattributes (void) const
					 {
					 	return (attrib != NULL);
					 }
					 
					 /// Access attributes as a value object.
	const value		&attributes (void) const
					 {
					   if (! attrib) return *this;
					   return *attrib;
					 }

					 /// Access attributes as a value object.
	value			&attributes (void)
					 {
					 	if (! attrib) return *this;
					 	return *attrib;
					 }
					 
					 /// Returns true if a key exists in the attributes.
					 /// \param id The key to look for.
	inline bool		 attribexists (const statstring &id) const
					 {
					 	if (! attrib) return false;
					 	return attrib->exists (id);
					 }
					 
					 /// Remove attribute for a key.
					 /// \string id The key to remove.
	inline void		 rmattrib (const statstring &id)
					 {
					 	if (attrib)
					 	{
					 		attrib->rmval (id);
					 	}
					 }
					 
					 /// Sort this object's child nodes.
	void			 sort (sortmethod);
	
					 /// Sort this object's child nodes.
					 /// Accepts an optional key string to be used by some 
					 /// sort methods.
	void			 sort (sortmethod, const string &);
	
					 /// Filter out children that have a child node
					 /// with a specified value.
					 /// \param label The grandchild key to investigate.
					 /// \param what The value to look out for.
	value			*filter (const statstring &label, const string &what) const;
	
					 /// Cut off a number of leftmost nodes.
					 /// \param num The number, if negative relative to the
					 ///            total array size.
	value			*cutleft (int num);

					 /// Copy a number of leftmost nodes.
					 /// \param num The number, if negative relative to the
					 ///            total array size.
	value			*copyleft (int num) const;

					 /// Cut off a number of rightmost nodes.
					 /// \param num The number, if negative relative to the
					 ///            total array size.
	value			*cutright (int num);

					 /// Copy a number of rightmost nodes.
					 /// \param num The number, if negative relative to the
					 ///            total array size.
	value			*copyright (int num) const;

	bool			 treecmp (const value &other) const;

protected:
					 /// Parse a line from an ini-file.
	value			*iniparse (const string &);
	
					 /// Internal method for ASCII export.
	void			 print (int, class file &, bool compact=false) const;
	
					 /// Internal method for ASCII conversion.
	void			 printstr (int, string &, bool compact=false) const;
	
					 /// Internal method for XML export.
	void			 printxml (int, string &, bool,
							   class xmlschema *, value *,
							   const statstring &, const statstring &) const;
							   
					 /// Internal method for plist export.
	void			 printplist (int, string &, bool compact=false) const;
	
					 /// Internal method for serializing as PHP data.
	void			 printphp (string &into, bool withattr);
	
					 /// Internal method for translating PHP data.
	const char		*phpdeserialize (const char *, bool);
	
					 /// Internal method for CXML writing.
	size_t			 printcompressed (size_t, string &,
									  const value &, class xmlschema &) const;

					 /// Internal method for CXML reading.
	size_t			 parsecompressed (size_t, string &,
									  class xmlschema &);
									  									  
					 /// Internal method for CXML writing.
	size_t			 compressbuiltin (size_t, string &, const char *,
									  const statstring &, const value &) const;

					 /// Internal method for CXML writing.
	size_t			 compressbuiltin (size_t sz, string &a, const char *b,
									  const string &c, const value &d) const
					 {
					 	statstring s;
					 	s = c;
					 	return compressbuiltin (sz, a, b, s, d);
					 };
		
	dtenum			 _type; ///< The registered type/class.
	string			 s; ///< The string value.
	dtype			 t; ///< The numeric value.
	unsigned char	 itype; ///< The intrinsic type.
	
	unsigned int	 key; ///< Numeric key.
	class statstring _name; ///< String key.
	
	value			*lower, *higher; ///< Hash tree links.
	int				 padding; ///< Ununsed
	value			*attrib; ///< Attributes.
	
	value			**array; ///< Child array.
	unsigned int	  arraysz; ///< Number of children.
	unsigned int	  arrayalloc; ///< Allocated array size.
	unsigned int	  ucount; ///< Number of unkeyed children.
	
	void			  alloc (unsigned int c); ///< Array allocation.
	
					 /// Access method for the visitor protocol.
	value			*findchild (const char *) const;
					 /// Access method for the visitor protocol.
	value			*findchild (const char *);
	value			*havechild (unsigned int, const char *) const;
					 /// Access method for the visitor protocol.
	value			*findchild (unsigned int, const char *) const;
					 /// Access method for the visitor protocol.
	value			*findchild (unsigned int, const char *);
					 /// Access method for the visitor protocol.
	value			*getposition (unsigned int);
					 /// Access method for the visitor protocol.
	value			*getposition (unsigned int) const;
						 /// Access method for the visitor protocol.
	value			*visitchild (const statstring &id)
					 {
					 	return havechild (id.key(), id.str());
					 }
					 /// Access method for the visitor protocol.
	value			*visitchild (int index)
					 {
					 	unsigned int pindex = (index<0) ? -index : index;
					 	if (!arraysz) return NULL;
					 	if (pindex >= arraysz) return NULL;
					 	return array[(index<0) ? arraysz-index : index];
					 }
};

string *ip2str (unsigned int ipaddr);
unsigned int str2ip (const string &str);

time_t __parse_timestr (const string &);
string *__make_timestr (time_t);

#endif
