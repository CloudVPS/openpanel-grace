// ========================================================================
// cmdtoken.cpp: Template/script handling class
//
// (C) Copyright 2003-2004 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam 
// ========================================================================

//      012345678 TAB-O-METER should measure 4
//      ^	^

#include <grace/cmdtoken.h>
#include <grace/visitor.h>

#define object(x) (*x)

// ========================================================================
// CONSTRUCTOR
// -----------
// This constructor initializes a cmdtoken object, optionally tied to
// a parent cmdtoken object.
// ========================================================================

cmdtoken::cmdtoken (tokentype t, cmdtoken *p)
{
	type = t;
	parent = p;
	next = NULL;
	child = NULL;
	
	if (parent)
	{
		if (parent->child)
		{
			cmdtoken *c = parent->child;
			while (c->next) c = c->next;
			c->next = this;
		}
		else
		{
			parent->child = this;
		}
	}
}

// ========================================================================
// METHOD ::run
// ------------
// Base run method. Calls run on all child nodes.
// ========================================================================

void cmdtoken::run (value &v, string &buf)
{
	cmdtoken *c = child;
	while (c)
	{
		c->run (v,buf);
		c = c->next;
	}
}

// ========================================================================
// METHOD ::run (cmdtoken_data)
// ----------------------------
// The run cycle of a data token. Outputs the data, expanding nested
// template references and variables.
// ========================================================================

void cmdtoken_data::run (value &v, string &buf)
{
	string dat;
	
	dat = cmdtoken_parsedata (v, data);
	if (dat.strstr ("<<") < 0)
	{
		buf += dat;
		buf += "\n";
		return;
	}
	
	int pos;
	int pos2;
	string tok;
	
	while ((pos = dat.strstr ("<<")) >= 0)
	{
		buf += dat.left (pos);
		dat = dat.mid(pos+2);
		pos2 = dat.strstr (">>");
		if (pos2>=0)
		{
			value decl;
			value ind;
			
			tok = dat.left (pos2);
			dat = dat.mid (pos2+2);			

			decl = strutil::splitquoted (tok, ' ');
			for (int i=1; i<decl.count(); ++i)
			{
				ind = strutil::splitquoted (decl[i].sval(),'=');
				if (ind.count() == 2)
				{
					v[ind[0].sval()] = ind[1];
				}
			}
			
			cmdtoken *p = this;
			while (p->parent) p = p->parent;
			
			scriptparser *pr = (scriptparser *) p;
			
			pr->run (v, buf, decl[0].sval());
		}
	}
}

// ========================================================================
// METHOD ::run (cmdtoken_loop)
// ----------------------------
// Executes a loop token, running every child for every row found in the
// loopvar.
// ========================================================================

void cmdtoken_loop::run (value &v, string &buf)
{
	static int seq = 0;
	value loopdata;
	
	string ploopvar = cmdtoken_parsestring (v, loopvar);
	//value oldv;
	loopdata = v[ploopvar];
	
	//oldv = v;
	
	for (int i=0; i<loopdata.count(); ++i)
	{
		if (loopdata[i].label())
		{
			v["@"] = loopdata[i].label().str();
		}
		if (! loopdata[i].count())
		{
			v["0"] = loopdata[i];
		}
		for (int j=0; j<loopdata[i].count(); ++j)
		{
			string str;
			
			if (! loopdata[i][j].count())
			{
				string str;
				str.printf ("%i", j);
				v[str] = loopdata[i][j];
			}
			v[loopdata[i][j].id()] = loopdata[i][j];
		}
		
		cmdtoken *c;
		
		c = child;
		while (c)
		{
			c->run (v, buf);
			c = c->next;
		}
		
		//v = oldv;
	}
}

// ========================================================================
// METHOD ::run (scriptparser)
// ----------------------------
// Execute the labelled "main" object
// ========================================================================

void scriptparser::run (value &v, string &buf)
{
	run (v, buf, "main");
}

void scriptparser::run(value &v, string &buf, const string &label)
{
	int pos;
	cmdtoken *c;
	
	pos = labels[label];
	c = child;
	for (int i=0; i<pos; ++i)
	{
		if (c) c = c->next;
	}
	if (c) c->run (v, buf);
}

// ========================================================================
// METHOD ::build
// --------------
// Parses a string containing a laoded script template into a parser tree.
// ========================================================================

void scriptparser::build (const string &src)
{
	int crsr = 0;
	cmdtoken *c;
	cmdtoken *cc;
	
	child = new cmdtoken (nopToken, this);
	c = child;
	cc = child;
	
	value lines;
	lines = strutil::split (src, '\n');
	
	for (int l=0; l<lines.count(); ++l)
	{
		string ln;
		string stripped;
		ln = lines[l];
		int ps = 0;
		
		if (! ln) continue;
		
		while ((ln[ps] == ' ')||(ln[ps] == '\t')) ++ps;
		stripped = ln.mid (ps);
		
		if (stripped[0] == '@') // command
		{
			string cmd;
			string args;
			
			args = stripped;
			cmd = args.cutat (' ');
			if (cmd.strlen() == 0)
			{
				cmd = args;
				args.crop();
			}
			
			if (cmd == "@section")
			{
				c = new cmdtoken (nopToken, this);
				crsr++;
				labels[args] = crsr;
				cc = c;
			}
			else if (cmd == "@switch")
			{
				cc = new cmdtoken_switch (args,cc);
			}
			else if (cmd == "@endswitch")
			{
				if ( (! cc->parent) || (cc->parent == this) )
					throw (EX_SCRIPT_ERR_UNBALANCED);
				
				if (cc->type == caseToken)
				{
					if ( (! cc->parent) || (cc->parent == this) )
						throw (EX_SCRIPT_ERR_UNBALANCED);
						
					cc = cc->parent;
				}
				if (cc->type != switchToken)
				{
					throw (EX_SCRIPT_ERR_UNBALANCED);
				}
			}
			else if (cmd == "@case")
			{
				if (cc->type == caseToken)
				{
					if ( (! cc->parent) || (cc->parent == this) )
						throw (EX_SCRIPT_ERR_UNBALANCED);
					
					cc = cc->parent;
				}
				if (cc->type != switchToken)
				{
					throw (EX_SCRIPT_ERR_UNBALANCED);
				}
				
				cc = new cmdtoken_case (args, cc);
			}
			else if (cmd == "@loop")
			{
				cc = new cmdtoken_loop (args,cc);
			}
			else if (cmd == "@endloop")
			{
				if (cc->parent && (cc->parent != this))
					cc = cc->parent;
			}
			else if (cmd == "@if")
			{
				cc = new cmdtoken_if (args,cc);
				cc = new cmdtoken (nopToken, cc);
			}
			else if (cmd == "@else")
			{
				if ( (! cc->parent) || (cc->parent == this) )
					throw (EX_SCRIPT_ERR_UNBALANCED);
				
				cc = cc->parent;
				if (cc->type != condToken)
					throw (EX_SCRIPT_ERR_UNBALANCED);
				
				cc = new cmdtoken (nopToken, cc);
			}
			else if (cmd == "@endif")
			{
				if ( (! cc->parent) || (cc->parent == this) )
					throw (EX_SCRIPT_ERR_UNBALANCED);

				cc = cc->parent;
				if ((cc->type != condToken) || (! cc->parent))
					throw (EX_SCRIPT_ERR_UNBALANCED);

				cc = cc->parent;
			}
			else if  (cmd == "@set")
			{
				new cmdtoken_set (args,cc);
			}
		}
		else
		{
			cmdtoken *t = new cmdtoken_data (ln, cc);
		}
	}
}

void cmdtoken_switch::run (value &v, string &buf)
{
	cmdtoken_case *c = (cmdtoken_case *) child;
	while (c)
	{
		if (c->type == caseToken)
		{
			if (v[switchkey].sval() == c->caselabel)
			{
				c->run (v,buf);
				c = NULL;
			}
		}
		if (c) c = (cmdtoken_case *) c->next;
	}
}

void cmdtoken_case::run (value &v, string &buf)
{
	cmdtoken *c = child;
	while (c)
	{
		c->run (v,buf);
		c = c->next;
	}
}

// ========================================================================
// METHOD cmdtoken_if::run
// -----------------------
// Implementation of the @if command
// ========================================================================

void cmdtoken_if::run (value &v, string &buf)
{
	value vleft, vright;
	string left, right;
	string opcode;
	bool condset = false;
	
	if (condition.count() == 1)
	{
		vleft = strutil::split (condition[0],'$');
		left = cmdtoken_parsedata (v, vleft);
		condset = (left.strlen());
	}
	else
	{
		vleft = strutil::split (condition[0],'$');
		vright = strutil::split (condition[2],'$');
		left = cmdtoken_parsedata (v, vleft);
		right = cmdtoken_parsedata (v, vright);
		opcode = condition[1];
		
		if (opcode == "==")
		{
			condset = (left == right);
		}
		else if (opcode == "!=")
		{
			condset = (left != right);
		}
		else if (opcode == "~=")
		{
			condset = left.globcmp (right);
		}
		else if (opcode == "<")
		{
			condset = (left.strcasecmp (right) < 0);
		}
		else if (opcode == ">")
		{
			condset = (left.strcasecmp (right) > 0);
		}
	}
	
	if (condset)
	{
		if (child) child->run (v, buf);
	}
	else
	{
		if (child && child->next) child->next->run (v, buf);
	}
}

// ========================================================================
// FUNCTION cmdtoken_parsedata
// ---------------------------
// Parses the data-stream, replacing variables where needed
// ========================================================================

string *cmdtoken_parsedata (value &env, value &str)
{
	string *res = new string;
	if (str.count() == 1)
	{
		(*res) = str[0].sval();
		return res;
	}
	for (int i=0; i<str.count(); ++i)
	{
		if (i & 1)
		{
			(*res) += cmdtoken_parseval (env, str[i].sval());
		}
		else
		{
			(*res) += str[i].sval();
		}
	}
	return res;
}

// ========================================================================
// FUNCTION cmdtoken_parseval
// --------------------------
// Parses an inline variable inside a data-stream
// ========================================================================

string *cmdtoken_parseval (value &env, const string &_expr)
{
	string expr;
	string *res = new string;
	value myval;
	char prefix;
	
	expr = _expr;
	static string prefices ("#/+`^");
	
	if (prefices.strchr (_expr[0]) >= 0)
	{
		expr = expr.mid(1);
		prefix = _expr[0];
	}
	else prefix = 0;
	
	if (expr.strstr ("::") >= 0)
	{
		visitor<value> probe (env);
		value splt;
		bool found = true;
		splt = strutil::split (expr, "::");
		
		for (int i=0; i < splt.count(); ++i)
		{
			if (! probe.enter (splt[i].sval()))
			{
				found = false;
				break;
			}
		}
		
		if (found)
			myval = probe.obj();
		else
			myval = "";
	}
	else myval = env[expr];
	
	value tmpval;
	string tmpstr;
	
	switch (prefix)
	{
		case '#':
			(*res).printf ("%i", myval.ival());
			break;
		case '/':
			(*res) = myval.sval();
			(*res).escape();
			break;
		case '`':
			(*res) = env[myval.sval()];
			break;
		case '^':
			(*res) = strutil::htmlize (myval.sval());
			break;
		
		default:
			(*res) = myval.sval();
			break;
	}
	
	return res;
}

// ========================================================================
// FUNCTION cmdtoken_parsestring
// -----------------------------
// Utility function for just using the data-stream parsing part of the
// scriptparser engine.
// ========================================================================

string *cmdtoken_parsestring (value &v, const string &s)
{
	value splt;
	
	splt = strutil::split (s, '$');
	return cmdtoken_parsedata (v, splt);
}
