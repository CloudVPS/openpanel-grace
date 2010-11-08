// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

// ========================================================================
// xmlschema.cpp: XML schema definition utility class
// ========================================================================

#include <grace/value.h>
#include <grace/visitor.h>
#include <grace/valueindex.h>
#include <grace/xmlschema.h>

// ------------------------------------------------------------------------
// Static keys used extensively in XML schemas. This will save space and
// needless hash calculations
// ------------------------------------------------------------------------

// ========================================================================
// CONSTRUCTOR xmlschema
// ========================================================================
xmlschema::xmlschema (const string &name)
{
	schema.loadxml (name,xmlschema::root());
}

xmlschema &xmlschema::root (void)
{
	static xmlschema x (XMLRootSchemaType);
	return x;
}

xmlschema &xmlschema::base (void)
{
	static xmlschema x (XMLBaseSchemaType);
	return x;
}

xmlschema &xmlschema::netdb (void)
{
	static xmlschema x (XMLNetDBSchemaType);
	return x;
}

xmlschema &xmlschema::runopt (void)
{
	static xmlschema x (XMLRunOptionsSchemaType);
	return x;
}

xmlschema &xmlschema::validator (void)
{
	static xmlschema x (XMLValidatorSchemaType);
	return x;
}

xmlschema &xmlschema::plist (void)
{
	static xmlschema x (XMLPlistSchemaType);
	return x;
}


// ========================================================================
// METHOD ::load
// ========================================================================
void xmlschema::load (const string &name)
{
	schema.loadxml (name,xmlschema::root());
	
	foreach (cl, schema)
	{
		if (cl.exists (key::xml_proplist))
		{
			foreach (member, cl[key::xml_proplist])
			{
				if (member.attribexists (key::type))
				{
					schema[member.id()][key::xml_type] = member (key::type);
				}
			}
		}
	}
}

// ========================================================================
// METHOD ::hasnamespaces
// ----------------------
// Determines whether the schema has any notion of namespaces and
// definitions how to deal with them.
// ========================================================================
bool xmlschema::hasnamespaces (void)
{
	if (! schema.exists (".options"))
	{
		return false;
	}
	if (! schema[".options"].exists ("namespaces"))
	{
		return false;
	}
	return true;
}

// ========================================================================
// METHOD ::hasrootclass
// ========================================================================
bool xmlschema::hasrootclass (void)
{
	if (! schema.exists (".options")) return false;
	if (! schema[".options"].exists ("rootclass")) return false;
	return true;
}

// ==========================================================================
// METHOD xmlschema::tagkey
// ==========================================================================
bool xmlschema::tagkey (void)
{
	if (! schema.exists (".options")) return false;
	if (! schema[".options"].exists ("tagkey")) return false;
	return schema[".options"]["tagkey"];
}

// ========================================================================
// METHOD ::getrootclass
// ========================================================================
const string &xmlschema::getrootclass (void)
{
	return schema[".options"]["rootclass"].sval();
}

// ========================================================================
// METHOD ::hasdoctype
// ========================================================================
bool xmlschema::hasdoctype (void)
{
	if (! schema.exists (".options")) return false;
	if (! schema[".options"].exists ("doctype")) return false;
	return true;
}

// ========================================================================
// METHOD ::doctype
// ========================================================================
const value &xmlschema::doctype (void)
{
	return schema[".options"]["doctype"];
}

// ========================================================================
// METHOD ::nstranstype
// --------------------
// Translates a type sting that is potentially prepended by a namespace
// prefix (i.e. <foo:barQuux/>) into one without a namespace or a
// namespace prefix indicated by the schema.
// ========================================================================
void xmlschema::nstranstype (value &nsNames, statstring &type)
{
	// No colon, no namespace, no work.
	if (type.sval().strchr (':') <0) return;
	
	// See if we have a cache entry
	if (nsNames.exists (type))
	{
		type = nsNames[type].sval();
		return;
	}
	
	string t1, t2, newType;
	statstring otype;
	
	t1 = type;
	otype = type;
	t2 = t1.cutat (':');
	
	if (nsNames.exists (t2)) // See if the namespace exists
	{
		if (nsNames[t2].sval().strlen()) // Replace?
		{
			// Replace namespace by the new one
			// Store it in the cache
			newType = "%s:%s" %format (nsNames[t2], t1);
			nsNames[otype] = newType;
			type = newType;
		}
		else // No, strip
		{
			type = t1; // just the type please
			nsNames[otype] = type.str();
		}
	}
	else // No such namespace
	{
		// See if there is a default namespace
		if (nsNames.exists ("*"))
		{
			if (nsNames["*"].sval().strlen()) // replace?
			{
				newType = "%s:%s" %format (nsNames["*"], t1);
				nsNames[otype] = newType;
				type = newType;
			}
			else // strip
			{
				type = t1;
				nsNames[otype] = type.sval();
			}
		}
		else // No default namespace, keep type intact and cache
		{
			nsNames[type] = type.sval();
		}
	}
}

// ========================================================================
// METHOD ::nstransatr
// -------------------
// Inspects an attribute name, and optionally its value. If the attribute
// name has a namespace, it looks at earlier definitions of that namespace
// using xmlns:namespace="uri" attributes. If it runs into an xmlns
// attribute, the uri is looked up inside the <xml.option.namespaces> part
// of the schema and the apropriate translation is made, either:
//
//   "keep"    - leaves the namespace alone
//   "strip"   - strips the namespace entirely
//   "replace" - replaces the namespace with a schema-defined prefix
// ========================================================================
void xmlschema::nstransattr (value &nsNames, statstring &attr, const value &v)
{
	// No namespace, no job
	if (attr.sval().strchr (':') < 0) return;
    
    // Look up cached resolve
    if (nsNames.exists (attr))
    {
    	attr = nsNames[attr].sval();
    	return;
    }

	string nsName, atName;
	atName = attr;
	nsName = atName.cutat (':');

	// namespace definition attribute?
	if (nsName == "xmlns")
	{
		string nsAction;
		string nsReplace;
		statstring nsUri;
		
		nsUri = v.sval();
		
		// Find a namespace with that URI
		if (schema[".options"]["namespaces"].exists (nsUri))
		{
			// Get the schema action and optional replace value for this
			// namespace
			nsAction = schema[".options"]["namespaces"][nsUri](key::action);
			nsReplace = schema[".options"]["namespaces"][nsUri](key::prefix);
			
			caseselector (nsAction)
			{
				incaseof ("keep") :
					nsNames[atName] = nsName;
					nsReplace = nsName;
					break;
				
				incaseof ("strip") :
					nsNames[atName] = "";
					nsReplace.crop (0);
					break;
				
				defaultcase :
					nsNames[atName] = nsReplace;
					break;
			}
			
			visitor<const value> probe (schema[".options"]["namespaces"][nsUri]);
			string aliasval;
			string orgtype;
			
			if (probe.first())
			{
				do {
					aliasval.crop(0);
					orgtype = "%s:%s" %format (atName, probe.obj().id());
					if (nsReplace.strlen())
					{
						aliasval = "%s:" %format (nsReplace);
					}
					aliasval.strcat (probe.obj()("alias").sval());
					nsNames[orgtype] = aliasval;
				} while (probe.next());
			}
		} 
	}
	else // Not an xmlns:foo attribute, so parse it.
	{
		string nsNew;
		statstring oldAttr;
		string newAttr;
		
		oldAttr = attr;
		
		// Is this namespace registred?
		if (nsNames.exists (nsName))
		{
			// Resolve it to a target namespace
			nsNew = nsNames[nsName];
			if (nsNew.strlen())
			{
				newAttr = "%s:%s" %format (nsNew, atName);
				attr = newAttr;
				nsNames[oldAttr] = newAttr;
			}
			else // empty target, strip the namespace
			{
				nsNames[oldAttr] = atName;
				attr = atName;
			}
		}
	}
}

// ========================================================================
// METHOD ::knownclass
// -------------------
// Determines whether a class with a specified name is defined in the
// schema
// ========================================================================
bool xmlschema::knownclass (const string &name)
{
	return schema.exists (name);
}

// ========================================================================
// METHOD ::resolveid
// ------------------
// Determines an implicit id for a given class with a given superclass
// ========================================================================
const string &xmlschema::resolveid (const string &forclass,
									const string &superclass)
{
	static string empty;	
	visitor<const value> probe (schema);
	
	if (! probe.enter (superclass))
	{
		if (tagkey()) return forclass;
		return empty;
	}
	if (! probe.enter (key::xml_proplist))
	{
		if (tagkey()) return forclass;
		return empty;
	}
		
	if (! probe.enter (forclass))
	{
		if (tagkey()) return forclass;
		return empty;
	}
	
	return probe.obj()(key::id).sval();
}

// ========================================================================
// METHOD ::wouldneedprecedents
// ----------------------------
// Determines if attribute-less storage needs to create attributes as
// precedent values. If the class is of the dict-type, the attributes
// can be embedded as child values.
// ========================================================================
bool xmlschema::wouldneedprecedents (const statstring &forclass)
{
	statstring itstype;
	
	itstype = resolvetype (forclass);
	if ((itstype != t_dict)&&(itstype != t_array)) return true;
	return false;
}

// ========================================================================
// METHOD ::resolvetype
// --------------------
// Figures out the internal type implied by a certain class.
// ========================================================================
const string &xmlschema::resolvetype (const statstring &forclass)
{
	if (schema.exists (forclass)) return schema[forclass][key::xml_type].sval();
	return forclass.sval();
}

// ========================================================================
// METHOD ::resolvetypeattrib
// --------------------------
// Determines the implied internal type of a named attribute for a given
// class
// ========================================================================
const string &xmlschema::resolvetypeattrib (const statstring &forclass,
											const statstring &withlabel)
{
	visitor<const value> probe (schema);
	
	if (! probe.enter (forclass)) return t_string.sval();
	if (! probe.enter (key::xml_attributes)) return t_string.sval();
	if (! probe.enter (withlabel)) return t_string.sval();
	return probe.obj()[key::xml_type].sval();
}

// ========================================================================
// METHOD ::resolveclass
// ---------------------
// Given a class, a superclass and an id-value, it determines whether the
// id-value implies a class-type from the schema.
// ========================================================================
void xmlschema::resolveclass (const statstring &id,
						      const statstring &currentclass,
							  const statstring &superclass,
							  const statstring &superid,
							        statstring &into)
{
	visitor<const value> probe (schema);
	bool contained = false;
	
	/*::printf ("resolveclass id=<%s> currentclass=<%s> "
			  "superclass=<%s> superid=<%s>\n",
			  id ? id.str() : "null",
			  currentclass ? currentclass.str() : "null",
			  superclass ? superclass.str() : "null",
			  superid ? superid.str() : "null"); */
	
	if (currentclass == superclass)
	{
		into = currentclass;
		return;
	}
	
	into = currentclass;
	
	if (probe.enter (currentclass))
	{
		into = probe.id();
		if (probe.obj()("contained").bval() == true)
		{
			//::printf ("currentclass found, but is contained\n");
			contained = true;
			probe.root ();
			if (probe.enter (superclass))
			{
				if (probe.obj()[key::xml_type] != key::container);
				contained = false;
				probe.root();
			}
		}
		else
		{
			probe.root ();
			if (! probe.enter (superclass))
				return;
			
			if (! probe.enter (key::xml_proplist))
				return;
				
			if (! probe.first())
				return;
				
			do
			{
				if (id == probe.obj()(key::id).sval())
				{
					into = probe.id();
					return;
				}
			} while (probe.next());
			
			probe.root();
			return;
		}
	}
	
	//::printf ("trying to enter superclass\n");
	if (! probe.enter (superclass))
	{
		if (! probe.enter (superid)) return;
	}
	
	if (probe.obj()[key::xml_type] == "container") contained = true;
	
	//if ( (! contained) && (! id) ) return currentclass;
	
	//::printf ("trying to enter xml.proplist\n");
	if (! probe.enter (key::xml_proplist))
	{
		//::printf ("no go\n");
		if (! contained) return;
		//::printf ("but we're contained!\n");
		if (! probe.enter (key::xml_container)) return;
		if (! probe.enter (key::xml_container_types)) return;
		//::printf ("reached xml.container.types\n");
		if (! probe.obj().exists (currentclass))
		{
			//::printf ("didn't find current class (%s) among the containees\n", currentclass.str());
			if (currentclass == t_bool)
			{
				//::printf ("bool exception\n");
				if (probe.obj().exists (t_bool_true))
				{
					//::printf ("resolved\n");
					into = t_bool_true;
					return;
				}
			}
			return;
		}
	
		into = probe.obj()[currentclass];
		return;
	}
	
	//if (probe.enter (currentclass)) return;
	if (! probe.first ()) return;

	do
	{
		if ((id == probe.obj()(key::id).sval()))
		{
			into = probe.id();
			return;
		}
		else if ((!contained) && (probe.obj()(key::id).sval().strlen() == 0))
		{
			if (schema.exists (probe.id()))
			{
				if (schema[probe.id()][key::xml_type] == currentclass.str())
				{
					into = probe.id();
					return;
				}
			}
			else
			{
				into = probe.id();
				return;
			}
		}
	} while (probe.next());
	
	return;
}

// ========================================================================
// METHOD ::resolveidexport
// ------------------------
// Figures out if an object's id is implied from the class.
// ========================================================================
const statstring &xmlschema::resolveidexport (const statstring &id,
											 const statstring &currentclass,
											 const statstring &superclass,
											 const statstring &superid)
{
	static statstring empty;
	visitor<const value> probe (schema);
	
	if (probe.enter (superclass) || probe.enter (superid))
	{
		if (probe.enter (key::xml_proplist))
		{
			if (id && probe.first())
			{
				do {
					if (id == probe.obj()(key::id).sval())
					{
						return empty;
					}
				} while (probe.next());
				probe.up();
			}
			if (probe.enter (currentclass))
			{
				if (probe.obj().attributes().exists (key::id))
				{
					return empty;
				}
			}
			else
			{
				return id;
			}
		}
	}
	return id;
}

// ========================================================================
// METHOD ::stringclassisbase64
// ========================================================================
bool xmlschema::stringclassisbase64 (const statstring &theclass)
{
	visitor<const value> probe (schema);
	
	if (probe.enter (theclass))
	{
		if (probe.obj()[key::xml_encoding] == "base64")
			return true;
	}
	return false;
}

// ========================================================================
// METHOD ::iswrap
// ========================================================================
bool xmlschema::iswrap (const statstring &theclass)
{
	if (! schema.exists (theclass)) return false;
	return schema[theclass](key::wrap).bval();
}

// ========================================================================
// METHOD ::iswrapcontainer
// ========================================================================
bool xmlschema::iswrapcontainer (const statstring &theclass)
{
	statstring ctype;
	
	if (! schema.exists (theclass)) return false;
	if (schema[theclass][key::xml_type] != "container") return false;
	
	ctype = schema[theclass][key::xml_container][key::xml_container_types][0];
	if (! ctype) return false;
	if (! schema.exists (ctype)) return false;
	if (schema[ctype](key::wrap) == true) return true;
	return false;
}

// ========================================================================
// METHOD ::containerhasattributes
// ========================================================================
bool xmlschema::containerhasattributes (const statstring &theclass)
{
	if (! schema.exists (theclass)) return false;
	if (schema[theclass].exists (key::xml_attributes)) return true;
	return false;
}

// ========================================================================
// METHOD ::containerhasattribute
// ========================================================================
bool xmlschema::containerhasattribute (const statstring &theclass,
									   const statstring &theattrib)
{
	if (schema[theclass][key::xml_attributes].exists (theattrib))
		return true;
	
	return false;
}

// ========================================================================
// METHOD ::hasvalueattribute
// ========================================================================
bool xmlschema::hasvalueattribute (const statstring &theclass)
{
	if (! schema.exists (theclass)) return false;
	if (! schema[theclass].attribexists (key::attribvalue)) return false;
	return true;
}

// ========================================================================
// METHOD ::resolvevalueattribute
// ========================================================================
const string &xmlschema::resolvevalueattribute (const statstring &theclass)
{
	return schema[theclass](key::attribvalue).sval();
}

// ========================================================================
// METHOD ::isiplicitarray
// ========================================================================
bool xmlschema::isimplicitarray (const statstring &theclass)
{
	if (! schema.exists (theclass)) return false;
	if (schema[theclass](key::array) == true) return true;
	return false;
}

// ========================================================================
// METHOD ::isunion
// ========================================================================
bool xmlschema::isunion (const statstring &theclass)
{
	if (! schema.exists (theclass)) return false;
	if (schema[theclass][key::xml_type] == "union") return true;
	return false;
}

// ========================================================================
// METHOD ::resolveunion
// ========================================================================
const statstring &xmlschema::resolveunion (const value *that,
										   const statstring &theclass)
{
	string matchtype;
	string matchlabel;
	string matchdata;
	
	foreach (udef, schema[theclass][key::xml_union])
	{
		matchtype = udef(key::type);
		matchlabel = udef(key::label);
		matchdata = udef.sval();
	
		caseselector (matchtype)
		{
			incaseof ("attribexists") :
				if (that->attribexists (matchlabel))
					return udef.id();
				break;
			
			incaseof ("memberexists") :
				if (that->exists (matchlabel))
					return udef.id();
				break;
			
			incaseof ("attribvalue") :
				if (that->attribexists (matchlabel))
				{
					if ((*that)(matchlabel).sval() == matchdata)
						return udef.id();
				}
				break;
			
			incaseof ("membervalue") :
				if (that->exists (matchlabel))
				{
					string tmp;
					tmp = (*that)[matchlabel].sval();
					if (tmp == matchdata)
						return udef.id();
				}
				break;
			
			defaultcase : return udef.id();
		}
	}
	return theclass;
}

// ========================================================================
// METHOD ::resolveunionbase
// ========================================================================
void xmlschema::resolveunionbase (statstring &theclass)
{
	if (! schema[theclass].attribexists ("union")) return;
	theclass = schema[theclass]("union");
}

bool xmlschema::iscontainerclass (const statstring &theclass)
{
	visitor<const value> probe (schema);
	
	if (probe.enter (theclass))
	{
		if (probe.obj()[key::xml_type] == "container")
		{
			return true;
		}
	}
	return false;
}

// ========================================================================
// METHOD ::resolvecontainerenvelope
// ========================================================================
const string &xmlschema::resolvecontainerenvelope (const statstring &theclass)
{
	static string empty;
	
	if (schema.exists (theclass))
	{
		return schema[theclass][key::xml_container]
					 [key::xml_container_envelope].sval();
	}
	return empty;
}

// ========================================================================
// METHOD ::resolvecontainerwrapclass
// ========================================================================
const string &xmlschema::resolvecontainerwrapclass (const statstring &theclass)
{
	static string empty;
	
	if (schema.exists (theclass))
	{
		return schema[theclass][key::xml_container][key::xml_container_wrapclass].sval();
	}
	return empty;
}

// ========================================================================
// METHOD ::resolvecontaineridclass
// ========================================================================
const string &xmlschema::resolvecontaineridclass (const statstring &theclass)
{
	static string empty;
	
	if (schema.exists (theclass))
	{
		return schema[theclass][key::xml_container][key::xml_container_idclass].sval();
	}
	return empty;
}

// ========================================================================
// METHOD ::resolvecontainervalueclass
// ========================================================================
const string &xmlschema::resolvecontainervalueclass (const statstring &thecl)
{
	static string empty;
	
	if (schema.exists (thecl))
		return schema[thecl][key::xml_container][key::xml_container_valueclass].sval();
	
	return empty;
}

// ========================================================================
// METHOD ::resolvecontainerarrayclass
// ========================================================================
string *xmlschema::resolvecontainerarrayclass (const statstring &thecl)
{
	returnclass (string) result retain;
	visitor<const value> probe (schema);

	if (probe.enter (thecl))
	{
		if (probe.enter (key::xml_container))
		{
			if (probe.enter (key::xml_container_types))
			{
				if (probe.obj().exists ("array"))
				{
					result = probe.obj()["array"].sval();
					return &result;
				}
			}
		}
	}
	
	result = "array";
	return &result;
}

// ========================================================================
// METHOD ::resolvecontainerdictclass
// ========================================================================
string *xmlschema::resolvecontainerdictclass (const statstring &thecl)
{
	returnclass (string) result retain;
	visitor<const value> probe (schema);

	if (probe.enter (thecl))
	{
		if (probe.enter (key::xml_container))
		{
			if (probe.enter (key::xml_container_types))
			{
				if (probe.obj().exists ("dict"))
				{
					result = probe.obj()["dict"].sval();
					return &result;
				}
			}
		}
	}
	
	result = "dict";
	return &result;
}

// ========================================================================
// METHOD ::resolvecontainerboolclass
// ========================================================================
string *xmlschema::resolvecontainerboolclass (const statstring &thecl,
											  bool val)
{
	returnclass (string) result retain;
	visitor<const value> probe (schema);

	if (probe.enter (thecl))
	{
		if (probe.enter (key::xml_container))
		{
			if (probe.enter (key::xml_container_types))
			{
				if (probe.obj().exists (t_bool))
				{
					result = probe.obj()[t_bool].sval();
					return &result;
				}
				else if (val && probe.obj().exists (t_bool_true))
				{
					result = probe.obj()[t_bool_true].sval();
					return &result;
				}
				else if (probe.obj().exists (t_bool_false))
				{
					result = probe.obj()[t_bool_true].sval();
					return &result;
				}
			}
		}
	}
	
	result = "bool";
	return &result;
}

// ========================================================================
// METHOD ::resolvecontainertypeclass
// ========================================================================
string *xmlschema::resolvecontainertypeclass (const statstring &thecl,
											  unsigned char itype)
{
	returnclass (string) result retain;
	statstring realtype;
	
	switch (itype)
	{
		case i_int: realtype = "integer"; break;
		case i_unsigned: realtype = "unsigned"; break;
		case i_double: realtype = "float"; break;
		case i_long: realtype = "long"; break;
		case i_ulong: realtype = "ulong"; break;
		case i_bool: realtype = "bool"; break;
		case i_string: realtype = "string"; break;
		case i_ipaddr: realtype = "ipaddr"; break;
		case i_date: realtype = "date"; break;
		default: realtype = "string"; break;
	}

	visitor<const value> probe (schema);

	if (probe.enter (thecl))
	{
		if (probe.enter (key::xml_container))
		{
			if (probe.enter (key::xml_container_types))
			{
				if (probe.obj().exists (realtype))
				{
					result = probe.obj()[realtype].sval();
					return &result;
				}
			}
		}
	}
	
	result = realtype.sval();
	return &result;
}

// ========================================================================
// METHOD ::resolveindexname
// -------------------------
// Determines the name of the index attribute of a given class
// ========================================================================
const statstring &xmlschema::resolveindexname (const statstring &ofclass)
{
	static statstring defid (key::id);
	visitor<const value> probe (schema);
	
	if (! probe.enter (ofclass)) return defid;
	if (! probe.enter (key::xml_attributes)) return defid;
	if (! probe.first()) return defid;
	do {
		if (probe.obj().attribexists(key::isindex))
		{
			if (probe.obj()(key::isindex) == true)
			{
				return probe.obj().label();
			}
		}
	} while (probe.next());
	
	return defid;
}

// ========================================================================
// METHOD ::isinternaltype
// -----------------------
// Checks if a given class/type name is an internal primitive type.
// ========================================================================
bool xmlschema::isinternaltype (const statstring &t)
{
	if (t == t_unset) return true;
	if (t == t_int) return true;
	if (t == t_bool) return true;
	if (t == t_string) return true;
	if (t == t_dict) return true;
	if (t == t_unsigned) return true;
	if (t == t_long) return true;
	if (t == t_ulong) return true;
	if (t == t_char) return true;
	if (t == t_uchar) return true;
	if (t == t_ipaddr) return true;
	if (t == t_date) return true;
	if (t == t_currency) return true;
	return false;
}

// ========================================================================
// METHOD ::validate
// -----------------
// Checks if a value object adheres to the given schema. The extend
// argument determines whether extra members/attributes should be
// allowed (it will then only fail if something is missing).
// ========================================================================
bool xmlschema::validate (const value &obj,
						  const value &parent,
						  xmlschema::extspec extend)
{
	visitor<const value> probe (schema);
	statstring impliedclass;
	
	// we will use the canonized class if necessary
	impliedclass = obj.type();
	
	// is the parent class type known?
	if ((extend == xmlschema::forbid) && (probe.enter (parent.type())))
	{
		// we should be in its proplist as a member
		if (! probe.enter (key::xml_proplist)) return false;
		
		// if our type exists, we're cool
		if (! probe.enter (obj.type()))
		{
			// it's not that easy, perhaps we have a class-implied id?
			bool found = false;
			if (! obj.label()) return false;
			if (! probe.first ()) return false;
			do
			{
				if (probe.obj()(key::id).sval() == obj.label())
				{
					found = true;
					impliedclass = probe.obj().label();
					break;
				}
			} while (probe.next());
			if (! found) return false;
		}
		else if (obj.label())
		{
			if (probe.obj().attributes().exists (key::id))
			{
				if (probe.obj()(key::id).sval() != obj.label()) return false;
			}
		}
		probe.root();
	}

	// if no class is defined for our impliedclass, it should only be
	// an internal type	
	if (! probe.enter (impliedclass))
		return (extend || isinternaltype (impliedclass));
	
	// if attributes are defined, figure out if we measure up
	if (probe.enter (key::xml_attributes))
	{
		if (probe.first())
		{
			do // iterate over the class chema's attributes
			{
				// is this the index attribute?
				if (probe.obj()(key::isindex) == true)
				{
					if (! obj.label())
					{
						if (probe.obj()(key::mandatory) == true)
							return false;
					}
				}
				else
				{
					if (probe.obj()(key::mandatory) == true)
					{
						if (! obj.haveattributes ()) return false;
						if (! obj.attributes().exists (probe.obj().label()))
							return false;
					}
				}
			} while (probe.next());
			probe.up ();
		}
		else if (! extend)
		{
			if (obj.haveattributes ()) return false;
		}
		
		if ((! extend) && (obj.haveattributes()))
		{
			int i, acount;
			
			acount = obj.attributes().count();
			for (i=0; i<acount; ++i)
			{
				if (! probe.obj().exists (obj.attributes()[i].label()))
					return false;
			}
		}
		probe.up ();
	}
	else if ( (!extend) && (obj.haveattributes ()) ) return false;
	
	if (probe.enter (key::xml_proplist))
	{
		if (probe.first())
		{
			do
			{
				statstring memberclass;
				statstring memberid;
				
				memberclass = probe.obj().label();
				if (probe.obj().attributes().exists (key::id))
				{
					memberid = probe.obj()(key::id).sval();
					if (! obj.exists (memberid))
					{
						if (probe.obj()(key::mandatory) == true)
							return false;
					}
				}
				else if (probe.obj()(key::mandatory)== true)
				{
					int i, acount;
					bool foundcl = false;
					
					acount = obj.count();
					for (i=0; i<acount; ++i)
					{
						if (obj[i].type() == memberclass)
						{
							foundcl = true;
							i = acount;
						}
					}
					if (! foundcl) return false;
				}
			} while (probe.next());
			
			probe.up();
		}
		
		if ((!extend) && (obj.count()))
		{
			valueindex members;
			members.indexproperty ((value &) probe.obj(), key::id);
			
			foreach (e, obj)
			{
				if (! probe.obj().exists (e.type()))
				{
					if (! members.exists (e.label()))
						return false;
				}
			}
		}
	}
	else if ((!extend) && obj.count()) return false;
	return true;
}

// ========================================================================
// METHOD ::create
// ---------------
// Fills in all the default attributes and members for a value of a given
// type. 
// ========================================================================
value *xmlschema::create (const statstring &type)
{
	returnclass (value) res retain;
	visitor<const value> probe (schema);

	res.type (type);	
	if (! probe.enter (type)) return &res;
	
	if (probe.enter (key::xml_attributes))
	{
		if (probe.first())
		{
			do
			{
				if ((probe.obj()(key::mandatory) == true) &&
				    (probe.obj()(key::isindex) == false))
				{
					if (probe.obj()[key::type] == "string")
					{
						res.setattrib (probe.obj().label(), "");
					}
					else if (probe.obj()[key::type] == "bool")
					{
						res.setattrib (probe.obj().label(), false);
					}
					else
					{
						res.setattrib (probe.obj().label(), 0);
					}
				}
			} while (probe.next());
			probe.up();
		}
		probe.up();
	}
	
	if (probe.enter (key::xml_proplist))
	{
		if (probe.first())
		{
			do
			{
				if ((probe.obj()(key::mandatory) == true) &&
				    (probe.obj().attributes().exists (key::id)))
				{
					res[probe.obj()(key::id).sval()] = create (probe.obj().label());
				}
			} while (probe.next());
			//probe.up();
		}
		//probe.up();
	}
	
	return &res;
}

// ========================================================================
// METHOD ::inspectcode
// --------------------
// Creates lookup-cache data for a 4-character CXML opcode. 
// ========================================================================
value &xmlschema::inspectcode (const char *opcode)
{
	// Look up the opcode in the cache
	if ((! codecache.exists (opcode)) ||
	    (! codecache[opcode].exists (key::type)))
	{
		// Iterate over all class objects inside the schema
		visitor<const value> probe (schema);
		if (probe.first()) do
		{
			// See if this one is ours
			if (probe.obj()[key::xml_code].sval() == opcode)
			{
				bool isdict = true;
				statstring objtype;
				statstring objclass;
				
				objclass = probe.obj().label();
				objtype = probe.obj()[key::xml_type].sval();
				if (isinternaltype (objtype))
				{
					if ( (objtype != t_dict) && (objtype != t_array) )
						isdict = false;
				}
				
				// Put all relevant data inside the cache
				codecache[opcode][key::isdict] = isdict;
				codecache[opcode][key::isattribute] = false;
				codecache[opcode][key::implicit] = false;
				codecache[opcode][key::isclass] = true;
				codecache[opcode][key::type] = probe.obj()[key::xml_type];
				codecache[opcode][key::klass] = probe.obj().label().str();
				
				// If there are attributes, go over them to get more
				// entries for the cache so we can properly resolve them
				if (probe.enter (key::xml_attributes))
				{
					statstring attrlabel;
					statstring attrcode;
					
					if (probe.first()) do
					{
						attrlabel = probe.obj().label();
						attrcode = probe.obj()[key::xml_code].sval();
						
						codecache[attrcode][key::isattribute] = true;
						if (probe.obj()(key::isindex) == true)
						{
							codecache[attrcode][key::isindex] = true;
						}
						if (!isdict)
						{
							codecache[attrcode][key::isprecedent] = true;
							codecache[attrcode][key::ofclass] = objclass.str();
						}
						codecache[attrcode][key::type] = probe.obj()[key::xml_type];
						codecache[attrcode][key::label] = attrlabel.str();
					} while (probe.next());
					probe.up();
					probe.up();
				}
				if (probe.enter (key::xml_proplist))
				{
					if (probe.first()) do
					{
						statstring objlabel;
						string topcode;
						
						objlabel = probe.obj().label();
						if (schema.exists (objlabel))
						{
							topcode = schema[objlabel][key::xml_code].sval();
							inspectcode (topcode.str());
							if (probe.obj().attributes().exists (key::id))
							{
								codecache[topcode][key::implicit] =
									true;
								codecache[topcode][key::id] =
									probe.obj()(key::id).sval();
							}
						}
					} while (probe.next());
					probe.up();
					probe.up();
				}
				return codecache[opcode];
			}
		} while (probe.next());
	}
	
	return codecache[opcode];
}

// ========================================================================
// METHOD ::isattribute
// --------------------
// Returns true if a provided CXML code is that of an attribute 
// ========================================================================
bool xmlschema::isattribute (const char *opcode)
{
	return (bool) inspectcode (opcode)[key::isattribute];
}

// ========================================================================
// METHOD ::isindex
// ----------------
// Returns true if the provided CXML opcode represents an index key.
// ========================================================================
bool xmlschema::isindex (const char *opcode)
{
	return (bool) inspectcode (opcode)[key::isindex];
}

// ========================================================================
// METHOD ::isprecedent
// --------------------
// Returns true if the opcode represents a precedent attribute
// ========================================================================
bool xmlschema::isprecedent (const char *opcode)
{
	return (bool) inspectcode (opcode)[key::isprecedent];
}

// ========================================================================
// METHOD ::xmltype
// ----------------
// Resolves the builtin type for the given opcode. 
// ========================================================================
const string &xmlschema::xmltype (const char *opcode)
{
	return inspectcode (opcode)[key::type].sval();
}

// ========================================================================
// METHOD ::attributelabel
// -----------------------
// Determines the label for an attribute opcode
// ========================================================================
const string &xmlschema::attributelabel (const char *opcode)
{
	return inspectcode (opcode)[key::label].sval();
}

// ========================================================================
// METHOD ::xmlclass
// -----------------
// Resolve an opcode to a schema-defined class
// ========================================================================
const string &xmlschema::xmlclass (const char *opcode)
{
	return inspectcode (opcode)[key::klass].sval();
}

// ========================================================================
// METHOD ::isdict
// ---------------
// Returns true if the given opcode represents a dict class 
// ========================================================================
bool xmlschema::isdict (const char *opcode)
{
	return (bool) inspectcode (opcode)[key::isdict];
}

// ========================================================================
// METHOD ::hasimplicit
// --------------------
// Returns true if the given opcode represents an object with an implicit
// index key. 
// ========================================================================
bool xmlschema::hasimplicit (const char *opcode)
{
	return (bool) inspectcode (opcode)[key::implicit];
}

// ========================================================================
// METHOD ::implicitid
// -------------------
// Resolves the implicit key for a given opcode 
// ========================================================================
const string &xmlschema::implicitid (const char *opcode)
{
	return inspectcode (opcode)[key::id].sval();
}

// ========================================================================
// METHOD ::precedentclass
// -----------------------
// Resolves to the class name associated with the precedent attribute
// represented by the opcode. 
// ========================================================================
const string &xmlschema::precedentclass (const char *opcode)
{
	return inspectcode (opcode)[key::ofclass].sval();
}

// ========================================================================
// METHOD ::resolvecode
// --------------------
// Resolves the opcode for a given class
// ========================================================================
const char *xmlschema::resolvecode (const statstring &forclass)
{
	static const char *nullcode = "VOID";
	if (! schema.exists (forclass)) return nullcode;
	return schema[forclass][key::xml_code].cval();
}

// ========================================================================
// METHOD ::resolvecodeid
// ----------------------
// Resolves the opcode for a given class's index attribute
// ========================================================================
const char *xmlschema::resolvecodeid (const statstring &forclass)
{
	visitor<const value> probe (schema);
	char *nullcode = NULL;
	
	if (! probe.enter (forclass)) return nullcode;
	if (! probe.enter (key::xml_attributes)) return nullcode;
	if (! probe.first()) return nullcode;
	do {
		if (probe.obj()(key::isindex) == true)
			return probe.obj()[key::xml_code].cval();
	} while (probe.next());
	
	return nullcode;
}

// ========================================================================
// METHOD ::resolvecode
// --------------------
// Resolves the opcode for a given class attribute
// ========================================================================
const char *xmlschema::resolvecodeattrib (const statstring &forclass,
										  const statstring &withname)
{
	visitor<const value> probe (schema);
	char *nullcode = NULL;
	
	if (! probe.enter (forclass)) return nullcode;
	if (! probe.enter (key::xml_attributes)) return nullcode;
	if (! probe.enter (withname)) return nullcode;
	return probe.obj()[key::xml_code].cval();
}
