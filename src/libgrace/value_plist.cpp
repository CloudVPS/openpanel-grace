// ========================================================================
// value_plist.cpp: Keyed generic data storage class
//
// (C) Copyright 2003-2005 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam 
// ========================================================================

//      012345678 TAB-O-METER should measure 4
//      ^	^


#include <grace/value.h>
#include <grace/file.h>
#include <grace/stack.h>
#include <grace/strutil.h>
#include <grace/filesystem.h>
#include <grace/xmlschema.h>

#include <stdio.h>
#include <string.h>

// ========================================================================
// METHOD ::toplist
// ========================================================================
string *value::toplist (bool compact)
{
	xmlschema schema (XMLPlistSchemaType);
	type ("plist");
	return toxml (compact,schema);
}

// ========================================================================
// METHOD ::saveplist
// ========================================================================
void value::saveplist (const string &filename, bool compact)
{
	xmlschema schema (XMLPlistSchemaType);
	type ("plist");
	savexml (filename, compact, schema);
}

// ========================================================================
// METHOD ::fromplist
// ========================================================================
void value::fromplist (const string &xml)
{
	xmlschema schema (XMLPlistSchemaType);
	fromxml (xml, schema);
}

// ========================================================================
// METHOD ::loadplist
// ========================================================================
void value::loadplist (const string &filename)
{
	xmlschema schema (XMLPlistSchemaType);
	loadxml (filename, schema);
}
