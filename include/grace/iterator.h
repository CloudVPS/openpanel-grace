#ifndef _ITERATOR_H
#define _ITERATOR_H 1

template<class kind,class ckind>
class iterator
{
public:
	iterator (kind &ref) { o = &ref; current = NULL; pos = -1; }
	~iterator (void) { }
	
	ckind &obj (void)
	{
		if (! current)
		{
			current = new ckind;
			return (ckind &)(*current);
		}
		return (ckind &) *current;
	}
	
	bool first (void)
	{
		pos = 0;
		return next ();
	}
	
	bool next (void)
	{
		if (pos<0) return false;
		current = o->visitchild (pos);
		if (! current)
		{
			pos = -1;
			return false;
		}
		pos++;
		return true;
	}
	
protected:
	kind *o;
	ckind *current;
	int pos;
};

#endif
