// ========================================================================
// xmlschema_def.cpp: XML schema definition utility class
//
// (C) Copyright 2004-2006 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam 
// ========================================================================

#include <grace/value.h>
#include <grace/visitor.h>
#include <grace/xmlschema.h>
#include <grace/valueindex.h>

// ========================================================================
// METHOD ::addbasemembers
// -----------------------
// Adds all basic types as possible members of a class proplist.
// ========================================================================
void xmlschema::addbasemembers (value &v)
{
	v[t_unsigned].type (key::xml_member);
	v[t_int].type (key::xml_member);
	v[t_string].type (key::xml_member);
	v[t_bool].type(key::xml_member);
	v[t_date].type(key::xml_member);
	v[t_ipaddr].type(key::xml_member);
	v[t_array].type(key::xml_member);
	v[t_dict].type(key::xml_member);
	v[t_long].type(key::xml_member);
	v[t_ulong].type(key::xml_member);
	v[t_short].type(key::xml_member);
	v[t_ushort].type(key::xml_member);
	v[t_char].type(key::xml_member);
	v[t_uchar].type(key::xml_member);
	v[t_unset].type (key::xml_member);
	v[t_double].type (key::xml_member);
	v[t_currency].type(key::xml_member);
}

// ========================================================================
// METHOD ::addbaseclass
// ---------------------
// Creates a base class definition with an index called 'id' in the
// schema
// ========================================================================
void xmlschema::addbaseclass (const statstring &basetype,
							  const char *type,
							  const char *code,
							  const char *xcode)
{
	schema[basetype].type (key::xml_class);
	schema[-1][key::xml_type] = type;
	schema[-1][key::xml_code] = code;
	schema[-1][key::xml_attributes][key::id].type (key::xml_attribute);
	schema[-1][-1][-1].setattrib (key::isindex,true);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][-1][key::xml_code] = xcode;
}

// ========================================================================
// FUNCTION ::xmlbaseschema
// ------------------------
// Creates the base schema
// ========================================================================
void xmlschema::xmlbaseschema (void)
{
	addbaseclass (t_unset, "dict", "VOID", "VOIx");
	addbasemembers (schema[t_unset][key::xml_proplist]);
	addbaseclass (t_unsigned, "unsigned", "UINT", "UINx");
	addbaseclass (t_int, "integer", "INTE", "INTx");
	addbaseclass (t_string, "string", "STRN", "STRx");
	addbaseclass (t_bool, "bool", "BOOL", "BOOx");
	addbaseclass (t_date, "date", "DATE", "DATx");
	addbaseclass (t_ipaddr, "ipaddr", "IPAD", "IPAx");
	addbaseclass (t_array, "dict", "ARRY", "ARRx");
	addbasemembers (schema[t_array][key::xml_proplist]);
	addbaseclass (t_dict, "dict", "DICT", "DICx");
	addbasemembers (schema[t_dict][key::xml_proplist]);
	addbaseclass (t_long, "long", "LONG", "LONx");
	addbaseclass (t_ulong, "ulong", "ULON", "ULOx");
	addbaseclass (t_short, "short", "SHRT", "SHRx");
	addbaseclass (t_ushort, "ushort", "USHR", "USHx");
	addbaseclass (t_char, "char", "CHAR", "CHAx");
	addbaseclass (t_uchar, "uchar", "UCHR", "UCHx");
	addbaseclass (t_double, "float", "FLOT", "FLOx");
	addbaseclass (t_currency, "currency", "CRCY", "CRCx");
}
