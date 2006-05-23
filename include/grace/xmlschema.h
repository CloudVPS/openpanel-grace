#ifndef _XMLSCHEMA_H
#define _XMLSCHEMA_H 1

#include <grace/str.h>
#include <grace/statstring.h>
#include <grace/filesystem.h>

// ------------------------------------------------------------------------
// Static keys used extensively in XML schemas. This will save space and
// needless hash calculations
// ------------------------------------------------------------------------

namespace key {
	extern const statstring xml_schema;
	extern const statstring xml_type;
	extern const statstring xml_code;
	extern const statstring xml_encoding;
	extern const statstring xml_proplist;
	extern const statstring xml_member;
	extern const statstring xml_class;
	extern const statstring xml_attributes;
	extern const statstring xml_attribute;
	extern const statstring xml_validate;
	extern const statstring xml_string;
	extern const statstring xml_regexp;
	extern const statstring xml_schema_options;
	extern const statstring xml_option_rootclass;
	extern const statstring xml_option_namespaces;
	extern const statstring xml_option_doctype;
	extern const statstring xml_namespace;
	extern const statstring xml_namespace_type;
	extern const statstring xml_union;
	extern const statstring xml_union_match;
	extern const statstring xml_container;
	extern const statstring xml_container_valueclass;
	extern const statstring xml_container_wrapclass;
	extern const statstring xml_container_envelope;
	extern const statstring xml_container_idclass;
	extern const statstring xml_container_types;
	extern const statstring xml_container_type;
	extern const statstring container;
	extern const statstring contained;
	extern const statstring name;
	extern const statstring status;
	extern const statstring dtd;
	extern const statstring alias;
	extern const statstring cunion;
	extern const statstring array;
	extern const statstring prefix;
	extern const statstring action;
	extern const statstring label;
	extern const statstring isindex;
	extern const statstring id;
	extern const statstring uri;
	extern const statstring klass;
	extern const statstring mandatory;
	extern const statstring type;
	extern const statstring isdict;
	extern const statstring isattribute;
	extern const statstring implicit;
	extern const statstring isclass;
	extern const statstring isprecedent;
	extern const statstring ofclass;
	extern const statstring strict;
	extern const statstring wrap;
	extern const statstring attribvalue;
	extern const statstring grace_netdb_host;
	extern const statstring host_name;
	extern const statstring host_address;
	extern const statstring hostname;
	extern const statstring address;
	extern const statstring grace_runoptions;
	extern const statstring grace_option;
	extern const statstring grace_long;
	extern const statstring grace_argc;
	extern const statstring grace_default;
	extern const statstring grace_help;
	extern const statstring g_long;
	extern const statstring g_argc;
	extern const statstring g_default;
	extern const statstring g_help;
	
	extern const statstring grace_validator;
	extern const statstring datarule;
	extern const statstring land;
	extern const statstring lor;
	extern const statstring match_child;
	extern const statstring match_id;
	extern const statstring match_class;
	extern const statstring match_type;
	extern const statstring match_data;
	extern const statstring match_attribute;
	extern const statstring match_hasindex;
	extern const statstring match_rule;
	extern const statstring text;
	extern const statstring regexp;
	extern const statstring lt;
	extern const statstring gt;
	extern const statstring minsize;
	extern const statstring maxsize;
	extern const statstring errorcode;
	extern const statstring errortext;
	extern const statstring match_mandatory;
	extern const statstring optional;
	extern const statstring key;
};

/// List of built-in schemas.
typedef enum
{
	XMLRootSchemaType, ///< The schema defining schema-files.
	XMLBaseSchemaType, ///< Primitive types.
	XMLNetDBSchemaType, ///< Schema used by the netdb class.
	XMLRunOptionsSchemaType, ///< Schema for grace.runoptions.xml
	XMLValidatorSchemaType ///< Schema for grace.validator
} hardCodedSchema;

/// A parsed xmlschema file.
/// This class is used by the value class for XML exports and imports.

class xmlschema
{
public:
						 /// Creator.
						 /// Loads disk schema file.
						 /// \param name path to schema file.
						 xmlschema (const string &name);
						 
						 /// Creator.
						 /// Initialize as a builtin schema.
						 xmlschema (hardCodedSchema i);
						 
						 /// Creator.
						 xmlschema (void)
						 {
						 }
						 
						 /// Load a schema file.
						 /// \param name File name.
	void				 load (const string &name);
						 
	enum 				 extspec {
							forbid = false,
							allow = true
						 };
	
						 /// Set up the builtin root schema.
	void				 xmlrootschema (void);

						 /// Set up the builtin base schema.
	void				 xmlbaseschema (void);

						 /// Set up the builtin netdb schema.
	void				 netdbschema (void);
	
						 /// Set up the builtin runoptions schema.
	void				 runoptschema (void);
	
						 /// Set up the builtin validator schema.
	void				 validatorschema (void);
	
						 /// Add definitions for all primitive types.
	void				 addbasemembers (value &);
	
						 /// Add a primitive type.
						 /// \param bt The registered type.
						 /// \param it The intrinsic type.
						 /// \param code CXML code for the class.
						 /// \param xcode CXML code for the index attribute.
	void				 addbaseclass (const statstring &bt, const char *it,
									   const char *cd, const char *icd);
	
						 /// Returns true if the schema has namespace
						 /// definitions.
	bool				 hasnamespaces (void);
	
						 /// Returns true if a root class is defined in
						 /// the xml.schema.options section.
	bool				 hasrootclass (void);
	
						 /// Return the root class if defined in the
						 /// xml.schema.options section.
	const string		&getrootclass (void);
	
						 /// Returns true if the schema-file defines a doctype.
	bool				 hasdoctype (void);
	
						 /// Get the doctype information.
						 /// The string data of the value is the doctype
						 /// string, with the attribute 'dtd' containing
						 /// the URI for the doctype DTD, the attribute
						 /// 'name' containing the short name for the
						 /// doctype and the attribute 'status' indicating
						 /// the doctype being either public or private.
	const value			&doctype (void);
	
						 /// Perform namespace translation of a type.
						 /// \param ns Namespace data.
						 /// \param typ The type. Will be changed by this method.
	void				 nstranstype (value &ns, statstring &typ);
	
						 /// Perform namespace translation of an attribute label
						 /// \param nm Namespace data.
						 /// \param at Attribute name. Will be changed.
						 /// \param v Reference to the value implicated.
	void				 nstransattr (value &nm, statstring &at, const value &v);
	
						 /// Returns true of a registered type is defined
						 /// by the schema.
	bool				 knownclass (const string &);
	
						 /// Determines an implicit id.
						 /// \param cl The current class.
						 /// \param scl The super-class.
	const string		&resolveid (const string &cl, const string &scl);
	
						 /// Used by CXML. Determines whether
						 /// attribute data should preceed object data.
	bool				 wouldneedprecedents (const statstring &);
	
						 /// Resolve a class to its intrinsic type.
						 /// \param cl Class name.
	const string		&resolvetype (const statstring &cl);
	
						 /// Resolve an attribute to its intrinsic type.
						 /// \param cl Class name.
						 /// \param an Attribute name.
	const string		&resolvetypeattrib (const statstring &cl, const statstring &an);

						 /// Resolve label of an index attribute for a class.
	const statstring	&resolveindexname (const statstring &);
	
						 /// Determines implicit class.
						 /// Looks at the context of the currently resolved
						 /// class, superclass, key and the parent's key
						 /// to see if any of these imply that the node
						 /// should be written with another registered
						 /// type.
	void				 resolveclass (const statstring &, const statstring &,
									   const statstring &, const statstring &,
									   statstring &);
									   
						 /// Resolve type to key.
						 /// If the schema implies that objects of a certain
						 /// class have a defined index key, this method
						 /// will return the value for that key.
	const statstring	&resolveidexport (const statstring &, const statstring &,
										  const statstring &, const statstring &);
										  
						 /// Returns true if the schema defined a string
						 /// object to use base64 encoding.
	bool				 stringclassisbase64 (const statstring &);

						 /// Returns true if the class is a container
						 /// with child objects having a wrap="true"
						 /// attribute indicating this class should
						 /// only do type wrapping and has no
						 /// child nodes.
	bool				 iswrapcontainer (const statstring &);
	
						 /// Returns true if the class is defined with
						 /// a wrap="true" attribute, indicating that
						 /// its contents should be wrapped into the
						 /// parent (container) class.
	bool				 iswrap (const statstring &);
	
						 /// Returns true if a class uses the 'container'
						 /// method for encoding. The main difference
						 /// with regular methods is that child nodes
						 /// instead of attributes are used for implying
						 /// an object's type or id.
	bool				 iscontainerclass (const statstring &);

	bool				 isunion (const statstring &);
	const statstring	&resolveunion (const value *, const statstring &);
	void				 resolveunionbase (statstring &);
	
	bool				 containerhasattributes (const statstring &);
	bool				 containerhasattribute (const statstring &,
												const statstring &);
	
						 /// If true, the class has an attribvalue
						 /// attribute, indicating that the class
						 /// data should be stored inside an
						 /// attribute.
	bool				 hasvalueattribute (const statstring &);
	const string		&resolvevalueattribute (const statstring &);
	
	bool				 isimplicitarray (const statstring &);
	
	const string		&resolvecontainerenvelope (const statstring &);
	const string		&resolvecontainerwrapclass (const statstring &);
	const string		&resolvecontaineridclass (const statstring &);
	const string 		&resolvecontainervalueclass (const statstring &);
	string				*resolvecontainerboolclass (const statstring &, bool);
	string				*resolvecontainertypeclass (const statstring &,
													unsigned char);
	string				*resolvecontainerarrayclass (const statstring &);
	string				*resolvecontainerdictclass (const statstring &);
	
						 /// Returns true for names of intrinsic types.
	bool				 isinternaltype (const statstring &);
	
						 /// Validate an object against the schema.
	bool				 validate (const value &, const value &, extspec);
	
						 /// Create new value object with all mandatory
						 /// schema elements filled in.
	value				*create (const statstring &);
	value				&inspectcode (const char *);
	bool				 isattribute (const char *);
	bool				 isindex (const char *);
	bool				 isprecedent (const char *);
	const string		&xmltype (const char *);
	const string		&attributelabel (const char *);
	const string		&xmlclass (const char *);
	bool				 isdict (const char *);
	bool				 hasimplicit (const char *);
	const string		&implicitid (const char *);
	const string		&precedentclass (const char *);
	const char			*resolvecode (const statstring &);
	const char			*resolvecodeid (const statstring &);
	const char			*resolvecodeattrib (const statstring &, const statstring &);

	value				 schema;	
protected:
	value				 codecache;
};

extern xmlschema XMLRootSchema;
extern xmlschema XMLBaseSchema;

#endif
