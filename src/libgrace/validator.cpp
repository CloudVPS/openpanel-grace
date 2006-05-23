// ========================================================================
// validator.cpp: Data validator
//
// (C) Copyright 2005-2006 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam 
// ========================================================================

//      012345678 TAB-O-METER should measure 4
//      ^	^

#include <grace/validator.h>
#include <grace/str.h>
#include <grace/statstring.h>
#include <grace/value.h>
#include <grace/regexpression.h>
#include <grace/xmlschema.h>

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
	xmlschema vSchema (XMLValidatorSchemaType);
	schema.loadxml (fn, vSchema);
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
		makeerror (error, 900, "Unknown rule-id", id.sval());
		return false;
	}
	
	
	def = schema[id];
	if (matchAnd (obj, def, error))
	{
		return true;
	}
	
	if (! error.strlen())
	{
		makeerror (error, 901, "Error matching rule", id.sval());
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
	
	makeerror (error, 902, "Unknown matchtype", mtype);
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
		makeerror (error, 903, "match.id of an object with no id",
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
	for (int i=0; i< def.count(); ++i)
	{
		if (! matchObject (obj, def[i], error))
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
	for (int i=0; i<def.count(); ++i)
	{
		if (matchObject (obj, def[i], error))
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
	
	for (i=0; i<def.count(); ++i)
	{
		action = def[i].type();
		matchtype = def[i]("type").sval();
		matchid = def[i]("key").sval();
		
		if (action == "or")
		{
			for (j=0; j<def[i].count(); ++j)
			{
				value tval;
				tval.newval() = def[i][j];
				if (matchMandatory (obj, tval, error))
				{
					return true;
				}
			}
			return false;
		}
		
		if (matchtype == "class")
		{
			for (j=0; j<obj.count(); ++j)
			{
				if (obj[j].type() == matchid)
				{
					if (action == "optional")
					{
						if (! matchMandatory (obj, def[i], error))
						{
							if (! error.strlen())
							{
								makeerror (error, 904, "Error matching "
										   "depending mandatory",
										   def[i]("key").sval());
							}
							return false;
						}
					}
					return true;
				}
			}
			makeerror (error, 914, "Error matching mandatory class member",
					   matchid);
			return false;
		}
		
		if (matchtype == "attribute") mattrib = true;
		else if (matchtype == "child") mattrib = false;
		else
		{
			makeerror (error, 915, "Undefined matchtype", matchtype);
			return false;
		}
		
		if (action == "optional")
		{
			if (mattrib ? obj.attribexists (matchid) : obj.exists (matchid))
			{
				if (! matchMandatory (obj, def[i], error))
				{
					if (! error.strlen())
					{
						makeerror (error, 904, "Error matching depending "
								   "mandatory", def[i]("key").sval());
					}
					return false;
				}
			}
		}
		else
		{
			if (! (mattrib ? obj.attribexists (matchid) : obj.exists (matchid)))
			{
				if (def[i].attribexists ("errortext"))
				{
					makeerror (error, def[i]("errorcode").ival(),
							   def[i]("errortext").sval(), "");
				}
				else
				{
					if (mattrib)
					{
						makeerror (error, 905, "Missing mandatory attribute",
								   matchid);
					}
					else
					{
						makeerror (error, 906, "Missing mandatory child",
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
	int i;
	for (i=0; i<obj.count(); ++i)
	{
		if (obj[i].id()) idchain.newval() = obj[i].id().sval();
		else
		{
			rkey.crop();
			rkey.printf ("%s[%i]", obj[i].type().str(), i);
			idchain.newval() = i;
		}
		
		if (! matchOr (obj[i], def, error))
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
	
	for (int i=0; i<obj.attributes().count(); ++i)
	{
		attrname = "ATTRIB::";
		attrname.strcat (obj.attributes()[i].id().sval());
		idchain.newval() = attrname;
		
		if (! matchOr (obj.attributes()[i], def, error))
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
					makeerror (error, 907, "Unknown attribute",
							   obj.attributes()[i].label().sval());
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
	
	for (int i=0; i<def.count(); ++i)
	{
		mtype = def[i].type();
		if (mtype == "text")
		{
			if (matchDataText (obj, def[i].sval()))
				return true;
		}
		else if (mtype == "regexp")
		{
			if (matchDataRegexp (obj, def[i].sval()))
				return true;
		}
		else if (mtype == "lt")
		{
			if (matchDataLessThan (obj, def[i].ival()))
				return true;
		}
		else if (mtype == "gt")
		{
			if (matchDataGreaterThan (obj, def[i].ival()))
				return true;
		}
		else if (mtype == "minsize")
		{
			if (matchDataMinSize (obj, def[i].ival()))
				return true;
		}
		else if (mtype == "maxsize")
		{
			if (matchDataMaxSize (obj, def[i].ival()))
				return true;
		}
		else
		{
			makeerror (error, 908, "Unknown match-type", mtype);
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
		makeerror (error, 909, "Data match failed", obj.sval());
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
		makeerror (error, 910, "Expected child with index key");
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
		makeerror (error, 912, "Unknown type constraint", def.sval());
		return false;
	}
	
	if (obj.itype == tmap[def.sval()].ival()) return true;
	
	if (def.attribexists ("errortext"))
	{
		makeerror (error, def("errorcode").ival(),
				   def("errortext").sval());
	}
	else
	{
		makeerror (error, 913, "Object has wrong type","");
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
	return (obj.sval().strlen() >= val);
}

// ========================================================================
// METHOD ::matchDataMaxSize
// -------------------------
// Handling for <match.data><maxsize>...</maxsize></match.data>.
// ========================================================================
bool validator::matchDataMaxSize (const value &obj, int val)
{
	return (obj.sval().strlen() <= val);
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
	string *res = new string;
	
	for (int i=0; i<idchain.count(); ++i)
	{
		res->strcat ('/');
		if (! idchain[i].sval().validate (V_NOENCODE))
		{
			res->printf ("\"%S\"", idchain[i].cval());
		}
		else
		{
			res->strcat (idchain[i].sval());
		}
	}
	return res;
}

void validator::makeerror (string &into, int errorcode,
						   const string &errortext,
						   const string &detail)
{
	into.crop (0);
	string path = encodeidchain();
	
	into.printf ("<%03i> [%s] %s", errorcode, path.str(),
								   errortext.str());
	
	if (detail.strlen())
		into.printf (" (%s)", detail.str());
}

