// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

// ========================================================================
// str.cpp: String handling class
// ========================================================================

//      012345678 TAB-O-METER should measure 4
//      ^	^

#define _STR_CPP 1

#include <grace/str.h>
#include <grace/regexpression.h>
#include <grace/defaults.h>

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

#define GR_DOUBLE_INDEX(ix) (((unsigned char *)&emagic)[7-(ix)])

namespace grace
{
	static const double emagic = 7.949928895127363e-275;
	double htond (double native)
	{
		double result;
		
		assert(sizeof(double) == 8);
		
		for (int i=0; i<8; i++)
		{
			((unsigned char *)&result)[GR_DOUBLE_INDEX(i)] =
				((unsigned char *)&native)[i];
		}
		return result;
	}
	double ntohd (double net)
	{
		double result;
		
		assert(sizeof(double) == 8);
		
		for (int i=0; i<8; ++i)
		{
			((unsigned char *)&result)[i] =
				((unsigned char *)&net)[GR_DOUBLE_INDEX(i)];
		}
		return result;
	}
}

// Handy macro to round up a size for allocating a buffer to multiples of
// 16 for numbers <256 or multiples of 256 in other cases.
#define GROW(n) ( ((n)<256) ? 16 + ((n)-((n)&15)) : 256 + ((n)-((n)&255)) )

// Conversion table for printing hexadecimal numbers
const char __HEXTAB[] = "0123456789abcdef";

// Conversion table for printing base64 data
const char __B64TAB[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvw"
						"xyz0123456789+/";

// Callback routine for pthreads to de-allocate a thread-specific
// storage containing the unique sequence number for a thread.
void removeref (void *refAddress)
{
	delete (threadref_t *) refAddress;
}

// ========================================================================
// FUNCTION getref
// ---------------
// Returns the unique sequence number for the current thread.
// ========================================================================
threadref_t getref (void)
{
	if (! __THREADED)
		return 0;
		
	static bool createdKey = false;
	static pthread_key_t pkey;
	static lock<threadref_t> sequence;
	
	threadref_t *res;
	
	if (! createdKey)
	{
		exclusivesection (sequence)
		{
			if (! createdKey)
			{
				pthread_key_create (&pkey, removeref);
				sequence = 1;
				createdKey = true;
			}
		}
	}
	
	res = (threadref_t *) pthread_getspecific (pkey);
	if (! res)
	{
		exclusivesection (sequence)
		{
			res = new threadref_t;
			(*res) = sequence++;
			pthread_setspecific (pkey, res);
		}
	}
	
	return (*res);
}

// ========================================================================
// CONSTRUCTOR
// -----------
// This constructor initializes an empty value object
// ========================================================================

string::string (void) : retainable()
{
	init (true);
}

string::string (unsigned int sz) : retainable()
{
	data = NULL;
	size = 0;
	offs = 0;
	alloc = sz + sizeof (refblock);
	data = (refblock *) malloc ((size_t) alloc);
	data->refcount = 0;
	data->threadref = getref();
}

// ========================================================================
// CONSTRUCTOR (c-string)
// ----------------------
// Copy-constructor for pointer-reference to a c-string
// ========================================================================
string::string (const char *s) : retainable()
{
	data = NULL;
	// Verify that the supplied pointer is valid
	
	if (s != NULL)
	{
		// Copy the c-string
		
		offs = 0;
		size = ::strlen (s);
		alloc = GROW(size+1+sizeof (refblock));
		data = (refblock *) malloc ((size_t) alloc);
		::strcpy (data->v, s);
		data->refcount = 0;
		data->threadref = getref();
	}
	else
	{
		// Allocate as empty
	
		size = 0;
		alloc = 0;
		offs = 0;
		data = NULL;
	}
}

string::string (const char *s, size_t sz) : retainable()
{
	data = NULL;
	size = alloc = offs = 0;
	strcpy (s, sz);
}

// ========================================================================
// CONSTRUCTOR
// ========================================================================
string::string (const unsigned char *ss)
{
	data = NULL;
	const char *s = (const char *) ss;
	// Verify that the supplied pointer is valid
	
	if (s != NULL)
	{
		// Copy the c-string
		offs = 0;
		size = ::strlen (s);
		alloc = GROW(size+1+sizeof (refblock));
		data = (refblock *) malloc ((size_t) alloc);
		::strcpy (data->v, s);
		data->refcount = 0;
		data->threadref = getref();
	}
	else
	{
		// Allocate as empty
	
		size = 0;
		offs = 0;
		alloc = 0;
		data = NULL;
	}
}

// ========================================================================
// COPY CONSTRUCTOR
// ----------------
// Creates an image after a referenced string, using copy-on-write
// symantics to preserve memory and performance.
// ========================================================================
string::string (const string &s) : retainable()
{
	data = NULL;
	// Verify the string is not empty
	size = 0;
	alloc = 0;
	offs = 0;
	data = NULL;
	
	if ((size = s.strlen()))
	{
		// Make a copy-on-write reference
	
		if (s.data->threadref != getref())
		{
			strclone (s);
			return;
		}
		offs = s.offs;
		alloc = s.alloc;
		size = s.size;
		data = s.data;
		data->refcount++;
	}
	else
	{
		// Allocate as empty
	
		size = 0;
		offs = 0;
		alloc = 0;
		data = NULL;
	}
}

// ========================================================================
// CONSTRUCTOR (statstring)
// ------------------------
// Yanks the string data out of an existing statstring (which contains
// a string object).
// ========================================================================
string::string (const statstring &s) : retainable()
{
	// Verify the string is not empty
	size = 0;
	alloc = 0;
	data = NULL;
	offs = 0;
	
	if (s && (size = s.sval().strlen()))
	{
		// Make a copy-on-write reference
		threadref_t me = getref();
	
		// That is, if we live on the same planet, copy-on-write
		// is not mixed between threads to make it unnecessary
		// to perform excessive locking.
		if (s.sval().data->threadref != me)
		{
			strclone (s.sval());
			return;
		}
		
		// Copy all the details and just copy the reference.
		alloc = s.sval().alloc;
		size = s.sval().size;
		data = s.sval().data;
		offs = s.sval().offs;
		data->refcount++;
		data->threadref = me;
	}
	else
	{
		// Allocate as empty
		size = 0;
		alloc = 0;
		data = NULL;
	}
}

// ========================================================================
// COPY CONSTRUCTOR (pointer)
// --------------------------
// Performs the same chores as the regular copy-constructor, but deletes
// the original object (useful for picking up function/method returns).
// ========================================================================
string::string (string *s) : retainable()
{
	if (s && s->strlen())
	{
		size = s->size;
		alloc = s->alloc;
		data = s->data;
		offs = s->offs;
		destroyvalue (s);
	}
	else
	{
		size = alloc = offs = 0;
		data = NULL;
		if (s) delete s;
	}
}

// ========================================================================
// DESTRUCTOR
// ----------
// Conditionally releases private data
// ========================================================================
string::~string (void)
{
	// Do we have string data?
	
	if (data)
	{
		// Is it shared by another reference?
	
		if (data->refcount)
		{
			// Decrease the reference count
			data->refcount--;
		}
		else
		{
			// Deallocate the data block
			free (data);
		}
	}
}

// ========================================================================
// METHOD ::operator==
// ========================================================================
bool string::operator== (const value &val) const
{
	return eq (val.sval());
}

bool string::operator== (const statstring &sstr) const
{
	return eq (sstr.sval());
}

// ========================================================================
// METHOD ::operator!=
// ========================================================================
bool string::operator!= (const statstring &sstr) const
{
	return (! eq (sstr.sval()));
}

bool string::operator!= (const value &val) const
{
	return (! eq (val.sval()));
}

// ========================================================================
// METHOD ::operator=
// ========================================================================
string &string::operator= (value &val)
{
	this->strcpy (val.sval());
	return (*this);
}

string &string::operator= (statstring *str)
{
	if (! str)
	{
		this->crop();
		return *this;
	}
	this->strcpy (str->sval());
	delete str;
	return *this;
}

string &string::operator= (const statstring &str)
{
	this->strcpy (str);
	return *this;
}

string &string::operator= (const value &val)
{
	this->strcpy (val.sval());
	return (*this);
}

// ========================================================================
// METHOD ::operator+=
// ========================================================================
string &string::operator+= (value &val)
{
	this->strcat (val.sval());
	return (*this);
}

string &string::operator+= (value *val)
{
	this->strcat (val->sval());
	delete val;
	return (*this);
}

// ========================================================================
// METHOD ::printf
// ---------------
// Formatted 'printing' into the string with sprintf() syntax
// ========================================================================
void string::printf (const char *fmtx, ...)
{
	va_list ap;
	
	va_start (ap, fmtx);
	printf_va (fmtx, &ap);
	va_end (ap);
}

void string::printf_va (const char *_fmtx, va_list *ap)
{
	unsigned char *fmt;
	unsigned char copy[20]; // Temporary storage for a format argument
	unsigned char sprintf_out[256]; // Temporary storage to build a string 
	unsigned char *copy_p; // Iterator
	fmt = (unsigned char *) _fmtx;
	int sz;
	string copy_s;
	
	while (*fmt)
	{
		// If there is no special format character, copy the current
		// character literally into the string
	
		if (*fmt != '%') strcat ((char) *fmt++);
		else
		{
			// A format character, oh joy
		
			++fmt;
			copy[0] = '%';
			for (copy_p = copy+1; copy_p < copy+19;)
			{
				switch ((*copy_p++ = *fmt++))
				{
					case 0:
						fmt--; goto CONTINUE;
					case '%':
						strcat ("%"); goto CONTINUE;
					
					case 'c':
						strcat ((char) va_arg(*ap, int));
						goto CONTINUE;
					
					case 'L':
						*copy_p = 0;
						sprintf ((char *) sprintf_out,
								 "%lli",
								 va_arg(*ap, long long));
						copy_p = sprintf_out;
						goto DUP;
					case 'U':
						*copy_p = 0;
						sprintf ((char *) sprintf_out,
								 "%llu",
								 va_arg(*ap, unsigned long long));
						copy_p = sprintf_out;
						goto DUP;
					case 'd':
					case 'i':
					case 'o':
					case 'u':
					case 'x':
					case 'X':
						*copy_p = 0;
						sprintf ((char *) sprintf_out,
								 (char *) copy,
								 va_arg(*ap, int));
						copy_p = sprintf_out;
						goto DUP;
					case 'e':
					case 'E':
					case 'f':
					case 'g':
						*copy_p = 0;
						sprintf ((char *) sprintf_out,
								 (char *) copy,
								 va_arg(*ap, double));
						copy_p = sprintf_out;
						goto DUP;
					case 'p':
						*copy_p = 0;
						sprintf ((char *) sprintf_out,
								 (char *) copy,
								 va_arg(*ap, void*));
						copy_p = sprintf_out;
						goto DUP;
					
					case 'S':
						copy_p = (unsigned char *) va_arg(*ap, char *);
						if (!copy_p) copy_p = (unsigned char *) "(null)";
						
						while (*copy_p)
						{
							if ( (*copy_p == '%') || (*copy_p == '\\') ||
								 (*copy_p == '\'') || (*copy_p == '\"') )
							{
								strcat ('\\');
								strcat ((char) *copy_p++);
							}
							else if (*copy_p < 32)
							{
								strcat ('%');
								strcat (__HEXTAB [(*copy_p >> 4) & 15]);
								strcat (__HEXTAB [(*copy_p++) & 15]);
							}
							else strcat ((char ) *copy_p++);
						}
						goto CONTINUE;

					case 'Z':
						copy_p = (unsigned char *) va_arg(*ap, char *);
						if (!copy_p) copy_p = (unsigned char *) "(null)";
						
						while (*copy_p)
						{
							if ( (*copy_p == '&') )
							{
								strcat ("&amp;");
								++copy_p;
							}
							else if ( (*copy_p == '<') )
							{
								strcat ("&lt;");
								++copy_p;
							}
							else if ( (*copy_p == '>') )
							{
								strcat ("&gt;");
								++copy_p;
							}
							else if ((*copy_p < 32) || (*copy_p > 127))
							{
								strcat ("&#");
								::sprintf ((char *) sprintf_out,
										   "%i", (int) *copy_p++);
								strcat ((char *) sprintf_out);
								strcat (';');
							}
							else strcat ((char) *copy_p++);
						}
						goto CONTINUE;

					case 'A':
						copy_p = (unsigned char *) va_arg(*ap, char *);
						if (!copy_p) copy_p = (unsigned char *) "(null)";
						
						while (*copy_p)
						{
							if ( (*copy_p == '&') )
							{
								strcat ("&amp;");
								++copy_p;
							}
							else if ( (*copy_p == '<') )
							{
								strcat ("&lt;");
								++copy_p;
							}
							else if ( (*copy_p == '>') )
							{
								strcat ("&gt;");
								++copy_p;
							}
							else if ( (*copy_p == '\"') )
							{
								strcat ("&quot;");
								++copy_p;
							}
							else if ((*copy_p < 32) || (*copy_p > 127))
							{
								strcat ("&#");
								::sprintf ((char *) sprintf_out,
										   "%i", (int) *copy_p++);
								strcat ((char *) sprintf_out);
								strcat (';');
							}
							else strcat ((char) *copy_p++);
						}
						goto CONTINUE;
					
					case 's':
						sz = atoi ((const char *)copy+1);
						copy_p = (unsigned char *) va_arg(*ap, char *);
						if (!copy_p) copy_p = (unsigned char *) "(null)";
						if (sz != 0)
						{
							int asz = sz;
							if (asz < 0) asz = -sz;
							
							copy_s = (const char *) copy_p;
							
							if (copy_s.strlen() < (unsigned int) asz)
							{
								string spc;
								spc = "                                                                                               ";
								spc.crop (asz - copy_s.strlen());
								if (sz < 0)
								{
									copy_s = spc;
									copy_s += (const char *) copy_p;
								}
								else
								{
									copy_s += spc;
								}
							}
							else
								copy_s.crop (sz);
								
							strcat (copy_s);
							copy_p = (unsigned char *) "";
						}
DUP:
						strcat ((char *) copy_p);
						goto CONTINUE;
				}
			}
		}
CONTINUE:;
	}
}

// ========================================================================
// METHOD ::escape
// ---------------
// Removes quotes, percent marks and high ascii/control characters from
// the string.
// ========================================================================
void string::escape (void)
{
	unsigned char c;
	unsigned int   old_size, old_offs;
	refblock *old_data = data;
	old_size = size;
	old_offs = offs;
	
	if (old_data)
	{
		data = NULL;
		size = 0;
		alloc = 0;
		offs = 0;
		for (unsigned int i=0; i<old_size; ++i)
		{
			c = (unsigned char) old_data->v[i+old_offs];
			if ( (c=='%') || (c=='\\') || (c=='\"') || (c=='\'') )
			{
				strcat ('\\');
				strcat ((char ) c);
			}
			else if (c=='\n')
			{
				strcat ('\\');
				strcat ('n');
				if (old_data->v[i+old_offs+1]=='\r') ++i;
			}
			else if (c=='\r')
			{
				if (old_data->v[i+old_offs+1] != '\n')
				{
					strcat ('\\');
					strcat ('r');
				}
			}
			else if ((c<32)||(c>127))
			{
				strcat ('%');
				strcat (__HEXTAB [(c >> 4)&15]);
				strcat (__HEXTAB [c & 15]);
			}
			else strcat ((char ) c);
		}
		if (old_data->refcount)
		{
			old_data->refcount--;
		}
		else
		{
			free (old_data);
		}
	}
}

// ========================================================================
// METHOD ::escapexml
// ========================================================================
void string::escapexml (void)
{
	unsigned char c;
	unsigned int old_size, old_offs;
	refblock *old_data = data;
	old_size = size;
	old_offs = offs;
	
	if (old_data)
	{
		data = NULL;
		size = 0;
		alloc = 0;
		for (unsigned int i=0; i<old_size; ++i)
		{
			c = (unsigned char) old_data->v[i+old_offs];
			if (c == '&')
			{
				strcat ("&amp;");
			}
			else if (c == '<')
			{
				strcat ("&lt;");
			}
			else if (c == '>')
			{
				strcat ("&gt;");
			}
			else if ((c<32)||(c>127))
			{
				strcat ("&#");
				printf ("%i", c);
				strcat (';');
			}
			else strcat ((char ) c);
		}
		if (old_data->refcount)
		{
			old_data->refcount--;
		}
		else
		{
			free (old_data);
		}
	}
}

// Macro converts a hex digit to its value.
#define fromhex(a) (((a-'0'<10) ? (a-'0') : (tolower(a)-'a'))&15)

// ========================================================================
// METHOD ::unescapexml
// ========================================================================
void string::unescapexml (void)
{
	char c;
	unsigned int old_size, old_offs;
	refblock *old_data = data;
	old_size = size;
	old_offs = offs;
	
	if (old_data)
	{
		data = NULL;
		size = 0;
		alloc = 0;
		offs = 0;
		for (unsigned int i=0; i<old_size; ++i)
		{
			c = old_data->v[i+old_offs];
			if (c == '&')
			{
				#define DAT(foo) (((i+foo)<old_size) ? old_data->v[i+old_offs+foo] : 0)
				
				if ( (DAT(1) == 'a') &&
					 (DAT(2) == 'm') &&
					 (DAT(3) == 'p') &&
					 (DAT(4) == ';') )
				{
					strcat ('&');
					i+=4;
				}
				else if ( (DAT(1) == 'l') &&
						  (DAT(2) == 't') &&
						  (DAT(3) == ';') )
				{
					strcat ('<');
					i+=3;
				}
				else if ( (DAT(1) == 'g') &&
						  (DAT(2) == 't') &&
						  (DAT(3) == ';') )
				{
					strcat ('>');
					i+=3;
				}
				else if ( (DAT(1) == '#') &&
						  (DAT(3) == ';') )
				{
					c = ::atoi (old_data->v+old_offs+(i+2));
						 
					strcat (c);
					i+=3;
				}
				else if ( (DAT(1) == '#') &&
						  (DAT(4) == ';') )
				{
					c = ::atoi (old_data->v+old_offs+(i+2));
						 
					strcat (c);
					i+=4;
				}
				else if ( (DAT(1) == '#') &&
						  (DAT(5) == ';') )
				{
					c = ::atoi (old_data->v+old_offs+(i+2));
						 
					strcat (c);
					i+=5;
				}
				else if ( (DAT(1) =='a') &&
					      (DAT(2) =='p') &&
						  (DAT(3) =='o') &&
 						  (DAT(4) =='s') &&
						  (DAT(5) == ';') )
				{
					strcat ('\'');
					i+=5;
				}
				else if ( (DAT(1) =='q') &&
					      (DAT(2) =='u') &&
						  (DAT(3) =='o') &&
 						  (DAT(4) =='t') &&
						  (DAT(5) == ';') )
				{
					strcat ('\"');
					i+=5;
				}
			}
			else strcat (c);
		}
		if (old_data->refcount)
		{
			old_data->refcount--;
		}
		else
		{
			free (old_data);
		}
	}
}

// ========================================================================
// METHOD ::unescape
// -----------------
// Restores a string that has been escaped with the escape method back to
// its original state.
// ========================================================================
void string::unescape (void)
{
	char c;
	unsigned int old_size, old_offs;
	refblock *old_data = data;
	old_size = size;
	old_offs = offs;
	
	if (old_data)
	{
		data = NULL;
		size = 0;
		alloc = 0;
		for (unsigned int i=0; i<old_size; ++i)
		{
			c = old_data->v[i+old_offs];
			if (c == '%')
			{
				if (old_data->v[i+old_offs+1] == '%')
				{
					strcat ('%');
					++i;
				}
				else if (i+2 < old_size)
				{
					c = (fromhex (old_data->v[i+old_offs+1]) << 4) +
						 fromhex (old_data->v[i+old_offs+2]);
						 
					strcat (c);
					i+=2;
				}
			}
			else if (c == '\\')
			{
				if (i+1 < old_size)
				{
					switch (old_data->v[i+old_offs+1])
					{
						case 'r':
							strcat ('\r');
							++i;
							break;
						
						case 'n':
							strcat ('\n');
						    ++i;
							break;
							
						default:
							strcat (old_data->v[i+old_offs+1]);
						    ++i;
							break;
					}
				}
			}
			else strcat (c);
		}
		if (old_data->refcount)
		{
			old_data->refcount--;
		}
		else
		{
			free (old_data);
		}
	}
}

// ========================================================================
// METHOD ::strcat
// ---------------
// Concatenates a a single character to the string.
// ========================================================================
void string::strcat (char s)
{
	// Does the datablock exist? If yes, grow it. If
	// no, make one.
	
	if (data != NULL)
	{
		if (data->refcount)
		{
			refblock *newdata = (refblock *) malloc ((size_t) alloc);
			bcopy (data->v+offs, newdata->v, size+1);
			newdata->refcount = 0;
			data->refcount--;
			data = newdata;
			data->threadref = getref();
			offs = 0;
		}
		
		++size;
		register size_t sz2 = size+2+sizeof (refblock);
		
		// New size bigger than allocated memory?
		
		if (sz2 >= alloc)
		{
			// Calculate new size
			
			alloc = 16 + (sz2 - (sz2 & 15) );
			if (alloc > 128)
				alloc = 256 + (sz2 - (sz2 & 255) );
			
			// Is this a copy-on-write refblock?
			
				data = (refblock *)
					realloc (data, (size_t) alloc);
		}
		
		// Append new data
		
		data->v[offs+size-1] = s;
		if (! data->refcount) data->v[offs+size] = '\0';
	}
	else
	{
		// Create a 14-byte string and insert
		// the new data at the start.
	
		alloc = sizeof (refblock) + 8;
		if (alloc<16) alloc = 16;
		data = (refblock *) malloc ((size_t) alloc);
		size = 1;
		offs = 0;
		data->refcount = 0;
		data->threadref = getref();
		data->v[0] = s;
		data->v[1] = '\0';
	}
}

// ==========================================================================
// METHOD string::strcat
// ==========================================================================
void string::strcat (const statstring &s)
{
	strcat (s.sval());
}

// ========================================================================
// METHOD ::insert
// ---------------
// Inserts data at the beginning of this object's buffer.
// ========================================================================
void string::insert (const string &s)
{
	if ( (data != NULL) )
	{
		if (data->refcount)
		{
			refblock *newdata = (refblock *) malloc ((size_t) alloc);
			bcopy (data->v+offs, newdata->v, size+1);
			newdata->refcount = 0;
			data->refcount--;
			data = newdata;
			data->threadref = getref();
			offs = 0;
		}
		
		unsigned int oldsize = size;
		unsigned int newsize = s.strlen();
		size += newsize;
		
		if ((size+1+sizeof (refblock)) >= alloc)
		{
			alloc = GROW(offs+size+1+sizeof (refblock));
			data = (refblock *) realloc (data, alloc);
		}
		
		if (oldsize)
			memmove (data->v + offs + newsize, data->v + offs, oldsize);
			
		if (newsize)
			memmove (data->v + offs, s.str(), newsize);
			
		data->v[offs+size] = '\0';
	}
	else
	{
		strcpy (s);
	}		
}

// ========================================================================
// METHOD ::strcat
// ---------------
// Concatenate string data from another object to the end of this one.
// ========================================================================
void string::strcat (const string &s)
{
	if ( (data != NULL) )
	{
		if (data->refcount)
		{
			refblock *newdata = (refblock *) malloc ((size_t) alloc);
			bcopy (data->v+offs, newdata->v, size+1);
			newdata->refcount = 0;
			data->refcount--;
			data = newdata;
			data->threadref = getref();
			offs = 0;
		}
		
		unsigned int oldsize = size;
		size += s.strlen();
		
		if ((size+1+sizeof (refblock)) >= alloc)
		{
			alloc = GROW(offs+size+1+sizeof (refblock));
			data = (refblock *) realloc (data, alloc);
		}
		
		memmove (data->v + offs + oldsize, s.str(), s.strlen());
		data->v[offs+size] = '\0';
	}
	else
	{
		strcpy (s);
	}
}

// ========================================================================
// METHOD ::strcat
// ---------------
// Concatenates the data of a pointed-to string object, then deletes it.
// ========================================================================
void string::strcat (string *s)
{
	strcat (*s);
	delete s;
}

// ==========================================================================
// METHOD string::strcat
// ==========================================================================
void string::strcat (const value &v)
{
	strcat (v.sval());
}

// ========================================================================
// METHOD string::utf8len
// ========================================================================
unsigned int string::utf8len (void) const
{
	unsigned int res = 0;
	if (size<2) return size;
	for (unsigned int i=0; i<size; ++i)
	{
		if ((data->v[offs+i] & 0xc0) != 0x80) res++;
	}
	return res;
}

// ========================================================================
// METHOD string::utf8pad
// ========================================================================
void string::utf8pad (int i, char with)
{
	if (i<0) return;
	unsigned int u8len = utf8len();
	unsigned int u8dif = strlen() - u8len;
	if (u8len < (unsigned int) i) pad (i+u8dif, with);
	else crop (utf8pos (i));
}

// ========================================================================
// METHOD string::utf8pos
// ========================================================================
unsigned int string::utf8pos (int i) const
{
	if (! i) return 0;
	if (! size) return 0;
	
	int ii = i;
	unsigned int crsr;
	
	if (i<0)
	{
		crsr = size-1;
		while (crsr && ii)
		{
			if ((data->v[offs+crsr] & 0xc0) != 0x80) ii++;
			crsr--;
		}
		return crsr;
	}
	
	crsr = 0;
	while ((crsr<size) && ii)
	{
		if ((data->v[offs+crsr] & 0xc0) != 0x80) ii--;
		crsr++;
	}
	return crsr;
}

// ========================================================================
// METHOD ::strcat
// ---------------
// A c-string variation that expects the size of the c-string to be known.
// ========================================================================
void string::strcat (const char *s, size_t sz)
{
	if (!sz) return;
	
	if (data && data->refcount)
	{
		refblock *newdata = (refblock *) malloc ((size_t) alloc);
		bcopy (data->v+offs, newdata->v, size+1);
		newdata->refcount = 0;
		data->refcount--;
		data = newdata;
		data->threadref = getref();
		offs = 0;
	}
	
	unsigned int oldsize = size;
	size += sz;
	
	if ((size+1+sizeof (refblock)) >= alloc)
	{
		alloc = GROW(offs+size+1+sizeof (refblock));
		if (oldsize) data = (refblock *) realloc (data, alloc);
		else
		{
			data = (refblock *) malloc (alloc);
			data->refcount = 0;
			data->threadref = getref();
		}
	}
	memmove (data->v + offs + oldsize, s, sz);
	data->v[offs+size] = '\0';
}

// ========================================================================
// METHOD ::strcat
// ---------------
// A c-string variation for c-strings with an unknown size.
// ========================================================================
void string::strcat (const char *s)
{
	bool testOne = false;
	bool testTwo = false;
	bool testThree = false;

	if (data != NULL) testOne = true;

	if ( (s != NULL) && (data != NULL) )
	{
		if (data->refcount)
		{
			testTwo = true;
			refblock *newdata = (refblock *) malloc ((size_t) alloc);
			bcopy (data->v+offs, newdata->v, size+1);
			newdata->refcount = 0;
			data->refcount--;
			data = newdata;
			data->threadref = getref();
			offs = 0;
		}
	
		unsigned int oldsize = size;
		
		unsigned int len = 0;
		while (s[len]) len++;
		
		size += len;
		if ((size+1+sizeof (refblock)) >= alloc)
		{
			testThree = true;
			alloc =  GROW(offs+size+1+sizeof (refblock));
			data = (refblock *) realloc (data, alloc);
		}
		memmove (data->v + offs + oldsize, s, size-oldsize);
		data->v[offs+size] = '\0';
	}
	else
	{
		if (s != NULL) strcpy (s);
	}
}

// ========================================================================
// METHOD ::strcpy
// ---------------
// Data-copy from c-string
// ========================================================================
void string::strcpy (const char *s)
{
	if (! s) return;
	if (! *s)
	{
		crop();
		return;
	}
	
	if (data && data->refcount)
	{
		data->refcount--;
		data = NULL;
		size = alloc = 0;
	}
	
	size = ::strlen (s);
	offs = 0;
	if ((size+1+sizeof (refblock)) >= alloc)
	{
		alloc = GROW(offs+size+1+sizeof (refblock));
		if (data)
		{
			data = (refblock *) realloc (data, alloc);
		}
		else
		{
			data = (refblock *) malloc (alloc);
			data->refcount = 0;
			data->threadref = getref();
		}
	}
	::strcpy (data->v+offs, s);
}

// ========================================================================
// METHOD ::strcpy
// ---------------
// Data-copy from string reference, uses copy-on-write reference to the
// original string's data block.
// ========================================================================
void string::strcpy (const string &s)
{
	if (data && (data == s.data))
	{
		if (! data->refcount) data->refcount = 1;
		offs = s.offs;
		size = s.size;
		return;
	}
	
	if (s.data)
	{
		threadref_t me;
		if (data) me = data->threadref;
		else me = getref();
	
		if (s.data->threadref != me) // different threads
		{
			strclone (s);
			return;
		}
	}
	
	if (data)
	{
		if (data->refcount)
		{
			data->refcount--;
		}
		else
		{
			free (data);
		}
	}

	if (s.data) s.data->refcount++;	
	size = s.strlen();
	alloc = s.alloc;
	data = s.data;
	offs = s.offs;
}

// ========================================================================
// METHOD ::strclone
// -----------------
// Create a clone copy of an original string's data, without using the
// regular copy-on-write references.
// ========================================================================
void string::strclone (const string &s)
{
	if (data)
	{
		if (data->refcount)
		{
			data->refcount--;
		}
		else
		{
			::free (data);
			data = NULL;
		}
	}
	
	size = s.strlen();
	alloc = s.alloc;
	if (s.data)
	{
		data = (refblock *) malloc (alloc);
		data->refcount = 0;
		data->threadref = getref();
		::memmove (data->v, s.data->v+s.offs, size+1);
	}
	else
	{
		data = NULL;
	}
	offs = 0;
}

// ========================================================================
// METHOD strcpy
// -------------
// For situations where you're dealing with Unixy data.
// ========================================================================
void string::strcpy (const char *src, size_t sz)
{
	if (data && data->refcount)
	{
		data->refcount--;
		data = NULL;
		alloc = 0;
	}
	
	size = sz;
	offs = 0;
	if ((size+1+sizeof (refblock)) >= alloc)
	{
		alloc = GROW(offs+size+1+sizeof (refblock));
		if (data)
		{
			data = (refblock *) realloc (data, alloc);
		}
		else
		{
			data = (refblock *) malloc (alloc);
			data->refcount = 0;
			data->threadref = getref();
		}
	}
	::memmove (data->v+offs, src, sz);
	data->v[offs+sz] = 0;
}

// ========================================================================
// METHOD ::strstr
// ---------------
// Search for the occurence of a specified c-string after <offs> characters
// into the string data.
// ========================================================================
int string::strstr (const char *substr, size_t slen, int offset) const
{
	if (data == NULL) return -1;
	if (offset >= (int) size) return -1;
	if (offset < 0) return -1;

	char *ptr = (data->v) + offs + offset;
	char *oldptr = ptr - offset;
	
	while ((ptr = (char *)(memchr (ptr, substr[0], size - (ptr - oldptr)))))
	{
		if (! memcmp (ptr, substr, slen)) return (ptr - (data->v + offs));
			
		++ptr;
		if (((unsigned int)(ptr - oldptr)) >= size) return -1;
	}
	return -1;
}

// ========================================================================
// METHOD ::strstr
// ---------------
// Variation for c-string of unknown size
// ========================================================================
int string::strstr (const char *substr, int offset) const
{
	if (substr == NULL) return -1;
	size_t slen = ::strlen (substr);
	if (! slen) return -1;

	return strstr (substr, slen, offset);
}

// ========================================================================
// METHOD ::strstr
// ---------------
// Variation for string argument.
// ========================================================================
int string::strstr (const string &substr, int offset) const
{
	if (substr.strlen() == 0) return -1;
	
	return strstr (substr.str(), substr.strlen(), offset);
}

// ========================================================================
// METHOD ::strcmp
// ---------------
// Compares the string to a c-string for equality
// ========================================================================
#define METASTRCMP(left,right,leftdone,rightdone) { \
	int res = 0; \
	unsigned int i = 0; \
	while ((!(leftdone)) && (!(rightdone)) && \
		   ((res = ((int) left) - ((int) right)) == 0)) i++; \
	if (res) { return res; } \
	if (leftdone) { \
		if (rightdone) return 0; \
		return -1; \
	} else { \
		if (rightdone) return 1; \
		return 0; \
	} }

int string::strcmp (const char *s) const
{
	if (! data)
	{
		if (s && *s) return -1;
		return 0;
	}
	if (! s ) return -1;
	size_t l = ::strlen (s);
	if (l != strlen()) return -1;
	
	METASTRCMP((int)data->v[offs+i], s[i], i>=size, !s[i]);
}

// ========================================================================
// METHOD ::strcmp
// ---------------
// Variation for a string argument
// ========================================================================
int string::strcmp (const string &s) const
{
	unsigned int osize = s.size;
	
	if (! size) return s.strlen();
	if ((data == s.data) && (offs == s.offs) && (size == s.size)) return 0;
	
	METASTRCMP((int)data->v[offs+i],
			   (int)s.data->v[s.offs+i],
			   i>=size, i>=osize);
}

// ========================================================================
// METHOD ::strcasecmp
// -------------------
// Variation for a case-insensitive comparison
// ========================================================================
int string::strcasecmp (const string &s) const
{
	unsigned int osize = s.size;
	
	if (! size) return s.strlen();
	if ((data == s.data) && (offs == s.offs) && (size == s.size)) return 0;
	
	METASTRCMP((int)tolower(data->v[offs+i]),
			   (int)tolower(s.data->v[s.offs+i]),
			   i >= size, i>= osize);
}

int string::strcasecmp (const char *s) const
{
	if (! size) return (*s != 0);
	METASTRCMP((int)tolower(data->v[offs+i]), (int)tolower(s[i]),
							i>=size, !s[i]);
}

// ========================================================================
// METHOD ::strncasecmp
// --------------------
// Variation for a case-insensitive comparison with length limit
// ========================================================================
int string::strncasecmp (const string &s, int sz) const
{
	if (! size) return (sz);
	unsigned int rsz = sz;
	if (! rsz)
	{
		rsz = s.strlen();
	}
	else if (rsz > s.strlen())
	{
		return 1;
	}

	if (rsz > size) return -1;
	
	METASTRCMP((int)tolower(data->v[offs+i]),
			   (int)tolower(s.data->v[s.offs+i]),
			   (i>=size)||(i>=rsz), i>=rsz);
}

// ========================================================================
// METHOD ::strncmp
// ---------------
// Variation for a string argument with size limit
// ========================================================================
int string::strncmp (const string &s, int sz) const
{
	if (! size) return (sz);
	unsigned int rsz = sz;
	if (! rsz)
	{
		rsz = s.strlen();
	}
	else if (rsz > s.strlen()) return 1;
	if (rsz > size) return -1;
	
	METASTRCMP((int)data->v[offs+i],
			   (int)s.data->v[s.offs+i],
			   (i>=size)||(i>=rsz), i>=rsz);
}

// ========================================================================
// METHOD ::globcmp
// ---------------
// Variation with glob-matching
// ========================================================================
bool string::globcmp (const string &s) const
{
	return globcmp (s.str());
}

bool string::globcmp (const char *s) const
{
	return wild_match ((char *) s, (char *) str());
}

// ========================================================================
// METHOD ::regcmp
// ========================================================================
bool string::regcmp (const string &s) const
{
	regexpression re (s);
	return re.eval (str());
}

// ========================================================================
// METHOD ©
// -------------
// Reset the string size to 0 bytes.
// ========================================================================
void string::crop (void)
{
	crop (0);
}

// ========================================================================
// METHOD ::mid
// ------------
// Creates a sized substring
// ========================================================================
string *string::mid (int pos, int psz) const
{
	if (! data) return NULL;
	if ((psz<0)||(pos > (int) size)) return new string;
	int sz = psz;
	if (!sz) sz = (size-pos);
	if ((pos+sz) > (int) size) sz = (size-pos);
	
	string *res = new (memory::retainable::onstack) string;

	threadref_t me = getref();
	if (data->threadref != me)
	{
		res->strcpy (((const char *)data->v) + pos + offs, sz);
		return res;
	}
	
	res->data = data;
	data->refcount++;
	res->size = sz;
	res->offs = pos+offs;
	res->alloc = alloc;
	
	return res;
}

// ========================================================================
// METHOD ::flush
// --------------
// Remove data without resizing allocation.
// ========================================================================
void string::flush (void)
{
	size = 0;
	if (data)
	{
		if (data->refcount)
		{
			data->refcount--;
			data = NULL;
			alloc = 0;
		}
		else
		{
			data->v[0] = 0;
		}
	}
}

// ========================================================================
// METHOD ::crop
// -------------
// Conditional string resizing: If the string's length exceeds ABS(sz), it
// is chopped off at the beginning (sz<0) or at the end (sz>0).
// ========================================================================
void string::crop (int sz)
{
	unsigned int _sz = (sz < 0) ? -sz : sz;
	
	if (sz == 0)
	{
		if (! data) return;
		
		if (data->refcount)
		{
			data->refcount--;
			data = NULL;
			alloc = size = offs = 0;
		}
		else
		{
			free (data);
			data = NULL;
			alloc = size = offs = 0;
		}
		
		return;
	}
	
	if (! data) return;
	
	if (size > _sz)
	{
		if ((size - _sz) < (unsigned int) tune::str::cropcopylimit)
		{
			if (sz<0) offs = ((size - _sz) + offs);
			size = _sz;
			return;
		}
		
		if (data->refcount)
		{
			refblock *newdata = (refblock *) malloc ((size_t) alloc);
			bcopy (data, newdata, alloc);
			newdata->refcount = 0;
			data->refcount--;
			data = newdata;
			data->threadref = getref();
		}
	
		if (sz<0)
		{
			memmove (data->v, data->v + (size - _sz), _sz);
		}
		size = _sz;
		data->v[offs+size] = '\0';
		if (GROW(offs+size+1+sizeof (refblock)) < alloc)
		{
			alloc = GROW(offs+size+1+sizeof (refblock));
			data = (refblock *) realloc (data, alloc);
		}
	}
}

// ========================================================================
// METHOD ::crop
// -------------
// Conditional string resizing: If the string's length exceeds sz, it
// is chopped off from the end. In cases where the string is shorter
// than the requested size, it is padded up to this length using the
// character passed in the second argument.
// ========================================================================
void string::pad (int psz, char p)
{
	unsigned int sz = (psz<0) ? -psz : psz;
	if (! data) strcat (p);
	
	if (sz < size)
	{
		crop (psz);
		return;
	}
	
	docopyonwrite();
	
	if ((offs+sz+sizeof(refblock)) >= alloc)
	{
		alloc = GROW(offs + sz + sizeof (refblock) +1);
		data = (refblock *) realloc (data, alloc);
	}
	if (p)
	{
		if (psz<0)
		{
			if (size<sz)
			{
				memmove (data->v + offs + (sz-size), data->v + offs, size);
				for (unsigned int i=0; i < (unsigned int)(sz-size); ++i)
				{
					data->v[offs+i] = p;
				}
			}
		}
		else
		{
			for (unsigned int i=size; i < (unsigned int) sz; ++i)
			{
				data->v[offs+i] = p;
			}
		}
	}
	data->v[offs+sz] = '\0';
	size = sz;
}

// ========================================================================
// METHOD ::binput32u
// ========================================================================
size_t string::binput32u (size_t offset, unsigned int val)
{
	docopyonwrite ();
	size_t crsr = offset+4;
	if (crsr > size) pad (crsr, 0);
	data->v[offs+offset]   = (val & 0xff000000) >> 24;
	data->v[offs+offset+1] = (val & 0x00ff0000) >> 16;
	data->v[offs+offset+2] = (val & 0x0000ff00) >> 8;
	data->v[offs+offset+3] = (val & 0x000000ff);
	if (crsr > size) size = crsr;
	return crsr;
}

// ========================================================================
// METHOD ::binput32
// ========================================================================
size_t string::binput32 (size_t offset, int val)
{
	docopyonwrite ();
	size_t crsr = offset+4;
	if (crsr > size) pad (crsr, 0);
	data->v[offs+offset]   = (val & 0xff000000) >> 24;
	data->v[offs+offset+1] = (val & 0x00ff0000) >> 16;
	data->v[offs+offset+2] = (val & 0x0000ff00) >> 8;
	data->v[offs+offset+3] = (val & 0x000000ff);
	if (crsr > size) size = crsr;
	return crsr;
}

// ========================================================================
// METHOD ::binput64
// ========================================================================
size_t string::binput64 (size_t offset, long long val)
{
	docopyonwrite ();
	size_t crsr = offset+8;
	if (crsr > size) pad (crsr, 0);
	data->v[offs+offset]   = (unsigned char) ((val & 0xff00000000000000LL) >> 56);
	data->v[offs+offset+1] = (unsigned char) ((val & 0x00ff000000000000LL) >> 48);
	data->v[offs+offset+2] = (unsigned char) ((val & 0x0000ff0000000000LL) >> 40);
	data->v[offs+offset+3] = (unsigned char) ((val & 0x000000ff00000000LL) >> 32);
	data->v[offs+offset+4] = (unsigned char) ((val & 0x00000000ff000000LL) >> 24);
	data->v[offs+offset+5] = (unsigned char) ((val & 0x0000000000ff0000LL) >> 16);
	data->v[offs+offset+6] = (unsigned char) ((val & 0x000000000000ff00LL) >> 8);
	data->v[offs+offset+7] = (unsigned char) ((val & 0x00000000000000ffLL));
	if (crsr > size) size = crsr;
	return crsr;
}

// ========================================================================
// METHOD ::binput64u
// ========================================================================
size_t string::binput64u (size_t offset, unsigned long long val)
{
	return binput64 (offset, (long long) val);
}

// ========================================================================
// METHOD ::binput16u
// ========================================================================
size_t string::binput16u (size_t offset, unsigned short val)
{
	docopyonwrite ();
	size_t crsr = offset+2;
	if (crsr > size) pad (crsr, 0);
	data->v[offs+offset]   = (val & 0xff00) >> 8;
	data->v[offs+offset+1] = (val & 0x00ff);
	if (crsr > size) size = crsr;
	return crsr;
}

// ========================================================================
// METHOD ::binput8u
// ========================================================================
size_t string::binput8u (size_t offset, unsigned char val)
{
	docopyonwrite ();
	size_t crsr = offset+1;
	if (crsr > size) pad (crsr, 0);
	data->v[offset+offs] = val;
	if (crsr > size) size = crsr;
	return crsr;
}

// ========================================================================
// METHOD ::binput8
// ========================================================================
size_t string::binput8 (size_t offset, signed char val)
{
	docopyonwrite ();
	size_t crsr = offset+1;
	if (crsr > size) pad (crsr, 0);
	data->v[offs+offset] = val;
	if (crsr > size) size = crsr;
	return crsr;
}

// ========================================================================
// METHOD ::binputopc
// ========================================================================
size_t string::binputopc (size_t offset, const char *opcode)
{
	docopyonwrite ();
	size_t crsr = offset+4;
	if (crsr > size) pad (crsr, 0);
	data->v[offs+offset] = opcode[0];
	data->v[offs+offset+1] = opcode[1];
	data->v[offs+offset+2] = opcode[2];
	data->v[offs+offset+3] = opcode[3];
	if (crsr > size) size = crsr;
	return crsr;
}

// ========================================================================
// METHOD ::binputnum32
// ========================================================================
size_t string::binputnum32 (size_t offset, const char *opcode,
						  unsigned int value)
{
	size_t crsr = offset;
	crsr = binputopc (crsr, opcode);
	crsr = binput32u (crsr, 4);
	crsr = binput32u (crsr, value);
	if (crsr > size) size = crsr;
	return crsr;
}

// ========================================================================
// METHOD ::binputnum64
// ========================================================================
size_t string::binputnum64 (size_t offset, const char *opcode,
							long long value)
{
	size_t crsr = offset;
	crsr = binputopc (crsr, opcode);
	crsr = binput32u (crsr, 8);
	crsr = binput64  (crsr, value);
	if (crsr > size) size = crsr;
	return crsr;
}

// ========================================================================
// METHOD ::binputnum16
// ========================================================================
size_t string::binputnum16 (size_t offset, const char *opcode,
						  unsigned int value)
{
	size_t crsr = offset;
	crsr = binputopc (crsr, opcode);
	crsr = binput32u (crsr, 2);
	crsr = binput16u (crsr, value);
	if (crsr > size) size = crsr;
	return crsr;
}

// ========================================================================
// METHOD ::binputnum8
// ========================================================================
size_t string::binputnum8 (size_t offset, const char *opcode,
						  unsigned int value)
{
	size_t crsr = offset;
	crsr = binputopc (crsr, opcode);
	crsr = binput32u (crsr, 1);
	crsr = binput8u (crsr, value);
	if (crsr > size) size = crsr;
	return crsr;
}

// ========================================================================
// METHOD ::binputstr
// ========================================================================
size_t string::binputstr (size_t offset, const char *opcode,
						  const string &value)
{
	size_t crsr;
	
	crsr = offset;
	crsr = binputopc (crsr, opcode);
	crsr = binput32u (crsr, value.strlen());
	if ((crsr+value.strlen()) > size) pad (crsr + value.strlen(), 0);
	memmove (data->v+offs+crsr, value.str(), value.strlen());
	crsr += value.strlen();
	if (crsr > size) size = crsr;
	return crsr;
}

// ========================================================================
// METHOD ::binget8
// ========================================================================
size_t string::binget8 (size_t offset, signed char &into) const
{
	if ((offset+1)>size) return 0;
	into = data->v[offs+offset];
	return offset+1;
}

// ========================================================================
// METHOD ::binget8u
// ========================================================================
size_t string::binget8u (size_t offset, unsigned char &into) const
{
	if ((offset+1)>size) return 0;
	into = data->v[offs+offset];
	return offset+1;
}

// ========================================================================
// METHOD ::binget16
// ========================================================================
size_t string::binget16 (size_t offset, signed short &into) const
{
	if ((offset+2) > size) return 0;
	into = ((data->v[offs+offset]&0xff) << 8) | (data->v[offs+offset+1] & 0xff);
	return offset+2;
}

// ========================================================================
// METHOD ::binget16u
// ========================================================================
size_t string::binget16u (size_t offset, unsigned short &into) const
{
	if ((offset+2) > size) return 0;
	into = ((data->v[offs+offset]&0xff) << 8) | (data->v[offs+offset+1] & 0xff);
	return offset+2;
}

// ========================================================================
// METHOD ::binget32
// ========================================================================
size_t string::binget32 (size_t offset, signed int &into) const
{
	if ((offset+4) > size) return 0;
	into = ((data->v[offs+offset] & 0xff) << 24) |
		   ((data->v[offs+offset+1] & 0xff) << 16) |
		   ((data->v[offs+offset+2] & 0xff) << 8) | 
		   (data->v[offs+offset+3] & 0xff);
	return offset+4;
}

// ========================================================================
// METHOD ::binget32u
// ========================================================================
size_t string::binget32u (size_t offset, unsigned int &into) const
{
	if ((offset+4) > size) return 0;
	into = ((data->v[offs+offset] & 0xff) << 24) |
		   ((data->v[offs+offset+1] & 0xff) << 16) |
		   ((data->v[offs+offset+2] & 0xff) << 8) | 
		   (data->v[offs+offset+3] & 0xff);
	return offset+4;
}

// ========================================================================
// METHOD ::binget64
// ========================================================================
size_t string::binget64 (size_t offset, signed long long &into) const
{
	if (offset+8 > size) return 0;
	into = ((long long)(data->v[offs+offset] & 0xff) << 56) |
		   ((long long)(data->v[offs+offset+1] & 0xff) << 48) |
		   ((long long)(data->v[offs+offset+2] & 0xff) << 40) | 
		   ((long long)(data->v[offs+offset+3] & 0xff) << 32) |
		   ((long long)(data->v[offs+offset+4] & 0xff) << 24) | 
		   ((long long)(data->v[offs+offset+5] & 0xff) << 16) |
		   ((long long)(data->v[offs+offset+6] & 0xff) << 8) | 
		   ((long long)(data->v[offs+offset+7]  & 0xff));
	return offset+8;
}

// ========================================================================
// METHOD ::binget64u
// ========================================================================
size_t string::binget64u (size_t offset, unsigned long long &into) const
{
	if (offset+8 > size) return 0;
	into = ((unsigned long long)(data->v[offs+offset] & 0xff) << 56) |
		   ((unsigned long long)(data->v[offs+offset+1] & 0xff) << 48) |
		   ((unsigned long long)(data->v[offs+offset+2] & 0xff) << 40) | 
		   ((unsigned long long)(data->v[offs+offset+3] & 0xff) << 32) |
		   ((unsigned long long)(data->v[offs+offset+4] & 0xff) << 24) | 
		   ((unsigned long long)(data->v[offs+offset+5] & 0xff) << 16) |
		   ((unsigned long long)(data->v[offs+offset+6] & 0xff) << 8) | 
		   ((unsigned long long)(data->v[offs+offset+7]  & 0xff));
	return offset+8;
}

// ========================================================================
// METHOD ::binputvint
// ========================================================================
size_t string::binputvint (size_t offset, unsigned int val)
{
	size_t vintsz;
	docopyonwrite ();
	
	if (val < 0x40) vintsz = 1;
	else if (val < 0x4000) vintsz = 2;
	else if (val < 0x400000) vintsz = 3;
	else if (val < 0x40000000) vintsz = 4;
	else return 0;
	
	size_t crsr = offset+vintsz;
	if (crsr > size) pad (crsr, 0);
	
	switch (vintsz)
	{
		case 1:
			data->v[offs+offset] = val & 0x3f;
			break;
		
		case 2:
			data->v[offs+offset  ] = 0x40 | (val >> 8);
			data->v[offs+offset+1] = val & 0xff;
			break;
		
		case 3:
			data->v[offs+offset  ] = 0x80 | (val >> 16);
			data->v[offs+offset+1] = (val & 0xff00) >> 8;
			data->v[offs+offset+2] = val & 0xff;
			break;
		
		case 4:
			data->v[offs+offset  ] = 0x80 | (val >> 24);
			data->v[offs+offset+1] = (val & 0xff0000) >> 16;
			data->v[offs+offset+2] = (val & 0x00ff00) >> 8;
			data->v[offs+offset+3] = val & 0xff;
			break;
	}

	if (crsr > size) size = crsr;
	return crsr;
}

// ========================================================================
// METHOD ::bingetvint
// ========================================================================
size_t string::bingetvint (size_t offset, unsigned int &val) const
{
	if (offset > size) return 0;
	if (! data) return 0;
	
	val = 0;
	
	switch (data->v[offs+offset] & 0xc0)
	{
		case 0x00:
			val = (data->v[offs+offset] & 0xff);
			return offset+1;
			
		case 0x40:
			val = ((data->v[offs+offset] & 0x3f) << 8) | 
				  ( data->v[offs+offset+1] &0xff);
			return offset+2;
		
		case 0x80:
			val = ((data->v[offs+offset] & 0x3f) << 16) |
				  ((data->v[offs+offset+1] & 0xff) << 8) |
				  ( data->v[offs+offset+2] & 0xff);
			return offset+3;
		
		case 0xc0:
			val = ((data->v[offs+offset] & 0x3f) << 24) |
				  ((data->v[offs+offset+1] & 0xff) << 16) |
				  ((data->v[offs+offset+2] & 0xff) << 8) |
				  ( data->v[offs+offset+3] & 0xff);
			return offset+4;
	}
	return 0;
}

// ========================================================================
// METHOD ::binputieee
// ========================================================================
size_t string::binputieee (size_t offset, double dat)
{
	docopyonwrite ();
	double net;
	if ((offset+8)>size) pad (offset+8, 0);
	net = grace::htond (dat);
	for (int i=0; i<8; ++i)
	{
		data->v[offs+offset+i] = ((char *)&net)[i];
	}
	return offset+8;
}

// ========================================================================
// METHOD ::bingetieee
// ========================================================================
size_t string::bingetieee (size_t offset, double &dat) const
{
	if (! data) return 0;
	double net;
	if ((offset+8)>size) return 0;
	for (int i=0; i<8; ++i)
	{
		((char *)&net)[i] = data->v[offs+offset+i];
	}
	dat = grace::ntohd (net);
	return offset+8;
}


// ========================================================================
// METHOD ::binputvstr
// ========================================================================
size_t string::binputvstr (size_t offset, const string &str)
{
	size_t cr;
	unsigned int slen = str.strlen();
	
	cr = binputvint (offset, slen);
	if (! cr) return 0;
	
	if ((cr+slen)>size) pad (cr+slen, 0);
	
	::memmove (data->v+cr+offs, str.str(), slen);
	cr += slen;
	return cr;
}

// ========================================================================
// METHOD ::bingetvstr
// ========================================================================
size_t string::bingetvstr (size_t offset, string &into) const
{
	if (! data) return 0;
	size_t crsr;
	unsigned int strsz;
	
	if (offset > size) return 0;
	
	crsr = bingetvint (offset, strsz);
	if (! crsr) return 0;
	
	if ((crsr + strsz) > size) return 0;
	
	into.strcpy (data->v+crsr+offs, strsz);
	crsr += strsz;
	return crsr;
}

// ========================================================================
// METHOD ::encode64
// -----------------
// Create a base64-encoded copy.
// ========================================================================
string *string::encode64 (void) const
{
	returnclass (string) result retain;
	
	if (! strlen()) return &result;
	
	unsigned int pos = 0;
	unsigned int i;
	int remains;
	char inbuf[3];
	char outbuf[5];
	
	outbuf[0] = 0;
	outbuf[4] = 0;
	
	while (true)
	{
		remains = (int) (strlen() - (int) pos);
		if (remains <=0) break;
		
		for (i=0; i<3; ++i)
		{
			unsigned int ix = pos+i;
			if (ix < strlen())
				inbuf[i] = data->v[offs+ix];
			else
				inbuf[i] = 0;
		}
		outbuf[0] = ( inbuf[0] & 0xfc ) >> 2;
		outbuf[1] = (( inbuf[0] & 0x03 ) << 4) |
					(( inbuf[1] & 0xf0 ) >> 4);
		outbuf[2] = (( inbuf[1] & 0x0f ) << 2) |
					(( inbuf[2] & 0xc0 ) >> 6);
		outbuf[3] = inbuf[2] & 0x3f;
		
		for (i=0; i<4; ++i)
		{
			if (i < (unsigned int) (remains+1))
			{
				outbuf[i] = __B64TAB[outbuf[i]&63];
			}
			else outbuf[i] = '=';
		}
		
		result.strcat (outbuf);
		pos += 3;
	}
	return &result;
}

// ========================================================================
// METHOD ::decode64
// -----------------
// Create a base64-decoded copy.
// ========================================================================
string *string::decode64 (void) const
{
	returnclass (string) result retain;
	
	if (! strlen()) return &result;
	
	int bufpos = 0;
	int srcpos = 0;
	char inbuf[4];
	char outbuf[4];
	bool dataleft = true;
	
	while (dataleft && ( srcpos < size ))
	{
		bool ignore = false;
		bool ending = false;

		char c;
		
		c = data->v[offs+srcpos++];
		if (! c) break;
		
		if ( (c >='A') && (c<='Z') ) c = c-'A';
		else if ( (c >='a') && (c<='z') ) c = c-'a' + 26;
		else if ( (c >='0') && (c<='9') ) c = c-'0' + 52;
		else if ( c=='+' ) c = 62;
		else if ( c=='/') c = 63;
		else if ( c=='=') ending = true;
		else ignore = true;
		
		if (! ignore)
		{
			int bufleft = 3;
			
			if (ending)
			{
				if (bufpos == 0) break;
				if (bufpos < 3) bufleft = 1;
				else bufleft = 2;
				while (bufpos<4) inbuf[bufpos++] = 0;
				dataleft = false;
			}
			else inbuf[bufpos++] = c;
			if (bufpos > 3)
			{
				bufpos = 0;
				outbuf[0] = ( inbuf[0]<<2 ) | (( inbuf[1]&0x30 ) >>4);
				outbuf[1] = (( inbuf[1] & 0x0f) <<4) | (( inbuf[2] & 0x3c) >> 2);
				outbuf[2] = (( inbuf[2] & 0x03) <<6) | (inbuf[3] & 0x3f);
				
				for (int i=0; i<bufleft; ++i)
				{
					result.strcat (outbuf[i]);
				}
			}
		}
	}
	return &result;
}

// ========================================================================
// METHOD ::toint
// --------------
// Convert to an integer.
// ========================================================================
int string::toint (int base) const
{
	if (! data) return 0;
	char *end;
	return ::strtol (str(), &end, base);
}

// ========================================================================
// METHOD ::validate
// -----------------
// Validate against a character set.
// ========================================================================
bool string::validate (const string &set) const
{
	if (! data) return true;
	if (! set) return true;
	
	for (unsigned int i=0; i<size; ++i)
	{
		if (set.strchr (data->v[offs+i]) < 0) return false;
	}
	return true;
}

// ========================================================================
// METHOD ::filter
// ---------------
// Return a subset of the string, copying only the characters that fit
// a provided set.
// ========================================================================
string *string::filter (const string &set) const
{
	if (! data) return NULL;
	if (! set) return NULL;
	
	returnclass (string) res retain;
	
	for (unsigned int i=0; i<size; ++i)
	{
		if (set.strchr (data->v[offs+i]) >= 0)
		{
			res.strcat (data->v[offs+i]);
		}
	}
	return &res;
}


// ========================================================================
// METHOD ::striplf
// ---------------
// Return a subset of the string, copying only the characters that do not
// match a linefeed
// ========================================================================
string *string::striplf (void)
{
	return this->rtrim ("\n\r");
}


// ========================================================================
// METHOD ::stripchar
// ---------------
// Return a subset of the string, copying only the characters that do not
// match the provided char
// ========================================================================
string *string::stripchar  (char stripchar)
{
	if (! data) return NULL;
	
	returnclass (string) res retain;
	
	for (unsigned int i=0; i<size; ++i)
	{
		if (data->v[offs+i] != stripchar)
			res.strcat (data->v[offs+i]);
	}
	return &res;
}
	
// ========================================================================
// METHOD ::stripchars
// ---------------
// Return a subset of the string, copying only the characters that do not
// match the provided set
// ========================================================================	
string *string::stripchars	(const string &stripset)
{
	if (! data) return NULL;
	if (! stripset) return NULL;
	
	returnclass (string) res retain;
	
	for (unsigned int i=0; i<size; ++i)
	{
		if ( stripset.strchr (data->v[offs+i]) == -1)
		{
			res.strcat (data->v[offs+i]);
		}
	}
	return &res;
}

// ========================================================================
// METHOD ::trim
// ---------------
// Trim the current string from the left and right until it finds a 
// character which is not in `set`
// ========================================================================	
string *string::trim (const string &set) const
{
	int left = 0;
	int right = size-1;
	
	if (! data) return NULL;
	if (! size) return NULL;
	
	while ((left<right)&&(set.strchr (data->v[offs+left]) >= 0)) left++;
	if (left == (right+1)) return NULL;
	
	while ((right>left)&&(set.strchr (data->v[offs+right]) >= 0)) right--;
	if (left == (right+1)) return NULL;
	
	right++;
	
	return mid (left, (right-left));
}
			 
// ========================================================================
// METHOD ::ltrim
// ---------------
// Trim the current string from the left until it finds a 
// character which is not in `set`
// ========================================================================	
string *string::ltrim (const string &set) const
{
	if(! data) return NULL;
	if(! set)  return NULL;
	
	unsigned int left = 0;
	while ((left<size) && (set.strchr (data->v[offs+left]) >= 0)) left++;
	if (left == size) return NULL;
	
	return mid (left);
}
					 
// ========================================================================
// METHOD ::rtrim
// ---------------
// Trim the current string from the right until it finds a 
// character which is not in `set`
// ========================================================================	
string *string::rtrim (const string &set) const
{
	if (! data) return NULL;
	if (! set)  return NULL;
	if (! size) return NULL;
	
	unsigned int right = size-1;
	while ((right>0) && (set.strchr (data->v[offs+right]) >= 0))
	{
		right--;
	}
	
	if (right<size) right++;
	if (right < 1) return NULL;
	return left (right);
}

// ========================================================================
// METHOD ::replace
// ----------------
// Simply replace all characters from a given set with a single
// replacement character.
// ========================================================================
void string::replace (const string &set, char with)
{
	if (! data) return;
	if (! set) return;
	
	docopyonwrite();
	
	for (unsigned int i=0; i<size; ++i)
	{
		if (set.strchr (data->v[offs+i]) >= 0)
		{
			data->v[offs+i] = with;
		}
	}
}

void string::replace (const value &set)
{
	string res (strlen()+16);
	
	charmatch M;
	str();
	
	foreach (node, set)
	{
		const string &from = node.id().sval();
		const string &to = node.sval();
		int ln = from.strlen();
		
		M.addmatch (from.str(), 0, ln, to);
	}
	
	for (unsigned int i=0; i<size; ++i)
	{
		charmatch *m = M.match (data->v+offs+i, size-i);
		if (m && m->replace)
		{
			res.strcat (*(m->replace));
			i += (m->lenflag-1);
		}
		else res.strcat (data->v[offs+i]);
	}
	
	(*this) = res;
}

// ========================================================================
// METHOD ::copyuntil
// ========================================================================
string *string::copyuntil (char c) const
{
	int pos;
	
	pos = strchr (c);
	if (pos < 0) return new (memory::retainable::onstack) string (*this);
	return left (pos);
}

string *string::copyuntil (const string &s) const
{
	int pos;
	
	pos = strstr (s);
	if (pos < 0) return new (memory::retainable::onstack) string (*this);
	return left (pos);
}

// ========================================================================
// METHOD ::copyuntillast
// ========================================================================
string *string::copyuntillast (char c) const
{
	int pos;
	int npos;
	
	pos = strchr (c);
	if (pos < 0) return new (memory::retainable::onstack) string (*this);
	
	npos = pos;
	while (npos>0)
	{
		pos = npos;
		npos = strchr (c, pos+1);
	}
	return left (pos);
}

string *string::copyuntillast (const string &s) const
{
	int pos;
	int npos;
	
	pos = strstr (s);
	if (pos < 0) return new (memory::retainable::onstack) string (*this);
	
	npos = pos;
	while (npos>0)
	{
		pos = npos;
		npos = strstr (s, pos+1);
	}
	return left (pos);
}


// ========================================================================
// METHOD ::copyafter
// ========================================================================
string *string::copyafter (char c) const
{
	int pos;
	
	pos = strchr (c);
	if (pos < 0) return NULL;
	return mid (pos+1);
}

string *string::copyafter (const string &s) const
{
	int pos;
	
	pos = strstr (s);
	if (pos < 0) return NULL;
	return mid (pos+s.strlen());
}

// ========================================================================
// METHOD ::copyafterlast
// ========================================================================
string *string::copyafterlast (char c) const
{
	int pos;
	int npos;
	
	pos = strchr (c);
	if (pos < 0) return NULL;
	
	npos = pos;
	while (npos>=0)
	{
		pos = npos;
		npos = strchr (c, pos+1);
	}
	return mid (pos+1);
}

string *string::copyafterlast (const string &s) const
{
	int pos;
	int npos;
	
	pos = strstr (s);
	if (pos < 0) return NULL;
	
	npos = pos;
	while (npos>=0)
	{
		pos = npos;
		npos = strstr (s, pos+1);
	}
	return mid (pos+1);
}

// ========================================================================
// METHOD ::ctolower
// ========================================================================
void string::ctolower (void)
{
	docopyonwrite();
	for (unsigned int i=0; i<size; ++i)
		data->v[offs+i] = tolower (data->v[offs+i]);
}

// ========================================================================
// METHOD ::ctoupper
// ========================================================================
void string::ctoupper (void)
{
	docopyonwrite();
	for (unsigned int i=0; i<size; ++i)
		data->v[offs+i] = toupper (data->v[offs+i]);
}

// ========================================================================
// METHOD ::lower
// ========================================================================
string *string::lower (void) const
{
	string *res = new (memory::retainable::onstack) string (*this);
	res->ctolower();
	return res;
}

// ========================================================================
// METHOD ::upper
// ========================================================================
string *string::upper (void) const
{
	string *res = new (memory::retainable::onstack) string (*this);
	res->ctoupper();
	return res;
}

// ========================================================================
// METHOD ::capitalize
// ========================================================================
void string::capitalize (void)
{
	ctolower();
	data->v[offs] = toupper (data->v[offs]);
}

// ========================================================================
// METHOD ::cutafter
// ========================================================================
string *string::cutafter (const string &s)
{
	int pos;
	string *res;
	
	pos = strstr (s);
	if (pos < 0) return NULL;
	
	res = mid (pos+s.strlen());
	size = pos;
	return res;
}

string *string::cutafter (char c)
{
	int pos;
	string *res;
	
	pos = strchr (c);
	if (pos < 0) return NULL;
	
	res = mid (pos+1);
	size = pos;
	return res;
}

// ========================================================================
// METHOD ::cutafterlast
// ========================================================================
string *string::cutafterlast (const string &s)
{
	string *res;
	int pos;
	int npos;
	
	pos = strstr (s);
	if (pos < 0) return NULL;
	
	npos = pos;
	while (npos>0)
	{
		pos = npos;
		npos = strstr (s, pos+1);
	}
	
	res = mid (pos+s.strlen());
	size = pos;
	return res;
}

string *string::cutafterlast (char c)
{
	string *res;
	int pos;
	int npos;
	
	pos = strchr (c);
	if (pos < 0) return NULL;
	
	npos = pos;
	while (npos>0)
	{
		pos = npos;
		npos = strchr (c, pos+1);
	}
	
	res = mid (pos+1);
	size = pos;
	return res;
}

// ========================================================================
// METHOD ::chomp
// --------------
// Cut off white space at either end of a string.
// ========================================================================
void string::chomp (void)
{
	int left = 0;
	int right = size-1;
	
	if (! data) return;
	if (! size) return;

	while ((left<=right)&&(isspace (data->v[offs+left]))) left++;
	if (left > right)
	{
		crop();
		return;
	}
	
	while ((right>=left)&&(isspace (data->v[offs+right]))) right--;
	if (left > right)
	{
		crop();
		return;
	}
	
	right++;
	offs += left;
	size = right-left;
}

void string::chomp (const string &set)
{
	(*this) = trim (set);
}


// ========================================================================
// METHOD ::init
// ========================================================================
void string::init (bool first)
{
	if (first)
	{
		size = 0;
		alloc = 0;
		data = NULL;
		offs = 0;
	}
	else
	{
		crop ();
	}
}

// ========================================================================
// METHOD ::countchr
// ========================================================================
int string::countchr (char c, int endpos) const
{
	if (! data) return 0;
	int res = 0;
	int end = endpos ? endpos : size;
	
	for (int i=0; i < end; ++i) if (data->v[offs+i] == c) ++res;
	return res;
}

// ========================================================================
// METHOD ::visitchild
// ========================================================================
char *string::visitchild (int pos) const
{
	if (! data) return NULL;
	if (pos<0) return NULL;
	if (pos>=(int) size) return NULL;
	return data->v+offs+pos;
}

// ========================================================================
// METHOD ::strchr
// ========================================================================
int string::strchr (char c, int left) const
{
	if (left<0) return -1;
	if (((unsigned int) left) >= size) return -1;
	
	char *res = (char*) memchr (data->v+left+offs, c, size-left);
	
	if (res) return (res - ((char *) data->v+offs));
	return -1;
}

// ========================================================================
// METHOD ::cropat
// ========================================================================
void string::cropat (char c)
{
	if (! size) return;
	int isthere = strchr (c);
	if (isthere<0) return;
	crop (isthere);
}

void string::cropat (const char *c)
{
	if (! c) return;
	if (! size) return;
	int isthere = strstr (c);
	if (isthere<0) return;
	crop (isthere);
}

// ========================================================================
// METHOD ::cropatlast
// ========================================================================
void string::cropatlast (char c)
{
	if (! size) return;
	int isthere, at;
	isthere = at = strchr (c);
	if (isthere<0) return;
	while ((isthere=strchr(c,at+1)) > 0) at = isthere;
	crop (at);
}

void string::cropatlast (const char *c)
{
	if (! c) return;
	if (! size) return;
	int isthere, at;
	isthere = at = strstr (c);
	if (isthere<0) return;
	while ((isthere=strstr(c,at+1)) > 0) at = isthere;
	crop (at);
}

// ========================================================================
// METHOD ::cropafter
// ========================================================================
void string::cropafter (char c)
{
	if (! size) return;
	int isthere = strchr (c);
	if (isthere<0)
	{
		crop();
		return;
	}
	++isthere;
	crop (isthere - strlen());
}

void string::cropafter (const char *c)
{
	if (! c) return;
	if (! size) return;
	int isthere = strstr (c);
	if (isthere<0)
	{
		crop();
		return;
	}
	isthere += ::strlen (c);
	crop (isthere - strlen());
}

// ========================================================================
// METHOD ::cropafterlast
// ========================================================================
void string::cropafterlast (char c)
{
	if (! size) return;
	int isthere, at;
	isthere = at = strchr (c);
	if (isthere<0)
	{
		crop ();
		return;
	}
	while ((isthere=strchr(c,at+1)) >= 0) at = isthere;
	++at;
	crop (at - strlen());
}

void string::cropafterlast (const char *c)
{
	if (! c) return;
	if (! size) return;
	int isthere, at;
	isthere = at = strstr (c);
	if (isthere<0) { crop(); return; }
	while ( (isthere=strstr(c,at+1)) >= 0 ) at = isthere;
	at += ::strlen (c);
	crop (at - strlen());
}

// ========================================================================
// METHOD ::cutat
// ========================================================================
string *string::cutat (char c)
{
	returnclass (string) res retain;
	
	if (! size) return &res;
	int isthere = strchr (c);
	if (isthere < 0) return &res;
	
	res = left (isthere);
	
	if (((unsigned int) isthere+1) >= size )
	{
	   crop ();
	   return &res;
	}
	
	offs += (isthere+1);
	size -= (isthere+1);
	return &res;
}

string *string::cutat (const char *c)
{
	returnclass (string) res retain;
	int ssz = ::strlen (c);
	
	if (! size) return &res;
	int isthere = strstr (c);
	if (isthere < 0) return &res;
	
	res = left (isthere);
	
	if ( ((unsigned int)isthere+ssz) >= size )
	{
		crop ();
		return &res;
	}
	
	offs += (isthere + ssz);
	size -= (isthere + ssz);
	return &res;
}

// ========================================================================
// METHOD ::cutatlast
// ========================================================================
string *string::cutatlast (char c)
{
	returnclass (string) res retain;
	
	if (! size) return &res;
	int nextmatch;
	int isthere = strchr (c);
	if (isthere < 0) return &res;
	
	while ( (nextmatch = strchr (c, isthere+1)) >= 0 )
		isthere = nextmatch;
	
	res = left (isthere);
	
	if ( ((unsigned int)isthere+1) >= size )
	{
		crop ();
		return &res;
	}
	
	offs += (isthere + 1);
	size -= (isthere + 1);
	return &res;
}

string *string::cutatlast (const char *c)
{
	returnclass (string) res retain;
	int ssz = ::strlen (c);

	if (! size) return &res;
	int nextmatch;
	int isthere = strstr (c);
	if (isthere < 0) return &res;
	
	while ( (nextmatch = strstr (c, isthere+1))	>0 )
		isthere = nextmatch;
		
	res = left (isthere);
	
	if ( ((unsigned int)isthere+ssz) >= size )
	{
		crop ();
		return &res;
	}
	
	offs += (isthere + ssz);
	size -= (isthere + ssz);
	return &res;
}

// ========================================================================
// CONSTRUCTOR charmatch
// ========================================================================
charmatch::charmatch (void)
{
	lenflag = 0;
	replace = NULL;
	memset (array, 0, 256 * sizeof (charmatch *));
}

// ========================================================================
// DESTRUCTOR charmatch
// ========================================================================
charmatch::~charmatch (void)
{
	if (replace) delete replace;
	for (int i=0; i<256; ++i)
	{
		if (array[i]) delete array[i];
	}
}

// ========================================================================
// METHOD charmatch::match
// ========================================================================
charmatch *charmatch::match (const char *str, int ln)
{
	if (lenflag) return this;
	if (! ln) return NULL;
	
	charmatch *m = array[(unsigned char)str[0]];
	if (m)
	{
		return m->match (str+1, ln-1);
	}
	return m;
}

// ========================================================================
// METHOD charmatch::addmatch
// ========================================================================
void charmatch::addmatch (const char *seq, int pos, int ln, const string &rep)
{
	if (pos == ln)
	{
		replace = new string (rep);
		lenflag = ln;
		return;
	}
	
	unsigned char c = (unsigned char) seq[pos];
	if (array[c] == NULL)
	{
		array[c] = new charmatch;
	}
	
	array[c]->addmatch (seq, pos+1, ln, rep);
}

// ========================================================================
// METHOD charmatch::replacement
// ========================================================================
const string &charmatch::replacement (void)
{
	if (replace) return *replace;
	return emptystring;
}

string emptystring;
