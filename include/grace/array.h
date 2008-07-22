#ifndef _ARRAY_H
#define _ARRAY_H 1

#include <grace/exception.h>
#include <stdlib.h>
#include <string.h>

$exception (arrayOutOfBoundsException, "Array out of bounds");

/// A template-implementation of a variable-size array
/// Elements are stored as pointers to existing objects. Memory management
/// for these objects is outside the scope of this class. Accessing the
/// array resolves to an object reference. The array itself is dynamically
/// grown with a power-of-two allocation, with a starting size of 8.
///
/// Example usage of an array container:
/// \verbinclude array_ex1.cpp
template<class kind>
class array
{
public:
					 /// Internal storage for an array node.
					 struct arraynode
					 {
					 	/// If true, the array should delete the object
					 	/// if the node is removed.
					 	bool dynamic; 
					 	
					 	/// Pointer to the object.
					 	kind *obj;
					 };
					 
					 /// Default constructor.
					 /// Leave the array unallocated, set all sizes
					 /// to 0.
					 array (void)
					 {
					 	_array = NULL;
					 	_count = 0;
					 	_arraysz = 0;
					 }
					 
					 /// Destructor. Only the array is freed, no object
					 /// destructors are called.
					~array (void)
					 {
					 	if (_array && _count)
					 	{
					 		for (int i=0; i<_count; ++i)
					 		{
					 			if (_array[i].dynamic)
						 			delete _array[i].obj;
						 	}
					 	}
					 	if (_array) ::free (_array);
					 }
					 
					 /// Add a non-dynamic entry to the array. Grows the
					 /// storage for the array as needed using power-of-two
					 /// preallocation.
					 /// \param element The elemnt-reference to add. Deleting
					 ///                this element before the array will
					 ///                lead to nasty behavior.
	void			 add (kind &element) { add (&element, false); }
					 
					 /// Add a new entry to the array.
					 /// Grows the storage for the array as needed using
					 /// power-of-two preallocation. If dynamic is true,
					 /// the object will be deleted if its node is removed.
					 ///
					 /// \param element Pointer to the entry
	void			 add (kind *element, bool dynamic = true)
					 {
						if (! _arraysz)
						{
							_arraysz = 8;
							_array = (arraynode*)
								calloc (_arraysz, sizeof (arraynode));
							_count = 0;
						}
						if ( (_count+1) >= _arraysz)
						{
							_arraysz = _arraysz << 1;
							_array = (arraynode *) realloc
								(_array, _arraysz * sizeof (arraynode));
						}
						if ( (_count+1) < _arraysz)
						{
							_array[_count].obj = element;
							_array[_count].dynamic = dynamic;
							_count++;
						}
					 }
	
					 /// Insert a reference at a specific array position.
					 /// This position must be within the range of
					 /// 0...count().
					 /// \param foo The referenced object to insert.
					 /// \param position The position to insert at.
					 /// \throw arrayOutOfBoundsException
	void			 insert (kind &foo, int position)
					 {
					 	insert (&foo, position, false);
					 }
					 
					 /// Insert a new element at a specific array position.
					 /// This position must be within the range of
					 /// 0...count(). If dynamic is true, the object
					 /// will be deleted if its node is removed.
					 /// \param foo The object to add
					 /// \param position The position to add it to.
					 /// \param dynamic Flags auto-delete.
					 /// \throw arrayOutOfBoundsException
	void			 insert (kind *foo, int position, bool dynamic = true)
					 {
					 	if (position < 0) throw arrayOutOfBoundsException();
					 	if (position == _count)
					 	{
					 		add (foo, dynamic);
					 		return;
					 	}
					 	if (position >= _count)
					 	{
					 		throw arrayOutOfBoundsException();
					 	}
						if (! _arraysz)
						{
							_arraysz = 8;
							_array = (arraynode*)
								calloc (_arraysz, sizeof (arraynode));
							_count = 0;
						}
						if ( (_count+1) >= _arraysz)
						{
							_arraysz = _arraysz << 1;
							_array = (arraynode *) realloc
								(_array, _arraysz * sizeof (arraynode));
						}
						if ( (_count+1) < _arraysz)
						{
					 		if ((position+1) < _count)
					 		{
					 			::memmove (_array+position+1, _array+position,
					 					   (_count - position) *
					 					   sizeof (arraynode));
					 		}
					 		_array[position].obj = foo;
					 		_array[position].dynamic = dynamic;
					 	}
					 	_count++;
					 }
	
					 /// Remove the node at a given position.
					 /// \param _pos Array position, if negative measured
					 ///             from the right.
					 /// \throws arrayOutOfBoundsException
	void			 remove (int _pos)
					 {
					 	int pos = _pos;
					 	if (pos<0) pos = _count + pos;
					 	if (pos<0) throw arrayOutOfBoundsException();
					 	if (pos >= _count) throw arrayOutOfBoundsException();
					 	if (_array[pos].dynamic) delete _array[pos].obj;
					 	if ((pos+1) < _count)
					 	{
						 	::memmove (_array+pos, _array+pos+1,
						 			   (_arraysz - (pos+1)) *
						 			   sizeof (arraynode));
						}
						_count--;
					 }
	
					 /// Swap two elements in the array. This method
					 /// does not accept negative offsets.
					 /// \param a Position of the first element.
					 /// \param b Position of the second element.
					 /// \throws arrayOutOfBoundsException
	void			 swap (int a, int b)
					 {
					 	if ((a<0) || (b<0)) throw arrayOutOfBoundsException();
						if ((a>=_count) || (b>=_count))
							throw arrayOutOfBoundsException();
						
						_swap (a, b);
					 }
					 
					 /// Move an element to another position in the array,
					 /// all other nodes will be shifted. This method does
					 /// not accept negative offsets.
	void			 move (int from, int to)
					 {
					 	if ((from<0)||(to<0))
					 	{
					 		throw arrayOutOfBoundsException();
					 	}
					 	if ((from>=_count) || (to>=_count))
					 	{
					 		throw arrayOutOfBoundsException();
					 	}
					 	if (from == to) return;
					 	
					 	int c = from;
					 	while (c != to)
					 	{
					 		if (c < to)
					 		{
					 			_swap (c, c+1);
					 			c++;
					 		}
					 		else
					 		{
					 			_swap (c, c-1);
					 			c--;
					 		}
					 	}
					 }
					 
					 /// Remove the last element of the array.
	void			 removelast (void) { remove (-1); }
					 
					 /// Access operator.
					 /// If a negative index is provided, the position
					 /// is measured from the right (with the last element
					 /// of the array being represented as -1).
					 ///
					 /// \param _pos Requested array position.
					 /// \throws arrayOutOfBoundsException
	kind			&operator[] (int _pos)
					 {
					 	int pos = _pos;
					 	if (pos<0) pos = _count + pos;
					 	if (pos<0) throw arrayOutOfBoundsException();
					 	if (pos >= _count) throw arrayOutOfBoundsException();
					 	return *(_array[pos].obj);
					 }
					 
					 /// Item count.
					 /// Returns the number of entries in the array.
	int				 count (void) { return _count; }
	
	kind			*visitchild (int pos)
					 {
					 	if (pos<0) return NULL;
					 	if (pos >= _count) return NULL;
					 	return (_array[pos].obj);
					 }

protected:
	arraynode		 *_array; ///< The actual array (allocated using malloc)
	int				 _count; ///< The number of active entries in the array
	int				 _arraysz; ///< The allocated size of the array
	
					 /// Internal swap-method, assumes bounds checking
					 /// already took place.
	void			 _swap (int a, int b)
					 {
						kind *tmp = _array[a].obj;
						bool dyn = _array[a].dynamic;
						_array[a].obj = _array[b].obj;
						_array[a].dynamic = _array[b].dynamic;
						_array[b].obj = tmp;
						_array[b].dynamic = dyn;
					 }
};

#endif
