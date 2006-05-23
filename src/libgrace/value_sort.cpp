// ========================================================================
// value_sort.cpp: Keyed generic data storage class
//
// (C) Copyright 2003-2006 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam 
// ========================================================================

//      012345678 TAB-O-METER should measure 4
//      ^	^

#include <grace/value.h>
#include <grace/file.h>
#include <grace/stack.h>
#include <grace/strutil.h>
#include <grace/filesystem.h>

#include <stdio.h>
#include <string.h>

// ========================================================================
// FUNCTION labelSort (left, right, opt)
// -------------------------------------
// Returns true if the label left is of higher alphabetical value than
// that one of right. The opt string is ignored.
// ========================================================================
bool labelSort (value *l, value *r, const string &opt)
{
	return (::strcasecmp (l->name(), r->name()) > 0);
}

// ========================================================================
// FUNCTION valueSort (left, right, opt)
// -------------------------------------
// Returns true if the value of left is higher than the value of right.
// The opt string is ignored.
// ========================================================================
bool valueSort (value *l, value *r, const string &opt)
{
	return ((*l) > (*r));
}

// ========================================================================
// FUNCTION recordSort (left, right, opt)
// --------------------------------------
// Returns true if a specified child value of left is higher than the
// equivalent child value of right. The opt string contains the key
// of the child value. If it is empty, the first child value in the array
// is used for comparison.
// ========================================================================
bool recordSort (value *l, value *r, const string &opt)
{
	if (opt.strlen())
		return ((*l)[opt] > (*r)[opt]);
	return ((*l)[0] > (*r)[0]);
}

// Arabic values of Roman NVMERAL digits

#define romvalue(foo) (foo=='I') ? 1 : \
					  (foo=='V') ? 5 : \
					  (foo=='X') ? 10 : \
					  (foo=='L') ? 50 : \
					  (foo=='C') ? 100 : \
					  (foo=='D') ? 500 : \
					  (foo=='M') ? 1000 : 0

// ========================================================================
// INTERNAL FUNCTION romanNumeralFromString (str, offset)
// ------------------------------------------------------
// Tries to parse string str, starting at the provided offset, as a roman
// number. The number should be followed by whitespace or it should
// terminate the string. Invalid numbers return 0.
// ========================================================================
int romanNumeralFromString (const string &str, int offs = 0)
{
	int valueLastServed = 0;
	int valueBeforeSwitch = 0;
	int valueSoFar = 0;
	int resultValue = 0;
	int cursorValue;
	char cursorChar;
	int cursor;
	
	for (cursor=offs;
		 (!isspace(str[cursor])) && (cursor<str.strlen());
		 ++cursor)
	{
		// Resolve the current character to its arabic value
		cursorChar = str[cursor];
		cursorValue = romvalue (cursorChar);
		
		// Not a roman digit? Then this is not a roman number.
		if (! cursorValue) return 0;
		
		// If the arabic value of this digit is lower than the
		// one before, there is no going back for the earlier
		// digits. Their sum can thus be safely added to the tab.
		if (cursorValue < valueLastServed)
		{
			valueBeforeSwitch = resultValue;
			valueLastServed = cursorValue;
			valueSoFar = cursorValue;
		}
		
		// If this one is higher than the previous, it means
		// that the previous digits were subtractive precedents.
		else if (cursorValue > valueLastServed)
		{
			// Note that valueBeforeSwitch contains the sum total
			// at the time before we encountered the first
			// roman digit of the type that now turns out to have
			// been a precedent. The valueSoFar contains the sum
			// total only of the digits of that same type, so
			// we subtract that.
			resultValue += valueBeforeSwitch - valueSoFar;
							   
			// The current digit accumulator should now be reset,
			// except for the newly accumulated value of this
			// particular digit.
			valueSoFar = cursorValue;
			valueLastServed = cursorValue;
		}
		else
		{
			// Accumulate...
			valueSoFar += cursorValue;
		}
	}
	
	// If there's anything left, add it to the tab
	resultValue += valueSoFar;
	return resultValue;
}

// ========================================================================
// FUNCTION naturalSort (left, right, opt)
// ---------------------------------------
// This sorter compares two strings on flexible attributes, specifically:
//
// * All amounts of any kind of whitespace are equivalent
// * Comparisons are case-insensitive
// * The word 'the' is discarded
// * Decimal digit sequences are compared by their number value
// * Roman numeral sequences are also compared by their integer value
//
// If the opt string is not empty, a child value with that string as a key
// will be used for comparison instead of the direct value.
// ========================================================================
bool naturalSort (value *l, value *r, const string &opt)
{
	value left,right;
	string leftString, rightString;
	int leftInt, rightInt;
	int lpos, rpos;
	int res;
	bool withRecord = opt.strlen();
	
	char leftChar, rightChar;
	int leftLength, rightLength;
	
	// Get the string data (from the values or the designated
	// child objects)
	
	leftString = withRecord ? (*l)[opt].sval() : l->sval();
	rightString = withRecord ? (*r)[opt].sval() : r->sval();
	leftLength = leftString.strlen();
	rightLength = rightString.strlen();
	
	// Iterate over both strings
	for (lpos=0,rpos=0;
		 (lpos < leftLength) && (rpos < rightLength);
		 ++lpos,++rpos)
	{
		leftChar = tolower (leftString[lpos]);
		
		// Anticipate use of the word "the", which we will
		// skip [left string variation]
		if ((leftChar == 't')&&((lpos+2)<leftLength))
		{
			if ((tolower (leftString[lpos+1]) == 'h') &&
				(tolower (leftString[lpos+2]) == 'e'))
			{
				if (((lpos+3) == leftLength) ||
					(isspace (leftString[lpos+3])))
				{
					lpos += 4;
					if (lpos >= leftLength)
						break;
					leftChar = tolower (leftString[lpos]);
				}
			}
		}
		else if ((leftChar =='a')&&(lpos<leftLength))
		{
			if (((lpos+1) == leftLength) || 
			   (isspace (leftString[lpos+1])))
			{
				lpos += 2;
				if (lpos >= leftLength)
					break;
				leftChar = tolower (leftString[lpos]);
			}
		}

		rightChar = tolower (rightString[rpos]);

		// Anticipate use of the word "the", which we will
		// also skip [right string variation]
		if ((rightChar == 't')&&((rpos+2)<rightLength))
		{
			if ((tolower (rightString[rpos+1]) == 'h') &&
				(tolower (rightString[rpos+2]) == 'e'))
			{
				if (((rpos+3) == rightLength) ||
					(isspace (rightString[rpos+3])))
				{
					rpos += 4;
					if (rpos >= rightLength)
						break;
					rightChar = tolower (rightString[rpos]);
				}
			}
		}
		else if ((rightChar =='a')&&(lpos<rightLength))
		{
			if (((rpos+1) == rightLength) || 
			   (isspace (rightString[rpos+1])))
			{
				rpos += 2;
				if (rpos >= rightLength)
					break;
				rightChar = tolower (rightString[rpos]);
			}
		}
		
		// If the left _and_ right cursor point to a decimal
		// digit, we should compare by integer value, not
		// by ASCII.
		if (isdigit (leftChar) && isdigit (rightChar))
		{
			leftInt = atoi (leftString.str() + lpos);
			rightInt = atoi (rightString.str() + rpos);
			
			// If the values differ, that is a sortable offense
			if (leftInt != rightInt)
				return (leftInt > rightInt);
			
			// Skip all decimal digits
			while (isdigit (leftChar = leftString[++lpos]));
			while (isdigit (rightChar = rightString[++rpos]));
		
			// Reached the end of the left string? Worst case
			// scenario we also reached the end of the right
			// string and we are equivalent, in which case
			// either return value is factually incorrect while
			// practically irrelevant. In other cases, the
			// right string is valued higher than the left.
			if (lpos == leftLength)
			{
				return false;
			}
			if (rpos == rightLength)
			{
				return true;
			}
			// Skip one back, will be undone in the for-loop
			--lpos;
			--rpos;
		}
		
		// Any amount of any kind of whitespace should be treated
		// as equivalent.
		else if (isspace (leftChar))
		{
			// Spaces under both cursors
			if (isspace (rightChar))
			{
				// Skip over them
				while (isspace (leftChar = leftString[++lpos]));
				while (isspace (rightChar == rightString[++rpos]));
				
				// Do the length evaluation thingy
				if (lpos == leftLength)
				{
					return false;
				}
				if (rpos == rightLength)
				{
					return true;
				}
				--lpos;
				--rpos;
			}
			
			// data > whitespace
			else return false;
		}
		else if (isspace (rightChar))
		{
			// whitespace < data
			return true;
		}
		
		// Going on a limb here, see if we can evaluate both strings
		// from the cursor forward (until the next issue of Whitespace
		// Magazine) as ROMAN NVMERALS
		else if ( (leftInt = romanNumeralFromString (leftString, lpos)) &&
				  (rightInt = romanNumeralFromString (rightString, rpos)) )
		{
			// If they're not the same, that is a sortable offense
			if (leftInt != rightInt)
			{
				return (leftInt > rightInt);
			}
			
			// They are the same, skip over the NVMERALS
			while ( (lpos < leftLength) && (! isspace (leftString[lpos])) )
				++lpos;
			while ( (rpos < rightLength) && (! isspace (rightString[rpos])) )
				++rpos;

			// Draw conclusions if we ran out of text
			if (lpos == leftLength)
			{
				return false;
			}
			if (rpos == rightLength)
			{
				return true;
			}
		}
		
		// No special circumstances, do a regular compare
		else
		{
			if (leftChar != rightChar)
				return (leftChar > rightChar);
		}
	}
	
	// If we get here, either string ran out of text. If that wasn't
	// the left string, it is superior.
	return (lpos<leftLength);
}

// ========================================================================
// FUNCTION naturalLabelSort (left, right, opt)
// --------------------------------------------
// Equivalent to naturalSort, but compares the value labels.
// ========================================================================
bool naturalLabelSort (value *l, value *r, const string &opt)
{
	value left,right;
	string leftString, rightString;
	int leftInt, rightInt;
	int lpos, rpos;
	int res;
	bool withRecord = opt.strlen();
	
	char leftChar, rightChar;
	int leftLength, rightLength;
	
	// TODO: withRecord should resolve to an attribute
	leftString = l->label();
	rightString = r->label();
	leftLength = leftString.strlen();
	rightLength = rightString.strlen();
	
	// For algorithm description, see the almost similar
	// version upstairs ^^^^
	for (lpos=0,rpos=0;
		 (lpos < leftLength) && (rpos < rightLength);
		 ++lpos,++rpos)
	{
		leftChar = tolower (leftString[lpos]);
		if ((leftChar == 't')&&((lpos+2)<leftLength))
		{
			if ((tolower (leftString[lpos+1]) == 'h') &&
				(tolower (leftString[lpos+2]) == 'e'))
			{
				if (((lpos+3) == leftLength) ||
					(isspace (leftString[lpos+3])))
				{
					lpos += 4;
					if (lpos >= leftLength)
						break;
					leftChar = tolower (leftString[lpos]);
				}
			}
		}
		else if ((leftChar =='a')&&(lpos<leftLength))
		{
			if (((lpos+1) == leftLength) || 
			   (isspace (leftString[lpos+1])))
			{
				lpos += 2;
				if (lpos >= leftLength)
					break;
				leftChar = tolower (leftString[lpos]);
			}
		}
		rightChar = tolower (rightString[rpos]);
		if ((rightChar == 't')&&((rpos+2)<rightLength))
		{
			if ((tolower (rightString[rpos+1]) == 'h') &&
				(tolower (rightString[rpos+2]) == 'e'))
			{
				if (((rpos+3) == rightLength) ||
					(isspace (rightString[rpos+3])))
				{
					rpos += 4;
					if (rpos >= rightLength)
						break;
					rightChar = tolower (rightString[rpos]);
				}
			}
		}
		else if ((rightChar =='a')&&(rpos<rightLength))
		{
			if (((rpos+1) == rightLength) || 
			   (isspace (rightString[rpos+1])))
			{
				rpos += 2;
				if (rpos >= rightLength)
					break;
				rightChar = tolower (rightString[rpos]);
			}
		}
		if (isdigit (leftChar) && isdigit (rightChar))
		{
			leftInt = atoi (leftString.str() + lpos);
			rightInt = atoi (rightString.str() + rpos);
			
			if (leftInt != rightInt)
				return (leftInt > rightInt);
			
			while (isdigit (leftChar = leftString[++lpos]));
			while (isdigit (rightChar = rightString[++rpos]));
		
			if (lpos == leftLength)
			{
				return false;
			}
			if (rpos == rightLength)
			{
				return true;
			}
			--lpos;
			--rpos;
		}
		else if (isspace (leftChar))
		{
			if (isspace (rightChar))
			{
				while (isspace (leftChar = leftString[++lpos]));
				while (isspace (rightChar == rightString[++rpos]));
				if (lpos == leftLength)
				{
					return false;
				}
				if (rpos == rightLength)
				{
					return true;
				}
				--lpos;
				--rpos;
			}
			else return false;
		}
		else if (isspace (rightChar))
		{
			return true;
		}
		else if ( (leftInt = romanNumeralFromString (leftString, lpos)) &&
				  (rightInt = romanNumeralFromString (rightString, rpos)) )
		{
			if (leftInt != rightInt)
			{
				return (leftInt > rightInt);
			}
			while ( (lpos < leftLength) && (! isspace (leftString[lpos])) )
				++lpos;
			while ( (rpos < rightLength) && (! isspace (rightString[rpos])) )
				++rpos;

			if (lpos == leftLength)
			{
				return false;
			}
			if (rpos == rightLength)
			{
				return true;
			}
		}
		else
		{
			if (leftChar != rightChar)
				return (leftChar > rightChar);
		}
	}
	
	return (lpos<leftLength);
}

// ========================================================================
// METHOD value::sort (compare, opt)
// ---------------------------------
// Uses the function 'compare' with optional string argument 'opt' to
// quicksort the array. FIXME: quicksort stinks.
// ========================================================================
void value::sort (sortmethod cmpare, const string &opt)
{
	int hits = 1;
	value *v;
	
	for (int x=0; (hits) && ((x+1)<arraysz); ++x)
	{
		hits = 0;
		for (int y=0; ((y+1)<arraysz); ++y)
		{
			if (cmpare (array[y], array[y+1], opt))
			{
				v = array[y];
				array[y] = array[y+1];
				array[y+1] = v;
				++hits;
			}
		}
	}
}

void value::sort (sortmethod compare)
{
	string opt;
	
	sort (compare, opt);
}

