// ========================================================================
// strutil.cpp: GRACE string manipulation utilities
//
// (C) Copyright 2003-2004 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam 
// ========================================================================

#include <stdio.h>
#include <grace/strutil.h>
#include <grace/statstring.h>
#include <grace/cmdtoken.h>
#include <grace/system.h>
#include <grace/defaults.h>

// ========================================================================
// STATIC METHOD ::splitspace
// --------------------------
// Splits up a string into a value array of strings on whitespace boun-
// daries.
// ========================================================================
value *strutil::splitspace (const string &str)
{
	returnclass (value) res retain;
	
	unsigned int left=0;
	unsigned int right=0;
	
	while ((left<str.strlen())&&(right<str.strlen()))
	{
		while (isspace (str[left])) ++left;
		right = left;
		while ( (right<str.strlen()) && 
			    (! isspace (str[right]))) ++right;
		
		if (right != left)
		{
			res.newval() = str.mid (left, right-left);
		}
		left = right;
	}
	
	return &res;
}

// ========================================================================
// STATIC METHOD ::split
// ---------------------
// Splits up a string into a value array using a provided separator
// character. The separator is not included in any results.
// ========================================================================
value *strutil::split (const string &str, char s)
{
	returnclass (value) res retain;
	
	int left=0;
	int right;
	
	while ((right = str.strchr (s, left)) >= 0)
	{
		if ((right-left)<1) res.newval() = "";
		else res.newval() = str.mid (left, right-left);
		left = right+1;
	}
	res.newval() = str.mid (left);
	
	return &res;
}

// ========================================================================
// STATIC METHOD ::split
// ---------------------
// Splits up a string into a value array using a provided separator
// string. The separator is not included in any results.
// ========================================================================
value *strutil::split (const string &str, const string &s)
{
	returnclass (value) res retain;
	
	int left = 0;
	int right;
	int lns = s.strlen();
	
	while ((right = str.strstr (s, left)) >= 0)
	{
		if ((right-left)<1) res.newval() = "";
		else res.newval() = str.mid (left, right-left);
		left = right+lns;
	}
	res.newval() = str.mid (left);
	
	return &res;
}

// ========================================================================
// STATIC METHOD ::crlfornot
// -------------------------
// Determines whether a string object contains <CRLF> or regular Unix
// end-of-line characters.
// ========================================================================
bool strutil::crlfornot (const string &str)
{
	int crlf, nl;
	value *res = NULL;
	
	crlf = str.strstr ("\r\n");
	nl = str.strchr ('\n');
	
	if ( (crlf<0) && (nl<0) )
	{
		// neither sequence occurs, technically it shouldn't matter
		// so we assume CRLF behavior.
		return true;
	}
	
	// If there's no CRLF sequence _or_ if a unix newline occurs before
	// the first CRLF, assume unix newlines.
	if ((crlf<0) || ((nl>=0) && (nl < crlf)))
		return false;

	// Ok, so CRLF it is	
	return true;
}

// ========================================================================
// STATIC METHOD ::parsemime
// -------------------------
// Takes a string containing one or multiple lines of MIME headers,
// optionally followed by a blank line and other content and processes
// the mime-headers into a value dictionary. If there was a data segment,
// it will be added as a string object in the dictionary keyed ".data".
// ========================================================================
value *strutil::parsemime (const string &str)
{
	returnclass (value) res retain;

	string hdr;
	string content;
	bool crlf;
	
	// Make a working copy of the string
	content = str;
	
	// Separate the mime-header from the data
	crlf = strutil::crlfornot (str);
	hdr = content.cutat (crlf ? "\r\n\r\n" : "\n\n");
	
	value hdrlin;
	
	// Split the header into lines
	hdrlin = strutil::splitlines (hdr);
	foreach (lin, hdrlin)
	{
		res << strutil::parsehdr (lin.sval());
	}
	
	res[".data"] = content;
	return &res;
}

// ========================================================================
// STATIC METHOD ::splitlines
// --------------------------
// Splits a string consisting of multiple lines of text into a value
// array of strings.
// ========================================================================
value *strutil::splitlines (const string &str)
{
	value *res;
	int left = 0;
	int right;
	
	int crlf, nl;

	if (! strutil::crlfornot (str))	
		res = strutil::split (str, '\n');
	
	else
		res = strutil::split (str, "\r\n");
		
	if (res->count() && (! (*res)[-1].sval().strlen()) )
		res->rmindex (res->count() -1);
		
	return res;
}

// ========================================================================
// STATIC METHOD ::splitquoted
// ---------------------------
// Splits a string on a character boundary, but gives special meaning
// to text between quotes and characters that are backspace-escaped, making
// it suitable for parsing program statements.
// ========================================================================
value *strutil::splitquoted (const string &str, char s, bool forxml)
{
	returnclass (value) res retain;

	int left=0;
	int right=0;
	int sz = str.strlen();
	int lastnspace = 0;
	bool escaped = false;
	
	char quot = 0;
	
	if (! str.strlen()) return &res;
	
	while (right <= sz)
	{
		char c = str[right];
		escaped = false;
		
		if ( (!forxml) && (c == '\\') )
		{
			if ((right+1)<sz) ++right;
			lastnspace = right;
			escaped = true;
			c = str[right];
			if (c == '\\') escaped = false;
		}
		if (!escaped)
		{
			if (c && quot)
			{
				lastnspace = right;
				if (c == quot)
				{
					quot = 0;
				}
			}
			else
			{
				if ( (!s) && (isspace (c)) ) c = 0;
				if ((c == '\"')||( (!forxml) && (c == '\'') ))
				{
					quot = c;
				}
				else if ((c == s)||(right == sz))
				{
					while ( (left<lastnspace) && 
							(isspace(str[left]))) ++left;
					
					if ((str[left] == '\"')||((!forxml)&&(str[left] == '\'')))
					{
						if ( (str[left] == str[lastnspace]) && 
							 (lastnspace>left))
						{
							++left;
							--lastnspace;
						}
					}
					
					if (((lastnspace+1)-left) < 1)
						res.newval() = "";
						
					else
					{
						string tmp;
						
						tmp = str.mid (left, (lastnspace+1) - left);
						if (! forxml) res.newval() = strutil::unescape (tmp);
						else res.newval() = tmp;
					}
					left = right+1;
				}
				else if (! isspace (c))
				{
					lastnspace = right;
				}
			}
		}
		++right;
	}
	
	return &res;
}

// ========================================================================
// STATIC METHOD ::encodecsv
// -------------------------
// Encode a string for csv export with proper escaping of the quote
// character (by doubling it).
// ========================================================================
string *strutil::encodecsv (const string &data)
{
	returnclass (string) res retain;
	
	char c;
	int sz = data.strlen();
	
	for (int i=0; i<sz; ++i)
	{
		c = data[i];
		if (c == '\"') res.strcat ('\"');
		res.strcat (c);
	}
	return &res;
}

// ========================================================================
// STATIC METHOD ::splitcsv
// ------------------------
// Split up a line of CSV data into an arrary.
// ========================================================================
value *strutil::splitcsv (const string &str)
{
	returnclass (value) res retain;

	int right=0;
	int left = 0;
	int sz = str.strlen();
	bool quoted = false;
	bool wasquoted = false;
	string nval;
	
	char quot = 0;
	while (right < sz)
	{
		char c = str[right];
		if (c == '\"')
		{
			if (quoted)
			{
				if (((right+1)<sz) && (str[right+1] == '\"'))
				{
					nval.strcat ('\"');
					++right;
				}
				else
				{
					quoted = false;
				}
			}
			else
			{
				quoted = true;
				wasquoted = true;
			}
		}
		else if ( (!quoted) && (c == ',') )
		{
			if (! wasquoted)
			{
				if (nval.strchr ('.') >= 0)
				{
					res.newval() = ::atof (nval.str());
				}
				else
				{
					res.newval() = nval.toint();
				}
			}
			else
			{
				res.newval() = nval;
			}
			nval = "";
			wasquoted = false;
		}
		else
		{
			nval.strcat (c);
		}
		++right;
	}
	
	if (nval.strlen())
	{
		if (! wasquoted)
		{
			if (nval.strchr ('.') >= 0)
			{
				res.newval() = ::atof (nval.str());
			}
			else
			{
				res.newval() = nval.toint();
			}
		}
		else
		{
			res.newval() = nval;
		}
	}
	else
	{
		if (wasquoted) res.newval() = "";
	}
	return &res;
}

// ========================================================================
// STATIC METHOD ::parsehdr
// ------------------------
// Parses a single MIME header line into a value object with attributes
// for mime attributes, meaning that a header like this:
//
//    Foobar: baz; quux="wibble"
//
// will result in a value object like this:
//
//    <string id="foobar" quux="wibble">baz</string>
//
// This makes it easy to parse the actual meaning of a mime header.
// ========================================================================
value *strutil::parsehdr (const string &hdr)
{
	returnclass (value) res retain;
	
	int clnpos = hdr.strchr (':');
	if (clnpos < 0)
	{
		res = hdr;
		return &res;
	}
	
	string hdrname;
	hdrname = hdr.left(clnpos);
	
	++clnpos;
	while (hdr[clnpos]==' ') ++clnpos;
	
	string hval;
	hval = hdr.mid(clnpos);
	
	value split;
	split = strutil::splitquoted (hval, ';');
	
	if ((split[0].sval().strchr ('(') < 0) ||
	    (split[0].sval().strchr (')') > 0))
	{
		res[hdrname] = split[0];
		split.rmindex (0);
		
		foreach (namevalue, split)
		{
			value nvpair;
			string sub;
			int nospc;
			
			nvpair = strutil::splitquoted (namevalue.sval(), '=');
			
			if (nvpair.count()>1)
				res[hdrname].setattrib (nvpair[0].sval(), nvpair[1].sval());
			else
				res[hdrname].setattrib (namevalue.sval(), "1");
		}
	}
	else
	{
		res[hdrname] = hval;
	}
	return &res;
}

// ========================================================================
// STATIC METHOD ::urldecode
// -------------------------
// Decodes a string that is encoded with %XX escapes for certain
// characters where XX is a hexadecimal representation of an ASCII
// character and the '+' character represents a whitespace.
// ========================================================================
string *strutil::urldecode (const string &src)
{
	returnclass (string) res retain;
	
	int ln = src.strlen();
	char c;
	
	for (int i=0; i<ln; ++i)
	{
		c = src[i];
		if (c=='+') c = ' ';
		else
		{
			if ((c=='%')&&((i+2)<ln))
			{
				c = (isdigit (src[i+1]) ? (src[i+1] - '0') 
									    : ((src[i+1]&0x0f) + 9));
				c *= 16;
				c += (isdigit (src[i+2]) ? (src[i+2] - '0')
										 : ((src[i+2]&0x0f) + 9));
				i += 2;
			}
		}
		res.strcat (c);
	}
	return &res;
}

// ========================================================================
// STATIC METHOD ::urlencode
// -------------------------
// Encodes all non-alphabetic characters in a string to %XX escapes and
// all spaces to the '+' character.
// ========================================================================
string *strutil::urlencode (const string &src)
{
	returnclass (string) res retain;
	
	int ln = src.strlen();
	char c;
	
	for (int i=0; i<ln; ++i)
	{
		c = src[i];
		if ((c != '-') && (! isdigit (c)) && (! isalpha (c)))
		{
			if (c == ' ') res.strcat ('+');
			else
			{
				res.strcat ('%');
				res.printf ("%02x", (unsigned char) c);
			}
		}
		else res.strcat (c);
	}
	return &res;
}

// ========================================================================
// STATIC METHOD ::unescape
// ------------------------
// Undoes the 'escape' method of encoding a string suitable for putting
// between double quotes in an external language expression.
// ========================================================================
string *strutil::unescape (const string &src)
{
	returnclass (string) res retain;
	
	int i=0;
	int mx=src.strlen();
	char leftd, rightd;
	
	for (i=0;i<mx;++i)
	{
		if (src[i]=='\\')
		{
			++i;
			res.strcat (src[i]);
		}
		else
		{
			if (src[i]=='%')
			{
				if (((i+1)<mx)&&(src[i+1] == '%'))
				{
					res.strcat ('%');
					i++;
				}
				else if ((i+2)<mx)
				{
					leftd = tolower(src[i+1]);
					rightd = tolower(src[i+2]);
					
					leftd = (((leftd>='0')&&(leftd<='9')) ? 
									leftd - '0' : 
									(leftd - 'a')+10 ) & 15;
									
					rightd = (((rightd>='0')&&(rightd<='9')) ? 
									rightd - '0' : 
									(rightd - 'a')+10 ) & 15;
									
					res.strcat ((char) (rightd + (leftd << 4)));
					i+=2;
				}
			}
			else
			{
				res.strcat (src[i]);
			}
		}
	}
	
	return &res;
}

// ========================================================================
// STATIC METHOD ::regexp
// ----------------------
// Performs a regular expression on a string, returns the resulting string.
// ========================================================================
string *strutil::regexp (const string &src, const string &expr)
{
	returnclass (string) result retain;
	
	regexpression r(expr);
	result = r.parse (src);
	return &result;
}

// ========================================================================
// STATIC METHOD ::wrap
// --------------------
// Takes an ASCII text consisting of only paragraphs with hard newlines
// and adds soft newlines to wrap to a specified width.
// ========================================================================
string *strutil::wrap (const string &src, unsigned int width)
{
	returnclass (string) res retain;
	
	value lines;
	value words;
	unsigned int crsr;
	unsigned int wd;
	
	lines = strutil::split (src, '\n');
	
	foreach (line, lines)
	{
		words = strutil::split (line, ' ');
		crsr = 0;
		int j=0;
		
		foreach (cword, words)
		{
			string word = cword;
			wd = word.strlen();
			if (j) wd+= 1;
			
			if (crsr + wd > width)
			{
				if (crsr)
				{
					res += "\n";
					crsr = 0;
					--wd;
				}
				if (wd > width)
				{
					string lft;

					while (word.strlen() > width)
					{					
						lft = word.left (width);
						word = word.mid (width);
						res.printf ("%s\n", lft.str());
					}
					
					wd = word.strlen();
					crsr = 0;
				}
			}
			
			if (j && crsr) res += " ";
			res += word;
			crsr += wd;
			j++;
		}
		res += "\n";
	}
	
	return &res;
}

// ========================================================================
// STATIC METHOD ::htmlize
// -----------------------
// Converts a string so that it is safe for inclusion into a HTML page
// without risks of cross site scripting.
// ========================================================================
string *strutil::htmlize (const string &src)
{
	returnclass (string) res retain;
	
	int ln = src.strlen();
	char c;
	
	for (int i=0; i<ln; ++i)
	{
		c = src[i];
		switch (c)
		{
			case '<':
				res.strcat ("&lt;");
				break;
			
			case '>':
				res.strcat ("&gt;");
				break;
			
			case '\r':
				if (src[i+1] != '\n') res.strcat ("<br>\n");
				break;
			
			case '\n':
				res.strcat ("<br>\n");
				break;
			
			default:
				res.strcat (c);
				break;
		}
	}

	return &res;
}

// ========================================================================
// STATIC METHOD ::xmlreadtag
// --------------------------
// Reads tag data from a string. Used by the value class.
// ========================================================================
void strutil::xmlreadtag (xmltag *tag, const string *xml)
{
	string tagBuf;
	string tstr;
	
	int leftb;
	int rightb;
		
	tag->properties.clear();
	tag->data.crop(0);
	tag->closed = false;
	tag->eof = false;
    tag->haschildren = false;
	tag->hasdata = false;
	
	// find the opening of the next tag.
	leftb = xml->strchr ('<', tag->crsr);
	rightb = leftb+1;
	
	// Find the end of the tag.
	bool inquote = false;
	if (leftb >=0) while (true)
	{
		if ((*xml)[rightb] == '\"') inquote = !inquote;
		else if ((*xml)[rightb] == '>')
		{
			if (! inquote) break;
		}
		++rightb;
		if (rightb == (int) xml->strlen())
		{
			rightb = -1;
			break;
		}
	}
		
	if ( (leftb<0) || (rightb<0) )
	{
		if (rightb<0)
		{
			tag->errorcond = true;
			tag->errorstr = "Unclosed XML tag at end of stream";
			tag->line = xml->countchr ('\n', leftb) +1;
		}
		tag->eof = true;
		return;
	}
	
	++leftb;
	tagBuf = xml->mid (leftb, rightb-leftb);

	tag->crsr = rightb+1;
	
	if (! tagBuf.strlen())
	{
		tag->eof = true;
		return;
	}
	
	if (tagBuf[0] == '!')
	{
		tag->type = "!";
		return;
	}
	
	if (tagBuf[0] == '?')
	{
		tag->closed = true;
		tag->type= "?";
		return;
	}

	if ( (tagBuf[tagBuf.strlen()-1] == '/') )
	{
		tag->closed = true;
		tagBuf.crop (tagBuf.strlen() -1);
	}
	
	value *tagAndProperties;
	value *propertyPair;
	string tagName;
	string searchStr;
	string ncomp;
	statstring propertyName;
	string propertyValue;
	string firstValue;

	tagAndProperties = strutil::splitquoted (tagBuf, 0, true);
	tagName = (*tagAndProperties)[0];

	tag->type = tagName;
	
	for (int arg=1; arg < (*tagAndProperties).count(); ++arg)
	{
		propertyPair = strutil::splitquoted
			((*tagAndProperties)[arg].sval(), '=', true);
		
		propertyName = (*propertyPair)[0].sval();
		propertyValue = (*propertyPair)[1].sval();
		propertyValue.unescapexml ();
		
		if (tag->properties.exists (propertyName))
		{
			if (! tag->properties[propertyName].count())
			{
				firstValue = tag->properties[propertyName].sval();
				tag->properties.rmval (propertyName);
				tag->properties[propertyName].newval() = firstValue;
			}
			tag->properties[propertyName].newval() = propertyValue;
		}
		else
		{
			tag->properties[propertyName] = propertyValue;
		}

		delete propertyPair;
	}
	
	delete tagAndProperties;
	tag->haschildren = false;

	if ( (! tag->closed) && (tagName[0] != '?') && (tagName[0] != '/') )
	{
		searchStr.crop (0);
		searchStr.printf ("</%s", tagName.str());
		int ctag;
		int ntag;
		
		ntag = xml->strchr ('<', tag->crsr-1);
		
		if (ntag < 0)
		{
			tag->eof = true;
			return;
		}
		
		if ( (*xml)[ntag+1] == '/')
		{
			ncomp = xml->mid (ntag+2, tagName.strlen());
			
			if ( (! defaults::xml::strictbalance) || (ncomp == tagName) )
			{
				if (ntag - (tag->crsr))
				{
					tag->data = xml->mid (tag->crsr, ntag - (tag->crsr));
					tag->data.unescapexml();
				}
				else tag->data = "";
				tag->crsr = ntag;
				tag->closed = true;
				ntag = xml->strchr ('>', ntag);
				if (ntag>0) tag->crsr = ntag+1;
			}
			else
			{
				tag->eof = true;
				tag->errorcond = true;
				tag->errorstr = "Unbalanced closing tag";
				tag->line = xml->countchr ('\n', ntag) +1;
				return;
			}
			tag->haschildren = false;
		}
		else
		{
			ncomp = xml->mid (ntag+1, 8);
			if (ncomp == "![CDATA[")
			{
				rightb = xml->strstr ("]]>",ntag);
				if (rightb<0)
				{
					tag->eof = true;
					return;
				}
				tag->hasdata = true;
				tag->data = xml->mid (ntag+9, rightb - (ntag+9));
			}
			else
			{
				tag->haschildren = true;
				for (int ic=tag->crsr; ic<ntag; ++ic)
				{
					if (! isspace ((*xml)[ic]))
					{
						tag->hasdata = true;
						break;
					}
				}
				if (tag->hasdata)
				{
					ntag = xml->strstr (searchStr, tag->crsr);
					if (ntag > tag->crsr)
					{
						if (! defaults::xml::permitmixed)
						{
							int lts;
							lts = xml->strchr ('<', tag->crsr);
							if (lts > tag->crsr)
							{
								tag->errorcond = true;
								tag->errorstr = "mixed sub-nodes and data";
								tag->line = xml->countchr ('\n', leftb) +1;
								tag->eof = true;
								return;
							}
						}
						
						tag->data = xml->mid (tag->crsr, ntag - (tag->crsr));
						tag->data.unescapexml();
						tag->crsr = ntag;
						tag->closed = true;
						ntag = xml->strchr ('>', ntag);
						if (ntag>0) tag->crsr = ntag+1;
					}
					else tag->data = "";
				}
			}
		}
	}
}

// ========================================================================
// STATIC METHOD ::titlecaps
// -------------------------
// Does a best effort to convert a string to Book Title Capitalization
// rules.
// ========================================================================
string *strutil::titlecaps (const string &src)
{
	returnclass (string) res retain;
	
	static value midwords;
	value splitup;
	string myword;
	
	if (! midwords.exists ("of"))
	{
		midwords["a"] = true;
		midwords["an"] = true;
		midwords["in"] = true;
		midwords["of"] = true;
		midwords["the"] = true;
		midwords["de"] = true;
		midwords["van"] = true;
		midwords["aan"] = true;
		midwords["ter"] = true;
		midwords["der"] = true;
		midwords["te"] = true;
	}
	
	splitup = strutil::splitspace (src);
	
	int i=0;
	foreach (word, splitup)
	{
		myword = word.sval();
		
		if ( (!i) || (! midwords.exists(myword)) )
		{
			myword.capitalize();
		}
		if (i) res.strcat (' ');
		res.strcat (myword);
		i++;
	}
	return &res;
}

// ========================================================================
// STATIC METHOD ::httpurldecode
// -----------------------------
// Decodes name/value pairs encoded with urlencoding as used by HTTP POST
// requests and the like.
// ========================================================================
value *strutil::httpurldecode (const string &data)
{
	returnclass (value) result retain;

	value ampSplit;
	value pairSplit;
	
	ampSplit = strutil::split (data, '&');
	foreach (e, ampSplit)
	{
		string elm;
		string key;
		string val;
		
		elm = e.sval();
		key = elm.cutat ('=');
		val = strutil::urldecode (elm);
		
		if (key.strlen())
		{
			result[key] = val;
		}
	}
	
	return &result;
}

// ========================================================================
// STATIC METHOD ::httpurlencode
// -----------------------------
// Encodes a value object using urlencode. Assumes a flat dictionary.
// ========================================================================
string *strutil::httpurlencode (value &data)
{
	returnclass (string) result retain;
	bool didfirst = false;
	
	foreach (nameval, data)
	{
		string encd;
		if (nameval.id())
		{
			encd = strutil::urlencode (nameval.sval());
			result.printf ("%s%s=%s", didfirst ? "&" : "",
						   nameval.name(), encd.str());
			didfirst = true;
		}
	}
	
	return &result;
}

// ========================================================================
// STATIC METHOD ::valueparse
// --------------------------
// Do variable replacement using the scriptparser tools on a string.
// ========================================================================
string *strutil::valueparse (const string &str, value &env)
{
	value tokens;
	
	tokens = strutil::split (str, '$');
	return cmdtoken_parsedata (env, tokens);
}

// ========================================================================
// STATIC METHOD ::makepath
// ------------------------
// Get the path element of a fully qualified path + filename.
// TODO: Unlame.
// ========================================================================
string *strutil::makepath (const string &str)
{
	returnclass (string) result retain;
	int p;
	
	if (str.strchr ('/') >= 0)
	{
		result = str;
		result = result.cutatlast ('/');
	}
	else
	{
		if ((p = str.strchr (':')) >= 0)
		{
			result = str.left (p+1);
		}
		else
		{
			result.crop();
		}
	}
	return &result;
}

// ========================================================================
// STATIC METHOD ::uuid
// --------------------
// Create a universally unique identifier string.
// ========================================================================
string *strutil::uuid (void)
{
	returnclass (string) res retain;
	static bool seeded (false);
	if (! seeded)
	{
		srand (kernel.time.now());
		seeded = true;
	}
	
	unsigned int rnda = rand();
	unsigned int rndb = rand() & 0xffff;
	unsigned int rndc = rand() & 0xffff;
	unsigned int rnde = rand() & 0xffff;
	unsigned int rndf = rand() & 0xffff;
	unsigned int rndg = rand();
	
	rndc = 0x4000 | (rndc & 0x0fff);
	// rnde = ... ;TODO: Set the two most significant bits (bits 6 and 7) of the clock_seq_hi_and_reserved to zero and one, respectively. RFC4122 4.4
	
	rnda ^= kernel.time.now();
	rndb ^= kernel.proc.self();
	rndg ^= ::getuid();
	
	res.printf ("%08x-%04x-%04x-%04x-%04x%08x", rnda, rndb,
				rndc, rnde, rndf, rndg);
	
	return &res;
}
