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
// CONSTRUCTOR xmlschema
// ---------------------
// Constructor for either the default RootSchema (used to define a schema)
// and BaseSchema (used to define data without a schema).
// ========================================================================
xmlschema::xmlschema (hardCodedSchema i)
{
	schema.type (key::xml_schema);
	
	switch (i)
	{
		case XMLRootSchemaType:
			xmlrootschema ();
			break;
			
		case XMLBaseSchemaType:
			xmlbaseschema ();
			break;
			
		case XMLNetDBSchemaType:
			netdbschema();
			break;
			
		case XMLRunOptionsSchemaType:
			runoptschema();
			break;
			
		case XMLValidatorSchemaType:
			validatorschema();
			break;
	}
}

// ========================================================================
// METHOD ::xmlrootschema
// ----------------------
// Creates the root schema
// ========================================================================
void xmlschema::xmlrootschema (void)
{
	// -------------------------------------------------------------------
	// class xml.schema
	// -------------------------------------------------------------------
	schema[key::xml_schema].type (key::xml_class);
	schema[-1][key::xml_type] = "dict";
	schema[-1][key::xml_code] = "XSch";

	schema[-1][key::xml_proplist][key::xml_class].type (key::xml_member);
    schema[-1][-1][key::xml_schema_options].type (key::xml_member);
    schema[-1][-1][-1].setattrib (key::id, ".options");
	
	// -------------------------------------------------------------------
	// class xml.schema.options
	// -------------------------------------------------------------------
	schema[key::xml_schema_options].type (key::xml_class);
	schema[-1][key::xml_type] = "dict";
	schema[-1][key::xml_code] = "XOpt";
	
	schema[-1][key::xml_proplist]
			  [key::xml_option_namespaces].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id, "namespaces");
	
	schema[-1][-1][key::xml_option_rootclass].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id, "rootclass");
	
	schema[-1][-1][key::xml_option_doctype].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id, "doctype");
	
	// -------------------------------------------------------------------
	// class xml.option.rootclass
	// -------------------------------------------------------------------
	schema[key::xml_option_rootclass].type (key::xml_class);
	schema[-1][key::xml_type] = "string";
	schema[-1][key::xml_code] = "XOrc";
	
	// -------------------------------------------------------------------
	// class xml.option.namespaces
	// -------------------------------------------------------------------
	schema[key::xml_option_namespaces].type (key::xml_class);
	schema[-1][key::xml_type] = "dict";
	schema[-1][key::xml_code] = "XOns";
	
	schema[-1][key::xml_proplist][key::xml_namespace].type (key::xml_member);
	
	// -------------------------------------------------------------------
	// class xml.namespace
	// -------------------------------------------------------------------
	schema[key::xml_namespace].type (key::xml_class);
	schema[-1][key::xml_type] = "dict";
	schema[-1][key::xml_code] = "XMns";
	
	schema[-1][key::xml_attributes][key::uri].type (key::xml_attribute);
	schema[-1][-1][-1].setattrib (key::mandatory, true);
	schema[-1][-1][-1].setattrib (key::isindex, true);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][-1][key::xml_code] = "NSur";
	
	schema[-1][-1][key::action].type (key::xml_attribute);
	schema[-1][-1][-1].setattrib (key::mandatory, true);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][-1][key::xml_code] = "NSax";
	
	schema[-1][-1][key::prefix].type (key::xml_attribute);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][-1][key::xml_code] = "NSpx";
	
	schema[-1][key::xml_proplist]
			  [key::xml_namespace_type].type (key::xml_attribute);

	// -------------------------------------------------------------------
	// class xml.namespace.type
	// -------------------------------------------------------------------
	schema[key::xml_namespace_type].type (key::xml_class);
	schema[-1][key::xml_type] = "void";
	schema[-1][key::xml_code] = "XnsT";
	
	schema[-1][key::xml_attributes]["class"].type (key::xml_attribute);
	schema[-1][-1][-1].setattrib (key::isindex, true);
	schema[-1][-1][-1].setattrib (key::mandatory, true);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][-1][key::xml_code] = "nsTC";
	
	schema[-1][-1][key::alias].type (key::xml_attribute);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][-1][key::xml_code] = "nsTA";
	
	// -------------------------------------------------------------------
	// class xml.option.doctype
	// -------------------------------------------------------------------
	schema[key::xml_option_doctype].type (key::xml_class);
	schema[-1][key::xml_type] = "string";
	schema[-1][key::xml_code] = "XOdt";
	
	schema[-1][key::xml_attributes][key::name].type (key::xml_attribute);
	schema[-1][-1][-1].setattrib (key::mandatory,true);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][-1][key::xml_code] = "XODn";
	
	schema[-1][key::xml_attributes][key::status].type (key::xml_attribute);
	schema[-1][-1][-1].setattrib (key::mandatory,true);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][-1][key::xml_code] = "XODs";
	
	schema[-1][key::xml_attributes][key::dtd].type (key::xml_attribute);
	schema[-1][-1][-1].setattrib (key::mandatory,true);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][-1][key::xml_code] = "XODd";
	
	// -------------------------------------------------------------------
	// class xml.class
	// -------------------------------------------------------------------
	schema[key::xml_class].type (key::xml_class);
	schema[-1][key::xml_type] = "dict";
	schema[-1][key::xml_code] = "XCla";
	
	schema[-1][key::xml_attributes]["name"].type (key::xml_attribute);
	schema[-1][-1][-1].setattrib (key::mandatory,true);
	schema[-1][-1][-1].setattrib (key::isindex,true);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][-1][key::xml_code] = "XCna";
	schema[-1][-1][-1][key::xml_validate].newval (key::xml_regexp) =
		"[[:alpha:]]?*(\\.[[:alpha:]]?*)*";

	schema[-1][-1][key::contained].type (key::xml_attribute);
	schema[-1][-1][-1][key::xml_type] = "bool";
	schema[-1][-1][-1][key::xml_code] = "XCco";
	
	schema[-1][-1][key::wrap].type (key::xml_attribute);
	schema[-1][-1][-1][key::xml_type] = "bool";
	schema[-1][-1][-1][key::xml_code] = "XCwr";
	
	schema[-1][-1][key::attribvalue].type (key::xml_attribute);
	schema[-1][-1][-1][key::xml_type] = "bool";
	schema[-1][-1][-1][key::xml_code] = "XCav";
	
	schema[-1][-1][key::cunion].type (key::xml_attribute);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][-1][key::xml_code] = "XCun";

	schema[-1][-1][key::array].type (key::xml_attribute);
	schema[-1][-1][-1][key::xml_type] = "bool";
	schema[-1][-1][-1][key::xml_code] = "XCar";
	
	schema[-1][key::xml_proplist][key::xml_type].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id,key::xml_type.sval());
	schema[-1][-1][-1].setattrib (key::mandatory,true);

	schema[-1][-1][key::xml_code].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id,key::xml_code.sval());
	schema[-1][-1][-1].setattrib (key::mandatory,true);
	
	schema[-1][-1][key::xml_encoding].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id,key::xml_encoding.sval());
	
	schema[-1][-1][key::xml_validate].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id,key::xml_validate.sval());
	
	schema[-1][-1][key::xml_attributes].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id,key::xml_attributes.sval());
	
	schema[-1][-1][key::xml_proplist].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id,key::xml_proplist.sval());
	
	schema[-1][-1][key::xml_container].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id,key::xml_container.sval());
	
	schema[-1][-1][key::xml_union].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id, key::xml_union.sval());
	
	// -------------------------------------------------------------------
	// class xml.type
	// -------------------------------------------------------------------
	schema[key::xml_type].type (key::xml_class);
	schema[-1][key::xml_type] = "string";
	schema[-1][key::xml_code] = "XTyp";
	schema[-1][key::xml_validate].newval (key::xml_string) = "dict";
	schema[-1][-1].newval (key::xml_string) = "string";
	schema[-1][-1].newval (key::xml_string) = "integer";
	schema[-1][-1].newval (key::xml_string) = "bool";
	schema[-1][-1].newval (key::xml_string) = "char";
	schema[-1][-1].newval (key::xml_string) = "short";
	schema[-1][-1].newval (key::xml_string) = "long";
	schema[-1][-1].newval (key::xml_string) = "unsigned";
	schema[-1][-1].newval (key::xml_string) = "uchar";
	schema[-1][-1].newval (key::xml_string) = "ushort";
	schema[-1][-1].newval (key::xml_string) = "ulong";
	schema[-1][-1].newval (key::xml_string) = "ipaddr";
	schema[-1][-1].newval (key::xml_string) = "date";
	schema[-1][-1].newval (key::xml_string) = "currency";
	
	// -------------------------------------------------------------------
	// class xml.code
	// -------------------------------------------------------------------
	schema[key::xml_code].type (key::xml_class);
	schema[-1][key::xml_type] = "string";
	schema[-1][key::xml_code] = "XCod";
	schema[-1][key::xml_validate].newval (key::xml_regexp) =
		"[[:alphanum:]]???";

	// -------------------------------------------------------------------
	// class xml.validate
	// -------------------------------------------------------------------
	schema[key::xml_validate].type (key::xml_class);	
	schema[-1][key::xml_type] = "dict";
	schema[-1][key::xml_code] = "XVal";
	schema[-1][key::xml_attributes][key::strict].type (key::xml_attribute);
	schema[-1][-1][-1][key::xml_type] = "bool";
	schema[-1][-1][-1][key::xml_code] = "XVst";
	schema[-1][key::xml_proplist][key::xml_string].type (key::xml_member);
	schema[-1][-1][key::xml_regexp].type (key::xml_member);
	
	// -------------------------------------------------------------------
	// class xml.string
	// -------------------------------------------------------------------
	schema[key::xml_string].type (key::xml_class);
	schema[-1][key::xml_type] = "string";
	schema[-1][key::xml_code] = "Xstr";
	
	// -------------------------------------------------------------------
	// class xml.regexp
	// -------------------------------------------------------------------
	schema[key::xml_regexp].type (key::xml_class);
	schema[-1][key::xml_type] = "string";
	schema[-1][key::xml_code] = "Xrex";

	// -------------------------------------------------------------------
	// class xml.attributes
	// -------------------------------------------------------------------
	schema[key::xml_attributes].type (key::xml_class);	
	schema[-1][key::xml_type] = "dict";
	schema[-1][key::xml_code] = "XAtl";
	schema[-1][key::xml_proplist]
			  [key::xml_attribute].newval (key::xml_member);
	
	// -------------------------------------------------------------------
	// class xml.attribute
	// -------------------------------------------------------------------
	schema[key::xml_attribute].type (key::xml_class);
	schema[-1][key::xml_type] = "dict";
	schema[-1][key::xml_code] = "XAtr";
	
	schema[-1][key::xml_attributes][key::label].type (key::xml_attribute);
	schema[-1][-1][-1].setattrib (key::mandatory,true);
	schema[-1][-1][-1].setattrib (key::isindex,true);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][-1][key::xml_code] = "XAlb";

	schema[-1][-1][key::mandatory].type (key::xml_attribute);
	schema[-1][-1][-1][key::xml_type] = "bool";
	schema[-1][-1][-1][key::xml_code] = "XAmd";

	schema[-1][-1][key::isindex].type (key::xml_attribute);
	schema[-1][-1][-1][key::xml_type] = "bool";
	schema[-1][-1][-1][key::xml_code] = "XAii";

	schema[-1][key::xml_proplist][key::xml_type].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id,key::xml_type.sval());
	schema[-1][-1][-1].setattrib (key::mandatory,true);

	schema[-1][key::xml_proplist][key::xml_code].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id,key::xml_code.sval());
	schema[-1][-1][-1].setattrib (key::mandatory,true);

	schema[-1][key::xml_proplist][key::xml_validate].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id,key::xml_validate.sval());

	// -------------------------------------------------------------------
	// class xml.proplist
	// -------------------------------------------------------------------
	schema[key::xml_proplist].type (key::xml_class);
	schema[-1][key::xml_type] = "dict";
	schema[-1][key::xml_code] = "XPls";
	schema[-1][key::xml_attributes][key::strict][key::xml_type] = "bool";
	schema[-1][-1][-1][key::xml_code] = "XPst";
	schema[-1][key::xml_proplist][key::xml_member].type (key::xml_member);
	
	// -------------------------------------------------------------------
	// class xml.member
	// -------------------------------------------------------------------
	schema[key::xml_member].type (key::xml_class);
	schema[-1][key::xml_type] = "dict";
	schema[-1][key::xml_code] = "XMem";
	schema[-1][key::xml_attributes][key::klass].type (key::xml_attribute);
	schema[-1][-1][-1].setattrib (key::mandatory,true);
	schema[-1][-1][-1].setattrib (key::isindex,true);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][-1][key::xml_code] = "XMcl";
	schema[-1][-1][key::id].type (key::xml_attribute);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][-1][key::xml_code] = "XMid";
	schema[-1][-1][key::type].type (key::xml_attribute);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][-1][key::xml_code] = "XMtp";
	schema[-1][-1][key::mandatory].type (key::xml_attribute);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][-1][key::xml_code] = "XMma";
	
	// -------------------------------------------------------------------
	// class xml.container
	// -------------------------------------------------------------------
	schema[key::xml_container].type (key::xml_class);
	schema[-1][key::xml_type] = "dict";
	schema[-1][key::xml_code] = "XCon";
	schema[-1][key::xml_proplist][key::xml_container_valueclass].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id,key::xml_container_valueclass.sval());
	schema[-1][key::xml_proplist][key::xml_container_wrapclass].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id,key::xml_container_wrapclass.sval());
	schema[-1][key::xml_proplist][key::xml_container_envelope].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id,key::xml_container_envelope.sval());
	schema[-1][-1][key::xml_container_idclass].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id,key::xml_container_idclass.sval());
	schema[-1][-1][key::xml_container_types].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id,key::xml_container_types.sval());
	
	schema[key::xml_container_valueclass].type (key::xml_class);
	schema[-1][key::xml_type] = "string";
	schema[-1][key::xml_code] = "XCvc";
	
	schema[key::xml_container_idclass].type (key::xml_class);
	schema[-1][key::xml_type] = "string";
	schema[-1][key::xml_code] = "XCic";

	schema[key::xml_container_envelope].type (key::xml_class);
	schema[-1][key::xml_type] = "string";
	schema[-1][key::xml_code] = "XCev";

	schema[key::xml_container_wrapclass].type (key::xml_class);
	schema[-1][key::xml_type] = "string";
	schema[-1][key::xml_code] = "XCwc";
	
	schema[key::xml_container_types].type (key::xml_class);
	schema[-1][key::xml_type] = "dict";
	schema[-1][key::xml_code] = "XCcT";
	schema[-1][key::xml_proplist][key::xml_container_type].type (key::xml_member);
	
	schema[key::xml_container_type].type (key::xml_class);
	schema[-1][key::xml_type] = "string";
	schema[-1][key::xml_code] = "XCct";
	schema[-1][key::xml_attributes][key::id].type (key::xml_attribute);
	schema[-1][-1][-1].setattrib (key::mandatory, true);
	schema[-1][-1][-1].setattrib (key::isindex, true);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][-1][key::xml_code] = "XctI";

	// -------------------------------------------------------------------
	// class xml.union
	// -------------------------------------------------------------------
	schema[key::xml_union].type (key::xml_class);	
	schema[-1][key::xml_type] = "dict";
	schema[-1][key::xml_code] = "XAun";
	schema[-1][key::xml_proplist]
			  [key::xml_union_match].newval (key::xml_member);
	
	schema[key::xml_union_match].type (key::xml_class);
	schema[-1][key::xml_type] = "string";
	schema[-1][key::xml_code] = "XAum";
	
	schema[-1][key::xml_attributes][key::klass].type (key::xml_attribute);
	schema[-1][-1][-1].setattrib (key::mandatory, true);
	schema[-1][-1][-1].setattrib (key::isindex, true);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][-1][key::xml_code] = "XUcl";

	schema[-1][key::xml_attributes][key::type].type (key::xml_attribute);
	schema[-1][-1][-1].setattrib (key::mandatory, true);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][-1][key::xml_code] = "XUtp";

	schema[-1][key::xml_attributes][key::label].type (key::xml_attribute);
	schema[-1][-1][-1].setattrib (key::mandatory, true);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][-1][key::xml_code] = "XUlb";
}

