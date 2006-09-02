#ifndef _ITERATOR_H
#define _ITERATOR_H 1

/// A template class implementing the iterator protocol. Its template
/// arguments are the class of the parentobject  (the iterator base) and
/// the class of the child objects. This class is used by the foreach
/// macro to iterate over any class that supports this protocol.
template<class kind,class ckind>
class iterator
{
public:
	/// Constructor. Stores a pointer to the iterator base object and
	/// initializes the cursor. 
	iterator (kind &ref) { o = &ref; current = NULL; pos = -1; }
	
	/// Destructor.
	~iterator (void) { }
	
	/// Access the object under the cursor.
	ckind &obj (void)
	{
		if (! current)
		{
			current = new ckind;
			return (ckind &)(*current);
		}
		return (ckind &) *current;
	}
	
	/// Start an iteration round. Sets the cursor to 0 and calls next().
	/// \return False, if the iterator base currently has no child nodes.
	bool first (void)
	{
		pos = 0;
		return next ();
	}
	
	/// Attempts to get a pointer to the object at the current cursor
	/// position. If it exists, the cursor is advanced, otherwise the
	/// cursor is reset.
	/// \return False, if the there is no object left in the collection.
	bool next (void)
	{
		if (pos<0) return false;
		current = o->visitchild (pos);
		if (! current)
		{
			pos = -1;
			return false;
		}
		pos++;
		return true;
	}
	
protected:
	kind *o; ///< Pointer to the iterator base object.
	ckind *current; ///< Pointer to the child object under the cursor.
	int pos; ///< The cursor, -1 means there is no loop active.
};

#endif
