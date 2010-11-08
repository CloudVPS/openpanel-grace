// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#include <grace/triplet.h>

// =======================================================================
// METHOD tstring::timport
// -----------------------
// Accepts a regular ASCII string as input, first converts it to a
// 'triplized' string of only characters that are valid triplet values,
// then goes over the string with a sliding window of 3 characters to
// collect triplets into the array.
// =======================================================================

void tstring::timport (const string &str)
{
	string triplized;
	bool didspace = true;
	ctriplet ct;
	
	// We always begin with a <SPC>
	triplized = " ";
	
	// Go over the original string
	for (int i=0; i<str.strlen(); ++i)
	{
		char c; // Cursor
		
		c = str[i];
		
		if (::isalpha (c)) // An alphabetical character
		{
			c = tolower (c);
			
			// Make certain characters redundant
			if (c == 'q') c = 'c';
			else if (c == 'z') c = 's';
			else if (c == 'w') c = 'v';
			else if (c == 'y') c = 'j';
			
			// Add the triplized value to the array
			triplized += c;
			didspace = false;
		}
		else if (::isdigit (c))
		{
			// The zero is redundant, replaced by the 'o'
			if (c == '0') c = 'o';
			triplized += c;
			didspace = false;
		}
		else
		{
			// if this is the first non-renderable value, mark it as a
			// <SPC>, otherwise skip it altogether.
			
			if (! didspace) triplized += ' ';
			didspace = true;
		}
	}
	
	// A triplet stream always ends with a <SPC>
	if (! didspace) triplized += ' ';
	
	// Go over the triplized string with a sliding window cursor
	// to collect the triplet array.
	for (int j=0; (j+2) < triplized.strlen(); ++j)
	{
		ct[0] = triplized[j];
		ct[1] = triplized[j+1];
		ct[2] = triplized[j+2];
		
		_array_add (tencode (ct));
	}
}

// =======================================================================
// METHOD tstring::_array_add
// --------------------------
// Internal method, adds a single triplet to the internal array
// =======================================================================

void tstring::_array_add (triplet t)
{
	// Create the array if there is none yet
	if (! _array)
	{
		// Let's start with 8 triplets
		_size = 8;
		_array = (triplet *) malloc (8 * sizeof (triplet));
		_count = 1;
		_array[0] = t;
		return;
	}
	// Do we need to grow the array?
	if ( _count >= _size )
	{
		_size *= 2;
		_array = (triplet *) realloc (_array, _size * sizeof (triplet));
	}
	
	// Add the new triplet
	_array[_count] = t;
	_size++;
}

// =======================================================================
// METHOD tstring::operator[]
// --------------------------
// Array accessor
// =======================================================================

triplet tstring::operator[] (int pos)
{
	if (pos < 0) return 0;
	if (pos >= _count) return 0;
	return _array[pos];
}

// =======================================================================
// METHOD refid_array::merge_and
// -----------------------------
// Combines a refid_array with another one, keeping only entries that
// appear in both arrays. For entries that remain, the weight of both
// entries is combined.
// =======================================================================

void refid_array::merge_and (refid_array &src)
{
	int i;
	int j;
	
	for (i=0; i < count(); ++i)
	{
		if ((j = src.find (*this[i])) < 0)
		{
			remove (i);
			--i;
		}
		else
		{
			_weight[i] += src.weight (j);
		}
	}
}

void refid_array::merge_combine (refid_array &src)
{
	int i;
	int j;
	
	for (i=0; i < count(); ++i)
	{
		if ((j = src.find (*this[i])) < 0)
		{
			if (_weight[i] == 0)
			{
				remove (i);
				--i;
			}
			else
			{
				_weight[i] = _weight[i] >> 1;
			}
		}
	}
	for (i=0; i < src.count(); ++i)
	{
		if (find (src[i]) < 0)
		{
			add (src[i], src.weight(i));
		}
	}		
}

void refid_array::add (triplet_refid r, triplet_weight w)
{
	int i;
	
	_grow();
	
	for (i=0; i<_count; ++i)
	{
		if (_array_id[i] > r)
		{
			memmove (_array_id + (i+1), array_id + i, (_count-i) * sizeof (triplet_refid));
			memmove (_array_weight + (i+1), _array_weight + i, (_count - i) * sizeof (triplet_weight));
			_array_id[i] = r;
			_array_weight[i] = w;
			++_count;
			return;
		}
	}
	++_count;
	_array_id[i] = r;
	_array_weight[i] = w;
}

void refid_array::remove (int pos)
{
	if (pos < 0) return;
	if (pos >= _count) return;
		
	if ((pos+1) >= _count) 
	{
		--_count;
		return;
	}
	memmove (_array_id + pos, _array_id + pos + 1, (_count - (pos+1)) * sizeof (triplet_refid));
	memmove (_array_weight + pos, array_weight + pos + 1, (_count - (pos+1)) * sizeof (triplet_weight));
	--_count;
}

void refid_array::_grow (void)
{
	if (! _size)
	{
		_array_id = (triplet_refid *) malloc (8 * sizeof (triplet_refid));
		_array_weight = (triplet_weight *) malloc (8 * sizeof (triplet_weight));
		_size = 8;
		_count = 0;
	}
	else
	{
		if (_count >= _size)
		{
			_size *= 2;
			_array_id = (triplet_refid *) realloc (_array_id, _size * sizeof (triplet_refid));
			_array_weight = (triplet_weight *) realloc (_array_id, _size * sizeof (triplet_weight));
		}
	}
}

int refid_array::find (triplet_refid r)
{
	int lowpos = 0;
	int hipos = _count;
	int c = (lowpos + hipos) / 2;
	
	while (lowpos != hipos)
	{
		if (_array_id[c] == r) return c;
		if (_array_id[c] < r)
		{
			lowpos = c;
			c = (lowpos + hipos) / 2;
		}
		else if (_array_id[c] > r)
		{
			hipos = c;
			c = (lowpos + hipos) / 2;
		}
	}
	return -1;
}

refid_array &refid_array::operator= (const refid_array &orig)
{
	if (_size)
	{
		free (_array_id);
		free (_array_weight);
		_count = _size = 0;
		_array_id = NULL;
		_array_weight = NULL;
	}
	
	for (int i=0; i<orig.count(); ++i)
	{
		add (orig[i], orig.weight(i));
	}
	
	return *this;
}

refid_array &refid_array::operator= (refid_array *orig)
{
	if (_size)
	{
		free (_array_id);
		free (_array_weight);
		_count = _size = 0;
		_array_id = NULL;
		_array_weight = NULL;
	}

	_array_id = orig->_array_id;
	_array_weight = orig->_array_weight;
	_count = orig->_count;
	_size = orig->_size;
	
	orig->_size = orig->_count = 0;
	orig->_array_id = orig->_array_weight = NULL;
	
	delete orig;
}

triplet_refid refid_array::operator[] (int pos)
{
	if (pos < 0) return 0;
	if (pos >= _count) return 0;
	return _array_id[pos];
}

triplet_weight refid_array::weight (int pos)
{
	if (pos < 0) return 0;
	if (pos >= _count) return 0;
	return _array_weight[pos];
}

void triplet_refdb::triplet_refdb (void)
{
}

void triplet_refdb::addref (triplet t, triplet_refid r)
{
	t &= 32767;
	int idx;
	int w;
	
	if ((idx = _array[t].find (r)) >= 0)
	{
		w = _array[t].weight (idx);
		_array[t].weight (idx, w+1);
	}
	else
	{
		_array[t].add (r, 1);
	}
}

void triplet_refdb::addref (tstring &str, triplet_refid r)
{
	for (int i=0; i<str.count(); ++i)
		addref (str[i], r);
}

refid_array *triplet_refdb::query_and (tstring &ts)
{
	refid_array *res;
	
	res = new refid_array;

	(*res) = _array[ts[0]];
	
	for (int i=1; i<ts.count(); ++i)
	{
		(*res).merge_and (_array[ts[i]]);
	}
	
	return res;
}

refid_array *triplet_refdb::query_combine (tstring &ts)
{
	refid_array *res;
	
	res = new refid_array;

	(*res) = _array[ts[0]];
	
	for (int i=1; i<ts.count(); ++i)
	{
		(*res).merge_combine (_array[ts[i]]);
	}
	
	return res;
}
