#include <grace/retain.h>

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

	void *pool::alloc (size_t sz)
	{
		size_t rndsz = sz+sizeof(block);
		sizepool *c, *lastc;
		c = lastc = pools;
		while (c)
		{
			lastc = c;
			if (c->sz == rndsz) break;
			c = c->next;
		}

		if (! c)
		{
			c = new sizepool;
			c->next = NULL;
			c->count = 16;
			c->sz = rndsz;
			c->blocks = (char *) calloc (16, rndsz);
			
			for (int i=0; i<16; ++i)
			{
				block *bl = (block *) (c->blocks + (i*c->sz));
				bl->pool = c;
			}
			
			block *b = (block *) c->blocks;
			b->status = wired;
			
			if (lastc) lastc->next = c;
			else pools = c;
			return (void *) b->dt;
		}
		
		c->lck.lockw();
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
		unsigned int oldcount = c->count;
		unsigned int cc;
		c->count = oldcount * 2;
		::printf ("reallocing %08x to sz=%08x\n", c->blocks, c->count*c->sz);
		c->blocks = (char *) realloc (c->blocks, c->count * c->sz);
		::printf ("realloc'ed to %08x\n", c->blocks);
		memset ((char *) c->blocks + oldcount * c->sz, 0, oldcount * c->sz);
		for (cc=oldcount; cc < c->count; ++cc)
		{
			block *bl = (block *) (c->blocks + (cc * c->sz));
			bl->status = memory::free;
			bl->pool = c;
		}
		
		block *b = (block *) (c->blocks + (oldcount * c->sz));
		b->status = wired;
		c->lck.unlock();
		
		return b->dt;
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
}
