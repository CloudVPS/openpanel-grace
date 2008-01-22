#ifndef _VALUE_H
#define _VALUE_H 1

#include <grace/statstring.h>
#include <grace/str.h>
#include <grace/file.h>
#include <grace/visitor.h>
#include <grace/currency.h>
#include <grace/dictionary.h>
#include <grace/stringdict.h>
#include <grace/retain.h>
#include <grace/flags.h>

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

#ifdef _VALUE_CPP
 #define TYPENAME statstring
 #define TVALUE(x) (x)
 #undef _VALUE_CPP
#else
 #define TYPENAME extern const statstring
 #define TVALUE(x)
#endif

TYPENAME t_char 		TVALUE ("char");
TYPENAME t_uchar 		TVALUE ("uchar");
TYPENAME t_short 		TVALUE ("short");
TYPENAME t_ushort 		TVALUE ("ushort");
TYPENAME t_int 			TVALUE ("integer");
TYPENAME t_unsigned 	TVALUE ("unsigned");
TYPENAME t_bool 		TVALUE ("bool");
TYPENAME t_bool_true 	TVALUE ("bool.true");
TYPENAME t_bool_false 	TVALUE ("bool.false");
TYPENAME t_double 		TVALUE ("float");
TYPENAME t_string 		TVALUE ("string");
TYPENAME t_ipaddr 		TVALUE ("ipaddress");
TYPENAME t_unset 		TVALUE ("void");
TYPENAME t_long 		TVALUE ("long");
TYPENAME t_ulong 		TVALUE ("ulong");
TYPENAME t_array 		TVALUE ("array");
TYPENAME t_dict 		TVALUE ("dict");
TYPENAME t_date 		TVALUE ("date");
TYPENAME t_currency 	TVALUE ("currency");

typedef statstring dtenum;
extern const class value emptyvalue;

/// Use this to access a constructor that allows the id/type to be set in
/// the arguments.
enum creatorlabel {
	valueWithKey,
	valueWithType,
};

/// List of intrinsic variable types.
/// Any value object has one of these types, regardless of what it's
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

$exception (valueFileNotFoundException, "File not found");
$exception (valueParsingException, "Error parsing file");

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

/// Function for the valuebuilder syntax. Creates a new value
/// with one keyed child node.
/// \param id The key.
/// \param v The value.
value *$ (const statstring &id, const value &v);

/// Valuebuilder: Create new value with unkeyed child.
/// \param v Child data.
value *$ (const value &v);

/// Valuebuilder: Create new value with a set attribute.
/// \param id Attribute name.
/// \param v Attribute value.
value *$attr (const statstring &id, const value &v);

/// Valuebuilder: Create new value with a set type.
/// \param t The type.
value *$type (const statstring &t);

/// Valuebuilder: Create new value as a copy of an existing
/// value that will get merged through further chaining.
/// \param v The value to copy.
value *$merge (const value &v);

/// Valuebuilder: Create a new value with specific data.
/// \param v Original data.
value *$val (const value &v);

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
class value : public memory::retainable
{
friend class visitor<value>;
friend class iterator<value,value>;
friend class iterator<const value,const value>;
friend class visitor<const value>;
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
					 
					 /// Const copy-constructor.
					 value (const value &);
					 
					 /// Copy-constructor (deletes original).
					 value (value *);
					 
					 /// Copy-constructor (from string).
					 value (const char *);
					 
					 /// Copy-constructor (from double).
					 value (double);
					 
					 /// Copy-constructor (from a valuable)
					 value (const class valuable &);
					 
					 /// Copy-constructor (from a retained valuable)
					 value (class valuable *);
					 
					 /// Copy-constructor (from a string)
					 value (const string &s);
					 
					 /// Copy-constructor (from a retained string)
					 value (string *s);
					 
					 /// Copy-constructor from a statstring
					 value (const statstring &s);
					 
					 /// Copy-constructor (from an integer)
					 value (int);
					 
					 /// Copy-constructor (unsigned integer)
					 value (unsigned int);
					 
					 /// Copyy-constructor (long long)
					 value (long long);
					 
					 /// Copy-constructor (unsigned long long)
					 value (unsigned long long);
					 
					 /// Copy-constructor (bool)
					 value (bool);
					 
					 /// Destructor.
					~value (void);
	
					 /// Options for xml encoding.
	enum			 xmlargs {
						compact = true, ///< Compacted output.
						nocompact = false ///< Indented output.
					 };

					 /// Returns true if the provided type string
					 /// represents a grace built-in.
	static bool		 isbuiltin (const statstring &type);
	
	/// Access by array index.
	const value		&operator[] (int i) const;

	/// Access by array index.
	value			&operator[] (int i);

	/// Access by string key.
	inline const value	&operator[] (const char *str) const
	{
		static value emptyval;
		value *v;

		v = findchild (str);
		if (!v) return emptyval;
		return *v;
	}

	/// Access by string key.
	inline value	&operator[] (const char *str)
	{
		if (_type == t_unset)
			_type = t_dict;
			
		value *v = findchild (str);
		return *v;
	}

	/// Access by string key.
	inline const value	&operator[] (const string &str) const
	{
		static value emptyval;
		value *v;

		v = findchild (str.str());
		if (!v) return emptyval;
		return *v;
	}

	/// Access by string key.
	inline value	&operator[] (const string &str)
	{
		if (_type == t_unset)
			_type = t_dict;
			
		value *v = findchild (str.str());
		return *v;
	}
	
	/// Access by string key.
	inline const value	&operator[] (const statstring &str) const
	{
		static value emptyval;
		value *v;

		v = findchild ((unsigned int) str.key(),
					   (const char *) str.str());
		if (!v) return emptyval;
		return *v;
	}

	/// Access by string key.
	inline value	&operator[] (const statstring &str)
	{
		if (_type == t_unset)
			_type = t_dict;
			
		value *v = findchild ((unsigned int) str.key(),
							  (const char *) str.str());
		return *v;
	}
	
	/// Access by value key (int or string).
	inline value	&operator[] (const value &va)
	{
		static value emptyval;
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
		
		if (! v) return emptyval;
		
		return *v;
	}

	/// Access by value key (int or string).
	inline value	&operator[] (const value &va) const
	{
		static value emptyval;
		value *v;
		if (va.type() == t_int)
		{			
			v = getposition ((unsigned int) va.ival());
		}
		else
		{
			v = findchild (va.sval());
		}

		if (! v) return emptyval;
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

			 /// Set as an IPv4 address. Address is in host format.	
	value	&setip (unsigned int);
	
	value	&operator= (const value &v);
	value	&operator= (value *v);
	value	&operator= (class valuable &);
	value	&operator= (class valuable *);

	/// Cast as bool.
	inline			 operator bool (void) const
	{
		return bval();
	}
	
	/// Cast as 64 bits signed.
	inline			 operator long long (void) const
	{
		return lval();
	}
	
	/// Cast as 64 bits unsigned.
	inline			 operator unsigned long long (void) const
	{
		return ulval();
	}
	
	/// Cast as int.
	inline			 operator int (void) const
	{
		return ival();
	}
	
	/// Cast as unsigned int.
	inline			 operator unsigned int (void) const
	{
		return uval();
	}
	
	/// Cast as const string.
	inline			 operator const string & (void) const
	{
		return sval();
	}
	
	/// Cast as c string.
	inline			 operator const char * (void) const
	{
		return cval();
	}
	
	/// Cast as double precision float.
	inline			 operator double (void) const
	{
		return dval();
	}
	
	/// Assign to bool.
	inline value	&operator= (bool bval)
	{
		t.ival = bval ? 1 : 0;
		itype = i_bool;
		if (_type == t_unset) _type = t_bool;
		return *this;
	}
	
	/// Assign to 64 bits signed.
	inline value	&operator= (long long dval)
	{
		t.lval = dval;
		itype = i_long;
		if (_type == t_unset) _type = t_long;
		return *this;
	}
	
	/// Assign to fixed point currency.
	inline value	&operator= (const class currency &c)
	{
		assign (c);
		return *this;
	}
	
	/// Assign to fixed point currency.
	inline value	&operator= (class currency &c)
	{
		assign (c);
		return *this;
	}
	
					 /// Assign to a currency object.
	void			 assign (const class currency &);
	
					 /// Assign to a currency object.
	void			 assign (class currency *);
	
	/// Assign to unsigned 64 bits.
	inline value	&operator= (unsigned long long val)
	{
		t.ulval = val;
		itype = i_ulong;
		if (_type == t_unset) _type = t_ulong;
		return *this;
	}
	
	/// Assign to c string.
	inline value	&operator= (const char *str)
	{
		s = str;
		itype = i_string;
		if (_type == t_unset) _type = t_string;
		return *this;
	}

	inline value	&operator= (const unsigned char *str)
	{
		s = str;
		itype = i_string;
		if (_type == t_unset) _type = t_string;
		return *this;
	}
	
	/// Assign to string.
	inline value	&operator= (const string &str)
	{
		s = str;
		itype = i_string;
		if (_type == t_unset) _type= t_string;
		return *this;
	}
	
	/// Assign to statstring.
	inline value	&operator= (const statstring &str)
	{
		s = str.sval();
		itype = i_string;
		if (_type == t_unset) _type= t_string;
		return *this;
	}

	inline value	&operator= (statstring *str)
	{
		s = str->sval();
		itype = i_string;
		if (_type == t_unset) _type= t_string;
		delete str;
		return *this;
	}
	
	/// Set the value as a fixed point decimal number with three digits
	/// behind the decimal point.
	inline void		 setcurrency (long long cnew)
	{
		t.lval = cnew;
		itype = i_currency;
		if (_type == t_unset) _type = t_currency;
	}
	
	/// Get the value as a fixed point decimal number with three digits
	/// behind the decimal point, giving a resolution of 0.1 cents of an
	/// arbitrary currency.
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
	
	/// Assign to retained string.
	inline value	&operator= (string *str)
	{
		// By assigning s to str, it will get pointer-nuked, plz not
		// to nuke it again ktxfreind!
		s = str;
		itype = i_string;
		if (_type == t_unset) _type = t_string;
		return *this;
	}
	
	/// Assign to int.
	value	&operator= (int i)
	{
		t.ival = i;
		itype = i_int;
		if (_type == t_unset) _type  = t_int;
		return *this;
		
	}
	
	/// Assign to unsigned int.
	inline value	&operator= (unsigned int i)
	{
		t.uval = i;
		itype = i_unsigned;
		if (_type == t_unset) _type = t_unsigned;
		return *this;
	}
	
	/// Assign to double precision float.
	inline value	&operator= (double d)
	{
		t.dval = d;
		itype = i_double;
		if (_type == t_unset) _type  = t_double;
		return *this;
	}
	
	/// Merge children of other value into current tree.
	inline value	&operator<< (value *v)
	{
		for (int i=0; i<(*v).count(); ++i)
		{
			(*this)[(*v)[i].name()] = (*v)[i];
		}
		delete v;
		return *this;
	}

	/// Merge children of other value into current tree.
	inline value	&operator<< (const value &v)
	{
		for (int i=0; i<v.count(); ++i)
		{
			(*this)[v[i].name()] = v[i];
		}
		return *this;
	}
	
	/// Case-sensitive string comparison, limited by size.
	/// \param st The other string.
	/// \param sz The maximum size to compare, if 0 the comparison
	///           stops at the size of the shortest of two strings.
	inline int		 strncmp (const string &st, int sz = 0) const
	{
		return sval().strncmp (st, sz);
	}
	
	/// Case-insensitive string comparison.
	inline int		 strcasecmp (const string &st) const
	{
		return sval().strcasecmp (st);
	}
	
	/// Case-insensitive string comparison, limited by size.
	inline int		 strncasecmp (const string &st, int sz = 0) const
	{
		return sval().strncasecmp (st, sz);
	}
	
						 /// Cast as string object. 
	const string		&sval (void) const;

						 /// Cast as c-string.
	const char			*cval (void) const; 
	
						 /// Alias for cval().
	inline const char	*str (void) const
						 {
						 	return cval();
						 }

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
	
	/// Comparison by value.
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

	/// Comparison by value.
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

	/// Comparison by value.
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

	/// Comparison by value.
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

	/// Comparison by value.
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

	/// Comparison by value.
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

	/// Comparison by value.
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

	/// Comparison by value.
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

	/// Comparison by value.
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

	/// Comparison by value.
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

	/// Comparison by value.
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

	/// Comparison by value.
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

	/// Case sensitive compare to c string.
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
	
	/// Case-sensitive string compare.
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
	inline bool operator!= (bool other) const
	{
		return (bval() != other);
	}
	inline bool operator!= (bool other)
	{
		return (bval() != other);
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
	
					 /// Wipe out value and children.
	void 			 clear (void);
	
					 /// Continuation method for the valuebuilder
					 /// syntax. The first node in an arraybuilder
					 /// declaration is actually a $(...)-style
					 /// function that creates a new object.
					 /// Further nodes are chained using the
					 /// regular '->' pointer follower.
					 /// \param id Key for the new node.
					 /// \param v Value for the new node.
	value			*$ (const statstring &id, const value &v)
					 {
						(*this)[id] = v;
						return this;
					 }
					 
					 /// Valuebuilder: add new unkeyed node.
					 /// \param v The new node value.
	value			*$ (const value &v)
					 {
					 	newval() = v;
					 	return this;
					 }
					 
					 /// Valuebuilder: add attribute node.
					 /// \param id Attribute key.
					 /// \param v Attribute value.
	value			*$attr (const statstring &id, const value &v)
					 {
					 	(*this)(id) = v;
					 	return this;
					 }
					 
					 /// Valuebulder: set value type().
					 /// \param t The type.
	value			*$type (const statstring &t)
					 {
					 	type (t);
					 	return this;
					 }
					 
					 /// Valuebuilder: merge another value.
					 /// \param v The value to merge.
	value			*$merge (const value &v)
					 {
					 	(*this) << v;
					 	return this;
					 }
					 
					 /// Valuebuilder: set explicit value.
					 /// \param v The explicit value. Attributes
					 /// are skipped unless if the original is
					 /// of an array type.
					 /// \param v Original value.
	value			*$val (const value &v)
					 {
					 	switch (v.itype)
					 	{
					 		case i_unset: break;
					 		case i_bool: (*this) = v.bval(); break;
					 		case i_long: (*this) = v.lval();
					 		case i_unsigned: (*this) = v.uval();
					 		case i_ulong: (*this) = v.ulval();
					 		case i_ipaddr: setip (v.ipval());
					 		case i_int: (*this) = v.ival(); break;
					 		case i_double: (*this) = v.dval(); break;
					 		case i_string:
					 		case i_date:
					 		case i_currency:
					 			(*this) = v.sval();
					 			break;
					 		default:
					 			(*this) = v;
					 			break;
					 	}
					 	return this;
					 }
	
					 /// Return reference to a new unkeyed child.
					 /// \param typ Registered type of the new child.
	value			&newval (dtenum typ=t_unset);
	
					 /// Return reference to a new unkeyed child.
					 /// \param atpos Insertion point.
					 /// \param typ Registered type of the new child.
	value			&insertval (int atpos=0, dtenum typ=t_unset);
	
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
	
					 /// Cast key as a c-string. Use id() and %format if
					 /// possible.
	const char		*name (void) const;
	
					 /// Cast key as a statstring. Deprecated, use id().
	const statstring &label (void) const;

					 /// Cast key as a statstring.
	const statstring &id (void) const;

					 /// Load data in plain ASCII format.
					 /// Does not support attributes.
					 /// \throw value::exception Loading/parsing exception
	void			 load (const string &);
	
					 /// Load ASCII data from an already open file.
					 /// \throw value::exception Loading/parsing exception
	void			 load (class file &);
	
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
					 /// Convert to CSV format. This export does not
					 /// encode attributes.
					 /// \param withHeaders If true, add a header row.
					 /// \param indexName Column name for the index field.
	string			*tocsv (bool withHeaders=true, const char *indexName="id");
	
					 /// Save in CSV format. This export does not
					 /// encode attributes.
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
					 /// \param csvData String containing the CSV data as text.
					 /// \param withHeaders True if the file has a header row.
					 /// \param key Label of the index row (empty for index on first row).
	bool			 fromcsv (const string &csvData, bool withHeaders=true,
							  const string &key="");

					 /// Deserialize PHP data.
					 /// \param phpdata A serialized PHP array.
	void			 phpdeserialize (const string &phpdata);
	
					 /// Serialize to a PHP array.
					 /// \param withattr If set, all objects will be split
					 ///                 in two levels. Arrays will gain
					 ///                 at least one extra node called
					 ///                 ".attr" if there are attributes.
					 ///                 Data objects will end up with
					 ///                 their actual value inside a
					 ///                 child-node called ".data" with,
					 ///                 again, any attributes inside
					 ///                 ".attr".
					 /// \return A new string object.
	string			*phpserialize (bool withattr = false) const;
					 
					 /// Save in XML format.
					 /// \param fn File name to save.
					 /// \param compact Set to value::compact or value::nocompact.
					 /// \param schema XML schema to apply, NULL for none.
					 /// \param tp Set to 'atomic' to write atomically using
					 ///           a temporary file.
	bool			 savexml (const string &fn, bool compact=false,
							  class xmlschema *schema=NULL,
							  flag::savetype tp = flag::normal) const;

					
					 /// Save in XML format.
					 /// \param fn File name to save.
					 /// \param compact Set to value::compact or value::nocompact.
					 /// \param tp Set to 'atomic' to write atomically using
					 ///           a temporary file.
	bool			 savexml (const string &fn, bool compact,
							  flag::savetype tp) const;
	
					 /// Save in XML format.
					 /// \param fn File name to save.
					 /// \param schema XML schema to use.
					 /// \param tp Set to 'atomic' to write atomically using
					 ///           a temporary file.
	bool			 savexml (const string &fn, class xmlschema &schema,
							  flag::savetype tp = flag::normal) const
					 {
					 	return savexml (fn, value::nocompact, schema, tp);
					 }
	
					 /// Save in XML format.
					 /// \param fn File name to save.
					 /// \param tp Set to 'atomic' to write atomically using
					 ///           a temporary file.
	bool			 savexml (const string &fn, flag::savetype tp) const;
	
					 /// Save in XML format.
					 /// \param fn File name to save.
					 /// \param compact Set to value::compact or value::nocompact.
					 /// \param schema XML schema to apply.
					 /// \param tp Set to 'atomic' to write atomically using
					 ///           a temporary file.
	bool			 savexml (const string &fn, bool compact,
							  class xmlschema &schema,
							  flag::savetype tp = flag::normal) const;
	
					 /// Convert to string containing XML data.
					 /// \param compact Set to value::compact or value::nocompact.
					 /// \param s XML schema to apply, NULL for none.
	string			*toxml (bool compact=false, class xmlschema *s=NULL) const;

					 /// Convert to string containing XML data.
					 /// \param compact Set to value::compact or value::nocompact.
					 /// \param schema XML schema to apply.
	string			*toxml (bool compact, class xmlschema &) const;
	
					 /// Convert from string with XML data.
					 /// \param d The XML text data.
					 /// \param s The schema to use for parsing. NULL for none.
	bool			 fromxml (const string &d, class xmlschema *s = NULL,
							  string *err = NULL);

	bool			 fromxml (const string &d, string &err)
					 {
					 	return fromxml (d, NULL, &err);
					 }

					 /// Convert from string with XML data.
					 /// \param d The XML text data.
					 /// \param s The schema to use for parsing.
	bool			 fromxml (const string &d, class xmlschema &s);
	
					 /// Load from an XML file.
					 /// \param path The file name.
					 /// \param s The schema to be used for parsing.
	bool			 loadxml (const string &path, class xmlschema &s);

					 /// Load from an XML file.
					 /// \param path The file name.
					 /// \param s The schema to be used for parsing. NULL for none.
	bool			 loadxml (const string &path, class xmlschema *s = NULL,
							  string *err = NULL);
	
					 /// Load from an XML file using the default grace
					 /// schema.
					 /// \param path The file name.
					 /// \param err Output string for parser errors.
	bool			 loadxml (const string &path, string &err)
					 {
					 	return loadxml (path, NULL, &err);
					 }
					 
					 /// Load from an XML file with schema and error reporting.
					 /// \param p The file name.
					 /// \param s The XML schema to use.
					 /// \param err Output string for the parser errors.
	bool			 loadxml (const string &p, class xmlschema &s, string &er);

					 /// Convert from an XML string with schema and error reporting.
					 /// \param p The file name.
					 /// \param s The XML schema to use.
					 /// \param err Output string for the parser errors.
	bool			 fromxml (const string &p, class xmlschema &s, string &er);
					 
					 /// Convert from a JSON-encoded string.
					 /// \param j JSON string.
	bool			 fromjson (const string &j);

					 /// Convert to a JSON-encoded string.
	string			*tojson (void) const;
	
	void			 encodegrace (string &into, int indent);

					 /// Convert to grace-style $() notation.
	string			*tograce (void);

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
	void			 saveplist (const string &fn, bool compact=false);
	
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

					 /// Load from the SHOX object format.
					 /// \param fname Filename to load.
	void			 loadshox (const string &fname);
	
					 /// Save to the SHOX object format.
					 /// \param fname Filename to save.
	bool			 saveshox (const string &fname) const;
	
					 /// Convert from SHOX string data.
					 /// \param shox The shox-encoded data.
	void			 fromshox (const string &shox);
	
					 /// Convert to SHOX string data.
	string			*toshox (void) const;
	
					 /// Return true if the value is empty: No data,
					 /// no attributes and no children.
	bool			 isempty (void) const;
	
					 /// Returns true if a child key exists.
	bool			 exists (const char *) const;

					 /// Returns true if a child key exists.
	bool			 exists (unsigned int, const char *) const;

					 /// Returns true if a child key exists.
	bool			 exists (const string &) const;

					 /// Returns true if a child key exists.
	bool			 exists (const statstring &) const;
	
					 /// Returns true if the object is an array, with
					 /// keyed and/or numbered child nodes.
	bool			 isarray (void) const;
	
					 /// Returns true if the object is a pure dict
					 /// with only keyed child nodes.
	bool			 isdict (void) const;
	
					 /// Returns true if the object is an array mix of
					 /// numbered and keyed child nodes.
	bool			 ismixed (void) const;

					 /// Returns true if the object has attributes.
					 /// Deprecated, use value::hasattributes().
	bool			 haveattributes (void) const
					 {
					 	return (attrib != NULL);
					 }
					 
					 /// Returns true if the object has attributes.
	bool			 hasattributes (void) const
					 {
					 	return (attrib != NULL);
					 }

					 /// Access attributes as a value object.
	const value		&attributes (void) const
					 {
					   if (! attrib) return emptyvalue;
					   return *attrib;
					 }

					 /// Access attributes as a value object.
	value			&attributes (void)
					 {
					 	if (! attrib) attrib = new value;
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
					 /// \param id The key to remove.
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

					 /// Compare two value objects on a tree level.
	bool			 treecmp (const value &other) const;

					 /// Access method for the visitor protocol.
	value			*visitchild (const statstring &id) const
					 {
					 	return havechild (id.key(), id.str());
					 }
					 /// Access method for the visitor protocol.
	value			*visitchild (int index) const
					 {
					 	if (index<0) return NULL;
					 	if (!arraysz) return NULL;
					 	if (index >= arraysz) return NULL;
					 	return array[(index<0) ? arraysz-index : index];
					 }

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
	void			 printphp (string &into, bool withattr) const;
	
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
	
					 /// Internal method for SHoX parsing.
	void			 readshox (class stringdict &, size_t &, const string &);
					 
					 /// Internal method for SHoX serialization.
	void			 printshox (string &, stringdict &) const;
		
	void			 encodejsonstring (string &into) const;
	void			 encodejsonid (string &into) const;
	void			 encodejson (string &into) const;
	
	const char		*decodejson (const char *);
	const char		*readjsonstring (const char *, string &);
	const char		*readjsonnumber (const char *, string &);

	dtenum			 _type; ///< The registered type/class.
	string			 s; ///< The string value.
	dtype			 t; ///< The numeric value.
	unsigned char	 itype; ///< The intrinsic type.
	
	unsigned int	 key; ///< Numeric key.
	class statstring _name; ///< String key.
	
	value			*lower, *higher; ///< Hash tree links.
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
	
public:
					 
	void			 init (bool first=true);
};


string *ip2str (unsigned int ipaddr);
unsigned int str2ip (const string &str);

time_t __parse_timestr (const string &);
string *__make_timestr (time_t);

#define SHOX_HAS_CLASSNAME		0x80
#define SHOX_HAS_ATTRIB			0x40
#define SHOX_HAS_CHILDREN		0x20

string *operator% (const char *args, const value &arglist);
value format (const value &);
value format (const value &, const value &);
value format (const value &, const value &, const value &);
value format (const value &, const value &, const value &, const value &);
value format (const value &, const value &, const value &, const value &, const value &);
value format (const value &, const value &, const value &, const value &, const value &, const value &);
value format (const value &, const value &, const value &, const value &, const value &, const value &, const value &);
value format (const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &);
value format (const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &);
value format (const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &);
value format (const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &);
value format (const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &);
value format (const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &);
value format (const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &);
value format (const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &);
value format (const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &, const value &);

#endif
