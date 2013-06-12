// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _CMDTOKEN_H
#define _CMDTOKEN_H 1

#include <grace/exception.h>
#include <grace/value.h>
#include <grace/str.h>
#include <grace/strutil.h>

/// Token types.
/// Lists the different cmdtoken classes.
typedef enum {
	nopToken, ///< Dummy or null.
	switchToken, ///< A switch/endswitch block
	caseToken, ///< A case
	dataToken, ///< Text data interwoven with variables.
	loopToken, ///< A loop.
	condToken, ///< A conditional.
	setToken, ///< A variable assignment.
	rootToken ///< Implements a script.
} tokentype;

$exception (scriptSyntaxException, "Syntax Error");
$exception (scriptUnbalancedConditionException, "Unbalanced condition");

/// A command unit in a script.
/// Base class for all scriptparser functionality.
class cmdtoken
{
public:
					 /// Constructor. Sets type, links parent.
					 /// \param t The type of token our subclass implements.
					 /// \param p Pointer to the parent token (NULL implies root).
					 cmdtoken (tokentype t = nopToken, cmdtoken *p = NULL);
					 /// Destructor. Also deletes children.
	virtual			~cmdtoken (void)
					 {
						 deltree();
					 }
					 /// Implementation method.
					 /// \param vars The current variable space.
					 /// \param buf The script's output buffer.
	virtual	void	 run (value &vars, string &buf);
	
	cmdtoken		*parent; ///< Link to parent
	cmdtoken		*next; ///< Link to array sibling.

	cmdtoken		*child; ///< Link to first child (if any).
	tokentype		 type; ///< Indication of our type (useful for debugging)

protected:
					 /// Deletes the child array.
	void			 deltree (void)
					 {
						 cmdtoken *c, *cc;
						 
						 c = child;
						 while (c)
						 {
							 cc = c->next;
							 delete c;
							 c = cc;
						 }
						 child = NULL;
					 }
	
};

/// A text data token. Prints out text with parseable elements between
/// '$' marks.
class cmdtoken_data : public cmdtoken
{
public:
					 /// Constructor.
					 /// \param dat The data.
					 /// \param p The parent token (NULL for no parent)
					 cmdtoken_data (const string &dat, cmdtoken *p = NULL)
						 : cmdtoken (dataToken, p)
					 {
						 data = strutil::split (dat, '$');
					 }
					 
					 /// Destructor.
	virtual			~cmdtoken_data (void) 
					 {
						 deltree();
					 }
					 
					 /// Run method.
					 /// \param env The environment.
					 /// \param into The output buffer.
	virtual void	 run (value &env, string &into);

protected:
	value			 data; ///< Internal storage for the data.
};

/// A token representing a 'loop' instruction.
class cmdtoken_loop : public cmdtoken
{
public:
					 /// Constructor.
					 /// \param lv The loop-variable.
					 /// \param p The parent.
					 cmdtoken_loop (const string &lv, cmdtoken *p = NULL) : cmdtoken (loopToken, p)
					 {
						 loopvar = lv;
					 }
					 
					 /// Destructor.
	virtual			~cmdtoken_loop (void)
					 {
						 deltree();
					 }
					 
					 /// Run method.
					 /// \param env The environment.
					 /// \param into The output buffer.
	virtual void	 run (value &env, string &into);
					 
protected:
	string			 loopvar; ///< The loop variable.
};

/// A token representing the 'if' instruction.
class cmdtoken_if : public cmdtoken
{
public:
					 /// Constructor.
					 /// \param stm The conditional statement.
					 /// \param p The parent.
					 cmdtoken_if (const string &stm, cmdtoken *p = NULL)
						: cmdtoken (condToken, p)
					 {
						 condition = strutil::splitquoted (stm, ' ');
					 }
					 
					 /// Destructor.
	virtual			~cmdtoken_if (void)
					 {
						 deltree();
					 }

					 /// Run method.
					 /// \param env The environment.
					 /// \param into The output buffer.
	virtual void	 run (value &env, string &into);

protected:
	value			 condition; ///< Storage for the conditional statement.
};

/// A token representing the 'case' instruction.
class cmdtoken_case : public cmdtoken
{
public:
					 /// Constructor.
					 /// \param _content The case label.
					 /// \param p The parent token.
					 cmdtoken_case (const string &_content,
					 				cmdtoken *p = NULL)
					 	: cmdtoken (caseToken, p)
					 {
					 	caselabel = _content;
					 }
					 
					 /// Destructor.
	virtual			~cmdtoken_case (void)
					 {
					 	deltree();
					 }

					 /// Run method.
					 /// \param env The environment.
					 /// \param into The output buffer.
	virtual void	 run (value &env, string &into);
	
	string			 caselabel; ///< The case label.
};

/// A token representing the 'switch' instruction
class cmdtoken_switch : public cmdtoken
{
public:
					 /// Constructor.
					 /// \param _content The variable name to switch on.
					 /// \param p The parent token.
					 cmdtoken_switch (const string &_content,
					 				  cmdtoken *p = NULL)
					 	: cmdtoken (switchToken, p)
					 {
					 	switchkey = _content;
					 }
					 
					 /// Destructor.
	virtual			~cmdtoken_switch (void)
					 {
					 	deltree();
					 }

					 /// Run method.
					 /// \param env The environment.
					 /// \param into The output buffer.
	virtual void	 run (value &env, string &into);

protected:
	statstring		 switchkey; ///< The switch label.
};

/// Internal parsing method for data that is already split on the
/// '$'-character.
string *cmdtoken_parsedata (value &, value &);

/// Split a string on the '$' character and run it through the
/// cmdtoken_parsedata() function.
string *cmdtoken_parsestring (value &v, const string &s);

/// Parse the 'variable' part of a string, normally enclosed by two
/// '$'-characters. It loads the value out of the environment, taking
/// a number of formatting hints:
/// - "#varnam" will yield env["varnam"] parsed as an integer
/// - "/varnam" will yield a quote-escaped version of the variable.
/// - "`varnam" will yield env[env["varnam"]].
/// - "^varnam" will yield a html-escaped version.
/// - "varnam::sub" will yield env["varnam"]["sub"].
/// \param env The environment.
/// \param stm The statement, as found between '$'-characters.
string *cmdtoken_parseval (value &env, const string &stm);

/// A token representing the 'set' instruction
class cmdtoken_set : public cmdtoken
{
public:
					 /// Constructor. Parses the statement into a
					 /// name and value part.
					 /// \param _content The data of the \@set statement.
					 cmdtoken_set (const string &_content,
								   cmdtoken *p = NULL)
						 : cmdtoken (setToken, p)
					 {
						 value tmp;
						 
						 tmp = strutil::splitquoted (_content, ' ');
						 
						 left = tmp[0];
						 operand = tmp[1];
						 right = tmp[2];
					 }
					 
					 /// Destructor.
	virtual			~cmdtoken_set (void)
					 {
						 deltree();
					 }
					 
					 /// Run-method.
					 /// \param env The variable environment.
					 /// \param buffer The output buffer.
	virtual void	 run (value &env, string &buffer)
					 {
						 value xright;
						 string rright;
						 value xleft;
						 string lleft;
						 
						 xright = strutil::split (right, '$');
						 rright = cmdtoken_parsedata (env, xright);
						 
						 xleft = strutil::split (left, '$');
						 lleft = cmdtoken_parsedata (env, xleft);
						 
						 if (operand == "=")
						 {
							 env[lleft.str()] = rright;
						 }
						 else if (operand == "+=")
						 {
							 if (rright.toint())
								 env[lleft] = env[left].ival() + rright.toint();
							 else
							 {
								 env[lleft] = env[left].sval() + rright;
							 }
						 }
						 else if (operand == "-=")
						 {
							 env[lleft] = env[left].ival() - rright.toint();
						 }
						 else if (operand == "%=")
						 {
							 if (rright.toint() > 0)
								 env[lleft] = env[left].ival() % rright.toint();
						 }
						 else if (operand == "~=")
						 {
							 env[lleft] = strutil::regexp (env[left].sval(), rright);
						 }
						 else if (operand == "/=")
						 {
						 	if (rright.toint() > 0)
						 	{
						 		env[lleft] = env[lleft].ival() / rright.toint();
						 	}
						 }
						 else if (operand == "*=")
						 {
							env[lleft] = env[lleft].ival() * rright.toint();
						 }
					 }
					 
protected:
	string			 left, operand, right;
};

/// A script parsing engine.
/// Builds a tree of cmdtoken objects out of a parsable textfile
/// containing stealthscript commands. 
class scriptparser : public cmdtoken
{
public:
					 /// Constructor.
					 scriptparser (void) : cmdtoken (rootToken) {}
					 
					 /// Destructor.
	virtual			~scriptparser (void)
					 {
						 deltree();
					 }
					 
					 /// Will run the code under @section main.
					 /// \param ev The environment variable space.
					 /// \param buf The output buffer.
	virtual void	 run (value &ev, string &buf);
	
					 /// Will evaluate code under a specified script section.
					 /// \param ev The environment variable space.
					 /// \param buf The output buffer.
					 /// \param sec The section name.
	void			 run (value &ev, string &buf, const string &sec);
	
					 /// Build a script tree from a text file.
					 /// \param dat The text data.
	void			 build (const string &dat);
	
					 /// Returns true if a named section exists inside the
					 /// script.
	bool			 sectionexists (const string &sec);

protected:
	value			 labels; ///< Mapping between labels and statement offset.
};

/// Convenience-typedef, some parts of the library refer to the root
/// scriptparser class as 'cmdtoken_root'.
typedef scriptparser cmdtoken_root;

#endif
