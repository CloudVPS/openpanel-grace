#ifndef _RETAIN_H
#define _RETAIN_H 1

#define returnclass(typ) typ *__grace_returnptr = new (memory::retainable::onstack) typ; typ &
#define retain = *__grace_returnptr;
#include <grace/lock.h>
#include <stdlib.h>

#define EX_MEMORY_RETAIN_MISMATCH 666

namespace memory
{
	struct sizepool
	{
		struct sizepool	*next;
		size_t			 sz;
		unsigned int	 count;
		char			*blocks;
		lock<bool>		 lck;
	};
	
	enum blockstatus { free = 0, wired = 1 };
	
	struct block
	{
		sizepool		*pool;
		size_t			 status;
		
		unsigned char	 dt[0];
	};
	
	class pool
	{
	public:
						 pool (void);
						~pool (void);
					
		void			*alloc (size_t sz);
		void			 free (void *ptr);
		size_t			 getsize (void *ptr);
		bool			 pooled (void *ptr);
	
	protected:
		sizepool		*pools;
	};
	
	pool *memory::getretain (void);
	
	inline pool &memory::retainpool (void) { return *memory::getretain(); }

	class retainable
	{
	public:
					 enum pooltype { onstack };
	
					 retainable (void) {}
					 retainable (retainable *r)
					 {
					 	retainvalue (r);
					 }
					 
		virtual		~retainable (void) {}
		void		*operator new (size_t sz);
		void		 operator delete (void *v);
		
		void		*operator new (size_t sz, pooltype r);
		void		 operator delete (void *v, pooltype r);
					 
					
		retainable	&operator= (retainable *r)
					 {
					 	retainvalue (r);
					 	return *this;
					 }
		
	protected:
		void		 retainvalue (retainable *r);
		void		 retaininitrefs (void)
					 {
					 	memset ((void *) this, 0, retainpool().getsize ((void *) this));
					 }
	};
};

extern memory::pool *__retain_ptr;
	

#endif
