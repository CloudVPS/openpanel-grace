// ========================================================================
// value_xml.cpp: Keyed generic data storage class
//
// (C) Copyright 2004 Pim van Riezen <pi@madscience.nl>
//                    Madscience Labs, Rotterdam 
// ========================================================================

//      012345678 TAB-O-METER should measure 4
//      ^	^


#include <grace/value.h>
#include <grace/file.h>
#include <grace/stack.h>
#include <grace/strutil.h>
#include <grace/filesystem.h>
#include <grace/xmlschema.h>
#include <grace/ipaddress.h>

#include <stdio.h>
#include <string.h>

void __value_xml_breakme (void) {}

// ========================================================================
// METHOD ::loadxml
// ----------------
// Open an xml-file and translate it.
// ========================================================================
bool value::loadxml (const string &path, xmlschema &schema)
{
	return loadxml (path, &schema);
}

bool value::loadxml (const string &path, xmlschema *schema, string *err)
{
	string xml;
	
	xml = fs.load (path);
	if (! xml.strlen())
	{
		if (err) err->strcpy ("Could not load file");
		return false;
	}
	
	return fromxml (xml, schema, err);	
}

bool value::loadxml (const string &path, xmlschema &s, string &err)
{
	return loadxml (path, &s, &err);
}

bool value::fromxml (const string &xml, xmlschema &s, string &err)
{
	return fromxml (xml, &s, &err);
}

// ========================================================================
// METHOD ::fromxml
// ----------------
// Uses strutil::xmlreadtag() liberally to parse XML data in the plist-
// format and using it to fill our tree.
// ========================================================================
bool value::fromxml (const string &xml, xmlschema &schema)
{
	return fromxml (xml, &schema, NULL);
}

bool value::fromxml (const string &xml, xmlschema *schema, string *err)
{
	string xmlsource;
	statstring __id__;
	value tagbuf;
	int xcrsr = 0;
	
	xmlsource = xml;
	
	// Nuke what we have now
	if (arraysz)
	{
		for (unsigned int i=0; i<arraysz; ++i)
		{
			delete array[i];
		}
	}
	if (array)
	{

		::free (array);
		array = NULL;
	}
	arrayalloc = 0;
	arraysz = 0;
	
	if (attrib)
	{
		delete attrib;
		attrib = NULL;
	}
	
	// Need a stack to keep loops/conditionals
	stack<value> treestack;
	stack<string> tagstack;
	value *crsr = this;
	value *newcrsr;
		
	value nsCache;
	bool nsAware;
	string ln;
	string name;
	value tmp;
	bool inplist = false;
	int cnt = 0;
	xmltag tag;
	bool first = true;
	statstring tagtype;
	statstring attrname;
	bool insidecontainer = false;
	bool containerhasid = false;
	bool insidecontainervalue = false;
	bool second = true;
	statstring containeridclass;
	statstring containervalueclass;
	statstring containerwrapclass;
	statstring containerenvelope;
	statstring containerid;
	statstring __val__;
	bool hasvalueattribute = false;
	
	nsAware = schema ? schema->hasnamespaces() : false;
	
	try
	{
		while (! tag.eof)
		{
			bool ignorethis = false;
			bool dealingwithcontainervalue = false;
			// xmlreadtag digs pointers for extra speed
			strutil::xmlreadtag (&tag,&xmlsource);
			
			if (tag.errorcond)
			{
				if (err)
				{
					err->printf ("line %i: %s", tag.line, tag.errorstr.str());
				}
				return false;
			}

			if (tag.eof) break;
						
			tagtype = tag.type;
			
			if (schema) schema->resolveunionbase (tagtype);
			
			if (schema && schema->hasvalueattribute (tagtype))
			{
				hasvalueattribute = true;
				__val__ = schema->resolvevalueattribute (tagtype);
			}
			else
			{
				hasvalueattribute = false;
			}
			
			if (nsAware)
			{
				schema->nstranstype (nsCache, tagtype);
			}
			
			cnt++;
			
			if ((tagtype.str()[0]=='?')||(tagtype.str()[0]=='!'))
			{
				// do nothing
			}
			else if (tagtype.str()[0] == '/')
			{
				string opener;
				string closer;
				if (tagstack.count() == 0)
				{
					if (err)
					{
						(*err) = "Extraneous closing tag";
						err->printf (" <%s> at end of file", tagtype.str());
					}
					return false;
				}
				
				opener = tagstack.pull ();
				closer = tagtype.sval().mid (1);
				
				if (closer.strlen() && (opener != closer))
				{
					__value_xml_breakme();
					if (err)
					{
						err->printf ("line %i: Unbalanced tag, got <%s> "
									 "expected </%s> closer.strlen=%i "
									 "closer <%s>",
									 tag.getline(&xmlsource),
									 tagtype.sval().str(), opener.str(),
									 closer.strlen(), closer.str());
					}
					return false;
				}
				
				if (treestack.count() == 0) break;
				crsr = treestack.pull ();
				if (schema && (! schema->iscontainerclass (crsr->_type)))
				{
					insidecontainer = false;
				}
				else if (schema)
				{
					insidecontainer = true;
					insidecontainervalue = false;
					containerhasid = false;
					containeridclass =
						schema->resolvecontaineridclass (crsr->_type);
					containervalueclass =
						schema->resolvecontainervalueclass (crsr->_type);
					containerwrapclass =
						schema->resolvecontainerwrapclass (crsr->_type);
					containerenvelope =
						schema->resolvecontainerenvelope (crsr->_type);
				}
			}
			else if (first)
			{
				tagstack.push (new string (tagtype));
				// special treatment of the first tag, it should envelope
				// the entire tree and cannot have a valid "index".
				
				_type = tagtype;
				first = false;

				if (schema) __id__ = schema->resolveindexname (tagtype);
				else __id__ = "id";

				if (schema && schema->iscontainerclass (tagtype))
				{
					insidecontainer = true;
					insidecontainervalue = false;
					containerhasid = false;
					containeridclass =
						schema->resolvecontaineridclass (crsr->_type);
					containervalueclass =
						schema->resolvecontainervalueclass (crsr->_type);
					containerwrapclass =
						schema->resolvecontainerwrapclass (crsr->_type);
					containerenvelope =
						schema->resolvecontainerenvelope (crsr->_type);
				}
				
				foreach (prop, tag.properties)
				{
					attrname = prop._name;
					if (hasvalueattribute && (attrname == __val__))
					{
						(*this) = prop.sval();
					}
					else if (nsAware)
					{
						schema->nstransattr (nsCache, attrname, prop);
						setattrib (attrname, prop.sval());
					}
					else
					{	
						setattrib (attrname, prop.sval());
					}
				}
				treestack.push (this);
			}
			else
			{
				if (! tag.closed)
				{
					tagstack.push (new string (tagtype));
				}
				if (second)
				{
					second = false;
					if (insidecontainer) ignorethis = true;
					if (schema && schema->iscontainerclass (crsr->type()))
					{
						if (! insidecontainer)
						{
							insidecontainer = true;
							insidecontainervalue = false;
							containerhasid = false;
							containeridclass =
								schema->resolvecontaineridclass (crsr->_type);
							containervalueclass =
								schema->resolvecontainervalueclass (crsr->_type);
							containerwrapclass =
								schema->resolvecontainerwrapclass (crsr->_type);
							containerenvelope =
								schema->resolvecontainerenvelope (crsr->_type);
						}
					}
				}
				if (insidecontainer && (tagtype == containervalueclass))
				{
					insidecontainervalue = true;
					ignorethis = true;
					treestack.push (crsr);
				}
				else if (insidecontainer && (tagtype == containerwrapclass))
				{
					ignorethis = true;
					treestack.push (crsr);
				}
				else if (insidecontainer && (tagtype == containerenvelope))
				{
					ignorethis = true;
					treestack.push (crsr);
				}
				else if (insidecontainer && (tagtype == containeridclass))
				{
					//::printf ("detected containeridclass\n");
					containerhasid = true;
					ignorethis = true;
					containerid = tag.data;
				}
				if (!insidecontainer && schema && (!tag.closed) &&
					schema->iscontainerclass (tagtype))
				{
					insidecontainer = true;
					insidecontainervalue = false;

					statstring impid;
					impid = schema->resolveid (tagtype, crsr->_type);
					if (impid)
					{
						if (schema->isimplicitarray (tagtype))
						{
							newcrsr = &( (*crsr)[impid].newval() );
						}
						else
						{
							newcrsr= &( (*crsr)[impid] );
						}
					}
					else
					{
						newcrsr = &( (*crsr).newval() );
					}
					
					newcrsr->_type = tagtype;
					
					containerhasid = false;
					containeridclass =
						schema->resolvecontaineridclass (tagtype);

					containervalueclass =
						schema->resolvecontainervalueclass (tagtype);
					
					containerwrapclass =
						schema->resolvecontainerwrapclass (tagtype);
					
					containerenvelope =
						schema->resolvecontainerenvelope (tagtype);
					
					treestack.push (crsr);
					crsr = newcrsr;
					
					// START paste
					
					if (schema->containerhasattributes (tagtype))
					{
						statstring propKey;
					
						foreach (prop, tag.properties)
						{
							propKey = prop._name;
							
							if (propKey && (propKey != __id__))
							{
								if (hasvalueattribute && (propKey == __val__))
								{
									(*newcrsr) = prop.sval();
								}
								else if (nsAware)
								{
									attrname = propKey;
									schema->nstransattr (nsCache,attrname,prop);
										
									if (prop.count() > 1)
									{
										(*newcrsr).attributes()[attrname] =
											prop;
									}
									else
									{
										(*newcrsr).setattrib (attrname,
											prop.sval());
									}
								}
								else // no namespaces
								{
									// multiple property values?
									if (prop.count() > 1)
									{
										(*newcrsr).attributes()[propKey] =
											prop;
									}
									else // no just set the attribute
									{
										(*newcrsr).setattrib (propKey,
											prop.sval());
									}
								}
							}
						}

					// END PASTE

					}
				}
				else
				{
					if (schema) __id__ = schema->resolveindexname (tagtype);
					else __id__ = "id";
					
					if ((!ignorethis) && (insidecontainervalue || insidecontainer))
					{
						//::printf ("we're inside a container but didn't run into valueclass object\n");
						
						if (containerhasid)
						{
							newcrsr = &( (*crsr)[containerid] );
						}
						else
						{
							if ( (! schema->iswrap (tagtype)) ||
								 (! (insidecontainer || insidecontainervalue)))
							{
								newcrsr = &( (*crsr).newval() );
							}
							else
							{
								newcrsr = crsr;
							}
						}
						newcrsr->_type = tagtype;
						
						if (schema->iscontainerclass (tagtype))
						{
							//::printf ("this itself is a container class\n");
							insidecontainer = true;
							insidecontainervalue = false;
		
							statstring impid;
							impid = schema->resolveid (tagtype, crsr->_type);
							if (impid)
							{
								containerhasid = true;
								containerid = impid;
								containeridclass =
									schema->resolvecontaineridclass (tagtype);
							}
							else
							{
								containerhasid = false;
								containeridclass =
									schema->resolvecontaineridclass (tagtype);
							}
							
							containervalueclass =
								schema->resolvecontainervalueclass (tagtype);
							
							containerwrapclass =
								schema->resolvecontainerwrapclass (tagtype);
							
							containerenvelope =
								schema->resolvecontainerenvelope (tagtype);
							
							treestack.push (crsr);
							crsr = newcrsr;
						}
						else
						{
							dealingwithcontainervalue = true;
							if (! schema->iscontainerclass (crsr->type()))
							{
								//::printf ("nocontainer2: %s\n", crsr->type().str());
								insidecontainer = false;
							}
						}
						insidecontainervalue = false;
					}
					else if ((!ignorethis) && tag.properties.exists(__id__))
					{
						newcrsr = &( (*crsr)[tag.properties[__id__].sval()] );
						newcrsr->_type = tagtype;
					}
					else if (! ignorethis)
					{
						// See if the schema defines an implicit key value
						// for tags of the given type.
						statstring impid;
						if (schema)
						{
							impid = schema->resolveid (tagtype, crsr->_type);
						}
						
						// If so, set it, otherwise just add it by type
						// with no index.
						if (impid)
						{
							if (schema->isimplicitarray (tagtype))
							{
								newcrsr = &((*crsr)[impid].newval());
							}
							else
							{
								newcrsr = &((*crsr)[impid]);
							}
							newcrsr->_type = tagtype;
						}
						else
						{
							newcrsr = &(crsr->newval (tagtype));
						}
						
					}
					
					// Copy all other properties
					statstring propKey;
					
					foreach (prop, tag.properties)
					{
						propKey = prop._name;
						
						if (propKey && (propKey != __id__))
						{
							if (hasvalueattribute && (propKey == __val__))
							{
								(*newcrsr) = prop.sval();
							}
							else if (nsAware)
							{
								attrname = propKey;
								schema->nstransattr (nsCache,attrname,prop);
									
								if (prop.count() > 1)
								{
									(*newcrsr).attributes()[attrname] = prop;
								}
								else
								{
									(*newcrsr).setattrib (attrname, prop.sval());
								}
							}
							else // no namespaces
							{
								// multiple property values?
								if (prop.count() > 1)
								{
									(*newcrsr).attributes()[propKey] = prop;
								}
								else // no just set the attribute
								{
									(*newcrsr).setattrib (propKey, prop.sval());
								}
							}
						}
					}
						
					if (! insidecontainer)
					{
						// Does the tag have any children?
						if (! tag.closed)
						{
							treestack.push (crsr);
							crsr = newcrsr;
						}
					}
					// Is there data to this tag?
					
					if (!ignorethis && (tag.data.strlen()))
					{
						//::printf ("we get to look at the data\n");
						statstring tp;
						tp = tagtype;
						
						// resolve back the tag type to a builtin type,
						// if defined by the schema.
						if (schema && schema->knownclass (tagtype))
						{
							tp = schema->resolvetype (tagtype);
						}
						
						//::printf ("type=%s int=%s\n", tp.str(), t_int.str());
						
						// signed number types
						if ((tp == t_int) || (tp == t_long) ||
							(tp == t_short) || (tp == t_char))
						{
							long long vl;
							
							vl = strtoll (tag.data.str(),NULL,10);
							if (tp == t_long)
								*newcrsr = vl;
							else
								*newcrsr = (int) (vl & 0xffffffff);
							
							if ((tagtype != t_int)&&(tagtype != t_long))
							{
								(void) (*newcrsr).sval();
								(*newcrsr)._type = tag.type;
							}
							//::printf ("itype=%i\n", newcrsr->itype);
						}
						
						// unsigned number types
						else if ((tp == t_unsigned) || (tp == t_ulong) ||
							(tp == t_ushort) || (tp == t_uchar))
						{
							unsigned long long vl;
							
							vl =strtoll (tag.data.str(),NULL,10);
							if (tp == t_ulong)
								*newcrsr = vl;
							else
								*newcrsr = (unsigned int) (vl & 0xffffffff);
							
							if ((tagtype != t_unsigned)&&(tagtype != t_ulong))
							{
								(void) (*newcrsr).sval();
								(*newcrsr)._type = tagtype;
							}
						}
						
						// float type
						else if (tp == t_double)
						{
							*newcrsr = ::atof (tag.data.str());
							
							if (tagtype != t_double)
							{
								(void) (*newcrsr).sval();
								(*newcrsr)._type = tagtype;
							}
						}
						
						// boolean type
						else if (tp == t_bool)
						{
							*newcrsr = tag.data.strcasecmp ("true") ?
															false:true;
							
							if (tagtype != t_bool)
							{
								(void) (*newcrsr).sval();
								(*newcrsr)._type = tagtype;
							}
						}
						else if (tp == t_bool_true)
						{
							*newcrsr = true;
							(*newcrsr)._type = t_bool;
						}
						else if (tp == t_bool_false)
						{
							*newcrsr = false;
							(*newcrsr)._type = t_bool;
						}
						
						// hmm, that shouldn't happen
						else if ((tp == t_unsigned)||(tp == t_ulong))
						{
							unsigned long long vl;
							
							vl = strtoull (tag.data.str(),NULL,10);
							if (vl & 0xffffffff00000000LL)
								*newcrsr = vl;
							else
								*newcrsr = (unsigned int) (vl & 0xffffffff);
								
							if ((tagtype != t_unsigned)&&(tagtype != t_ulong))
							{
								(void)(*newcrsr).sval();
								(*newcrsr)._type = tagtype;
							}
						}
						
						// date description type
						else if (tp == t_date)
						{
							*newcrsr = (unsigned int)
											__parse_timestr (tag.data);
							newcrsr->itype = i_date;
							
							if (tagtype != t_date)
							{
								(void)(*newcrsr).sval();
								(*newcrsr)._type = tagtype;
							}
							else
							{
								(*newcrsr)._type = t_date;
							}
						}
						else if (tp == t_currency)
						{
							newcrsr->setcurrency (parsecurrency (tag.data));
							
							if (tagtype != t_currency)
							{
								(void)(*newcrsr).sval();
								(*newcrsr)._type = tagtype;
							}
						}
						else if (tp == t_ipaddr)
						{
							ipaddress i = tag.data;
							newcrsr->setip (i);
							
							if (tagtype != t_ipaddr)
							{
								(void)(*newcrsr).sval();
								(*newcrsr)._type = tagtype;
							}
						}
						else // bullshit type
						{
							if (schema && schema->stringclassisbase64 (tagtype))
							{
								(*newcrsr) = tag.data.decode64();
							}
							else
							{
								(*newcrsr) = tag.data;
							}
							(*newcrsr)._type = tagtype;
						}
					}
					else if (! ignorethis)
					{
						statstring tp;
						tp = tagtype;
						if (schema && schema->knownclass (tagtype))
						{
							tp = schema->resolvetype (tagtype);
							if (tp == t_bool_true)
							{
								*newcrsr = true;
								newcrsr->_type = t_bool;
							}
							else if (tp == t_bool_false)
							{
								*newcrsr = false;
								newcrsr->_type = t_bool;
							}
						}
					}
				}
			}
		}
	}
	catch (...)
	{
		// If the stack is not empty, the file was fscked. Whine.
	}
	if (treestack.count())
	{
		if (err && (! err->strlen())) err->strcpy ("Unexpected end of file");
		return false;
		//throw (exUnexpectedEndOfFile);
	}
	return true;
}

// ========================================================================
// METHOD ::savexml
// ----------------
// Converts to XML and writes to a file. TODO: we're not being terribly
// smart here, requiring the XML representation to be always built as
// a string. There's the same kind of fucked-up duplication of effort
// going on in other serialization places, this should be managed by
// abusing a small template class abstracting a "printable" stream, be
// it file or string object.
// ========================================================================
bool value::savexml (const string &filename, bool compact,
					 xmlschema *schema, flag::savetype tp) const
{
	string xml;
	
	xml = toxml (compact, schema);
	return fs.save (filename, xml, tp);
}

bool value::savexml (const string &filename, bool compact,
					 xmlschema &schema, flag::savetype fl) const
{
	return savexml (filename, compact, &schema, fl);
}

bool value::savexml (const string &fn, bool compact, flag::savetype tp) const
{
	return savexml (fn, compact, NULL, tp);
}

bool value::savexml (const string &fn, flag::savetype tp) const
{
	return savexml (fn, false, NULL, tp);
}

extern const char *_VALUE_INDENT_TABS;
#define _VIDENT (compact ? "" : _VALUE_INDENT_TABS + 16 - (ind&15))
#define _VEOL (compact ? "" : "\n")

// ========================================================================
// METHOD ::printxml
// ----------------
// Output all of a value's data in XML format, optionally assisted by
// an xmlschema definition.
// ========================================================================
void value::printxml (int indent, string &out, bool compact,
					  xmlschema *schema, value *par, const statstring &ptype,
					  const statstring &pid) const
{
	string outstr, nm;
	int ind;
	statstring rtype; // resolved type
	statstring rid; // resolved id
	statstring __id__; // resolved attribute name for id
	bool indeximplied = false;
	bool wascontainer = false;
	bool wascontained = false;
	bool isemptyboolvalue = false;
	bool wasouter = false;
	statstring containerclass;
	string containervalueclass;
	string containerwrapclass;
	string containerenvelope;
	statstring impliedtype;
	
	ind = indent;
	
	// resolve the class, id and indexname from the schema, or
	// resort to defaults
	
	/*::printf ("   -> resolve name=<%s> type=<%s> ptype=<%s> pid=<%s>\n",
			  _name ? _name.str() : "null",
			  _type ? _type.str() : "null",
			  ptype ? ptype.str() : "null",
			  pid ? pid.str() : "null");*/
	
	impliedtype = _type;
	if (indent<0)
	{
		if (schema && schema->hasrootclass ())
		{
			impliedtype = schema->getrootclass();
		}
	}
	
	if (schema)
	{
		schema->resolveclass (_name, impliedtype, ptype, pid, rtype);
		
		/*::printf ("resolved: rtype=<%s>\n", rtype.str());*/
		
		if (rtype == t_bool_true)
		{
			isemptyboolvalue = true;
			if (! bval())
			{
				schema->resolveclass (_name, t_bool_false, ptype, pid, rtype);
			}
			else
			{
				schema->resolveclass (_name, t_bool_true, ptype, pid, rtype);
			}
		}
		
		if (schema->isunion (rtype))
		{
			rtype = schema->resolveunion (this, rtype);
		}
		
		if (indent>=0) rid = schema->resolveidexport
								(_name, impliedtype, ptype, pid);
		
		// If the outer type is a container, we need to perform
		// some special work here.
		if ( ((indent<0) && schema->iscontainerclass (rtype)) ||
		     (schema->iswrapcontainer (rtype)) )
		{
			statstring containeridclass;
			wascontainer = true;
			wasouter = true;
			containerclass = rtype;
			
			if (rid)
			{
				containeridclass = schema->resolvecontaineridclass (rtype);
			}
			containervalueclass = schema->resolvecontainervalueclass (rtype);
			containerwrapclass = schema->resolvecontainerwrapclass (rtype);

			if (ind<16) ind++;

			if (attrib && schema->containerhasattributes (rtype))
			{
				out.printf ("%s<%s", _VIDENT, rtype.str());
				string sv;
				statstring sn;
				foreach (attr, (*attrib))
				{
					sn = attr.label();
					if ( (!rid) || (sn != __id__) ) // we already covered the index
					{
						if (schema->containerhasattribute (rtype,
								attr.id()))
						{
							if (attr.count() > 1) // multiple attributes?
							{
								for (int j=0; j<attr.count(); ++j)
								{
									sv = attr[j].sval();
									if (sv.strlen())
										out.printf (" %s=\"%A\"", sn.str(), sv.str());
								}
							}
							else // single named attribute
							{
								sv = attr.sval();
						
								out.printf (" %s=\"%A\"", sn.str(), sv.str());
							}
						}
					}
				}
				out.printf (">%s", _VEOL);
			}
			else
			{
				out.printf ("%s<%s>%s", _VIDENT, rtype.str(), _VEOL);
			}
			if (ind<16) ind++;
			if (containervalueclass.strlen())
				out.printf ("%s<%s>%s", _VIDENT, containervalueclass.str(), _VEOL);
			else
				ind--;

			if (count())
			{
				if (ucount == (unsigned int) count())
				{
					rtype = schema->resolvecontainerarrayclass (rtype);
				}
				else
				{
					rtype = schema->resolvecontainerdictclass (rtype);
				}
			}
			else
			{
				if (itype == i_bool)
				{
					rtype = schema->resolvecontainerboolclass (rtype, bval());
				}
				else
				{
					rtype = schema->resolvecontainertypeclass (rtype, itype);
				}
			}
			
			if (schema->isunion (rtype))
			{
				rtype = schema->resolveunion (this, rtype);
			}
		}
		else if (schema->iscontainerclass (ptype))
		{
			statstring containeridclass;
			wascontained = true;
			
			ind++;
			
			containerwrapclass = schema->resolvecontainerwrapclass (ptype);
			if (containerwrapclass.strlen())
			{
				out.printf ("%s<%s>%s", _VIDENT, containerwrapclass.str(), _VEOL);
				ind ++;
			}
			
			if (rid)
			{
				containeridclass = schema->resolvecontaineridclass (ptype);
				out.printf ("%s<%s>%s</%s>%s",
							_VIDENT,
							containeridclass.str(),
							rid.str(),
							containeridclass.str(),
							_VEOL);
			}

			containervalueclass = schema->resolvecontainervalueclass (ptype);
			if (containervalueclass.strlen())
				out.printf ("%s<%s>%s", _VIDENT, containervalueclass.str(), _VEOL);
			else
				ind--;

			if (count())
			{
				if (ucount == (unsigned int) count())
				{
					rtype = schema->resolvecontainerarrayclass (ptype);
				}
				else
				{
					rtype = schema->resolvecontainerdictclass (ptype);
				}
				containerenvelope = schema->resolvecontainerenvelope (ptype);
			}
			else
			{
				if (itype == i_bool)
				{
					rtype = schema->resolvecontainerboolclass (ptype, bval());
				}
				else
				{
					rtype = schema->resolvecontainertypeclass (ptype, itype);
				}
			}
			
			if (schema->isunion (rtype))
			{
				rtype = schema->resolveunion (this, rtype);
			}
		}
		__id__ = schema->resolveindexname (rtype);
	}
	else
	{
		rid = _name;
		
		switch (itype)
		{
			case i_int:
				//::printf ("+++ i_int\n");
				rtype = t_int; break;
			
			case i_unsigned:
				//::printf ("+++ i_unsigned\n");
				rtype = t_unsigned; break;
				
			case i_double:
				rtype = t_double; break;
			
			case i_long:
				rtype = t_long; break;
			
			case i_ulong:
				rtype = t_ulong; break;
				
			case i_bool:
				//::printf ("+++ i_bool\n");
				rtype = t_bool; break;
				
			case i_ipaddr:
				rtype = t_ipaddr; break;
			
			case i_date:
				rtype = t_date; break;
			
			case i_currency:
				rtype = t_currency; break;
				
			default:
				if (indent <0)
				{
					rtype = impliedtype;
					break;
				}
				//::printf ("default: %i\n", itype);
				if (ucount)
				{
					if ((unsigned int) count() != ucount)
						rtype = t_dict;
					else
						rtype = t_array;
				}
				else if (count())
				{
					rtype = t_dict;
				}
				else rtype = t_string;
		}
		
		__id__ = "id";
	}
	ind++;
	if (ind>15) ind = 15;
	else if (ind<0) ind = 0;
	
	if (schema)
	{
		containerenvelope = schema->resolvecontainerenvelope (rtype);
		if (ucount && schema->isimplicitarray (rtype))
		{
			for (unsigned int p=0; p<ucount; ++p)
			{
				array[p]->printxml (indent, out, compact, schema, par,
									ptype, pid);
			}
			return;
		}
	}
	
	out.printf ("%s<%s", _VIDENT, rtype.str());
	
	// Print id attribute if it's there
	if ((!wascontainer) && (!wascontained) && rid)
		out.printf (" %s=\"%A\"", __id__.str(), rid.str());
	
	bool hadattr = false;
	if (schema && (wascontainer || wascontained))
	{
		hadattr = schema->containerhasattributes (containerclass);
	}
	
	// If we have attributes, now we print them
	if (/*(!wascontainer) && (!wascontained) && */attrib)
	{
		string sv;
		statstring sn;
		foreach (attr,(*attrib))
		{
			sn = attr.label();

			if (hadattr && schema->containerhasattribute (containerclass, sn))
				continue;

			if ( (!rid) || (sn != __id__) ) // we already covered the index
			{
				if (attr.count() > 1) // multiple attributes?
				{
					for (int j=0; j<attr.count(); ++j)
					{
						sv = attr[j].sval();
						if (sv.strlen())
							out.printf (" %s=\"%A\"", sn.str(), sv.str());
					}
				}
				else // single named attribute
				{
					sv = attr.sval();
			
					out.printf (" %s=\"%A\"", sn.str(), sv.str());
				}
			}
		}
	}
	
	// if the value is empty and there are no children, the story ends here
	
	if ( (! arraysz) &&
	     ( (itype != i_unsigned) && (itype != i_int) &&
	       (itype != i_long) && (itype != i_ulong) &&
	       (itype != i_double) && 
	       (itype != i_bool) && (itype != i_currency) &&
	       (! sval().strlen()) ) )
	{
		out.printf ("/>%s", _VEOL);
		if (wascontainer)
		{
			if (containervalueclass.strlen())
			{
				ind--;
				out.printf ("%s</%s>%s", _VIDENT,
							containervalueclass.str(),
							_VEOL);
			}
			ind--;
			out.printf ("%s</%s>%s", _VIDENT,
						containerclass.str(),
						_VEOL);
		}
		return;
	}
	
	string *datum;

	// Time to print out the explicit value. Either as an attribute
	// if so demanded, or as node data.
	if (schema && schema->hasvalueattribute (rtype))
	{
		out.printf (" %s=\"%Z\"/>%s",
					schema->resolvevalueattribute (rtype).str(),
					s.str(),
					_VEOL);
	}
	else switch (itype)
	{
		case i_int:
			out.printf (">%i</%s>%s", ival(), rtype.str(), _VEOL);
			break;
		case i_string:
			if (schema && schema->stringclassisbase64 (rtype))
			{
				string tmp;
				tmp = s.encode64 ();
				out.printf (">%s</%s>%s", tmp.str(), rtype.str(), _VEOL);
			}
			else
			{
				out.printf (">%Z</%s>%s", s.str(), rtype.str(), _VEOL);
			}
			break;
		case i_bool:
			if (isemptyboolvalue)
			{
				out.printf ("/>%s", _VEOL);
			}
			else
			{
				out.printf  (">%s</%s>%s", bval() ? "true" : "false",
										   rtype.str(), _VEOL);
			}
			break;
		case i_double:
			out.printf (">%f</%s>%s", dval(), rtype.str(), _VEOL);
			break;
		case i_long:
			out.printf (">%L</%s>%s", lval(), rtype.str(), _VEOL);
			break;
		case i_ipaddr:
			out.printf (">%s</%s>%s", sval().str(), rtype.str(), _VEOL);
			break;
		case i_unsigned:
			out.printf (">%u</%s>%s", uval(), rtype.str(), _VEOL);
			break;
		case i_date:
			datum = __make_timestr (uval());
			out.printf (">%s</%s>%s", datum->str(), rtype.str(), _VEOL);
			delete datum;
			break;
		case i_ulong:
			out.printf (">%U</%s>%s", ulval(), rtype.str(), _VEOL);
			break;
		case i_currency:
			out.strcat (">");
			printcurrency (out, getcurrency());
			out.printf ("</%s>%s", rtype.str(), _VEOL);
			break;
		default:
			// A possible dictionary type
			if (arraysz)
			{	
				out.printf (">%s", _VEOL);
				if (containerenvelope.strlen())
				{
					ind++;
					out.printf ("%s<%s>%s", _VIDENT, containerenvelope.str(), _VEOL);
					if (ind>15) ind = 15;
				}
				for (unsigned int i=0; i<arraysz; ++i)
				{
					array[i]->printxml (ind, out, compact,
										schema, (value *) this,
										rtype, rid);
				}
				if (containerenvelope.strlen())
				{
					out.printf ("%s</%s>%s", _VIDENT, containerenvelope.str(), _VEOL);
					ind--;
					if (ind>15) ind = 15;
				}
				out.printf ("%s</%s>%s", _VIDENT, rtype.str(), _VEOL);
			}
			else
			{
				if (schema && schema->stringclassisbase64 (rtype))
				{
					string tmp;
					tmp = s.encode64 ();
					out.printf (">%s</%s>%s", tmp.str(), rtype.str(), _VEOL);
				}
				else
				{
					out.printf (">%Z</%s>%s", s.str(), rtype.str(), _VEOL);
				}
			}
			break;
	}
	if ((wascontainer)||(wascontained))
	{
		if (containervalueclass.strlen())
		{
			ind--;
			out.printf ("%s</%s>%s", _VIDENT,
						containervalueclass.str(),
						_VEOL);
		}
		if (wascontained && containerwrapclass.strlen())
		{
			ind--;
			out.printf ("%s</%s>%s", _VIDENT,
						containerwrapclass.str(),
						_VEOL);
		}
		if (wascontainer)
		{
			ind--;
			out.printf ("%s</%s>%s", _VIDENT,
						containerclass.str(),
						_VEOL);
		}
	}

}

// ========================================================================
// METHOD ::toxml
// ========================================================================
string *value::toxml (bool compact, xmlschema *schema) const
{
	returnclass (string) res retain;
	value mparent;
	statstring empty;
	
	if (!compact)
	{
		res.strcat ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		if (schema && schema->hasdoctype())
		{
			const value &dt = schema->doctype();
			
			res += "<!DOCTYPE %[name]s %[status]s \"%{1}s\" \"%[dtd]s\">\n"
						%format (dt.attributes(), dt);
						
		}
	}
	
	printxml (-1, res, compact, schema, &mparent, empty, empty);
	return &res;
}

string *value::toxml (bool compact, xmlschema &schema) const
{
	return toxml (compact, &schema);
}
