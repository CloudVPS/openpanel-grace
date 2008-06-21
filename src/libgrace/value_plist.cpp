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
	type ("plist");
	return toxml (compact,xmlschema::plist());
}

// ========================================================================
// METHOD ::saveplist
// ========================================================================
void value::saveplist (const string &filename, bool compact)
{
	type ("plist");
	savexml (filename, compact, xmlschema::plist());
}

// ========================================================================
// METHOD ::fromplist
// ========================================================================
void value::fromplist (const string &xml)
{
	fromxml (xml, xmlschema::plist());
}

// ========================================================================
// METHOD ::loadplist
// ========================================================================
void value::loadplist (const string &filename)
{
	loadxml (filename, xmlschema::plist());
}
