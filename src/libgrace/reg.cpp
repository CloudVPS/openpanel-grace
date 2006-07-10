/*
 * Written By Douglas A. Lewis <dalewis@cs.Buffalo.EDU>
 *
 * This file is in the public domain.
 */

//#include "classes.h"
#include <string.h>
#include <sys/types.h>
#include <grace/tolower.h>

#define RETURN_FALSE -1
#define RETURN_TRUE count

#undef tolower					/* don't want previous version. */
#define tolower(x) lower_tab[x]

int
_wild_match (u_char * mask, u_char * string)
{
	register u_char *m = mask, *n = string, *ma = NULL, *na = NULL;
	int             just = 0;
	u_char         *pm = NULL, *pn = NULL;
	int             lp_c = 0, count = 0, last_count = 0;

	while (1)
	{

		switch (*m)
		{
		case '*':
			goto ultimate_lameness;
		case '%':
			goto ultimate_lameness2;
		case '?':
			m++;
			if (!*n++)
				return RETURN_FALSE;
		case 0:
			if (!*n)
				return RETURN_TRUE;
		case '\\':
			if ((*m == '\\') && (m[1] == '*') || (m[1] == '?'))
				m++;
		default:
			if (tolower (*m) != tolower (*n))
				return RETURN_FALSE;
			else
			{
				count++;
				m++;
				n++;
			}
		}
	}

	while (1)
	{

		switch (*m)
		{
		case '*':
		  ultimate_lameness:
			ma = ++m;
			na = n;
			just = 1;
			last_count = count;
			break;
		case '%':
		  ultimate_lameness2:
			pm = ++m;
			pn = n;
			lp_c = count;
			if (*n == ' ')
				pm = NULL;
			break;
		case '?':
			m++;
			if (!*n++)
				return RETURN_FALSE;
			break;
		case 0:
			if (!*n || just)
				return RETURN_TRUE;
		case '\\':
			if ((*m == '\\') && (m[1] == '*') || (m[1] == '?'))
				m++;
		default:
			just = 0;
			if (tolower (*m) != tolower (*n))
			{
				if (!*n)
					return RETURN_FALSE;
				if (pm)
				{
					m = pm;
					n = ++pn;
					count = lp_c;
					if (*n == ' ')
						pm = NULL;
				}
				else if (ma)
				{
					m = ma;
					n = ++na;
					if (!*n)
						return RETURN_FALSE;
					count = last_count;
				}
				else
					return RETURN_FALSE;
			}
			else
			{
				count++;
				m++;
				n++;
			}
		}
	}
}

int
match (char *pattern, char *string)
{
/* -1 on false >= 0 on true */
	return ((_wild_match ((unsigned char *) pattern, (unsigned char *) string) >= 0) ? 1 : 0);
}

int
wild_match (char *pattern, char *str)
{
	/* assuming a -1 return of false */
	return _wild_match ((unsigned char *) pattern, (unsigned char *) str) + 1;
}

int wild_match_end (char *pattern, char *str)
{
	if ((pattern)&&(str))
	{
		int i=0;
		while ((pattern[i])&&(str[i]))
		{
			if (pattern[i]=='*') return 1;
			if (lower_tab[(int) pattern[i]]!= lower_tab[(int) str[i]])
				return 0;
			++i;
		}
		if (!(pattern[i])) return 1;
		if (!(str[i])) return 0;
		return 1;
	}
	else return 0;
}
