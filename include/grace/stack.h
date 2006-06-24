#ifndef _STACK_H
#define _STACK_H 1

#include <stdlib.h>
#include <unistd.h>

/// Stack related exceptions.
enum stackException
{
	exStackUnderFlow, /// Stack pull on empty stack.
	exOutOfMemory /// Ran out of juice.
};

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
					throw (exStackUnderFlow);
				 }
				 
	inline void	 push (kind *k)
				 {
				 	if (! array)
					{
						array = (kind **) malloc (4 * sizeof (kind *));
						if (! array) throw (exOutOfMemory);
						asz = 4;
					}
					if (cnt+1 > asz)
					{
						asz *= 2;
						array = (kind **) realloc (array, asz * sizeof (kind *));
						if (! array) throw (exOutOfMemory);
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
				 	return *(array[pos]);
				 }

private:
	int			 cnt; ///< Object count.
	int			 asz; ///< Allocated array size.
	kind		**array; ///< Object array.
};

#endif
