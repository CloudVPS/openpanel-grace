// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

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
