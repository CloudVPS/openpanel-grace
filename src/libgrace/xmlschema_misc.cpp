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
// METHOD ::netdbschema
// --------------------
// Creates the schema for returns of the netdb class
// ========================================================================
void xmlschema::netdbschema (void)
{
	// -------------------------------------------------------------------
	//  <xml.class name="grace.netdb.host">
	//    <xml.type>dict</xml.type>
	//    <xml.code>ndbH</xml.code>
	//    <xml.proplist>
	//      <xml.member class="host.name" id="name"/>
	//      <xml.member class="host.address" id="address"/>
	//    </xml.proplist>
	//  </xml.class>
	// -------------------------------------------------------------------
	
	schema[key::grace_netdb_host].type (key::xml_class);
	schema[-1][key::xml_type] = "dict";
	schema[-1][key::xml_code] = "ndbH";
	
	schema[-1][key::xml_proplist][key::host_name].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id, "name");
	schema[-1][-1][key::host_address].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id, "address");
	
	// -------------------------------------------------------------------
	//  <xml.class name="host.name">
	//    <xml.type>array</xml.type>
	//    <xml.code>ndHN</xml.code>
	//    <xml.proplist>
	//      <xml.member class="hostname"/>
	//    </xml.proplist>
	//  </xml.class>
	// -------------------------------------------------------------------
	
	schema[key::host_name].type (key::xml_class);
	schema[-1][key::xml_type] = "array";
	schema[-1][key::xml_code] = "ndHN";
	
	schema[-1][key::xml_proplist][key::hostname].type (key::xml_member);
	
	// -------------------------------------------------------------------
	//  <xml.class name="hostname">
	//    <xml.type>string</xml.type><xml.code>ndHn</xml.code>
	//  </xml.class>
	// -------------------------------------------------------------------

	schema[key::hostname].type (key::xml_class);
	schema[-1][key::xml_type] = "string";
	schema[-1][key::xml_code] = "ndHn";

	// -------------------------------------------------------------------
	//  <xml.class name="host.address">
	//    <xml.type>array</xml.type>
	//    <xml.code>ndAD</xml.code>
	//    <xml.proplist>
	//      <xml.member class="address"/>
	//    </xml.proplist>
	//  </xml.class>
	// -------------------------------------------------------------------

	schema[key::host_address].type (key::xml_class);
	schema[-1][key::xml_type] = "array";
	schema[-1][key::xml_code] = "ndAD";
	schema[-1][key::xml_proplist][key::address].type (key::xml_member);
	
	// -------------------------------------------------------------------
	//  <xml.class name="address">
	//    <xml.type>ipaddress</xml.type><xml.code>ndAd</xml.code>
	//  </xml.class>
	// -------------------------------------------------------------------

	schema[key::address].type (key::xml_class);
	schema[-1][key::xml_type] = "ipaddress";
	schema[-1][key::xml_code] = "ndAd";
}

// ========================================================================
// METHOD ::runoptschema
// --------------------
// Creates the schema for the grace.runoptions.xml file used by the
// application class to determine commandline arguments.
// ========================================================================
void xmlschema::runoptschema (void)
{
	// -------------------------------------------------------------------
	//  <xml.class name="grace.runoptions">
	//    <xml.type>dict</xml.type>
	//    <xml.code>OPTV</xml.code>
	//    <xml.proplist>
	//      <xml.member class="grace.option"/>
	//    </xml.proplist>
	//  </xml.class>
	// -------------------------------------------------------------------

	schema[key::grace_runoptions].type (key::xml_class);
	schema[-1][key::xml_type] = "dict";
	schema[-1][key::xml_code] = "OPTV";
	schema[-1][key::xml_proplist][key::grace_option].type (key::xml_member);

	// -------------------------------------------------------------------
 	// <xml.class name="grace.option">
 	//   <xml.type>dict</xml.type>
 	//   <xml.code>OPTN</xml.code>
 	//   <xml.attributes>
 	//     <xml.attribute label="id" mandatory="true" isindex="true">
 	//       <xml.type>string</xml.type>
 	//       <xml.code>OPTX</xml.code>
 	//     </xml.attribute>
 	//   </xml.attributes>
 	//   <xml.proplist>
 	//     <xml.member class="grace.long" id="long"/>
 	//     <xml.member class="grace.argc" id="argc"/>
 	//     <xml.member class="grace.default" id="default"/>
 	//     <xml.member class="grace.help" id="help"/>
 	//   </xml.proplist>
 	// </xml.class>
	// -------------------------------------------------------------------

	schema[key::grace_option].type (key::xml_class);
	schema[-1][key::xml_type] = "dict";
	schema[-1][key::xml_code] = "OPTN";
	
	schema[-1][key::id].type (key::xml_attribute);
	schema[-1][-1].setattrib ("mandatory",true);
	schema[-1][-1].setattrib ("isindex",true);
	schema[-1][-1][key::xml_type] = "string";
	schema[-1][-1][key::xml_code] = "OPTX";
	
	schema[-1][key::xml_proplist][key::grace_long].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id,(const char *) "long");
	
	schema[-1][-1][key::grace_argc].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id,(const char *) "argc");
	
	schema[-1][-1][key::grace_default].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id,(const char *) "default");
	
	schema[-1][-1][key::grace_help].type (key::xml_member);
	schema[-1][-1][-1].setattrib (key::id,(const char *) "help");

	// -------------------------------------------------------------------
	//  <xml.class name="grace.long">
	//    <xml.type>string</xml.type>
	//    <xml.code>OLNG</xml.code>
	//  </xml.class>
	// -------------------------------------------------------------------
  
	schema[key::grace_long].type (key::xml_class);
	schema[-1][key::xml_type] = "string";
	schema[-1][key::xml_code] = "OLNG";
	
	// -------------------------------------------------------------------
	//  <xml.class name="grace.argc">
	//    <xml.type>integer</xml.type>
	//    <xml.code>OARC</xml.code>
	//  </xml.class>
	// -------------------------------------------------------------------

	schema[key::grace_argc].type (key::xml_class);
	schema[-1][key::xml_type] = "integer";
	schema[-1][key::xml_code] = "OARC";

	// -------------------------------------------------------------------
	//  <xml.class name="grace.default">
	//    <xml.type>string</xml.type>
	//    <xml.code>ODFL</xml.code>
	//  </xml.class>
	// -------------------------------------------------------------------

	schema[key::grace_default].type (key::xml_class);
	schema[-1][key::xml_type] = "string";
	schema[-1][key::xml_code] = "ODFL";

	// -------------------------------------------------------------------
	//  <xml.class name="grace.help">
	//    <xml.type>string</xml.type>
	//    <xml.code>OHLP</xml.code>
	//  </xml.class>
	// -------------------------------------------------------------------

	schema[key::grace_help].type (key::xml_class);
	schema[-1][key::xml_type] = "string";
	schema[-1][key::xml_code] = "OHLP";
}

// ========================================================================
// METHOD ::runoptschema
// --------------------
// Creates the schema for the <grace.validator> data validation system.
// ========================================================================
void xmlschema::validatorschema (void)
{
	// -------------------------------------------------------------------
	// <xml.class name="grace.validator">
	//   <xml.type>dict</xml.type>
	//   <xml.proplist>
	//     <xml.member class="datarule"/>
	//   </xml.proplist>
	// </xml.class>
	// -------------------------------------------------------------------
	
	schema[key::grace_validator].type (key::xml_class);
	schema[-1][key::xml_type] = "dict";
	schema[-1][key::xml_proplist][key::datarule].type (key::xml_member);
	
	// -------------------------------------------------------------------
	//   <xml.class name="datarule">
	//     <xml.type>array</xml.type>
	//     <xml.attributes>
	//       <xml.attribute label="id" mandatory="true" isindex="true">
	//         <xml.type>string</xml.type>
	//       </xml.attribute>
	//     </xml.attributes>
	//     <xml.proplist>
	//       <xml.member class="match.mandatory"/>
	//       <xml.member class="and"/>
	//       <xml.member class="or"/>
	//       <xml.member class="match.child"/>
	//       <xml.member class="match.id"/>
	//       <xml.member class="match.class"/>
	//       <xml.member class="match.type"/>
	//       <xml.member class="match.data"/>
	//       <xml.member class="match.attribute"/>
	//       <xml.member class="match.hasindex"/>
	//       <xml.member class="match.rule"/>
	//     </xml.proplist>
	//   </xml.class>
	// -------------------------------------------------------------------

	schema[key::datarule].type (key::xml_class);
	schema[-1][key::xml_type] = "array";
	schema[-1][key::xml_attributes][key::id].type (key::xml_attribute);
	schema[-1][-1][-1](key::mandatory) = true;
	schema[-1][-1][-1](key::isindex) = true;
	
	schema[-1][key::xml_proplist][key::match_mandatory].type (key::xml_member);
	schema[-1][-1][key::land].type (key::xml_member);
	schema[-1][-1][key::lor].type (key::xml_member);
	schema[-1][-1][key::match_child].type (key::xml_member);
	schema[-1][-1][key::match_id].type (key::xml_member);
	schema[-1][-1][key::match_class].type (key::xml_member);
	schema[-1][-1][key::match_type].type (key::xml_member);
	schema[-1][-1][key::match_data].type (key::xml_member);
	schema[-1][-1][key::match_attribute].type (key::xml_member);
	schema[-1][-1][key::match_hasindex].type (key::xml_member);
	schema[-1][-1][key::match_rule].type (key::xml_member);

	// -------------------------------------------------------------------
	//   <xml.class name="match.mandatory">
	//     <xml.type>array</xml.type>
	//     <xml.proplist>
	//       <xml.member class="optional"/>
	//       <xml.member class="mandatory"/>
	//	     <xml.member class="or"/>
	//     </xml.proplist>
	//   </xml.class>
	// -------------------------------------------------------------------

	schema[key::match_mandatory].type (key::xml_class);
	schema[-1][key::xml_type] = "array";
	schema[-1][key::xml_proplist][key::optional].type (key::xml_member);
	schema[-1][-1][key::mandatory].type (key::xml_member);
	schema[-1][-1][key::lor].type (key::xml_member);

	// -------------------------------------------------------------------
	//   <xml.class name="optional">
	//     <xml.type>array</xml.type>
	//     <xml.attributes>
	//       <xml.attribute label="type" mandatory="true">
	//         <xml.type>string</xml.type>
	//       </xml.attribute>
	//       <xml.attribute label="key" mandatory="true">
	//         <xml.type>string</xml.type>
	//       </xml.attribute>
	//     </xml.attributes>
	//     <xml.proplist>
	//       <xml.member class="mandatory"/>
	//     </xml.proplist>
	//   </xml.class>
	// -------------------------------------------------------------------

	schema[key::optional].type (key::xml_class);
	schema[-1][key::xml_type] = "array";
	schema[-1][key::xml_attributes][key::type].type (key::xml_attribute);
	schema[-1][-1][-1](key::mandatory) = true;
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][key::key].type (key::xml_attribute);
	schema[-1][-1][-1](key::mandatory) = true;
	schema[-1][-1][-1][key::xml_type] = "string";

	schema[-1][key::xml_proplist][key::mandatory].type (key::xml_member);

	// -------------------------------------------------------------------
	//   <xml.class name="mandatory">
	//     <xml.type>void</xml.type>
	//     <xml.attributes>
	//       <xml.attribute label="type" mandatory="true">
	//         <xml.type>string</xml.type>
	//       </xml.attribute>
	//       <xml.attribute label="key" mandatory="true">
	//         <xml.type>string</xml.type>
	//       </xml.attribute>
	//       <xml.attribute label="errorcode"><xml.type>integer</xml.type></xml.attribute>
	//       <xml.attribute label="errortext"><xml.type>string</xml.type></xml.attribute>
	//     </xml.attributes>
	//   </xml.class>
	// -------------------------------------------------------------------
	
	schema[key::mandatory].type (key::xml_class);
	schema[-1][key::xml_type] = "void";
	schema[-1][key::xml_attributes][key::type].type (key::xml_attribute);
	schema[-1][-1][-1](key::mandatory) = true;
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][key::key].type (key::xml_attribute);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][key::errorcode].type (key::xml_attribute);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][key::errortext].type (key::xml_attribute);
	schema[-1][-1][-1][key::xml_type] = "string";
	
	// -------------------------------------------------------------------
	//   <xml.class name="and">
	//     <xml.type>array</xml.type>
	//     <xml.proplist>
	//       <xml.member class="and"/>
	//       <xml.member class="or"/>
	//       <xml.member class="match.child"/>
	//       <xml.member class="match.id"/>
	//       <xml.member class="match.class"/>
	//       <xml.member class="match.type"/>
	//       <xml.member class="match.data"/>
	//       <xml.member class="match.attribute"/>
	//       <xml.member class="match.hasindex"/>
	//       <xml.member class="match.rule"/>
	//     </xml.proplist>
	//   </xml.class>
	// -------------------------------------------------------------------

	schema[key::land].type (key::xml_class);
	schema[-1][key::xml_type] = "array";
	schema[-1][key::xml_attributes][key::id].type (key::xml_attribute);
	schema[-1][-1][-1](key::mandatory) = true;
	schema[-1][-1][-1](key::isindex) = true;
	
	schema[-1][key::xml_proplist][key::land].type (key::xml_member);
	schema[-1][-1][key::lor].type (key::xml_member);
	schema[-1][-1][key::match_child].type (key::xml_member);
	schema[-1][-1][key::match_id].type (key::xml_member);
	schema[-1][-1][key::match_class].type (key::xml_member);
	schema[-1][-1][key::match_type].type (key::xml_member);
	schema[-1][-1][key::match_data].type (key::xml_member);
	schema[-1][-1][key::match_attribute].type (key::xml_member);
	schema[-1][-1][key::match_hasindex].type (key::xml_member);
	schema[-1][-1][key::match_rule].type (key::xml_member);

	// -------------------------------------------------------------------
	//   <xml.class name="or">
	//     <xml.type>array</xml.type>
	//     <xml.proplist>
	//       <xml.member class="and"/>
	//       <xml.member class="or"/>
	//       <xml.member class="match.child"/>
	//       <xml.member class="match.id"/>
	//       <xml.member class="match.class"/>
	//       <xml.member class="match.type"/>
	//       <xml.member class="match.data"/>
	//       <xml.member class="match.attribute"/>
	//       <xml.member class="match.hasindex"/>
	//       <xml.member class="match.rule"/>
	//     </xml.proplist>
	//   </xml.class>
	// -------------------------------------------------------------------

	schema[key::lor].type (key::xml_class);
	schema[-1][key::xml_type] = "array";
	schema[-1][key::xml_attributes][key::id].type (key::xml_attribute);
	schema[-1][-1][-1](key::mandatory) = true;
	schema[-1][-1][-1](key::isindex) = true;
	
	schema[-1][key::xml_proplist][key::land].type (key::xml_member);
	schema[-1][-1][key::lor].type (key::xml_member);
	schema[-1][-1][key::match_child].type (key::xml_member);
	schema[-1][-1][key::match_id].type (key::xml_member);
	schema[-1][-1][key::mandatory].type (key::xml_member);
	schema[-1][-1][key::optional].type (key::xml_member);
	schema[-1][-1][key::match_class].type (key::xml_member);
	schema[-1][-1][key::match_type].type (key::xml_member);
	schema[-1][-1][key::match_data].type (key::xml_member);
	schema[-1][-1][key::match_attribute].type (key::xml_member);
	schema[-1][-1][key::match_hasindex].type (key::xml_member);
	schema[-1][-1][key::match_rule].type (key::xml_member);

	// -------------------------------------------------------------------
	//   <xml.class name="match.child">
	//     <xml.type>array</xml.type>
	//     <xml.attributes>
	//       <xml.attribute label="errorcode"><xml.type>integer</xml.type></xml.attribute>
	//       <xml.attribute label="errortext"><xml.type>string</xml.type></xml.attribute>
	//     </xml.attributes>
	//     <xml.proplist>
	//       <xml.member class="and"/>
	//       <xml.member class="or"/>
	//       <xml.member class="match.child"/>
	//       <xml.member class="match.id"/>
	//       <xml.member class="match.class"/>
	//       <xml.member class="match.type"/>
	//       <xml.member class="match.data"/>
	//       <xml.member class="match.attribute"/>
	//       <xml.member class="match.hasindex"/>
	//       <xml.member class="match.rule"/>
	//     </xml.proplist>
	//   </xml.class>
	// -------------------------------------------------------------------

	schema[key::match_child].type (key::xml_class);
	schema[-1][key::xml_type] = "array";
	schema[-1][key::xml_attributes][key::id].type (key::xml_attribute);
	schema[-1][-1][-1](key::mandatory) = true;
	schema[-1][-1][-1](key::isindex) = true;
	schema[-1][key::xml_attributes][key::errorcode].type (key::xml_attribute);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][key::errortext].type (key::xml_attribute);
	schema[-1][-1][-1][key::xml_type] = "string";
	
	schema[-1][key::xml_proplist][key::land].type (key::xml_member);
	schema[-1][-1][key::lor].type (key::xml_member);
	schema[-1][-1][key::match_child].type (key::xml_member);
	schema[-1][-1][key::match_id].type (key::xml_member);
	schema[-1][-1][key::match_class].type (key::xml_member);
	schema[-1][-1][key::match_type].type (key::xml_member);
	schema[-1][-1][key::match_data].type (key::xml_member);
	schema[-1][-1][key::match_attribute].type (key::xml_member);
	schema[-1][-1][key::match_hasindex].type (key::xml_member);
	schema[-1][-1][key::match_rule].type (key::xml_member);

	// -------------------------------------------------------------------
	//   <xml.class name="match.attribute">
	//     <xml.type>array</xml.type>
	//     <xml.attributes>
	//       <xml.attribute label="errorcode"><xml.type>integer</xml.type></xml.attribute>
	//       <xml.attribute label="errortext"><xml.type>string</xml.type></xml.attribute>
	//     </xml.attributes>
	//     <xml.proplist>
	//       <xml.member class="and"/>
	//       <xml.member class="or"/>
	//       <xml.member class="match.id"/>
	//       <xml.member class="match.class"/>
	//       <xml.member class="match.type"/>
	//       <xml.member class="match.data"/>
	//     </xml.proplist>
	//   </xml.class>
	// -------------------------------------------------------------------

	schema[key::match_attribute].type (key::xml_class);
	schema[-1][key::xml_type] = "array";
	schema[-1][key::xml_attributes][key::id].type (key::xml_attribute);
	schema[-1][-1][-1](key::mandatory) = true;
	schema[-1][-1][-1](key::isindex) = true;
	schema[-1][key::xml_attributes][key::errorcode].type (key::xml_attribute);
	schema[-1][-1][-1][key::xml_type] = "string";
	schema[-1][-1][key::errortext].type (key::xml_attribute);
	schema[-1][-1][-1][key::xml_type] = "string";
	
	schema[-1][key::xml_proplist][key::land].type (key::xml_member);
	schema[-1][-1][key::lor].type (key::xml_member);
	schema[-1][-1][key::match_id].type (key::xml_member);
	schema[-1][-1][key::match_class].type (key::xml_member);
	schema[-1][-1][key::match_type].type (key::xml_member);
	schema[-1][-1][key::match_data].type (key::xml_member);

	// -------------------------------------------------------------------
	//   <xml.class name="match.id">
	//     <xml.type>string</xml.type>
	//   </xml.class>
	// -------------------------------------------------------------------

	schema[key::match_id].type (key::xml_class);
	schema[-1][key::xml_type] = "string";

	// -------------------------------------------------------------------
	//   <xml.class name="match.class">
	//     <xml.type>string</xml.type>
	//   </xml.class>
	// -------------------------------------------------------------------

	schema[key::match_class].type (key::xml_class);
	schema[-1][key::xml_type] = "string";

	// -------------------------------------------------------------------
	//   <xml.class name="match.type">
	//     <xml.type>string</xml.type>
	//   </xml.class>
	// -------------------------------------------------------------------

	schema[key::match_type].type (key::xml_class);
	schema[-1][key::xml_type] = "string";

	// -------------------------------------------------------------------
	//   <xml.class name="match.data">
	//     <xml.type>array</xml.type>
	//     <xml.proplist>
	//       <xml.member class="text"/>
	//       <xml.member class="regexp"/>
	//       <xml.member class="lt"/>
	//       <xml.member class="gt"/>
	//       <xml.member class="minsize"/>
	//       <xml.member class="maxsize"/>
	//     </xml.proplist>
	//   </xml.class>
	// -------------------------------------------------------------------

	schema[key::match_data].type (key::xml_class);
	schema[-1][key::xml_type] = "array";
	schema[-1][key::xml_proplist][key::text].type (key::xml_member);
	schema[-1][-1][key::regexp].type (key::xml_member);
	schema[-1][-1][key::lt].type (key::xml_member);
	schema[-1][-1][key::gt].type (key::xml_member);
	schema[-1][-1][key::minsize].type (key::xml_member);
	schema[-1][-1][key::maxsize].type (key::xml_member);
	
	// -------------------------------------------------------------------
	//   <xml.class name="text"><xml.type>string</xml.type></xml.class>
	//   <xml.class name="regexp"><xml.type>string</xml.type></xml.class>
	//   <xml.class name="lt"><xml.type>integer</xml.type></xml.class>
	//   <xml.class name="gt"><xml.type>integer</xml.type></xml.class>
	//   <xml.class name="minsize"><xml.type>integer</xml.type></xml.class>
	//   <xml.class name="maxsize"><xml.type>integer</xml.type></xml.class>
	// -------------------------------------------------------------------

	schema[key::text].type (key::xml_class);
	schema[-1][key::xml_type] = "string";

	schema[key::regexp].type (key::xml_class);
	schema[-1][key::xml_type] = "string";

	schema[key::lt].type (key::xml_class);
	schema[-1][key::xml_type] = "integer";

	schema[key::gt].type (key::xml_class);
	schema[-1][key::xml_type] = "integer";

	schema[key::minsize].type (key::xml_class);
	schema[-1][key::xml_type] = "integer";

	schema[key::maxsize].type (key::xml_class);
	schema[-1][key::xml_type] = "integer";
	
	// -------------------------------------------------------------------
	//   <xml.class name="match.hasindex"><xml.type>void</xml.type></xml.class>
	//   <xml.class name="match.rule"><xml.type>string</xml.type></xml.class>
	// -------------------------------------------------------------------
	
	schema[key::match_hasindex].type (key::xml_class);
	schema[-1][key::xml_type] = "void";
	
	schema[key::match_rule].type (key::xml_class);
	schema[-1][key::xml_type] = "string";
}
