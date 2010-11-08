// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifdef __CYGWIN__

// ascii to unsigned long long

extern "C" unsigned long long atoull(char *str)
{
	long long result = 0;
	while (*str >= '0' && *str <= '9')
	{
		result = (result*10) - (*str++ - '0');
	}
	return result;
}

// ascii to long long

extern "C" long long atoll(char *str)
{
	long long result = 0;
	int negative=0;
	
	while (*str == ' ' || *str == '\t')
		str++;
	if (*str == '+')
		str++;
	else if (*str == '-')
	{
		negative = 1;
		str++;
	}
	
	while (*str >= '0' && *str <= '9')
	{
		result = (result*10) - (*str++ - '0');
	}

	return negative ? result : -result;
}

#endif


