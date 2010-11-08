// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

// ========================================================================
// validator.cpp: Data validator
// ========================================================================

//      012345678 TAB-O-METER should measure 4
//      ^	^

#include <grace/validator.h>
#include <grace/str.h>
#include <grace/statstring.h>
#include <grace/value.h>
#include <grace/regexpression.h>
#include <grace/xmlschema.h>
#include <grace/defaults.h>

// ========================================================================
// CONSTRUCTOR
// ========================================================================
validator::validator (void)
{
}

validator::validator (const string &fn)
{
	load (fn);
}

// ========================================================================
// DESTRUCTOR
// ========================================================================
validator::~validator (void)
{
}

// ========================================================================
// METHOD ::load
// -------------
// Load the schema file
// ========================================================================
bool validator::load (const string &fn)
{
	schema.loadxml (fn, xmlschema::validator());
	return (schema.count());
}

// ========================================================================
// METHOD ::check
// --------------
// Check a data object against the schema.
// ========================================================================
bool validator::check (const value &data, string &error)
{
	error.crop(0);
	idchain.clear();
	return checkobject (data, "root", error);
}

// ========================================================================
// METHOD ::checkobject
// --------------------
// Check a specific (sub-)object against a specific validation rule.
// ========================================================================
bool validator::checkobject (const value &obj, const statstring &id,
							 string &error)
{
	value def;
	string errtxt;
	
	if (! schema.exists (id))
	{
		makeerror (error, 900, errortext::validator::rule_unknown, id.sval());
		return false;
	}
	
	currentid = id;
	def = schema[id];
	if (matchAnd (obj, def, error))
	{
		return true;
	}
	
	if (! error.strlen())
	{
		makeerror (error, 901, errortext::validator::rule_error, id.sval());
	}
	return false;
}

// ========================================================================
// METHOD ::matchObject
// --------------------
// Matches a given object to a subdefinition (generally called from
// iterators).
// ========================================================================
bool validator::matchObject (const value &obj, const value &def, string &error)
{
	string mtype;
	statstring sdef;

	mtype = def.type();
	
	if (mtype == "match.mandatory") return matchMandatory (obj, def, error);
	if (mtype == "and") return matchAnd (obj, def, error);
	if (mtype == "or") return matchOr (obj, def, error);
	if (mtype == "match.id") return matchId (obj, def, error);
	if (mtype == "match.child") return matchChild (obj, def, error);
	if (mtype == "match.attribute") return matchAttrib (obj, def, error);
	if (mtype == "match.hasindex") return matchHasIndex (obj, def, error);
	if (mtype == "match.type") return matchType (obj, def, error);
	if (mtype == "match.class") return matchClass (obj, def, error);
	if (mtype == "match.data") return matchData (obj, def, error);
	if (mtype == "match.rule")
	{
		sdef = def.sval();
		return checkobject (obj, sdef, error);
	}
	
	makeerror (error, 902, errortext::validator::matchtype, mtype);
	return false;
}

// ========================================================================
// METHOD ::matchId
// ----------------
// Match an object's id/key against a subdefinition.
// ========================================================================
bool validator::matchId (const value &obj, const value &def, string &error)
{
	if (! obj.id())
	{
		makeerror (error, 903, errortext::validator::matchid,
				   obj.type().sval());
		return false;
	}
	
	if (obj.id() != def.sval())
	{
		return false;
	}
	return true;
}

// ========================================================================
// METHOD ::matchAnd
// -----------------
// Parses an <and></and> sub-definition.
// ========================================================================
bool validator::matchAnd (const value &obj, const value &def, string &error)
{
	foreach (e, def)
	{
		if (! matchObject (obj, e, error))
		{
			return false;
		}
	}
	return true;
}

// ========================================================================
// METHOD ::matchOr
// ----------------
// Parses an <or></or> sub-definition.
// ========================================================================
bool validator::matchOr (const value &obj, const value &def, string &error)
{
	foreach (e, def)
	{
		if (matchObject (obj, e, error))
		{
			return true;
		}
	}
	return false;
}

// ========================================================================
// METHOD ::matchMandatory
// -----------------------
// Parses a <match.mandatory/>
// ========================================================================
bool validator::matchMandatory (const value &obj, const value &def,
								string &error)
{
	int i;
	int j;
	string action;
	string matchtype;
	string matchid;
	bool mattrib;
	
	foreach (e, def)
	{
		action = e.type();
		matchtype = e("type").sval();
		matchid = e("key").sval();
		
		if (action == "or")
		{
			foreach (ee, e)
			{
				value tval;
				tval.newval() = ee;
				if (matchMandatory (obj, tval, error))
				{
					return true;
				}
			}
			return false;
		}
		
		if (matchtype == "class")
		{
			foreach (node, obj)
			{
				if (node.type() == matchid)
				{
					if (action == "optional")
					{
						if (! matchMandatory (obj, e, error))
						{
							if (! error.strlen())
							{
								makeerror (error, 904,
										   errortext::validator::optmdep,
										   e("key").sval());
							}
							return false;
						}
					}
					return true;
				}
			}
			makeerror (error, 914, errortext::validator::mdep, matchid);
			return false;
		}
		
		if (matchtype == "attribute") mattrib = true;
		else if (matchtype == "child") mattrib = false;
		else
		{
			makeerror (error, 915, errortext::validator::matchtype, matchtype);
			return false;
		}
		
		if (action == "optional")
		{
			if (mattrib ? obj.attribexists (matchid) : obj.exists (matchid))
			{
				if (! matchMandatory (obj, e, error))
				{
					if (! error.strlen())
					{
						makeerror (error, 904, errortext::validator::optmdep,
								   e("key").sval());
					}
					return false;
				}
			}
		}
		else
		{
			if (! (mattrib ? obj.attribexists (matchid) : obj.exists (matchid)))
			{
				if (e.attribexists ("errortext"))
				{
					makeerror (error, e("errorcode").ival(),
							   e("errortext").sval(), "");
				}
				else
				{
					if (mattrib)
					{
						makeerror (error, 905, errortext::validator::mdep,
								   matchid);
					}
					else
					{
						makeerror (error, 906, errortext::validator::mattrdep,
								   matchid);
					}
				}
				return false;
			}
		}
	}
	return true;
}

// ========================================================================
// METHOD ::matchMandatory
// -----------------------
// Parses a <match.child/>
// ========================================================================
bool validator::matchChild (const value &obj, const value &def, string &error)
{
	string rkey;
	int i = 0;
	
	foreach (node, obj)
	{
		if (node.id()) idchain.newval() = node.id().sval();
		else
		{
			rkey.crop();
			rkey.printf ("%s[%i]",node.type().str(), i);
			idchain.newval() = i;
		}
		
		if (! matchOr (node, def, error))
		{
			if (! error.strlen())
			{
				if (def.attribexists ("errortext"))
				{
					makeerror (error, def("errorcode").ival(),
							   def("errortext").sval());
				}
			}
			return false;
		}
		
		idchain.rmindex (idchain.count() -1);
		++i;
	}
	return true;
}

// ========================================================================
// METHOD ::matchMandatory
// -----------------------
// Parses a <match.attrib/>
// ========================================================================
bool validator::matchAttrib (const value &obj, const value &def, string &error)
{
	if (! obj.haveattributes()) return true;
	string attrname;
	
	foreach (node, obj.attributes())
	{
		attrname = "ATTRIB::";
		attrname.strcat (node.id().sval());
		idchain.newval() = attrname;
		
		if (! matchOr (node, def, error))
		{
			if (! error.strlen())
			{
				if (def.attribexists ("errortext"))
				{
					makeerror (error, def("errorcode").ival(),
							   def("errortext").sval());
				}
				else
				{
					makeerror (error, 907, errortext::validator::attr_unknown,
							   node.id().sval());
				}
			}
			return false;
		}
		
		idchain.rmindex (idchain.count() -1);
	}
	
	return true;
}

// ========================================================================
// METHOD ::matchMandatory
// -----------------------
// Parses a <match.data/>
// ========================================================================
bool validator::matchData (const value &obj, const value &def, string &error)
{
	statstring mtype;
	
	foreach (e, def)
	{
		mtype = e.type();
		if (mtype == "text")
		{
			if (matchDataText (obj, e.sval()))
				return true;
		}
		else if (mtype == "regexp")
		{
			if (matchDataRegexp (obj, e.sval()))
				return true;
		}
		else if (mtype == "lt")
		{
			if (matchDataLessThan (obj, e.ival()))
				return true;
		}
		else if (mtype == "gt")
		{
			if (matchDataGreaterThan (obj, e.ival()))
				return true;
		}
		else if (mtype == "minsize")
		{
			if (matchDataMinSize (obj, e.ival()))
				return true;
		}
		else if (mtype == "maxsize")
		{
			if (matchDataMaxSize (obj, e.ival()))
				return true;
		}
		else
		{
			makeerror (error, 908, errortext::validator::matchtype, mtype);
			return false;
		}
	}

	if (def.attribexists ("errortext"))
	{
		makeerror (error, def("errorcode").ival(),
				   def("errortext").sval(), obj.sval());
	}
	else
	{
		makeerror (error, 909, errortext::validator::nomatch, obj.sval());
	}
	return false;
}

// ========================================================================
// METHOD ::matchHasIndex
// ----------------------
// Parses a <match.id/>
// ========================================================================
bool validator::matchHasIndex (const value &obj, const value &def,
							   string &error)
{
	if (obj.id()) return true;
	
	if (def.attribexists ("errortext"))
	{
		makeerror (error, def("errorcode").ival(),
				   def("errortext").sval());
	}
	else
	{
		makeerror (error, 910, errortext::validator::noindex);
	}
	
	return false;
}

// ========================================================================
// METHOD ::matchClass
// -------------------
// Parses a <match.class/>
// ========================================================================
bool validator::matchClass (const value &obj, const value &def, string &error)
{
	
	if (obj.type() == def.sval()) return true;
	
	if (def.attribexists ("errortext"))
	{
		makeerror (error, def("errorcode").ival(),
				   def("errortext").sval());
	}

	return false;
}

// ========================================================================
// METHOD ::matchType
// ------------------
// Parses a <match.type/>
// ========================================================================
bool validator::matchType (const value &obj, const value &def, string &error)
{
	static value tmap;
	
	if (! tmap.exists ("string"))
	{
		tmap["void"] = i_unset;
		tmap["integer"] = i_int;
		tmap["unsigned"] = i_unsigned;
		tmap["float"] = i_double;
		tmap["long"] = i_long;
		tmap["ulong"] = i_ulong;
		tmap["bool"] = i_bool;
		tmap["string"] = i_string;
		tmap["ipaddr"] = i_ipaddr;
		tmap["date"] = i_date;
	}
	
	if (! tmap.exists (def.sval()))
	{
		makeerror (error, 912, errortext::validator::type_unknown, def.sval());
		return false;
	}
	
	if (obj.itype() == tmap[def.sval()].ival()) return true;
	
	if (def.attribexists ("errortext"))
	{
		makeerror (error, def("errorcode").ival(),
				   def("errortext").sval());
	}
	else
	{
		makeerror (error, 913, errortext::validator::wrongtype,"");
	}
	return false;
}

// ========================================================================
// METHOD ::matchDataText
// ----------------------
// Handling for <match.data><text>...</text></match.data>.
// ========================================================================
bool validator::matchDataText (const value &obj, const string &text)
{
	return (obj.sval() == text);
}

// ========================================================================
// METHOD ::matchDataRegexp
// ------------------------
// Handling for <match.data><regexp>...</regexp></match.data>.
// ========================================================================
bool validator::matchDataRegexp (const value &obj, const string &expr)
{
	regexpression re (expr);
	
	return re.eval (obj.cval());
}

// ========================================================================
// METHOD ::matchLessThan
// ----------------------
// Handling for <match.data><lt>...</lt></match.data>.
// ========================================================================
bool validator::matchDataLessThan (const value &obj, int val)
{
	return (obj.ival() < val);
}

// ========================================================================
// METHOD ::matchDataGreaterThan
// -----------------------------
// Handling for <match.data><gt>...</gt></match.data>.
// ========================================================================
bool validator::matchDataGreaterThan (const value &obj, int val)
{
	return (obj.ival() > val);
}

// ========================================================================
// METHOD ::matchDataMinSize
// -------------------------
// Handling for <match.data><minsize>...</minsize></match.data>.
// ========================================================================
bool validator::matchDataMinSize (const value &obj, int val)
{
	return (obj.sval().strlen() >= (unsigned int) val);
}

// ========================================================================
// METHOD ::matchDataMaxSize
// -------------------------
// Handling for <match.data><maxsize>...</maxsize></match.data>.
// ========================================================================
bool validator::matchDataMaxSize (const value &obj, int val)
{
	return (obj.sval().strlen() <= (unsigned int) val);
}

#define V_NOENCODE "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"\
				   "0123456789.-_:"

// ========================================================================
// METHOD ::encodeidchain
// -----------------------
// Creates a printable string from the idchain (representing the path
// being looked at).
// ========================================================================
string *validator::encodeidchain (void)
{
	returnclass (string) res retain;
	
	foreach (e, idchain)
	{
		res.strcat ('/');
		if (! e.sval().validate (V_NOENCODE))
		{
			res.printf ("\"%S\"", e.cval());
		}
		else
		{
			res.strcat (e.sval());
		}
	}
	return &res;
}

// ==========================================================================
// METHOD validator::makeerror
// ==========================================================================
void validator::makeerror (string &into, int errorcode,
						   const string &errortext,
						   const string &detail)
{
	into.crop (0);
	string path = encodeidchain();
	
	into.printf ("<%s:%03i> [%s] %s", currentid.str(), errorcode, path.str(),
								   errortext.str());
	
	if (detail.strlen())
		into.printf (" (%s)", detail.str());
}

