// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _GRACE_PCRE_H
#define _GRACE_PCRE_H 1

#include <pcre.h>

#include <grace/dictionary.h>
#include <grace/lock.h>
#include <grace/value.h>

typedef dictionary<pcre*> pcredict;

/// A cache for compiled perl compatible regular expressions.
class pcredb
{
public:
					 /// Constructor.
					 pcredb (void);
					 
					 /// Destructor.
					~pcredb (void);
					
					 /// Get the pcre object for a specific expression
					 /// with provided flags. The cache is incapable
					 /// of keeping copies of the same expression
					 /// with different flags, it is assumed that this
					 /// is a reasonable trade-off.
					 /// \param expr The regular expression string
					 /// \param o A binary OR of all related option
					 ///          flags. See pcregexp::setoptions.
	pcre			*get (const statstring &expr, int o);
	
protected:
	lock<pcredict>	 expressions; ///< Dictionary of cached expressions.
};

/// An object representing a perl compatible regular expression.
class pcregexp
{
public:
					 /// Constructor.
					 pcregexp (void);
					 
					 /// Constructor with arguments.
					 /// \param expr The regular expression.
					 /// \param o Option flags (pcregexp::setoptions).
					 pcregexp (const statstring &expr, int o=0);
					 
					 /// Destructor.
					~pcregexp (void);
					
					 /// PCRE flag mappings
					 enum {
					 	anchored = PCRE_ANCHORED,
					 	ignorecase = PCRE_CASELESS,
					 	dollarendonly = PCRE_DOLLAR_ENDONLY,
					 	dotall = PCRE_DOTALL,
					 	extended = PCRE_EXTENDED,
					 	multiline = PCRE_MULTILINE,
					 	ungreedy = PCRE_UNGREEDY
					 };
	
					 /// Assignment operator. Assumes options have
					 /// already been set by the constructor or
					 /// through setoptions().
	pcregexp		&operator= (const statstring &expr);
	
					 /// Assignment call. Assumes options have been set by
					 /// the constructor or through setoptions().
	pcregexp		&set (const statstring &expr);
	
					 /// Set the pcre flags.
					 /// The following option flags are recognized:
					 /// - pcregexp::anchored (PCRE_ANCHORED)
					 /// - pcregexp::ignorecase (PCRE_CASELESS)
					 /// - pcregexp::dollarendonly (PCRE_DOLLAR_ENDONLY)
					 /// - pcregexp::dotall (PCRE_DOTALL)
					 /// - pcregexp::extended (PCRE_EXTENDED)
					 /// - pcregexp::multiline (PCRE_MULTILINE)
					 /// - pcregexp::ungreedy (PCRE_UNGREEDY)
					 /// \param o Binary OR of options that apply.
	pcregexp		&setoptions (int o);
	
					 /// Perform a match operation with optional
					 /// storage for backreferences.
					 /// \param to The string to match the regexp against.
					 /// \param outrefs Optional pointer to the value object
					 ///                that will be used as an array to
					 ///                hold backreferences.
	bool			 match (const string &to, value *outrefs = NULL);
	
					 /// Perform a match operation with mandatory
					 /// outvalue for backreferences.
					 /// \param to The string to match the regexp against.
					 /// \param outrefs Reference to the value object that
					 ///                will be used as an array to hold
					 ///                backreferences.
	bool			 match (const string &to, value &outrefs);
	
					 /// Return captured backreferences.
					 /// \param from The string to match against.
					 /// \return A value object containing an array
					 ///         if all captured backreferences.
	value			*capture (const string &from);
	
					 /// Replace matched part of a string with something
					 /// else.
					 /// \param what The string to match against
					 /// \param with The replacement string for the match,
					 ///             may contain sed-style backslash
					 ///             elements to refer to numbered
					 ///             backreferences.
	string			*replace (const string &what, const string &with,
							  bool g=false);
	
protected:
	pcre			*pobj; ///< The compiled expression.
	int				 options; ///< The selected pcre options.
};

/// Perform a sed-style regular expression replacement on a string.
/// \param orig The string to mangle.
/// \param expr The expression to apply. This paramater wants expressions
///             in the syntax of sed: A command character (currently
///             always 's', followed by a separator, followed by a
///             regular expression match, followed by another separator,
///             followed by the replacement string (using backslashes
///             with numbers to indicate backreferences), followed by
///             a final separator and the options (currently only 'i' for
///				ignoring case and 'g' for multiple replacements are
/// 			supported as options).
/// \return The mangled string.
string *$expr (const string &orig, const string &expr);

/// Capture a single string using a regular expression.
/// \param orig The string to capture out of
/// \param expr The regular expression
/// \param flags Optional PCRE flags (see pcregexp::setoptions)
string *$capture1 (const string &orig, const string &expr, int flags=0);

/// Capture multiple values out of a string using a regular expression.
/// \param orig The string to capture out of
/// \param expr The regular expression
/// \param flags Optional PCRE flags (see pcregexp::setoptions)
value *$capture (const string &orig, const string &expr, int flags=0);

#endif
