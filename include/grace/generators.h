#ifndef _GENERATORS_H
#define _GENERATORS_H 1

class rangegen
{
public:
	rangegen (int _start, int _end) : start (_start), end (_end)
	{
		stp = 1;
	}
	~rangegen (void) {}
	
	int *visitchild (int pos)
	{
		if (pos<0) return NULL;
		if (start < end)
		{
			curval = start + (stp*pos);
			if (curval > end) return NULL;
		}
		else
		{
			curval = start - (stp*pos);
			if (curval < end) return NULL;
		}
		return &curval;
	}
	
	rangegen &step (int s) { stp = s; return *this; }
	
protected:
	int curval;
	int start;
	int end;
	int stp;
};

#define $range(x) rangegen(true ? x, false ? x).step(1)

#endif
