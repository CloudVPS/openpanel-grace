// ========================================================================
// str.h: String Handling class declarations
//
// (C) Copyright 2004 Pim van Riezen <pi@madscience.nl>
//                    Madscience Labs, Rotterdam 
// ========================================================================

//      012345678 TAB-O-METER should measure 4
//      ^	^

#ifndef _STR_H

#include <grace/value.h>
#define _STR_H 1

#include <grace/statstring.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>

#undef tolower

#include <grace/reg.h>

// ========================================================================
// The refblock is a structure for holding a c-string with a reference
// counter, allowing for copy-on-write.
//
// How this works...
// 
// If the strcpy() method is called (directly or through operator=) with
// a string object as the source argument, the string will copy the
// source string's pointer to the refblock without touching the data.
// The refcount is increased by one.
//
// Operations that alter the string check the value of the refblock's
// refcount. If it is not 0, a local copy of the refblock is created with
// the value of refcount set to 0 and the original refcount is decreased
// by one.
//
// The string class destructor will only free() the refblock if its
// refcount is 0, otherwise the refcount will be decreased.
//
// The operator= method, when it is passed a _pointer_ to a string in
// stead of a reference uses the abovementioned behaviour for a specific
// task: It invokes strcpy() then deletes the pointed-to source object.
//
// This makes it easy for functions and object methods to return a
// pointer to a string object without the need for explicit disposal of
// the return object by the calling function or copying of data.
// ========================================================================

/// Reference counter for string data.
typedef unsigned short refc_t;

/// Unique thread key for string reference blocks.
typedef unsigned short threadref_t;

#ifndef _REFBLOCK_T
#define _REFBLOCK_T 1

/// Shared data object.
/// A refblock is a construct to allow multiple string objects to
/// share their string data if they copy eachother.
struct refblock
{
	refc_t			refcount; ///< Reference count.
	threadref_t		threadref; ///< Id of the creating thread.
	char			v[1]; ///< Actual array.
};

#endif

/// Returns the unique id for the current thread.
threadref_t getref (void);

// ========================================================================
// The string baseclass represents a flexible string object. Through this
// class, buffer overflows can be banished to the past.
// ========================================================================

#ifndef _STRING_T
#define _STRING_T 1

/// Generic string storage.
/// Uses copy-on-write shared storage for efficiency.
class string
{
public:
					 /// Constructor.
					 string (void);
					 
					 /// Copy-constructor (C string).
					 string (const char *);
					 
					 /// Copy-constructor.
					 /// Shares the other object's refblock.
					 string (const string &);
					 
					 /// Copy-constructor (statstring).
					 string (const class statstring &);
					 
					 /// Copy-constructor (deletes original).
					 /// Takes over the other object's refblock, unlinks
					 /// the refblock from the original owner and then
					 /// deletes the old object.
					 string (string *);
					 
					 /// Constructor (pre-allocated).
					 /// Sets up the buffer for the string to at
					 /// least the required size.
					 /// \param sz Requested array size.
					 string (unsigned int sz);
					 
					~string (void);

	// --------------------------------------------------------------------
	// Through the [] operator, an index character can be selected out
	// of the string.
	// --------------------------------------------------------------------
	
					 /// Array access.
					 /// Out-of-range indices return a '\0'.
					 /// \param n Position. Use negative for position from the right.
	inline char		 operator[] (int n) const
					 {
					 	if (! data) return 0;
					 	if (((unsigned int) n) > size) return 0;
					 	if (n < 0)
					 	{
					 		if ((-n) > ((int)size)) return 0;
					 		return data->v[size+n];
					 	}
						return data->v[n];
					 }
	
	// --------------------------------------------------------------------
	// Various string compare methods
	// --------------------------------------------------------------------

	inline bool		 operator== (const char *str) const
					 {
						return eq (str);
					 }

	inline bool		 operator== (const string &str) const
					 {
						return eq (str);
					 }
	
	// --------------------------------------------------------------------
	// String assignment
	// --------------------------------------------------------------------

	inline string	&operator= (const string &str)
					 {
						if (*this != str)
						{
							this->strcpy (str);
						}
						return *this;
					 }
					 
	string	&operator= (class value &);
	string  &operator= (const class value &);
	
	inline string	&operator= (const char *str)
					 {
					 	if (! str)
						{
							this->strcpy ("");
							return *this;
						}
						this->strcpy (str);
						return *this;
					 }
	
	inline string	&operator= (string *str)
					 {
					 	if (! str)
					 	{
					 		crop (0);
					 		return *this;
					 	}
					 	this->strcpy (*str);
					 	delete str;
						return *this;
					 }
					 
	inline			 operator const char * (void) const
					 {
					 	if (data)
							return data->v;
						return (const char *) "";
					 }
	
	// --------------------------------------------------------------------
	// Other operators
	// --------------------------------------------------------------------

	inline bool		 operator!= (const string &str) const
					 {
						return (! eq (str));
					 }
	inline bool		 operator!= (const char *str) const
					 {
					 	return (! eq (str));
					 }
	inline string	&operator+= (const string &str)
					 {
						strcat (str);
						return (*this);
					 }
	inline string	&operator+= (string *str)
					 {
					 	strcat (*str);
						delete str;
						return (*this);
					 }
	
	string			&operator+= (class value *);
	string			&operator+= (class value &);
	
	inline string	&operator+= (const char *str)
					 {
						strcat (str);
						return (*this);
					 }
	inline string	*operator+ (const string &str) const
					 {
					 	string *s = new string(*this);
						s->strcat (str);
						return s;
					 }
					 
					 /// Bool cast. Returns false if the string is empty.
					 operator bool (void) const
					 {
					 	return size ? true : false;
					 }
	
	// --------------------------------------------------------------------
	// Format string
	// --------------------------------------------------------------------

					 /// Add data with libc-style format string.
					 /// There are some extra format characters to keep
					 /// in mind:
					 ///
					 ///   - %%L formats as long long
					 ///   - %%U formats as unsigned long long
					 ///   - %%S Escapes a string argument.
					 ///   - %%Z Escapes a string argument for XML.
					 /// 
					 /// Keep in mind that the formatted text is
					 /// \b added to the string buffer. Use
					 /// string::crop() to empty the buffer first
					 /// if you want to start with a clear string.
	void			 printf (const char *, ...);
	
					 /// Add data from vararg. See string::printf().
	void			 printf_va (const char *, va_list *);

	// --------------------------------------------------------------------
	// Remote trailing CR/LF
	// --------------------------------------------------------------------

					 /// Remove trailing newline.
					 /// Takes away a trailing \\n or \\r\\n sequence.
	void			 striplf (void);

	// --------------------------------------------------------------------
	// Libc memorial methods
	// --------------------------------------------------------------------

					 /// Get string length.
					 /// \return Size in characters.
	unsigned int	 strlen (void) const
					 {
						return size;
					 }
					 
					 /// Add string data (C-string).
	void			 strcat (const char *);
	
					 /// Add string data.
	void			 strcat (const string &);
	
					 /// Add string data (remove original).
					 /// Concatenates the buffer data from the original
					 /// string then removes the original string and its
					 /// refblock.
	void			 strcat (string *);
	
					 /// Add binary data.
					 /// \param dt The data block.
					 /// \param sz Number of bytes to copy.
	void			 strcat (const char *dt, size_t sz);
	
					 /// Add character.
	void			 strcat (char s);
	
					 /// Add a printed integer.
					 /// Converts the integer to a base 10 ASCII
					 /// string and adds this representation to
					 /// the buffer.
	void			 strcat (int);
	
					 /// Add a printed floating point number.
					 /// Converts the double to an ASCII
					 /// representation of the floating point
					 /// data.
	void			 strcat (double);
	
					 /// Add string data at the start.
					 /// Inserts the data from the other string
					 /// at the left side of the buffer.
	void			 insert (const string &);
	
					 /// Copy from c-string.
	void			 strcpy (const char *);
	
					 /// Copy from other string.
	void			 strcpy (const string &);
	
					 /// Clone from other string.
					 /// Ensures that no reference is shared with the
					 /// original.
	void			 strclone (const string &);
	
					 /// Copy from binary data.
					 /// Buffer will be cleared.
					 /// \param dt The data location.
					 /// \param sz The number of ytes to copy.
	void			 strcpy (const char *dt, size_t sz);
	
					 /// Find a sequence.
					 /// \param dat The search data.
					 /// \param sz Size of the sequence.
					 /// \param offset Index to start the search at.
					 /// \return Sequence position or -1.
	int				 strstr (const char *dat, size_t sz, int offset=0) const;

					 /// Find a string sequence.
					 /// \return Sequence position or -1.
	int				 strstr (const char *, int offset=0) const;

					 /// Find a string sequence.
					 /// \return Sequence position or -1.
	int				 strstr (const string &, int offset=0) const;
					 
	// --------------------------------------------------------------------
	// Comparison methods
	// --------------------------------------------------------------------
					 
					 /// Do libc-style string comparison.
	int				 strcmp (const char *) const;
					 /// Do libc-style string comparison.
	int				 strcmp (const string &) const;
					 /// Do libc-style size-limited string comparison.
	int				 strncmp (const string &, int sz=0) const;
					 /// Do libc-style string comparison (case-insensitive).
	int				 strcasecmp (const string &) const;
					 /// Do libc-style string comparison (case-insensitive).
	int				 strcasecmp (const char *) const;
					 /// Do libc-style size-limited string comparison (case-insensitive).
	int				 strncasecmp (const string &, int sz=0) const;
	
					 /// Perform a glob match against a wildcard.
					 /// \param str The glob-style wildcard statement.
					 /// \return Match result, \b true if positive.
	bool			 globcmp (const string &str) const;

					 /// Perform a glob match against a wildcard.
					 /// \param str The glob-style wildcard statement.
					 /// \return Match result, \b true if positive.
	bool			 globcmp (const char *str) const;

					 /// Compare to c string.
	inline bool		 eq (const char *s) const
					 {
						return (! strcmp (s));
					 }
					 
					 /// Compare to other string.
	inline bool		 eq (const string &s) const
					 {
						return (! strcmp (s));
					 }
	
	// --------------------------------------------------------------------
	// Binary stream utility methods
	// --------------------------------------------------------------------
	
					 /// Add binary data.
	size_t			 binput64  (size_t offset, long long val);
					 /// Add binary data.
	size_t			 binput32u (size_t offset, unsigned int val);
					 /// Add binary data.
	size_t			 binput32  (size_t offset, int);
					 /// Add binary data.
	size_t			 binput32o (size_t offset, const char *offs);
					 /// Add binary data.
	size_t			 binput16u (size_t offset, unsigned short val);
					 /// Add binary data.
	size_t			 binput16  (size_t offset, short);
					 /// Add binary data.
	size_t			 binput8u  (size_t offset, unsigned char val);
					 /// Add binary data.
	size_t			 binput8   (size_t offset, char);
					 /// Add a CXML opcode.
	size_t			 binputopc (size_t offset, const char *opcode);
					 /// Add a CXML long long.
	size_t			 binputnum64 (size_t offset, const char *opcode,
								  long long value);
					 /// Add a CXML integer.
	size_t			 binputnum32 (size_t offset, const char *opcode,
								  unsigned int value);
					 /// Add a CXML short.
	size_t			 binputnum16 (size_t offset, const char *opcode,
								  unsigned int value);
					 /// Add a CXML string.
	size_t			 binputstr (size_t offset, const char *opcode,
								const string &value);
					 /// Add a CXML byte.
	size_t			 binputnum8 (size_t offset, const char *opcode,
								 unsigned int value);
	
	// --------------------------------------------------------------------
	// Escape character handling
	// --------------------------------------------------------------------

					 /// Escape data in the buffer. Useful for SQL or other
					 /// areas where quotes, backslashes, the percent-sign
					 /// and control characters are unwanted.
	void			 escape (void);
	
					 /// Unescape data in the buffer. Uses the reverse rules
					 /// of escape().
	void			 unescape (void);
	
					 /// Escape data in the buffer for XML. Encodes any
					 /// characters that would mess up with CDATA-like
					 /// situations:
					 ///   - The ampersand '&' is replaced by the
					 ///     sequence '&amp;'
					 ///   - The less-than symbol '<' is replaced by
					 ///     the sequence '&lt;'
					 ///   - Although not strictly necessary, the
					 ///     greather-than symbol '>' is replaced by
					 ///     the sequence '&gt;'
	void			 escapexml (void);
	
					 /// Unescape data in the buffer from XML.
	void			 unescapexml (void);

	// --------------------------------------------------------------------
	// Conditional sizing
	// --------------------------------------------------------------------

					 /// Remove buffer data without resizing the buffer.
					 /// The strlen() will be reset to 0 but there will
					 /// be no reallocation of the buffer.
	void			 flush (void);
	
					 /// Empty the buffer.
					 /// Reallocates the buffer storage to minimum size.
	void			 crop (void);
	
					 /// Crop the buffer to a set size.
					 /// \param sz Desired maximum size. If negative, keep the rightmost part of the original.
	void			 crop (int sz);
	
					 /// Pad/crop the buffer.
					 /// Useful for creating aligned layouts with
					 /// fixed space fonts.
					 /// \param sz Desired size (only positive).
					 /// \param filler Filler character to use for padding.
	void			 pad (int sz, char filler);
	
	// --------------------------------------------------------------------
	// Substring derivation
	// --------------------------------------------------------------------

					 /// Derive new string from range.
					 /// \param pos Index of the start of the range.
					 /// \param sz Number of bytes to copy, 0 for all.
					 /// \return New object with the result.
	string			*mid (int pos, int sz=0) const;
	
					 /// Derive new string from left range.
					 /// \param sz Number of bytes from the left to copy.
					 /// \return New object with the result.
	inline string	*left (int sz) const
					 {
					 	return mid (0, sz);
					 }
					 
					 /// Derive new string from right range.
					 /// \param sz Number of bytes from the right to copy.
					 /// \return New object with the result.
	inline string	*right (int sz) const
					 {
					 	if (((unsigned int)sz) > size)
							return new string(*this);
						return mid (size-sz, 0);
					 }

	// --------------------------------------------------------------------
	// Base64 encoding
	// --------------------------------------------------------------------

					 /// Create base64-encoded version of self.
					 /// \return New encoded object.
	string			*encode64 (void) const;
	
					 /// Create base64-decoded version of self.
					 /// \return New decoded object.
	string			*decode64 (void) const;

	// --------------------------------------------------------------------
	// Conversion to C string
	// --------------------------------------------------------------------
	
					 /// Create private reference.
					 /// If the string data is shared with other
					 /// references, a new unique copy is allocated
					 /// so that we can mess with our data without
					 /// interfering with that of other string objects.
	inline void		 docopyonwrite (void)
					 {
						 if (data && data->refcount)
						 {
						 	 threadref_t me = getref();
						 	 
							 refblock *old = data;
							 data = (refblock *) malloc ((size_t) alloc);
							 bcopy (old->v, data->v, size);
							 old->refcount--;
							 data->refcount = 0;
							 data->v[size] = 0;
							 data->threadref = me;
						 }
					 }
					 
					 /// Cast to c string.
	inline const char *str (void) const
					 {
					 	return (data ? data->v : "");
					 }
					 
					 /// Cast to c string (alternative name).
	inline const char *cval (void) const
					 {
						 return (data ? data->v : "");
					 }

					 /// Validate string against a set of 'legal' characters.
					 /// \param set The collection of permitted chars.
					 /// \return Status, \b true if validated ok.
	bool			 validate (const string &set) const;
	
					 /// Create a new string with only a subset of
					 /// characters.
					 /// \param set The character set.
					 /// \return Copy with all characters not in the set
					 ///         omitted.
	string			*filter (const string &set);
					 
					 /// Create a new string without the given character
					 /// \param stripchar The character to strip
					 /// \return Copy with the given character stripped 
	string			*stripchar  (char stripchar);
	
					 /// Create a new string without the given characters
					 /// \param stripset Strips a set of characters
					 /// \return Copy without the given characters
	string			*stripchars	(const string &stripset);			 
					 
					 /// Trims charcters from the left and right of the
					 /// current string until it finds an not given 
					 /// character
					 /// \param set Set of special characters
					 /// \return New trimmed string
	string			*trim (const string &set=" ");			 
					 
					 /// Trims charcters from the left of the
					 /// current string until it finds an not given 
					 /// character
					 /// \param set Set of special characters
					 /// \return New trimmed string
	string			*ltrim (const string &set=" ");			 
					 
					 /// Trims charcters from the right of the
					 /// current string until it finds an not given 
					 /// character
					 /// \param set Set of special characters
					 /// \return New trimmed string
	string			*rtrim (const string &set=" ");			 

					 
					 /// Filter the string with a subset of
					 /// characters replaced by a single replacement.
					 /// \param set The character set.
					 /// \param with The character to replace characters
					 ///             in the set with.
					 /// \return Copy with all characters not in the set
					 ///         omitted.
	void			 replace (const string &set, char with);
	
					 /// Find string sequence. 
					 /// \return Sequence osition or \b -1 if not found.
	inline int		 strchr (char c, int left=0) const
					 {
					 	if (left<0) return -1;
					 	if (((unsigned int)left) >= size) return -1;
						
					    char *res = (char *) memchr (data->v+left, c, size-left);
						
						if (res) return (res - (char *) data->v);
						return -1;
					 }
					 
					 /// Split the string in two parts. Returns a new
					 /// object containing the left half. The first
					 /// occurence of a separator character is used
					 /// as a splitting point.
					 /// \param c The character to be used as a separator.
					 ///          It will be included in neither of the two.
					 ///          If the sequence is not found, an empty
					 ///          string object is returned and the local
					 ///          string data is left untouchd.
					 /// \return Cut data.
	inline string	*cutat (char c)
					 {
					 	string *res = new string;
					 	if (! size) return res;
					 	int isthere = strchr (c);
						if (isthere < 0) return res;
						(*res) = *this;
						docopyonwrite();
						(*res).crop (isthere);
						if ( ((unsigned int)isthere+1) >= size )
						{
							data->v[0] = size = 0;
							return res;
						}
						memcpy (data->v, data->v + isthere+1, size - (isthere+1));
						size -= (isthere+1);
						data->v[size] = '\0';
						return res;
					 }
					 
					 /// Split the string in two parts. Returns a new
					 /// object containing the left half. The last
					 /// occurence of a separator character is used
					 /// as a splitting point.
					 /// \param c The character to be used as a separator.
					 ///          It will be included in neither of the two.
					 ///          If the sequence is not found, an empty
					 ///          string object is returned and the local
					 ///          string data is left untouched.
					 ///
					 /// Usage example:
					 /// \verbinclude string_ex1.cpp
					 /// \return Cut data.
	inline string	*cutatlast (char c)
					 {
					 	string *res = new string;
					 	if (! size) return res;
					 	int nextmatch;
					 	int isthere = strchr (c);
						if (isthere < 0) return res;
						while ( (nextmatch = strchr (c, isthere+1)) >= 0 )
							isthere = nextmatch;
						(*res) = *this;
						docopyonwrite();
						(*res).crop (isthere);
						if ( ((unsigned int)isthere+1) >= size )
						{
							data->v[0] = size = 0;
							return res;
						}
						memcpy (data->v, data->v + isthere+1, size - (isthere+1));
						size -= (isthere+1);
						data->v[size] = '\0';
						return res;
					 }
					 
					 /// Split the string in two parts. Returns a new
					 /// object containing the left half. The first
					 /// occurence of a separator sequence is used
					 /// as a splitting point.
					 /// \param c The string to be used as a separator.
					 ///          It will be included in neither of the two.
					 ///          If the sequence is not found, an empty
					 ///          string object is returned and the local
					 ///          string data is left untouched.
					 /// \return Cut data.
	inline string	*cutat (const char *c)
					 {
					 	int ssz = ::strlen (c);
					 	string *res = new string;
					 	if (! size) return res;
					 	int isthere = strstr (c);
					 	if (isthere < 0) return res;
					 	(*res) = *this;
					 	(*res).crop (isthere);
						docopyonwrite();
						if ( ((unsigned int)isthere+1) >= size )
						{
							data->v[0] = size = 0;
							return res;
						}
					 	memcpy (data->v, data->v + isthere+ssz, size - (isthere+ssz));
					 	size -= (isthere+ssz);
					 	data->v[size] = '\0';
					 	return res;
					 }

					 /// Split the string in two parts. Returns a new
					 /// object containing the left half. The last
					 /// occurence of a separator sequence is used
					 /// as a splitting point.
					 /// \param c The string to be used as a separator.
					 ///          It will be included in neither of the two.
					 ///          If the sequence is not found, an empty
					 ///          string object is returned and the local
					 ///          string data is left untouched.
					 /// \return Cut data.
	inline string	*cutatlast (const char *c)
					 {
					 	int ssz = ::strlen (c);
					 	string *res = new string;
					 	if (! size) return res;
					 	int nextmatch;
					 	int isthere = strstr (c);
					 	if (isthere < 0) return res;
					 	while ( (nextmatch = strstr (c, isthere+1))	>0 )
					 		isthere = nextmatch;
					 	(*res) = *this;
					 	(*res).crop (isthere);
						docopyonwrite();
						if ( ((unsigned int)isthere+1) >= size )
						{
							data->v[0] = size = 0;
							return res;
						}
					 	memcpy (data->v, data->v + isthere+ssz, size - (isthere+ssz));
					 	size -= (isthere+ssz);
					 	data->v[size] = '\0';
					 	return res;
					 }
	
					 /// Split the string in two parts. Returns a new
					 /// object containing the right half.
					 /// \param str The separation sequence.
					 /// \return Cut data.
	string			*cutafter (const string &str);
	
					 /// Split the string in two parts. Returns a new
					 /// object containing the right half.
					 /// \param c The separation char.
					 /// \return Cut data.
	string			*cutafter (char c);
	
					 /// Split the string in two parts. Returns a new
					 /// object containing the right half. The split is
					 /// performed at the last location of the separator.
					 /// \param str The separation sequence.
					 /// \return Cut data.
	string			*cutafterlast (const string &str);

					 /// Split the string in two parts. Returns a new
					 /// object containing the right half. The split is
					 /// performed at the last location of the separator.
					 /// \param c The separation char.
					 /// \return Cut data.
	string			*cutafterlast (char c);
	
					 /// Get a copy of the string until the first occurence
					 /// of a separator.
					 /// \param s The separator.
					 /// \return Copied object.
	string			*copyuntil (const string &s);
	
					 /// Get a copy of the string until the first occurence
					 /// of a separator.
					 /// \param c The separator.
					 /// \return Copied object.
	string			*copyuntil (char c);

					 /// Get a copy of the string until the last occurence
					 /// of a separator.
					 /// \param s The separator.
					 /// \return Copied object.
	string			*copyuntillast (const string &s);
	
					 /// Get a copy of the string until the last occurence
					 /// of a separator.
					 /// \param c The separator.
					 /// \return Copied object.
	string			*copyuntillast (char c);

					 /// Get a copy of the string after the first occurence
					 /// of a separator.
					 /// \param s The separator.
					 /// \return Copied object.
	string			*copyafter (const string &s);
	
					 /// Get a copy of the string after the first occurence
					 /// of a separator.
					 /// \param c The separator.
					 /// \return Copied object.
	string			*copyafter (char c);

					 /// Get a copy of the string after the last occurence
					 /// of a separator.
					 /// \param s The separator.
					 /// \return Copied object.
	string			*copyafterlast (const string &s);
	
					 /// Get a copy of the string after the last occurence
					 /// of a separator.
					 /// \param c The separator.
					 /// \return Copied object.
	string			*copyafterlast (char c);
	
					 /// Convert buffer to lowercase.
	void			 ctolower (void)
					 {
						docopyonwrite();
					 	for (unsigned int i=0; i<size; ++i)
							data->v[i] = tolower (data->v[i]);
					 }
					 
					 /// Convert buffer to upper case.
	void			 ctoupper (void)
					 {
						docopyonwrite();
					 	for (unsigned int i=0; i<size; ++i)
							data->v[i] = toupper (data->v[i]);
					 }
	
					 /// Make lowercase copy.
	string			*lower (void) const
					 {
					 	string *res = new string (*this);
					 	res->ctolower();
					 	return res;
					 }

					 /// Make uppercase copy.
	string			*upper (void) const
					 {
					 	string *res = new string (*this);
					 	res->ctolower();
					 	return res;
					 }
					 
					 /// Convert buffer's first character to upper case.
	void			 capitalize (void)
					 {
					 	ctolower();
					 	data->v[0] = toupper (data->v[0]);
					 }
					 
					 /// Cast to integer.
					 /// \param base Optional math base to use.
	int				 toint (int base=10) const;
	
					 /// Strip leading and trailing whitespace.
	void			 chomp (void);

protected:
	unsigned int	 size; ///< Size of the string data.
	unsigned int	 alloc; ///< Allocated array size.
	refblock		*data; ///< Reference block for our string data.
	
};

#endif
#endif
