#ifndef _VALUEINDEX_H
#define _VALUEINDEX_H 1

#include <grace/value.h>
#include <grace/statstring.h>

/// A reference to a child value object that has been indexed.
/// Offers an array of values that match a common index key.
class indexreference
{
friend class valueindex;
public:
					 /// Creator.
					 /// \param id The index key for this reference.
					 /// \param p Link to the parent index.
					 indexreference (const statstring &id, class valueindex *p);
					~indexreference (void);

					 /// Add a reference to the array.
					 /// \param v Pointer to a matching value.
	void			 addreference (value *v);
	
	value			&operator[] (int);
	
					 /// Return number of entries in the array.
	int				 count (void);

protected:
	statstring		 _refvalue; ///< Index key.
	value		   **_refarray; ///< Child array.
	int				 _count; ///< Number of children.
	int				 _arraysz; ///< Allocated array size.
	indexreference	*higher, *lower;
};

/// Alternative index for a value tree.
/// This class allows you to create an alternative index of a value's
/// child nodes, without affecting the original 'indexed-by-id' layout.
class valueindex
{
friend class indexreference;
public:
						 valueindex (void);
						~valueindex (void);
	
						 /// Index child nodes by their registered type.
						 /// \param v The value object to index.
	void				 indextypes (value &v);
	
						 /// Index child nodes by an attribute.
						 /// \param v The value object to index.
						 /// \param k The attribute name to use.
	void				 indexproperty (value &v, const statstring &k);

						 /// Index child nodes by a keyed grandchild.
						 /// For every record in the array, a look is
						 /// taken for a child object with the provided
						 /// key and its value is used for indexing.
						 /// \param v The value object to index.
						 /// \param k The grandchild key.
	void				 indexrecord (value &v, const statstring &k); 
	
	indexreference		&operator[] (const statstring &);
	
						 /// Return first entry of the index.
	value				&first (const statstring &);
	
						 /// Returns true if a key exists in the index.
	bool				 exists (const statstring &);

protected:	
	indexreference		*root;
	indexreference		*find (const statstring &);
};

#endif
