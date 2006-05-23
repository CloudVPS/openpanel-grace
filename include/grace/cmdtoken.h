#ifndef _CMDTOKEN_H
#define _CMDTOKEN_H 1

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

/// Exceptions related to scriptparser / cmdtoken objects.
enum scriptpaserExcetion {
	EX_SCRIPT_ERR_SYNTAX		= 0xd12955b9, ///< Syntax error
	EX_SCRIPT_ERR_UNBALANCED	= 0xd0a261fe ///< Unbalanced conditionals
};

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

class cmdtoken_data : public cmdtoken
{
public:
					 cmdtoken_data (const string &dat, cmdtoken *p = NULL)
						 : cmdtoken (dataToken, p)
					 {
						 data = strutil::split (dat, '$');
					 }
	virtual			~cmdtoken_data (void) 
					 {
						 deltree();
					 }
	virtual void	 run (value &, string &);

protected:
	value			 data;
};

class cmdtoken_loop : public cmdtoken
{
public:
					 cmdtoken_loop (const string &lv, cmdtoken *p = NULL) : cmdtoken (loopToken, p)
					 {
						 loopvar = lv;
					 }
	virtual			~cmdtoken_loop (void)
					 {
						 deltree();
					 }
	virtual void	 run (value &, string &);
					 
protected:
	string			 loopvar;
};

class cmdtoken_if : public cmdtoken
{
public:
					 cmdtoken_if (const string &stm, cmdtoken *p = NULL)
						: cmdtoken (condToken, p)
					 {
						 condition = strutil::splitquoted (stm, ' ');
					 }
	virtual			~cmdtoken_if (void)
					 {
						 deltree();
					 }
	virtual void	 run (value &, string &);

protected:
	value			 condition;
};

class cmdtoken_case : public cmdtoken
{
public:
					 cmdtoken_case (const string &_content,
					 				cmdtoken *p = NULL)
					 	: cmdtoken (caseToken, p)
					 {
					 	caselabel = _content;
					 }
	virtual			~cmdtoken_case (void)
					 {
					 	deltree();
					 }
	virtual void	 run (value &, string &);
	
	string			 caselabel;
};

class cmdtoken_switch : public cmdtoken
{
public:
					 cmdtoken_switch (const string &_content,
					 				  cmdtoken *p = NULL)
					 	: cmdtoken (switchToken, p)
					 {
					 	switchkey = _content;
					 }
	virtual			~cmdtoken_switch (void)
					 {
					 	deltree();
					 }
	virtual void	 run (value &, string &);

protected:
	statstring		 switchkey;
};

string *cmdtoken_parsedata (value &, value &);
string *cmdtoken_parsestring (value &v, const string &s);
string *cmdtoken_parseval (value &, const string &);

class cmdtoken_set : public cmdtoken
{
public:
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
	virtual			~cmdtoken_set (void)
					 {
						 deltree();
					 }
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
							 if (atoi (rright))
								 env[lleft.str()] = env[left.str()].ival() + atoi (rright);
							 else
							 {
								 env[lleft.str()] = env[left.str()].sval() + rright;
							 }
						 }
						 else if (operand == "-=")
						 {
							 env[lleft.str()] = env[left.str()].ival() - atoi (rright);
						 }
						 else if (operand == "%=")
						 {
							 if (atoi (rright) > 0)
								 env[lleft.str()] = env[left.str()].ival() % atoi (rright);
						 }
						 else if (operand == "~=")
						 {
							 env[lleft.str()] =
							 	strutil::regexp (env[left.str()].sval(), rright);
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
					 scriptparser (void) : cmdtoken (rootToken) {}
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

protected:
	value			 labels;
};

typedef scriptparser cmdtoken_root;

#endif
