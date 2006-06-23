#ifndef _VISITOR_H
#define _VISITOR_H 1

#include <grace/str.h>
#include <grace/statstring.h>
#include <grace/stack.h>

/// Introspection class.
/// Any class that implements the visitor interface can be accessed
/// with this class to go over its tree.
template<class kind>
class visitor
{
public:
						 /// Constructor.
						 visitor (void);
						 
						 /// Constructor.
						 /// \param v The object to inspect.
						 visitor (kind &v)
						 {
				 			_root = current = &v;
				 			idx = NULL;
						 }
						 
						 /// Constructor.
						 /// \param v The object to inspect.
						 visitor (kind *v)
						 {
						 	_root = current = v;
						 }
						 
						 /// Destructor.
						~visitor (void)
						 {
						 	root();
						 }
	
						 /// Move the cursor to a child with a key.
	inline bool			 enter (const statstring &str)
						 {
						 	if (! str) return false;
						 	kind *v;
						 	v = current->visitchild (str);
						 	if (v)
						 	{
								sti.push (idx);
								stk.push ((kind *) current);
								idx = NULL;
						 		current = v;
						 		return true;
						 	}
						 	return false;
						 }
						 
						 /// Move the cursor to the first child node.
	inline bool			 first (void)
						 {
						 	kind *v;
						 	sti.push (idx);
						 	idx = NULL;
						 	
						 	v = current->visitchild (0);
						 	if (v)
						 	{
							 	idx = new int (0);
						 		stk.push ((kind *) current);
						 		current = v;
						 		return true;
						 	}
						 	idx = sti.pull();
						 	return false;
						 }
						 
						 /// Move the cursor to a sibling node.
	inline bool			 next (void)
						 {
						 	kind *v;
						 	if (! idx) return false;
						 	
						 	current = stk.pull ();
						 	(*idx)++;
						 	v = current->visitchild (*idx);
						 	if (v)
						 	{
						 		stk.push ((kind *) current);
						 		current = v;
						 		return true;
						 	}
						 	stk.push ((kind *) current);
						 	(*idx)--;
						 	return false;
						 }
						 
						 /// Move the cursor back up the tree.
	inline bool			 up (void)
						 {
						 	if (current == _root) return false;

							current = stk.pull ();
							if (idx) delete idx;
							idx = sti.pull();
							return true;
						 }
						 
						 /// Move cursor back to the root of the
						 /// inspected object.
	inline void			 root (void)
						 {
						 	if (idx) delete idx;
						 	idx = NULL;
						 	while (sti.count())
						 	{
						 		idx = sti.pull();
						 		if (idx)
						 		{
						 			delete idx;
						 			idx = NULL;
						 		}
						 	}
						 	while (stk.count()) stk.pull();
						 	current = _root;
						 }
						 
						 /// Returns true if a key exists as a child
						 /// for the object at the cursor position.
						 /// \param id The key.
	inline bool			 exists (const statstring &id)
						 {
						 	return current->exists (id);
						 }
						 
						 /// Returns key for object currently under cursor.
	inline const statstring &id (void)
						 {
						 	return current->_name;
						 }
						 
						 /// Get reference to object under cursor.
	inline kind			&obj (void)
						 {
						 	return *current;
						 }
				 
protected:
	kind		*_root; //< The root object.
	kind		*current; //< The cursor.
	int			*idx; //< Array index of cursor.
	stack<int>	 sti; //< Index stack.
	stack<kind> stk; //< Object stack.
};

#define foreach(iterator,object) \
    for (struct __foreachstr { bool __forfirst; visitor<typeof(object)> __foreachv; } __foreachctx = { true, object }; \
    __foreachctx.__forfirst && __foreachctx.__foreachv.first() || __foreachctx.__foreachv.next(); \
    __foreachctx.__forfirst = false) \
        for (bool __flipme=true; __flipme;) \
            for (typeof(object) &iterator = __foreachctx.__foreachv.obj(); __flipme; __flipme = false)
    
#endif
