#ifndef _ARRAY_H
#define _ARRAY_H 1

#include <stdlib.h>

#include <grace/str.h>

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
					 /// Default constructor.
					 /// Leave the array unallocated, set all sizes
					 /// to 0.
					 array (void)
					 {
					 	_array = NULL;
					 	_count = 0;
					 	_arraysz = 0;
					 }
					~array (void)
					 {
					 	if (_array) ::free (_array);
					 }
					 
					 /// Add a new entry to the array.
					 /// Grows the storage for the array as needed using
					 /// power-of-two preallocation.
					 ///
					 /// \param element Pointer to the entry
	void			 add (kind *element)
					 {
						if (! _arraysz)
						{
							_arraysz = 8;
							_array = (kind **)
								calloc (_arraysz, sizeof (kind *));
							_count = 0;
						}
						if ( (_count+1) >= _arraysz)
						{
							_arraysz = _arraysz << 1;
							_array = (kind **) realloc
								(_array, _arraysz * sizeof (kind *));
						}
						if ( (_count+1) < _arraysz)
						{
							_array[_count] = element;
							_count++;
						}
					 }
	
					 /// Insert a new element at a specific array position.
					 /// This position must be within the range of
					 /// 0...count().
					 /// \param foo The object to add
					 /// \param position The position to add it to.
	void			 insert (kind *foo, int position)
					 {
					 	if (position < 0) return;
					 	if (position == _count)
					 	{
					 		add (foo);
					 		return;
					 	}
					 	if (position >= _count) return;
						if (! _arraysz)
						{
							_arraysz = 8;
							_array = (kind **)
								calloc (_arraysz, sizeof (kind *));
							_count = 0;
						}
						if ( (_count+1) >= _arraysz)
						{
							_arraysz = _arraysz << 1;
							_array = (kind **) realloc
								(_array, _arraysz * sizeof (kind *));
						}
						if ( (_count+1) < _arraysz)
						{
					 		if ((position+1) < _count)
					 		{
					 			::memmove (_array+position+1, _array+position,
					 					   (_count - position) *
					 					   sizeof (kind *));
					 		}
					 		_array[position] = foo;
					 	}
					 	_count++;
					 }
	
					 /// Remove the node at a given position.
					 /// \param _pos Array position, if negative measured
					 ///             from the right.
	void			 remove (int _pos)
					 {
					 	int pos = _pos;
					 	if (pos<0) pos = _count + pos;
					 	if (pos<0) pos = 0;
					 	if (pos >= _count) pos = _count-1;
					 	delete _array[pos];
					 	if ((pos+1) < _count)
					 	{
						 	::memmove (_array+pos, _array+pos+1,
						 			   (_arraysz - (pos+1)) *
						 			   sizeof (kind *));
						}
						_count--;
					 }
					 
					 /// Remove the last element of the array.
	void			 removelast (void) { remove (-1); }
					 
					 /// Access operator.
					 /// If a negative index is provided, the position
					 /// is measured from the right (with the last element
					 /// of the array being represented as -1).
					 ///
					 /// \param _pos Requested array position.
	kind			&operator[] (int _pos)
					 {
					 	int pos = _pos;
					 	if (pos<0) pos = _count + pos;
					 	if (pos<0) pos = 0;
					 	if (pos >= _count) pos = _count-1;
					 	return *(_array[pos]);
					 }
					 
					 /// Item count.
					 /// Returns the number of entries in the array.
	int				 count (void) { return _count; }

protected:
	kind			**_array; ///< The actual array (allocated using malloc)
	int				 _count; ///< The number of active entries in the array
	int				 _arraysz; ///< The allocated size of the array
};

#endif
