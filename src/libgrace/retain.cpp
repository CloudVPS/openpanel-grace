#include <grace/retain.h>
#include <grace/defaults.h>

memory::pool *__retain_ptr;

namespace memory
{
	pool *getretain (void)
	{
		static memory::pool *p = new memory::pool;
		__retain_ptr = p;
		return p;
	}

	pool::pool (void)
	{
		pools = NULL;
	}

	pool::~pool (void)
	{
		sizepool *c, *nc;
		c = pools;
		while (c)
		{
			nc = c->next;
			::free (c->blocks);
			delete c;
			c = nc;
		}
		pools = NULL;
	}
	
	sizepool *mkpool (unsigned int rndsz)
	{
		unsigned int count;
		sizepool *c = new sizepool;
		if (! c) return NULL;
		
		// For tiny sizes, allocate in 8K blocks. For medium, use 64K blocks.
		// For larger objects, keep a count of 16.
		if (rndsz < 512) count = 8192/rndsz;
		else if (rndsz < 4096) count = 65536/rndsz;
		else count = 16;
		
		c->next = NULL;
		c->extend = NULL;
		c->count = count;
		c->sz = rndsz;
		c->blocks = (char *) calloc (count, rndsz);
		
		for (unsigned int i=0; i<count; ++i)
		{
			block *bl = (block *) (c->blocks + (i*c->sz));
			bl->pool = c;
		}
		
		return c;
	}

	void *pool::alloc (size_t sz)
	{
		// The requested size does not include the overhead for the
		// block header. For efficiency purposes, we use a 64 bits
		// boundary.
		size_t rndsz = (sz+sizeof(block)+7) & 0xfffffff8;
		
		// First let's hunt for an existing primary size pool.
		sizepool *c, *lastc;
		c = lastc = pools;
		while (c)
		{
			lastc = c;
			if (c->sz == rndsz) break;
			c = c->next;
		}

		// c is set if we found a pool with a matching blocksize.
		// if not, we need to allocate it.
		if (! c)
		{
			c = mkpool (rndsz);
			if (! c) return NULL;

			block *b = (block *) c->blocks;
			b->status = wired;
			
			if (lastc) lastc->next = c;
			else pools = c;
			return (void *) b->dt;
		}
		
		// Set a lock on the current sizepool.
		c->lck.lockw();
		
		// Look for a free block.
		for (unsigned int i=0; i<c->count; ++i)
		{
			block *b = (block *) (c->blocks + (i * c->sz));
			
			// Are you free, Mr. Humphrey?
			if (b->status == memory::free)
			{
				// Jay, we can stop looking. Claim the block and
				// unlock the allocation pool.
				b->status = wired;
				b->pool = c;
				c->lck.unlock();
				return (void *) b->dt;
			}
		}
		
		// The initial pool is full. If leak protection is set in the
		// defaults, this is the end of the line.
		if (defaults::memory::leakprotection)
		{
			c->lck.unlock();
			throw (EX_MEMORY_LEAK);
		}
		
		// Ok, the user actually _wants_ these massive amounts of
		// retainable objects, we will use the extent pointer to
		// create/access extension pools.
		while (true)
		{
			sizepool *tc = c;
			if (c->extend) c = c->extend;
			else
			{
				c->extend = mkpool (rndsz);
				c = c->extend;
				if (! c)
				{
					tc->lck.unlock();
					return NULL;
				}
			}
			c->lck.lockw();
			tc->lck.unlock();
			
			for (unsigned int i=0; i<c->count; ++i)
			{
				block *b = (block *) (c->blocks + (i * c->sz));
				if (b->status == memory::free)
				{
					b->status = wired;
					b->pool = c;
					c->lck.unlock();
					return (void *) b->dt;
				}
			}
		}
		
		// End never reached.
		return NULL;
	}
	
	void pool::free (void *ptr)
	{
		block *b = (block *) (((char*)ptr) - sizeof (block));
		
#ifdef PARANOID_MEMORY_CHECKS
		sizepool *c = pools;
		while (c)
		{
			if ( ((char*)b) >= ((char*)c->blocks) )
			{
				if ( ((char*)b) < (((char*)c->blocks)+(c->sz * c->count)) )
				{
					break;
				}
			}
			c = c->next;
		}
		if (!c)
		{
			throw (EX_MEMORY_DEFUNCT_POINTER);
		}
#endif
		b->status = memory::free;
	}
	
	size_t pool::getsize (void *ptr)
	{
		block *b = (block *) (((char*)ptr) - sizeof (block));
		if (! b->pool) return b->status;
		return b->pool->sz - sizeof (block);
	}

	bool pool::pooled (void *ptr)
	{
		block *b = (block *) (((char*)ptr) - sizeof (block));
		if (! b->pool) return false;
		return true;
	}

	void *retainable::operator new (size_t sz)
	{
		block *b = (block *) malloc (sz + sizeof (block));
		b->pool = NULL;
		b->status = sz;
		return b->dt;
	}

	void retainable::operator delete (void *v)
	{
		block *b = (block *) ((char *) v - sizeof (block));
		if (b->pool)
		{
			retainpool().free (v);
			return;
		}
		
		::free (((char *)v)-sizeof (block));
	}
	
	void *retainable::operator new (size_t sz, pooltype r)
	{
		void *res = retainpool().alloc (sz);
		return res;
	}
	
	void retainable::operator delete (void *v, pooltype r)
	{
		retainpool().free (v);
	}
	
	void retainable::retainvalue (retainable *r)
	{
		if (! r)
		{
			init (false);
			return;
		}
		size_t mysz = retainpool().getsize (r);
		if (! mysz)
		{
			return;
		}
		memcpy (this, r, mysz);
		if (retainpool().pooled(r))
			retainpool().free (r);
		else
		{
			::free (((char *)r) - sizeof (block));;
		}
	}
	void retainable::destroyvalue (retainable *r)
	{
		if (! r) return;
		if (retainpool().pooled (r))
		{
			retainpool().free (r);
		}
		else
		{
			::free (((char *)r) - sizeof (block));
		}
	}
}
