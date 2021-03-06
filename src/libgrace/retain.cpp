// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#include <grace/retain.h>
#include <grace/defaults.h>
#include <grace/file.h>
#include <signal.h>

memory::pool *__retain_ptr;

void poolsighandler (int sig)
{
	__retain_ptr->dump ("memory.dump");
	signal (SIGUSR2, poolsighandler);
}

void __pool_breakme (void) { }

namespace memory
{
	
	// ====================================================================
	// FUNCTION getretain
	// ====================================================================
	pool *getretain (void)
	{
		static memory::pool *p = new memory::pool;
		__retain_ptr = p;
		return p;
	}

	// ====================================================================
	// CONSTRUCTOR pool
	// ====================================================================
	pool::pool (void)
	{
		signal (SIGUSR2, poolsighandler);
		pools = NULL;
	}

	// ====================================================================
	// DESTRUCTOR pool
	// ====================================================================
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
	
	// ====================================================================
	// FUNCTION mkpool
	// ====================================================================
	sizepool *mkpool (unsigned int rndsz)
	{
		unsigned int count;
		sizepool *c = new sizepool;
		if (! c) return NULL;
		
		c->lck.lockw ();
		
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
		
		c->lck.unlock ();
		return c;
	}

	// ====================================================================
	// METHOD ::alloc
	// ====================================================================
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

			c->lck.lockw ();

			block *b = (block *) c->blocks;
			b->status = wired;
			
			if (lastc) lastc->next = c;
			else pools = c;
			
			c->lck.unlock ();
			
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
			
			if (i>96)
			{
				__pool_breakme();
			}
		}
		
		// The initial pool is full. If leak protection is set in the
		// defaults, this is the end of the line.
		if (defaults::memory::leakprotection)
		{
			dump ("memoryleak.dump");
			c->lck.unlock();
			abort();
			// just in case some jackass catches SIGABRT.
			throw memoryLeakException();
		}
		if (defaults::memory::leakcallback)
		{
			defaults::memory::leakcallback();
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
	
	// ====================================================================
	// METHOD ::free
	// ====================================================================
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
			throw memoryInvalidAddressException();
		}
#endif
		
		b->status = memory::free;
	}
	
	// ====================================================================
	// METHOD ::exit
	// ====================================================================
	void pool::exit (void)
	{
#ifdef DISABLE_RETAINPOOLS
		return;
#endif
		sizepool *c, *nc, *nnc;
		
		c = pools;
		nc = nnc = NULL;
		
		while (c)
		{
			nnc = c->next;
			nc = c->extend;
			
			::free (c->blocks);
			delete c;
			
			for (c=nc;c;c=nc)
			{
				nc = c->extend;
				::free (c->blocks);
				delete c;
			}
			
			c = nnc;
		}
		
		pools = NULL;
	}
	
	// ====================================================================
	// METHOD ::dumpmem
	// ====================================================================
	void pool::dumpmem (block *b, size_t sz, FILE *finto)
	{
		size_t j;
		
		for (size_t i=0; i<sz; i+=8)
		{
			fprintf (finto, "        ");
			for (j=0; (j<8)&&((i+j)<sz); ++j)
			{
				fprintf (finto, "%02x ", b->dt[i+j]);
			}
			
			for (;j<8;++j) fprintf (finto, "   ");
			
			for (j=0;(j<8)&&((i+j)<sz);++j)
			{
				unsigned char cr = (unsigned char) b->dt[i+j];
				if (cr<32 || cr & 128) fprintf (finto, ".");
				else fprintf (finto, "%c", cr);
			}
			
			fprintf (finto, "\n");
		}
	}
	
	// ====================================================================
	// METHOD ::dump
	// ====================================================================
	void pool::dump (const char *where)
	{
		FILE *finto;
		
		finto = fopen (where, "w");
		if (! finto) return;
		
		sizepool *c = pools;
		sizepool *nc, *nnc = NULL;

		//if (c) c->lck.lockr();

		while (c)
		{
			fprintf (finto, "sizepool %i\n", c->sz);
			// Look for a free block.
			for (unsigned int i=0; i<c->count; ++i)
			{
				block *b = (block *) (c->blocks + (i * c->sz));
				
				// Are you free, Mr. Humphrey?
				if (b->status != memory::free)
				{
					fprintf (finto, "  alloc %u\n", i);
				}
				dumpmem (b, c->sz, finto);
			}
			
			nc = c->extend;
			//if (nc) nc->lck.lockr();
			//c->lck.unlock();
			
			while (nc)
			{
				fprintf (finto, "extension pool %i\n", nc->sz);
				for (unsigned int i=0; i<nc->count; ++i)
				{
					block *b = (block *) (nc->blocks + (i * c->sz));
					
					// Are you free, Mr. Humphrey?
					if (b->status != memory::free)
					{
						fprintf (finto, "  alloc %u\n", i);
					}
				}
				nnc = nc->extend;
				//if (nnc) nnc->lck.lockr();
				//nc->lck.unlock();
				nc = nnc;
			}
			
			nc = c->next;
			//if (nc) nc->lck.lockr();
			//c->lck.unlock();
			c = c->next;
		}
		fclose (finto);
	}
	
	// ====================================================================
	// METHOD ::getsize
	// ====================================================================
	size_t pool::getsize (void *ptr)
	{
		block *b = (block *) (((char*)ptr) - sizeof (block));
		if (! b->pool) return b->status;
		return b->pool->sz - sizeof (block);
	}

	// ====================================================================
	// METHOD ::pooled
	// ====================================================================
	bool pool::pooled (void *ptr)
	{
		block *b = (block *) (((char*)ptr) - sizeof (block));
		if (! b->pool) return false;
		return true;
	}

	// ====================================================================
	// METHOD ::operator new
	// ====================================================================
	void *retainable::operator new (size_t sz)
	{
		block *b = (block *) malloc (sz + sizeof (block));
		b->pool = NULL;
		b->status = sz;
		return b->dt;
	}

	// ====================================================================
	// METHOD ::operator delete
	// ====================================================================
	void retainable::operator delete (void *v)
	{
		block *b = (block *) ((char *) v - sizeof (block));

#ifndef DISABLE_RETAINPOOLS
		if (b->pool)
		{
			retainpool().free (v);
			return;
		}
#endif
		
		::free (((char *)v)-sizeof (block));
	}
	
	// ====================================================================
	// METHOD ::operator new
	// ====================================================================
	void *retainable::operator new (size_t sz, pooltype r)
	{
#ifdef DISABLE_RETAINPOOLS
		block *b = (block *) malloc (sz + sizeof (block));
		b->pool = NULL;
		b->status = sz;
		return b->dt;
#endif
		void *res = retainpool().alloc (sz);
		return res;
	}
	
	// ====================================================================
	// METHOD ::operator delete
	// ====================================================================
	void retainable::operator delete (void *v, pooltype r)
	{
#ifdef DISABLE_RETAINPOOLS
		::free (((char *)v)-sizeof (block));
		return;
#endif
		retainpool().free (v);
	}
	
	// ====================================================================
	// METHOD ::retainvalue
	// ====================================================================
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
		memmove (this, r, mysz);
		if (retainpool().pooled(r))
			retainpool().free (r);
		else
		{
			::free (((char *)r) - sizeof (block));;
		}
	}
	// ====================================================================
	// METHOD ::destroyvalue
	// ====================================================================
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
