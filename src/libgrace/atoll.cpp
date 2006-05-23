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


