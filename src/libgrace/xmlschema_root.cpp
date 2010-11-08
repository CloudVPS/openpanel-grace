// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

// ========================================================================
// xmlschema_def.cpp: XML schema definition utility class
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
			
		case XMLPlistSchemaType:
			plistschema();
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
	schema =	$type("xml.schema") ->
				$("xml.schema",
					$type("xml.class") ->
					$("xml.type","dict") ->
					$("xml.code","XSch") ->
					$("xml.proplist",
						$("xml.class",
							$type("xml.member")) ->
						$("xml.schema.options",
							$type("xml.member") ->
							$attr("id",".options")))) ->
				$("xml.schema.options",
					$type("xml.class") ->
					$("xml.type","dict") ->
					$("xml.code","XOpt") ->
					$("xml.proplist",
						$("xml.option.namespaces",
							$type("xml.member") ->
							$attr("id","namespaces")) ->
						$("xml.option.rootclass",
							$type("xml.member") ->
							$attr("id","rootclass")) ->
						$("xml.option.doctype",
							$type("xml.member") ->
							$attr("id","doctype"))->
						$("xml.option.defaulttagkey",
							$type("xml.member") ->
							$attr("id","tagkey")))) ->
				$("xml.option.rootclass",
					$type("xml.class") ->
					$("xml.type","string") ->
					$("xml.code","XOrc")) ->
				$("xml.option.namespaces",
					$type("xml.class") ->
					$("xml.type","dict") ->
					$("xml.code","XOns") ->
					$("xml.proplist",
						$("xml.namespace",
							$type("xml.member")))) ->
				$("xml.namespace",
					$type("xml.class") ->
					$("xml.type","dict") ->
					$("xml.code","XMns") ->
					$("xml.attributes",
						$("uri",
							$type("xml.attribute") ->
							$attr("mandatory","true") ->
							$attr("isindex","true") ->
							$("xml.type","string") ->
							$("xml.code","NSur")) ->
						$("action",
							$type("xml.attribute") ->
							$attr("mandatory","true") ->
							$("xml.type","string") ->
							$("xml.code","NSax")) ->
						$("prefix",
							$type("xml.attribute") ->
							$("xml.type","string") ->
							$("xml.code","NSpx"))) ->
					$("xml.proplist",
						$("xml.namespace.type",
							$type("xml.attribute")))) ->
				$("xml.namespace.type",
					$type("xml.class") ->
					$("xml.type","void") ->
					$("xml.code","XnsT") ->
					$("xml.attributes",
						$("class",
							$type("xml.attribute") ->
							$attr("isindex","true") ->
							$attr("mandatory","true") ->
							$("xml.type","string") ->
							$("xml.code","nsTC")) ->
						$("alias",
							$type("xml.attribute") ->
							$("xml.type","string") ->
							$("xml.code","nsTA")))) ->
				$("xml.option.doctype",
					$type("xml.class") ->
					$("xml.type","string") ->
					$("xml.code","XOdt") ->
					$("xml.attributes",
						$("name",
							$type("xml.attribute") ->
							$attr("mandatory","true") ->
							$("xml.type","string") ->
							$("xml.code","XODn")) ->
						$("status",
							$type("xml.attribute") ->
							$attr("mandatory","true") ->
							$("xml.type","string") ->
							$("xml.code","XODs")) ->
						$("dtd",
							$type("xml.attribute") ->
							$attr("mandatory","true") ->
							$("xml.type","string") ->
							$("xml.code","XODd")))) ->
				$("xml.option.defaulttagkey",
					$type("xml.class") ->
					$("xml.type","bool") ->
					$("xml.code","XOtk")) ->
				$("xml.class",
					$type("xml.class") ->
					$("xml.type","dict") ->
					$("xml.code","XCla") ->
					$("xml.attributes",
						$("name",
							$type("xml.attribute") ->
							$attr("mandatory","true") ->
							$attr("isindex","true") ->
							$("xml.type","string") ->
							$("xml.code","XCna") ->
							$("xml.validate",
								$(
									$type("xml.regexp") ->
									$val("[[:alpha:]]?*(\\.[[:alpha:]]?*)*")))) ->
						$("contained",
							$type("xml.attribute") ->
							$("xml.type","bool") ->
							$("xml.code","XCco")) ->
						$("wrap",
							$type("xml.attribute") ->
							$("xml.type","bool") ->
							$("xml.code","XCwr")) ->
						$("attribvalue",
							$type("xml.attribute") ->
							$("xml.type","bool") ->
							$("xml.code","XCav")) ->
						$("union",
							$type("xml.attribute") ->
							$("xml.type","string") ->
							$("xml.code","XCun")) ->
						$("array",
							$type("xml.attribute") ->
							$("xml.type","bool") ->
							$("xml.code","XCar"))) ->
					$("xml.proplist",
						$("xml.type",
							$type("xml.member") ->
							$attr("id","xml.type") ->
							$attr("mandatory","true")) ->
						$("xml.code",
							$type("xml.member") ->
							$attr("id","xml.code") ->
							$attr("mandatory","true")) ->
						$("xml.encoding",
							$type("xml.member") ->
							$attr("id","xml.encoding")) ->
						$("xml.validate",
							$type("xml.member") ->
							$attr("id","xml.validate")) ->
						$("xml.attributes",
							$type("xml.member") ->
							$attr("id","xml.attributes")) ->
						$("xml.proplist",
							$type("xml.member") ->
							$attr("id","xml.proplist")) ->
						$("xml.container",
							$type("xml.member") ->
							$attr("id","xml.container")) ->
						$("xml.union",
							$type("xml.member") ->
							$attr("id","xml.union")))) ->
				$("xml.type",
					$type("xml.class") ->
					$("xml.type","string") ->
					$("xml.code","XTyp") ->
					$("xml.validate",
						$(
							$type("xml.string") ->
							$val("dict")) ->
						$(
							$type("xml.string") ->
							$val("string")) ->
						$(
							$type("xml.string") ->
							$val("integer")) ->
						$(
							$type("xml.string") ->
							$val("bool")) ->
						$(
							$type("xml.string") ->
							$val("char")) ->
						$(
							$type("xml.string") ->
							$val("short")) ->
						$(
							$type("xml.string") ->
							$val("long")) ->
						$(
							$type("xml.string") ->
							$val("unsigned")) ->
						$(
							$type("xml.string") ->
							$val("uchar")) ->
						$(
							$type("xml.string") ->
							$val("ushort")) ->
						$(
							$type("xml.string") ->
							$val("ulong")) ->
						$(
							$type("xml.string") ->
							$val("ipaddr")) ->
						$(
							$type("xml.string") ->
							$val("date")) ->
						$(
							$type("xml.string") ->
							$val("currency")))) ->
				$("xml.code",
					$type("xml.class") ->
					$("xml.type","string") ->
					$("xml.code","XCod") ->
					$("xml.validate",
						$(
							$type("xml.regexp") ->
							$val("[[:alphanum:]]???")))) ->
				$("xml.validate",
					$type("xml.class") ->
					$("xml.type","dict") ->
					$("xml.code","XVal") ->
					$("xml.attributes",
						$("strict",
							$type("xml.attribute") ->
							$("xml.type","bool") ->
							$("xml.code","XVst"))) ->
					$("xml.proplist",
						$("xml.string",
							$type("xml.member")) ->
						$("xml.regexp",
							$type("xml.member")))) ->
				$("xml.string",
					$type("xml.class") ->
					$("xml.type","string") ->
					$("xml.code","Xstr")) ->
				$("xml.regexp",
					$type("xml.class") ->
					$("xml.type","string") ->
					$("xml.code","Xrex")) ->
				$("xml.attributes",
					$type("xml.class") ->
					$("xml.type","dict") ->
					$("xml.code","XAtl") ->
					$("xml.proplist",
						$("xml.attribute",$($type("xml.member"))))) ->
				$("xml.attribute",
					$type("xml.class") ->
					$("xml.type","dict") ->
					$("xml.code","XAtr") ->
					$("xml.attributes",
						$("label",
							$type("xml.attribute") ->
							$attr("mandatory","true") ->
							$attr("isindex","true") ->
							$("xml.type","string") ->
							$("xml.code","XAlb")) ->
						$("mandatory",
							$type("xml.attribute") ->
							$("xml.type","bool") ->
							$("xml.code","XAmd")) ->
						$("isindex",
							$type("xml.attribute") ->
							$("xml.type","bool") ->
							$("xml.code","XAii"))) ->
					$("xml.proplist",
						$("xml.type",
							$type("xml.member") ->
							$attr("id","xml.type") ->
							$attr("mandatory","true")) ->
						$("xml.code",
							$type("xml.member") ->
							$attr("id","xml.code") ->
							$attr("mandatory","true")) ->
						$("xml.validate",
							$type("xml.member") ->
							$attr("id","xml.validate")))) ->
				$("xml.proplist",
					$type("xml.class") ->
					$("xml.type","dict") ->
					$("xml.code","XPls") ->
					$("xml.attributes",
						$("strict",
							$("xml.type","bool") ->
							$("xml.code","XPst"))) ->
					$("xml.proplist",
						$("xml.member",
							$type("xml.member")))) ->
				$("xml.member",
					$type("xml.class") ->
					$("xml.type","dict") ->
					$("xml.code","XMem") ->
					$("xml.attributes",
						$("class",
							$type("xml.attribute") ->
							$attr("mandatory","true") ->
							$attr("isindex","true") ->
							$("xml.type","string") ->
							$("xml.code","XMcl")) ->
						$("id",
							$type("xml.attribute") ->
							$("xml.type","string") ->
							$("xml.code","XMid")) ->
						$("type",
							$type("xml.attribute") ->
							$("xml.type","string") ->
							$("xml.code","XMtp")) ->
						$("mandatory",
							$type("xml.attribute") ->
							$("xml.type","string") ->
							$("xml.code","XMma")))) ->
				$("xml.container",
					$type("xml.class") ->
					$("xml.type","dict") ->
					$("xml.code","XCon") ->
					$("xml.proplist",
						$("xml.container.valueclass",
							$type("xml.member") ->
							$attr("id","xml.container.valueclass")) ->
						$("xml.container.wrapclass",
							$type("xml.member") ->
							$attr("id","xml.container.wrapclass")) ->
						$("xml.container.envelope",
							$type("xml.member") ->
							$attr("id","xml.container.envelope")) ->
						$("xml.container.idclass",
							$type("xml.member") ->
							$attr("id","xml.container.idclass")) ->
						$("xml.container.types",
							$type("xml.member") ->
							$attr("id","xml.container.types")))) ->
				$("xml.container.valueclass",
					$type("xml.class") ->
					$("xml.type","string") ->
					$("xml.code","XCvc")) ->
				$("xml.container.idclass",
					$type("xml.class") ->
					$("xml.type","string") ->
					$("xml.code","XCic")) ->
				$("xml.container.envelope",
					$type("xml.class") ->
					$("xml.type","string") ->
					$("xml.code","XCev")) ->
				$("xml.container.wrapclass",
					$type("xml.class") ->
					$("xml.type","string") ->
					$("xml.code","XCwc")) ->
				$("xml.container.types",
					$type("xml.class") ->
					$("xml.type","dict") ->
					$("xml.code","XCcT") ->
					$("xml.proplist",
						$("xml.container.type",
							$type("xml.member")))) ->
				$("xml.container.type",
					$type("xml.class") ->
					$("xml.type","string") ->
					$("xml.code","XCct") ->
					$("xml.attributes",
						$("id",
							$type("xml.attribute") ->
							$attr("mandatory","true") ->
							$attr("isindex","true") ->
							$("xml.type","string") ->
							$("xml.code","XctI")))) ->
				$("xml.union",
					$type("xml.class") ->
					$("xml.type","dict") ->
					$("xml.code","XAun") ->
					$("xml.proplist",
						$("xml.union.match",
							$type("xml.member")))) ->
				$("xml.union.match",
					$type("xml.class") ->
					$("xml.type","string") ->
					$("xml.code","XAum") ->
					$("xml.attributes",
						$("class",
							$type("xml.attribute") ->
							$attr("mandatory","true") ->
							$attr("isindex","true") ->
							$("xml.type","string") ->
							$("xml.code","XUcl")) ->
						$("type",
							$type("xml.attribute") ->
							$attr("mandatory","true") ->
							$("xml.type","string") ->
							$("xml.code","XUtp")) ->
						$("label",
							$type("xml.attribute") ->
							$attr("mandatory","true") ->
							$("xml.type","string") ->
							$("xml.code","XUlb"))));
	
}

