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
// METHOD ::netdbschema
// --------------------
// Creates the schema for returns of the netdb class
// ========================================================================
void xmlschema::netdbschema (void)
{
	schema =	$type("xml.schema") ->
				$("grace.netdb.host",
					$type("xml.class") ->
					$("xml.type","dict") ->
					$("xml.code","ndbH") ->
					$("xml.proplist",
						$("host.name",
							$type("xml.member") ->
							$attr("id","name")) ->
						$("host.address",
							$type("xml.member") ->
							$attr("id","address")))) ->
				$("host.name",
					$type("xml.class") ->
					$("xml.type","array") ->
					$("xml.code","ndHN") ->
					$("xml.proplist",
						$("hostname",
							$type("xml.member")))) ->
				$("hostname",
					$type("xml.class") ->
					$("xml.type","string") ->
					$("xml.code","ndHn")) ->
				$("host.address",
					$type("xml.class") ->
					$("xml.type","array") ->
					$("xml.code","ndAD") ->
					$("xml.proplist",
						$("address",
							$type("xml.member")))) ->
				$("address",
					$type("xml.class") ->
					$("xml.type","ipaddress") ->
					$("xml.code","ndAd"));
}

// ========================================================================
// METHOD ::runoptschema
// --------------------
// Creates the schema for the grace.runoptions.xml file used by the
// application class to determine commandline arguments.
// ========================================================================
void xmlschema::runoptschema (void)
{
	schema =	$type("xml.schema") ->
				$("grace.runoptions",
					$type("xml.class") ->
					$("xml.type","dict") ->
					$("xml.code","OPTV") ->
					$("xml.proplist",
						$("grace.option",
							$type("xml.member")))) ->
				$("grace.option",
					$type("xml.class") ->
					$("xml.type","dict") ->
					$("xml.code","OPTN") ->
					$("id",
						$type("xml.attribute") ->
						$attr("mandatory","true") ->
						$attr("isindex","true") ->
						$("xml.type","string") ->
						$("xml.code","OPTX")) ->
					$("xml.proplist",
						$("grace.long",
							$type("xml.member") ->
							$attr("id","long")) ->
						$("grace.argc",
							$type("xml.member") ->
							$attr("id","argc")) ->
						$("grace.default",
							$type("xml.member") ->
							$attr("id","default")) ->
						$("grace.help",
							$type("xml.member") ->
							$attr("id","help")) ->
						$("grace.hide",
							$type("xml.member") ->
							$attr("id","hide")))) ->
				$("grace.long",
					$type("xml.class") ->
					$("xml.type","string") ->
					$("xml.code","OLNG")) ->
				$("grace.argc",
					$type("xml.class") ->
					$("xml.type","integer") ->
					$("xml.code","OARC")) ->
				$("grace.default",
					$type("xml.class") ->
					$("xml.type","string") ->
					$("xml.code","ODFL")) ->
				$("grace.help",
					$type("xml.class") ->
					$("xml.type","string") ->
					$("xml.code","OHLP")) ->
				$("grace.hide",
					$type("xml.class") ->
					$("xml.type","bool") ->
					$("xml.code","OHID"));
}

// ========================================================================
// METHOD ::runoptschema
// --------------------
// Creates the schema for the <grace.validator> data validation system.
// ========================================================================
void xmlschema::validatorschema (void)
{
	schema =	$type("xml.schema") ->
				$("grace.validator",
					$type("xml.class") ->
					$("xml.type","dict") ->
					$("xml.proplist",
						$("datarule",
							$type("xml.member")))) ->
				$("datarule",
					$type("xml.class") ->
					$("xml.type","array") ->
					$("xml.attributes",
						$("id",
							$type("xml.attribute") ->
							$attr("mandatory","true") ->
							$attr("isindex","true"))) ->
					$("xml.proplist",
						$("match.mandatory",$type("xml.member")) ->
						$("and",$type("xml.member")) ->
						$("or",$type("xml.member")) ->
						$("match.child",$type("xml.member")) ->
						$("match.id",$type("xml.member")) ->
						$("match.class",$type("xml.member")) ->
						$("match.type",$type("xml.member")) ->
						$("match.data",$type("xml.member")) ->
						$("match.attribute",$type("xml.member")) ->
						$("match.hasindex",$type("xml.member")) ->
						$("match.rule",$type("xml.member")))) ->
				$("match.mandatory",
					$type("xml.class") ->
					$("xml.type","array") ->
					$("xml.proplist",
						$("optional",$type("xml.member")) ->
						$("mandatory",$type("xml.member")) ->
						$("or",$type("xml.member")))) ->
				$("optional",
					$type("xml.class") ->
					$("xml.type","array") ->
					$("xml.attributes",
						$("type",
							$type("xml.attribute") ->
							$attr("mandatory","true") ->
							$("xml.type","string")) ->
						$("key",
							$type("xml.attribute") ->
							$attr("mandatory","true") ->
							$("xml.type","string"))) ->
					$("xml.proplist",
						$("mandatory",$type("xml.member")))) ->
				$("mandatory",
					$type("xml.class") ->
					$("xml.type","void") ->
					$("xml.attributes",
						$("type",
							$type("xml.attribute") ->
							$attr("mandatory","true") ->
							$("xml.type","string")) ->
						$("key",
							$type("xml.attribute") ->
							$("xml.type","string")) ->
						$("errorcode",
							$type("xml.attribute") ->
							$("xml.type","string")) ->
						$("errortext",
							$type("xml.attribute") ->
							$("xml.type","string")))) ->
				$("and",
					$type("xml.class") ->
					$("xml.type","array") ->
					$("xml.attributes",
						$("id",
							$type("xml.attribute") ->
							$attr("mandatory","true") ->
							$attr("isindex","true"))) ->
					$("xml.proplist",
						$("and",$type("xml.member")) ->
						$("or",$type("xml.member")) ->
						$("match.child",$type("xml.member")) ->
						$("match.id",$type("xml.member")) ->
						$("match.class",$type("xml.member")) ->
						$("match.type",$type("xml.member")) ->
						$("match.data",$type("xml.member")) ->
						$("match.attribute",$type("xml.member")) ->
						$("match.hasindex",$type("xml.member")) ->
						$("match.rule",$type("xml.member")))) ->
				$("or",
					$type("xml.class") ->
					$("xml.type","array") ->
					$("xml.attributes",
						$("id",
							$type("xml.attribute") ->
							$attr("mandatory","true") ->
							$attr("isindex","true"))) ->
					$("xml.proplist",
						$("and",$type("xml.member")) ->
						$("or",$type("xml.member")) ->
						$("match.child",$type("xml.member")) ->
						$("match.id",$type("xml.member")) ->
						$("mandatory",$type("xml.member")) ->
						$("optional",$type("xml.member")) ->
						$("match.class",$type("xml.member")) ->
						$("match.type",$type("xml.member")) ->
						$("match.data",$type("xml.member")) ->
						$("match.attribute",$type("xml.member")) ->
						$("match.hasindex",$type("xml.member")) ->
						$("match.rule",$type("xml.member")))) ->
				$("match.child",
					$type("xml.class") ->
					$("xml.type","array") ->
					$("xml.attributes",
						$("id",
							$type("xml.attribute") ->
							$attr("mandatory","true") ->
							$attr("isindex","true")) ->
						$("errorcode",
							$type("xml.attribute") ->
							$("xml.type","string")) ->
						$("errortext",
							$type("xml.attribute") ->
							$("xml.type","string"))) ->
					$("xml.proplist",
						$("and",$type("xml.member")) ->
						$("or",$type("xml.member")) ->
						$("match.child",$type("xml.member")) ->
						$("match.id",$type("xml.member")) ->
						$("match.class",$type("xml.member")) ->
						$("match.type",$type("xml.member")) ->
						$("match.data",$type("xml.member")) ->
						$("match.attribute",$type("xml.member")) ->
						$("match.hasindex",$type("xml.member")) ->
						$("match.rule",$type("xml.member")))) ->
				$("match.attribute",
					$type("xml.class") ->
					$("xml.type","array") ->
					$("xml.attributes",
						$("id",
							$type("xml.attribute") ->
							$attr("mandatory","true") ->
							$attr("isindex","true")) ->
						$("errorcode",
							$type("xml.attribute") ->
							$("xml.type","string")) ->
						$("errortext",
							$type("xml.attribute") ->
							$("xml.type","string"))) ->
					$("xml.proplist",
						$("and",$type("xml.member")) ->
						$("or",$type("xml.member")) ->
						$("match.id",$type("xml.member")) ->
						$("match.class",$type("xml.member")) ->
						$("match.type",$type("xml.member")) ->
						$("match.data",$type("xml.member")))) ->
				$("match.id",
					$type("xml.class") ->
					$("xml.type","string")) ->
				$("match.class",
					$type("xml.class") ->
					$("xml.type","string")) ->
				$("match.type",
					$type("xml.class") ->
					$("xml.type","string")) ->
				$("match.data",
					$type("xml.class") ->
					$("xml.type","array") ->
					$("xml.proplist",
						$("text",$type("xml.member")) ->
						$("regexp",$type("xml.member")) ->
						$("lt",$type("xml.member")) ->
						$("gt",$type("xml.member")) ->
						$("minsize",$type("xml.member")) ->
						$("maxsize",$type("xml.member")))) ->
				$("text",
					$type("xml.class") ->
					$("xml.type","string")) ->
				$("regexp",
					$type("xml.class") ->
					$("xml.type","string")) ->
				$("lt",
					$type("xml.class") ->
					$("xml.type","integer")) ->
				$("gt",
					$type("xml.class") ->
					$("xml.type","integer")) ->
				$("minsize",
					$type("xml.class") ->
					$("xml.type","integer")) ->
				$("maxsize",
					$type("xml.class") ->
					$("xml.type","integer")) ->
				$("match.hasindex",
					$type("xml.class") ->
					$("xml.type","void")) ->
				$("match.rule",
					$type("xml.class") ->
					$("xml.type","string"));

}

void xmlschema::plistschema (void)
{
	schema =	$type("xml.schema") ->
				$("plist",
					$type("xml.class") ->
					$("xml.type","container") ->
					$("xml.attributes",
						$("version",
							$type("xml.attribute") ->
							$("xml.type","string"))) ->
					$("xml.container",
						$("xml.container.idclass","key") ->
						$("xml.container.types",
							$("dict","dict")))) ->
				$("dict",
					$type("xml.class") ->
					$attr("contained","true") ->
					$("xml.type","container") ->
					$("xml.container",
						$("xml.container.idclass","key") ->
						$("xml.container.types",
							$("dict","dict") ->
							$("array","array") ->
							$("integer","integer") ->
							$("string","string") ->
							$("date","timestamp") ->
							$("float","real") ->
							$("bool.true","true") ->
							$("bool.flase","false")))) ->
				$("array",
					$type("xml.class") ->
					$attr("contained","true") ->
					$("xml.type","container") ->
					$("xml.container",
						$("xml.container.types",
							$("dict","dict") ->
							$("array","array") ->
							$("integer","integer") ->
							$("string","string") ->
							$("date","timestamp") ->
							$("float","real") ->
							$("bool.true","true") ->
							$("bool.flase","false")))) ->
				$("integer",
					$type("xml.class") ->
					$attr("contained","true") ->
					$("xml.type","integer")) ->
				$("string",
					$type("xml.class") ->
					$attr("contained","true") ->
					$("xml.type","string")) ->
				$("timestamp",
					$type("xml.class") ->
					$attr("contained","true") ->
					$("xml.type","date")) ->
				$("real",
					$type("xml.class") ->
					$attr("contained","true") ->
					$("xml.type","float")) ->
				$("true",
					$type("xml.class") ->
					$attr("contained","true") ->
					$("xml.type","bool.true")) ->
				$("false",
					$type("xml.class") ->
					$attr("contained","true") ->
					$("xml.type","bool.false"));
}
