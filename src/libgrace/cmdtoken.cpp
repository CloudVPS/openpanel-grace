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
			string section = decl[0];
			decl.rmindex (0);
			
			foreach (namevalue, decl)
			{
				ind = strutil::splitquoted (namevalue.sval(),'=');
				if (ind.count() == 2)
				{
					v[ind[0].sval()] = ind[1];
				}
			}
			
			cmdtoken *p = this;
			while (p->parent) p = p->parent;
			
			scriptparser *pr = (scriptparser *) p;
			
			pr->run (v, buf, section);
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
	value loopdata;
	
	string ploopvar = cmdtoken_parsestring (v, loopvar);
	//value oldv;
	loopdata = v[ploopvar];
	
	//oldv = v;
	
	foreach (loopnode, loopdata)
	{
		if (loopnode.label())
		{
			v["@"] = loopnode.id().str();
		}
		if (! loopnode.count())
		{
			v["0"] = loopnode;
		}
		
		int j=0;
		
		foreach (var, loopnode)
		{
			string str;
			
			if (! var.count())
			{
				string str = "%i" %format (j);
				v[str] = var;
			}
			v[var.id()] = var;
			j++;
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

bool scriptparser::sectionexists (const string &label)
{
	return labels.exists (label);
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
	
	foreach (ln,lines)
	{
		string stripped;
		int ps = 0;
		
		if (! ln.sval().strlen()) continue;
		
		while ((ln.sval()[ps] == ' ')||(ln.sval()[ps] == '\t')) ++ps;
		stripped = ln.sval().mid (ps);
		
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
			
			caseselector (cmd)
			{
				incaseof ("@section") :
					c = new cmdtoken (nopToken, this);
					crsr++;
					labels[args] = crsr;
					cc = c;
					break;

				incaseof ("@switch") :
					cc = new cmdtoken_switch (args,cc);
					break;
				
				incaseof ("@endswitch") :
					if ( (! cc->parent) || (cc->parent == this) )
						throw (scriptUnbalancedConditionException());
					
					if (cc->type == caseToken)
					{
						if ( (! cc->parent) || (cc->parent == this) )
							throw (scriptUnbalancedConditionException());
							
						cc = cc->parent;
					}
					if (cc->type != switchToken)
					{
						throw (scriptUnbalancedConditionException());
					}
					break;
				
				incaseof ("@case") :
					if (cc->type == caseToken)
					{
						if ( (! cc->parent) || (cc->parent == this) )
							throw (scriptUnbalancedConditionException());
						
						cc = cc->parent;
					}
					if (cc->type != switchToken)
					{
						throw (scriptUnbalancedConditionException());
					}
					
					cc = new cmdtoken_case (args, cc);
					break;
				
				incaseof ("@loop") :
					cc = new cmdtoken_loop (args,cc);
					break;

				incaseof ("@endloop") :
					if (cc->parent && (cc->parent != this))
						cc = cc->parent;
					break;

				incaseof ("@if") :
					cc = new cmdtoken_if (args,cc);
					cc = new cmdtoken (nopToken, cc);
					break;
				
				incaseof ("@else") :
					if ( (! cc->parent) || (cc->parent == this) )
						throw (scriptUnbalancedConditionException());
					
					cc = cc->parent;
					if (cc->type != condToken)
						throw (scriptUnbalancedConditionException());
					
					cc = new cmdtoken (nopToken, cc);
					break;
					
				incaseof ("@endif") :
					if ( (! cc->parent) || (cc->parent == this) )
						throw (scriptUnbalancedConditionException());
	
					cc = cc->parent;
					if ((cc->type != condToken) || (! cc->parent))
						throw (scriptUnbalancedConditionException());
	
					cc = cc->parent;
					break;
					
				incaseof ("@set") :
					new cmdtoken_set (args, cc);
					break;
					
				defaultcase : break;
			}
		}
		else
		{
			new cmdtoken_data (ln, cc);
		}
	}
}

// ========================================================================
// METHOD cmdtoken_switch::run
// ========================================================================
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

// ========================================================================
// METHOD cmdtoken_case::run
// ========================================================================
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

		caseselector (opcode)
		{
			incaseof ("==") : condset = (left == right); break;
			incaseof ("!=") : condset = (left != right); break;
			incaseof ("~=") : condset = left.globcmp (right); break;
			incaseof ("<")  : condset = (left.strcasecmp (right)<0); break;
			incaseof (">")  : condset = (left.strcasecmp (right)>0); break;
			defaultcase     : condset = false;
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
	returnclass (string) res retain;

	if (str.count() == 1)
	{
		res = str[0].sval();
		return &res;
	}
	
	int i=0;
	
	foreach (element,str)
	{
		if (i & 1)
		{
			if (! element.sval()) res += "$";
			else res += cmdtoken_parseval (env, element.sval());
		}
		else
		{
			res += element.sval();
		}
		++i;
	}
	return &res;
}

// ========================================================================
// FUNCTION cmdtoken_parseval
// --------------------------
// Parses an inline variable inside a data-stream
// ========================================================================

string *cmdtoken_parseval (value &env, const string &_expr)
{
	returnclass (string) res retain;

	string expr;
	value myval;
	char prefix;
	
	expr = _expr;
	static string prefices ("#/+`^~");
	
	if (prefices.strchr (_expr[0]) >= 0)
	{
		expr = expr.mid(1);
		prefix = _expr[0];
		myval = env[expr];
	}
	else
	{
		prefix = 0;
		if (expr.strstr ("::") >= 0)
		{
			visitor<value> probe (env);
			value splt;
			bool found = true;
			splt = strutil::split (expr, "::");
			
			foreach (sclass,splt)
			{
				if (! probe.enter (sclass.sval()))
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
	}
	
	value tmpval;
	string tmpstr;
	
	switch (prefix)
	{
		case '#':
			res = "%i" %format (myval);
			break;
		case '/':
			res = myval.sval();
			res.escape();
			break;
		case '`':
			res = env[myval.sval()];
			break;
		case '^':
			res = strutil::htmlize (myval.sval());
			break;
			
		case '~':
			res = strutil::urlencode (myval.sval());
			break;
		
		default:
			res = myval.sval();
			break;
	}
	
	return &res;
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
