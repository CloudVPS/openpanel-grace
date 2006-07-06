#ifndef _XMLSCHEMA_H
#define _XMLSCHEMA_H 1

#include <grace/str.h>
#include <grace/statstring.h>
#include <grace/filesystem.h>
#include <grace/commonkeys.h>

// ------------------------------------------------------------------------
// Static keys used extensively in XML schemas. This will save space and
// needless hash calculations
// ------------------------------------------------------------------------


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
						 /// \param cd CXML code for the class.
						 /// \param icd CXML code for the index attribute.
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

						 /// Returns true if the class is a union class.
	bool				 isunion (const statstring &);
	
						 /// Resolve union object to id.
	const statstring	&resolveunion (const value *, const statstring &);
	
						 /// Resolve union name to base type.
	void				 resolveunionbase (statstring &);
	
						 /// Return true if the provided class has
						 /// attributes.
	bool				 containerhasattributes (const statstring &);
	
						 /// Return true if the provided class has
						 /// a specific attribute.
	bool				 containerhasattribute (const statstring &,
												const statstring &);
	
						 /// If true, the class has an attribvalue
						 /// attribute, indicating that the class
						 /// data should be stored inside an
						 /// attribute.
	bool				 hasvalueattribute (const statstring &);
	
						 /// Return the attribute name for a
						 /// type that encodes its value as such.
	const string		&resolvevalueattribute (const statstring &);
	
						 /// Whether elements of this type should
						 /// be automatically put inside their own array,
	bool				 isimplicitarray (const statstring &);
	
						 /// Resolve container envelope class.
	const string		&resolvecontainerenvelope (const statstring &);
	
						 /// Resolve container wrapper class.
	const string		&resolvecontainerwrapclass (const statstring &);
	
						 /// Resolve container id class.
	const string		&resolvecontaineridclass (const statstring &);
	
						 /// Resolve container value class.
	const string 		&resolvecontainervalueclass (const statstring &);
	
						 /// Resolve container bool class.
	string				*resolvecontainerboolclass (const statstring &, bool);
	
						 /// Resolve container type class.
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

	value				 schema; //< Loaded schema data.
protected:
	value				 codecache; //< Cache dictionary of CXML codes.
};

extern xmlschema XMLRootSchema;
extern xmlschema XMLBaseSchema;

#endif
