// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _COMMONKEYS_H
#define _COMMONKEYS_H 1

#ifdef _KEYS_CPP
  #define STRINGRSRC statstring
  #define VALUE(x) (x)
  #undef _KEYS_CPP
#else
  #define STRINGRSRC extern const statstring
  #define VALUE(x) 
#endif

#include <grace/statstring.h>

namespace key {
	STRINGRSRC xml_schema					VALUE ("xml.schema");
	STRINGRSRC xml_type						VALUE ("xml.type");
	STRINGRSRC xml_code						VALUE ("xml.code");
	STRINGRSRC xml_encoding					VALUE ("xml.encoding");
	STRINGRSRC xml_proplist					VALUE ("xml.proplist");
	STRINGRSRC xml_member					VALUE ("xml.member");
	STRINGRSRC xml_class					VALUE ("xml.class");
	STRINGRSRC xml_attributes				VALUE ("xml.attributes");
	STRINGRSRC xml_attribute  				VALUE ("xml.attribute");
	STRINGRSRC xml_validate					VALUE ("xml.validate");
	STRINGRSRC xml_string					VALUE ("xml.string");
	STRINGRSRC xml_regexp					VALUE ("xml.regexp");
	STRINGRSRC xml_schema_options 			VALUE ("xml.schema.options");
	STRINGRSRC xml_option_rootclass 		VALUE ("xml.option.rootclass");
	STRINGRSRC xml_option_namespaces 		VALUE ("xml.option.namespaces");
	STRINGRSRC xml_option_doctype			VALUE ("xml.option.doctype");
	STRINGRSRC xml_namespace      			VALUE ("xml.namespace");
	STRINGRSRC xml_namespace_type 			VALUE ("xml.namespace.type");
	STRINGRSRC xml_union					VALUE ("xml.union");
	STRINGRSRC xml_union_match				VALUE ("xml.union.match");
	STRINGRSRC xml_container				VALUE ("xml.container");
	STRINGRSRC xml_container_envelope		VALUE ("xml.container.envelope");
	STRINGRSRC xml_container_valueclass		VALUE ("xml.container.valueclass");
	STRINGRSRC xml_container_wrapclass		VALUE ("xml.container.wrapclass");
	STRINGRSRC xml_container_idclass		VALUE ("xml.container.idclass");
	STRINGRSRC xml_container_types			VALUE ("xml.container.types");
	STRINGRSRC xml_container_type			VALUE ("xml.container.type");
	STRINGRSRC container					VALUE ("container");
	STRINGRSRC contained					VALUE ("contained");
	STRINGRSRC alias						VALUE ("alias");
	STRINGRSRC cunion						VALUE ("union");
	STRINGRSRC array						VALUE ("array");
	STRINGRSRC wrap							VALUE ("wrap");
	STRINGRSRC attribvalue					VALUE ("attribvalue");
	STRINGRSRC prefix						VALUE ("prefix");
	STRINGRSRC action						VALUE ("action");
	STRINGRSRC name							VALUE ("name");
	STRINGRSRC status						VALUE ("status");
	STRINGRSRC dtd							VALUE ("dtd");
	STRINGRSRC label						VALUE ("label");
	STRINGRSRC isindex						VALUE ("isindex");
	STRINGRSRC id							VALUE ("id");
	STRINGRSRC uri    		           		VALUE ("uri");
	STRINGRSRC klass						VALUE ("class");
	STRINGRSRC mandatory					VALUE ("mandatory");
	STRINGRSRC type							VALUE ("type");
	STRINGRSRC isdict						VALUE ("isdict");
	STRINGRSRC isattribute					VALUE ("isattribute");
	STRINGRSRC implicit						VALUE ("implicit");
	STRINGRSRC isclass						VALUE ("isclass");
	STRINGRSRC isprecedent					VALUE ("isprecedent");
	STRINGRSRC ofclass						VALUE ("ofclass");
	STRINGRSRC strict						VALUE ("strict");
	STRINGRSRC grace_netdb_host				VALUE ("grace.netdb.host");
	STRINGRSRC host_name					VALUE ("host.name");
	STRINGRSRC host_address					VALUE ("host.address");
	STRINGRSRC hostname						VALUE ("hostname");
	STRINGRSRC address						VALUE ("address");
	
	STRINGRSRC grace_runoptions				VALUE ("grace.runoptions");
	STRINGRSRC grace_option					VALUE ("grace.option");
	STRINGRSRC grace_long					VALUE ("grace.long");
	STRINGRSRC grace_argc					VALUE ("grace.argc");
	STRINGRSRC grace_default				VALUE ("grace.default");
	STRINGRSRC grace_help					VALUE ("grace.help");
	STRINGRSRC grace_hide					VALUE ("grace.hide");
	STRINGRSRC g_long						VALUE ("long");
	STRINGRSRC g_argc						VALUE ("argc");
	STRINGRSRC g_default					VALUE ("default");
	STRINGRSRC g_help						VALUE ("help");
	
	STRINGRSRC grace_validator				VALUE ("grace.validator");
	STRINGRSRC datarule						VALUE ("datarule");
	STRINGRSRC land							VALUE ("and");
	STRINGRSRC lor							VALUE ("or");
	STRINGRSRC match_child					VALUE ("match.child");
	STRINGRSRC match_id						VALUE ("match.id");
	STRINGRSRC match_class					VALUE ("match.class");
	STRINGRSRC match_type					VALUE ("match.type");
	STRINGRSRC match_data					VALUE ("match.data");
	STRINGRSRC match_attribute				VALUE ("match.attribute");
	STRINGRSRC match_hasindex				VALUE ("match.hasindex");
	STRINGRSRC match_rule					VALUE ("match.rule");
	STRINGRSRC text							VALUE ("text");
	STRINGRSRC regexp						VALUE ("regexp");
	STRINGRSRC lt							VALUE ("lt");
	STRINGRSRC gt							VALUE ("gt");
	STRINGRSRC minsize						VALUE ("minsize");
	STRINGRSRC maxsize						VALUE ("maxsize");
	STRINGRSRC errorcode					VALUE ("errorcode");
	STRINGRSRC errortext					VALUE ("errortext");
	STRINGRSRC match_mandatory				VALUE ("match.mandatory");
	STRINGRSRC optional						VALUE ("optional");
	STRINGRSRC key							VALUE ("key");
	
	STRINGRSRC plist						VALUE ("plist");
	STRINGRSRC version						VALUE ("version");
	STRINGRSRC date							VALUE ("date");
	STRINGRSRC booltrue						VALUE ("bool.true");
	STRINGRSRC boolfalse					VALUE ("bool.flase");
	STRINGRSRC tfloat						VALUE ("float");
	STRINGRSRC tstring						VALUE ("string");
	STRINGRSRC timestamp					VALUE ("timestamp");
	STRINGRSRC real							VALUE ("real");
	STRINGRSRC dict							VALUE ("dict");
	STRINGRSRC integer						VALUE ("integer");
};

#undef STRINGRSRC
#undef VALUE

#endif
