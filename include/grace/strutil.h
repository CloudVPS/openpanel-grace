#ifndef _STRUTIL_H
#define _STRUTIL_H 1

#include <grace/str.h>
#include <grace/value.h>
#include <grace/regexpression.h>
#include <grace/statstring.h>

#define XML_TAG_NAME "__tag_name"
#define XML_TAG_CLOSED "__tag_closed"
#define XML_EOF "__eof"
#define XML_DATA "__data"

/// Class used for XML parsing.
class xmltag
{
public:
				xmltag (void)
				{
					crsr = 0;
					closed = eof = false;
				}

	statstring	type; //< Tag type.
	string		data; //< Data contents.
	value		properties; //< Tag properties.
	bool		closed; //< True if there was a closing tag.
	bool		eof; //< True if end-of-file reached.
	bool		haschildren; //< True if object has children.
	bool		hasdata; //< True if object has data.
	int			crsr; //< Cursor position.
};

/// String utility class.
/// Contains solely static methods for advanced string parsing and manipulation.
class strutil
{
public:
						 /// Split string into array of words.
	static value		*splitspace (const string &);
	
						 /// Split string into array of words.
						 /// Looks for a specified separator character.
						 /// \param str The string data.
						 /// \param sep The separator character.
	static value		*split (const string &str, char sep);
	
						 /// Split string into array of words.
						 /// Looks for a specified separator sequence.
						 /// \param str The string data.
						 /// \param sep The separator sequence.
	static value		*split (const string &str, const string &seq);
	
						 /// Split string into array of arguments. Accepts
						 /// double and single quotes as means of escaping
						 /// the separator.
						 /// \param str The string data.
						 /// \param sep The separator sequence.
	static value		*splitquoted (const string &str, char sep = 0);
	
						 /// Split CSV-line string data into array.
	static value		*splitcsv (const string &);
	
						 /// Encode string data using CSV rules.
						 /// \return New encoded string object.
	static string		*encodecsv (const string &);
	
						 /// Parse an internet header.
						 /// Stores the header, with its headername as
						 /// the key, inside a value object that can be
						 /// merged by the caller into a value object
						 /// that is to keep all headers.
						 /// Parameters inside a header are also parsed,
						 /// so a situation like "Foo: bar/baz; x=25" will
						 /// yield a res["foo"] containing "bar/baz" with
						 /// an attribute res["foo"]("x") valued as 25.
						 /// \param hdr The header string.
						 /// \return New value object.
	static value 		*parsehdr (const string &hdr);
	
						 /// Scan text to determine whether to use CRLF
						 /// or just LF for end-of-line parsing.
						 /// \param str Text data with newlines.
						 /// \return Status, \b true if CRLF should be used
						 ///         or \b false if LF is the native newline.
	static bool			 crlfornot (const string &str);
	
						 /// Parse a MIME multipart text block.
						 /// \param dat Text data with multipart mime.
						 /// \return New value object with parsed mime data.
	static value		*parsemime (const string &dat);
	
						 /// Split a text block into an array of lines.
						 /// \param str Text data with multiple lines.
						 /// \return New value object with numbered array.
	static value		*splitlines (const string &str);
	
						 /// Encode a string suitable for encoding as a
						 /// parameter in a HTTP GET request.
						 /// \param str The unencoded string.
						 /// \return New string object with encoded data.
	static string		*urlencode (const string &str);
	
						 /// Decode a string from a HTTP GET request.
						 /// \param str The encoded string.
						 /// \return New string object with decoded data.
	static string		*urldecode (const string &);
	
						 /// Derive variables from a GET request.
						 /// \param str Text data in QUERY_STRING format.
						 /// \return New value object with key/value pairs
						 ///         out of the query string.
	static value		*httpurldecode (const string &str);
	
						 /// Encode a keyed list of values as a GET request.
						 /// \param v A value object containing a flat
						 ///          collection of key/value pairs.
						 /// \return New string object with encoded data.
	static string		*httpurlencode (value &v);
	
						 /// Create an unescaped copy of a string.
						 /// \param str Escaped data.
						 /// \return New copied object.
	static string		*unescape (const string &str);
	
						 /// Perform a regular expression on a string.
						 /// \param str The source string.
						 /// \param exp The regular expression.
						 /// \return New copied object.
	static string		*regexp (const string &str, const string &exp);
	
						 /// Create a copy of a text block that applies
						 /// word wrapping.
						 /// \param str The original string.
						 /// \param marg The right margin to apply.
						 /// \return New copied object.
	static string		*wrap (const string &str, unsigned int marg);
	
						 /// Create a copy of a text block that converts
						 /// ASCII to escaped valid html data.
						 /// \return New copied object.
	static string		*htmlize (const string &);
	
						 /// Utility function for parsing XML data.
	static void			 xmlreadtag (xmltag *, const string *);
	
						 /// Convert a string to title caps.
						 /// Capitalizes every word except for
						 /// certqain words like 'the', 'of', 'de', etc.
						 /// \return New copied object.
	static string		*titlecaps (const string &);
	
						 /// Parses a string of text using the scriptparser.
						 /// \param str The text data.
						 /// \param env The variable space.
						 /// \return New string object.
	static string		*valueparse (const string &str, value &env);
	
						 /// Convert a relative or absolute path
						 /// with trailing filename into a string
						 /// containing only the path component.
	static string		*makepath (const string &str);
	
						 /// Create a UUID value in the form
						 /// NNNNNNNN-NNNN-NNNN-NNNN-NNNNNNNN
	static string		*uuid (void);
};

#endif
