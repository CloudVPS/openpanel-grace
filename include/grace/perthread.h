#ifndef _PERTHREAD_H
#define _PERTHREAD_H 1

#include <pthread.h>
#include <grace/lock.h>

template<class kind>
class perthreadnode
{
public:
	perthreadnode (void) { next = prev = NULL; }
	~perthreadnode (void) { }
	
	pthread_t     		  thr;
	perthreadnode		 *next, *prev;
	kind		  		  obj;
};

template<class kind>
class perthread
{
public:
	perthread (void)
	{
		first = last = NULL;
	}
	~perthread (void)
	{
		perthreadnode<kind> *c, *nc;
		
		exclusivesection (lck)
		{
			c = first;
			while (c)
			{
				nc = c->next;
				delete c;
				c = nc;
			}
			first = last = NULL;
		}
	}
	
	kind &get (void)
	{
		pthread_t me = pthread_self();
		sharedsection (lck)
		{
			perthreadnode<kind> *c = first;
			while (c)
			{
				if (c->thr == me) breaksection return c->obj;
				c = c->next;
			}
		}
		
		perthreadnode<kind> *c = NULL;
		
		exclusivesection (lck)
		{
			c = new perthreadnode<kind>;
			c->thr = me;
			c->next = NULL;
			if (last)
			{
				c->prev = last;
				last->next = c;
				last = c;
			}
			else
			{
				c->prev = NULL;
				first = last = c;
			}
		}
		return c->obj;
	}
	
	perthread &operator= (kind &i) { get() = i; }

protected:
	lock<bool> lck;
	perthreadnode<kind> *first, *last;
};

#endif
