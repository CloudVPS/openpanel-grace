#ifndef _RETAIN_H
#define _RETAIN_H 1

#define returnclass(typ) typ *__grace_returnptr = new (memory::retainable::onstack) typ; typ &
#define retain = *__grace_returnptr;

extern void __pool_breakme (void);

#include <grace/lock.h>
#include <stdlib.h>

THROWS_EXCEPTION (memoryMismatchException, 0x2d7c1121, "Mismatch in retainable");
THROWS_EXCEPTION (memoryInvalidAddressException, 0x396c43d0, "Invalid address pointer");
THROWS_EXCEPTION (memoryLeakException, 0x28b8afca, "Possible memory leak "
				  "detected, set defaults::memory::leakprotection to false "
				  "if you know what you are doing");

void poolsighandler (int);
				  
/// Namespace for custom memory management.
namespace memory
{
	/// A pool of memory blocks that are of equal size.
	struct sizepool
	{
		struct sizepool	*next; ///< Linked list node.
		size_t			 sz; ///< Allocation size.
		unsigned int	 count; ///< Blockcount.
		char			*blocks; ///< Block data.
		struct sizepool	*extend; ///< Extension node.
		lock<bool>		 lck; ///< Thread lock.
	};
	
	/// Status of a memory block.
	enum blockstatus { free = 0, wired = 1 };

	/// Allocate a sizepool object.
	/// \param sz The block size.
	sizepool *mkpool (unsigned int sz);
					
	/// A memory block.
	struct block
	{
		sizepool		*pool; ///< Owning sizepool.
		size_t			 status; ///< Allocation status.
		
		unsigned char	 dt[0]; ///< Data offset.
	};
	
	/// A memory pool for retained pointer objects.
	class pool
	{
	public:
						 /// Constructor.
						 pool (void);
						 
						 /// Destructor.
						~pool (void);
						
						 /// Allocate a block.
						 /// \param sz The object's size.
						 /// \return Memory position.
		void			*alloc (size_t sz);
		
						 /// Free a block.
						 /// \param ptr Pointer to the block's data.
		void			 free (void *ptr);
		
						 /// Get a block's size.
						 /// \param ptr Pointer to the block's data.
		size_t			 getsize (void *ptr);
		
						 /// Determine if an allocation was pooled.
						 /// \param ptr Pointer to the block's data.
		bool			 pooled (void *ptr);
		
						 /// Create a debugging dump.
		void			 dump (const char *);
	
	protected:
		sizepool		*pools; ///< Linked list of sizepools.
	};
	
	pool *getretain (void);
	
	inline pool &retainpool (void) { return *memory::getretain(); }

	/// Base class for return-by-pointer memory management.
	/// Uses a custom allocator if the memory::retainable::onstack argument
	/// is given to new, in which case the memory will not be allocated
	/// through normal malloc(). Instead, a block is taken out of the
	/// preallocated memory pool.
	class retainable
	{
	public:
					 enum pooltype { onstack };
	
					 /// Basic constructor.
					 retainable (void) {}
					 					 
					 /// Virtual destructor.
		virtual		~retainable (void) {}
		
					 /// Memory alloc for non-retained pointers.
		void		*operator new (size_t sz);
		
					 /// Memory free for non-retained pointers.
		void		 operator delete (void *v);
		
					 /// Memory alloc for retained pointers.
		void		*operator new (size_t sz, pooltype r);
		
					 /// Memory free for retained pointers.
		void		 operator delete (void *v, pooltype r);
					
	protected:
					 /// Call this from any operator= or copy
					 /// constructor methods.
		void		 retainvalue (retainable *r);
		void		 destroyvalue (retainable *r);
		
		virtual void init (bool first = true)
					 {
					 	memset ((void *) this, 0, retainpool().getsize ((void *) this));
					 }
	};
};

/// Global pointer for debugging purposes.
extern memory::pool *__retain_ptr;
	
#endif
