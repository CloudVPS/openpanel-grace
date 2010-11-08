// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _STACK_H
#define _STACK_H 1

#include <stdlib.h>
#include <unistd.h>

$exception (stackUnderflowException, "Stack underflow");
$exception (stackOutOfMemoryException, "Stack out of memory");
$exception (stackAccessRangeException, "Stack access ouf of range");

/// A generic stack.
/// Template class for keeping a stack of objects.
template<class kind>
class stack
{
public:
				 stack (void)
				 {
				 	cnt = asz = 0;
					array = NULL;
				 }
				~stack (void)
				 {
				 	if (array) ::free (array);
				 }
				
	inline kind	*pull (void)
				 {
				 	if (cnt)
					{
						--cnt;
						return array[cnt];
					}
					if (array) ::free (array);
					array = NULL;
					throw stackUnderflowException();
				 }
				 
	inline void	 push (kind *k)
				 {
				 	if (! array)
					{
						array = (kind **) malloc (4 * sizeof (kind *));
						if (! array) throw stackOutOfMemoryException();
						asz = 4;
					}
					if (cnt+1 > asz)
					{
						asz *= 2;
						array = (kind **) realloc (array, asz * sizeof (kind *));
						if (! array) throw stackOutOfMemoryException();
					}
					array[cnt++] = k;
				 }
				 
				 /// Pointer to top entry.
				 /// Will read the top entry without pulling it.
	inline kind *peek (void)
				 {
				 	if ( (!array) || (!cnt) ) return NULL;
				 	return array[cnt-1];
				 }
				 
				 /// Return number of objects on the stack.
	inline int	 count (void)
				 {
				 	return cnt;
				 }
	inline kind	&operator[] (int pos)
				 {
				 	if (pos < 0) throw stackAccessRangeException();
				 	if (pos >= cnt) throw stackAccessRangeException();
				 	return *(array[(cnt-1) - pos]);
				 }

private:
	int			 cnt; ///< Object count.
	int			 asz; ///< Allocated array size.
	kind		**array; ///< Object array.
};

#endif
