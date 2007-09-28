#include "platform.h"

#include <grace/timestamp.h>
#include <grace/system.h>
#include <grace/value.h>
#include <grace/strutil.h>
#include <string.h>

int __system_local_timezone = 0;
bool __system_timezone_set = false;

// ========================================================================
// CONSTRUCTOR timestamp
// ========================================================================
timestamp::timestamp (void)
{
	init();
}

// ========================================================================
// CONSTRUCTOR timestamp
// ========================================================================
timestamp::timestamp (const timestamp &orig)
{
	copy (orig);
}

timestamp::timestamp (timestamp *orig)
{
	copy (*orig);
	delete orig;
}

timestamp::timestamp (time_t orig)
{
	init ();
	tvval.tv_sec  = orig;
	tvval.tv_usec = 0;
}


timestamp::timestamp (timeval orig)
{
	init ();
	tvval.tv_sec  = orig.tv_sec;
	tvval.tv_usec = orig.tv_usec;
}

// ========================================================================
// METHOD ::init
// ========================================================================
void timestamp::init (void)
{
	tmval.tm_sec = tmval.tm_min = tmval.tm_hour =
	tmval.tm_mday = tmval.tm_mon = tmval.tm_year =
	tmval.tm_wday = tmval.tm_yday = tmval.tm_isdst = 0;
	tmset = false;
	timezone = 0;
	tvval.tv_sec = 0;
	tvval.tv_usec = 0;
	
	stval.crop (0);
	stset = false;
	stformat.crop (0);
	
	if (! __system_timezone_set)
	{
		struct tm tmp;
		time_t ti;
		
		ti = kernel.time.now();
		localtime_r (&ti, &tmp);
#ifdef HAVE_GMTOFF
		tmval.tm_gmtoff = 0;
		__system_local_timezone = tmp.tm_gmtoff;
#else
		struct tm gtmp;
		gmtime_r (&ti, &gtmp);
		__system_local_timezone = mktime (&tmp) - mktime (&gtmp);
#endif
		__system_timezone_set = true;
	}
}

// ========================================================================
// METHOD ::copy
// ========================================================================
void timestamp::copy (const timestamp &orig)
{
	init();
	
	tvval.tv_sec = orig.tvval.tv_sec;
	tvval.tv_usec = orig.tvval.tv_usec;
	
	if (orig.tmset)
	{
		memmove (&tmval, &(orig.tmval), sizeof (struct tm));
		tmset = true;
	}
	
	if (orig.stset)
	{
		stset = orig.stset;
		stformat = orig.stformat;
		stval = orig.stval;
	}
}

// ========================================================================
// METHOD ::unixtime
// ========================================================================
time_t timestamp::unixtime (void) const
{
	return (tvval.tv_sec + timezone);
}

// ========================================================================
// METHOD ::tm
// ========================================================================
const struct tm &timestamp::tm (void) const
{
	if (tmset) return tmval;
	
	time_t tmp;
	tmp = tvval.tv_sec;// - timezone;
	
	localtime_r ((const time_t *) &tmp, (struct tm *) &tmval);
	return tmval;
}

const struct tm &timestamp::tm (void)
{
	if (tmset) return tmval;
	
	time_t tmp;
	tmp = tvval.tv_sec;// - timezone;
	
	localtime_r ((const time_t *) &tmp, (struct tm *) &tmval);
	tmset = true;
	return tmval;
}

// ========================================================================
// METHOD ::rfc822
// ========================================================================
const string &timestamp::rfc822 (void)
{
	return format ("%a, %d %b %Y %H:%M:%S %z");
}

const string &timestamp::rfc822 (void) const
{
	return format ("%a, %d %b %Y %H:%M:%S %z");
}

// ========================================================================
// METHOD ::ctime
// ========================================================================
const string &timestamp::ctime (void) const
{
	return format ("%a %b %e %H:%M:%S %Y");
}

const string &timestamp::ctime (void)
{
	return format ("%a %b %e %H:%M:%S %Y");
}

// ========================================================================
// METHOD ::iso
// ========================================================================
const string &timestamp::iso (void)
{
	return format ("%Y-%m-%dT%H:%M:%S");
}

const string &timestamp::iso (void) const
{
	return format ("%Y-%m-%dT%H:%M:%S");
}

// ========================================================================
// METHOD ::isodate
// ========================================================================
const string &timestamp::isodate (void)
{
	return format ("%Y-%m-%d");
}

// ========================================================================
// METHOD ::format
// ========================================================================
const string &timestamp::format (const string &formatstr) const
{
	if (stset && (stformat == formatstr)) return stval;
		
	char tmp[256];
	tm();
	
	strftime (tmp, 255, formatstr.str(), &tmval);
	(string &) stval = tmp;
	
	return stval;
}

const string &timestamp::format (const string &formatstr)
{
	if (stset && (stformat == formatstr)) return stval;
	
	stset = true;
	stformat = formatstr;
	
	char tmp[256];
	tm();
	
	strftime (tmp, 255, stformat.str(), &tmval);
	stval = tmp;
	
	return stval;
}

// ========================================================================
// METHOD ::iso
// ========================================================================
void timestamp::iso (const string &isodate)
{
	string datepart;
	string timepart;
	string tzpart;
	string in = isodate;
	
	if (in.strchr ('T') >= 0)
	{
		datepart = in.cutat ('T');
		if (in.strchr ('Z') >= 0)
		{
			timepart = in.cutat ('Z');
			tzpart = in;
		}
		else timepart = in;
	}
	else datepart = in;

	if (datepart.strlen() != 10) return;
	init ();
	
	tmval.tm_year = ::atoi (datepart.str()) - 1900;
	tmval.tm_mon = ::atoi (datepart.str()+5) - 1;
	tmval.tm_mday = ::atoi (datepart.str()+8);
	
	if (timepart.strlen() == 8)
	{
		tmval.tm_hour = ::atoi (timepart.str());
		tmval.tm_min = ::atoi (timepart.str() + 3);
		tmval.tm_sec = ::atoi (timepart.str() + 6);
	}
#ifdef HAVE_GMTOFF
	tmval.tm_gmtoff = __system_local_timezone;
#endif
    //timezone = __system_local_timezone;
	tmset = false;
	tvval.tv_sec = mktime (&tmval);
	tvval.tv_usec = 0;
}

#define MONCMP(str,pa,pb) (((*str)==pa) && ( (*((str)+2)) == pb) )

// ========================================================================
// METHOD ::ctime
// ========================================================================
void timestamp::ctime (const string &timestr)
{
    //            1  1  1  2
    // 0...4...9..2..5..8..1
	// Mon Aug  2 11:38:02 2004
	
	if (timestr.strlen() < 24) return;
	const char *tstr = timestr.str();
	
	init ();
	
	if (MONCMP(tstr+4,'J','n')) tmval.tm_mon = 0;
	else if (MONCMP(tstr+4,'F','b')) tmval.tm_mon = 1;
	else if (MONCMP(tstr+4,'M','r')) tmval.tm_mon = 2;
	else if (MONCMP(tstr+4,'A','r')) tmval.tm_mon = 3;
	else if (MONCMP(tstr+4,'M','y')) tmval.tm_mon = 4;
	else if (MONCMP(tstr+4,'J','n')) tmval.tm_mon = 5;
	else if (MONCMP(tstr+4,'J','l')) tmval.tm_mon = 6;
	else if (MONCMP(tstr+4,'A','g')) tmval.tm_mon = 7;
	else if (MONCMP(tstr+4,'S','p')) tmval.tm_mon = 8;
	else if (MONCMP(tstr+4,'O','t')) tmval.tm_mon = 9;
	else if (MONCMP(tstr+4,'N','v')) tmval.tm_mon = 10;
	else if (MONCMP(tstr+4,'D','c')) tmval.tm_mon = 11;
	
	tmval.tm_mday = ::atoi (tstr+9);
	tmval.tm_hour = ::atoi (tstr+12);
	tmval.tm_min = ::atoi (tstr+15);
	tmval.tm_sec = ::atoi (tstr+18);
	tmval.tm_year = ::atoi (tstr+21) - 1900;
#ifdef HAVE_GMTOFF
	tmval.tm_gmtoff = __system_local_timezone;
#endif
    //timezone = __system_local_timezone;
	tmset = true;
	tvval.tv_sec = mktime (&tmval);
	tvval.tv_usec = 0;
}

// ========================================================================
// METHOD ::rfc822
// ========================================================================
void timestamp::rfc822 (const string &timestr)
{
	value splt;
	const char *mon;
	const char *tim;
	const char *tof;
	
	// 0    1 2   3    4        5     [6]
	// Mon, 4 Apr 2003 14:23:15 +0200 (CEST)
	
	splt = strutil::split (timestr, ' ');
	
	init();
	
	tmval.tm_mday = splt[1].ival();
	
	if (splt[2].sval().strlen() > 2)
	{
		mon = splt[2].cval();
		
		if (MONCMP(mon,'J','n')) tmval.tm_mon = 0;
		else if (MONCMP(mon,'F','b')) tmval.tm_mon = 1;
		else if (MONCMP(mon,'M','r')) tmval.tm_mon = 2;
		else if (MONCMP(mon,'A','r')) tmval.tm_mon = 3;
		else if (MONCMP(mon,'M','y')) tmval.tm_mon = 4;
		else if (MONCMP(mon,'J','n')) tmval.tm_mon = 5;
		else if (MONCMP(mon,'J','l')) tmval.tm_mon = 6;
		else if (MONCMP(mon,'A','g')) tmval.tm_mon = 7;
		else if (MONCMP(mon,'S','p')) tmval.tm_mon = 8;
		else if (MONCMP(mon,'O','t')) tmval.tm_mon = 9;
		else if (MONCMP(mon,'N','v')) tmval.tm_mon = 10;
		else if (MONCMP(mon,'D','c')) tmval.tm_mon = 11;
	}
	
	tmval.tm_year = splt[3].ival();
	
	if (splt[4].sval().strlen() > 6)
	{
		tim = splt[4].cval();
	
		tmval.tm_hour = ::atoi (tim);
		tmval.tm_min = ::atoi (tim+3);
		tmval.tm_sec = ::atoi (tim+6);
	}
	
	int off;
	
	if (splt[5].sval().strlen() > 4)
	{
		bool negative = false;
		int hours;
		int minutes;
		int seconds;
		
		tof = splt[5].cval();
		if ((*tof) == '-') negative = true;
		if (tof[1] == '1') hours = 10;
		else hours = 0;
		hours += (tof[2] - '0');
		minutes = ::atoi (tof+3);
		
		seconds = (3600 * hours) + minutes;
		if (negative) seconds = -seconds;
		
#ifdef HAVE_GMTOFF
		tmval.tm_gmtoff = seconds;
#endif
		//timezone = seconds;
	}

	tmset = true;
	tvval.tv_sec = mktime (&tmval);
	tvval.tv_usec = 0;
}

// ========================================================================
// METHOD ::unxtime
// ========================================================================
void timestamp::unixtime (time_t in)
{
	init();
	tvval.tv_sec = in;
	tvval.tv_usec = 0;
}

// ========================================================================
// METHOD ::timeofday
// ========================================================================
void timestamp::timeofday (timeval in)
{
	init();
	tvval 	= in;
	//tvval.tv_sec += __system_local_timezone;
}

// ========================================================================
// METHOD ::tm
// ========================================================================
void timestamp::tm (const struct tm &in)
{
	init();
	memmove (&tmval, &in, sizeof (struct tm));
	tmset = true;
	tvval.tv_sec = mktime (&tmval);
	tvval.tv_usec = 0;
}

void timestamp::tm (const struct tm *in)
{
	tm (*in);
}

// ========================================================================
// METHOD ::operator+=
// ========================================================================
timestamp &timestamp::operator+= (time_t add)
{
	time_t now;
	now = tvval.tv_sec;
	init();
	tvval.tv_sec = now + add;
	return *this;
}

timestamp &timestamp::operator+= (timeval add)
{
	unsigned long long current;
	unsigned long long given;
	unsigned long long ntime;
													
	init();											
													
	current = (tvval.tv_sec * 1000000LL) 
				 + tvval.tv_usec;
									 
	given   = (add.tv_sec * 1000000LL) 
				 + add.tv_usec;

	ntime = current + given;

	// Parse the new time into the nTime structure
	tvval.tv_sec 	= ntime / 1000000LL;
	tvval.tv_usec 	= ntime % 1000000LL;
	
	return *this;
}

// ========================================================================
// METHOD ::operator-=
// ========================================================================
timestamp &timestamp::operator-= (time_t sub)
{
	time_t now;
	now = tvval.tv_sec;
	init();
	tvval.tv_sec = now - sub;
	return *this;
}

timestamp &timestamp::operator-= (timeval sub)
{	
	unsigned long long current;
	unsigned long long given;
	unsigned long long ntime;
													
	init();													
													
	current = (tvval.tv_sec * 1000000LL) 
				 + tvval.tv_usec;
									 
	given   = (sub.tv_sec * 1000000LL) 
				 + sub.tv_usec;

	ntime = current - given;

	// Parse the new time into the nTime structure
	tvval.tv_sec 	= ntime / 1000000LL;
	tvval.tv_usec 	= ntime % 1000000LL;

	return *this;
}

// ========================================================================
// METHOD ::operator-
// ========================================================================
timeval timestamp::operator- (const timestamp &theother) const
{
	unsigned long long 	current;
	unsigned long long 	given;
	unsigned long long 	ntime;
	struct timeval		nTime;
												
	current = (tvval.tv_sec * 1000000LL) 
				 + tvval.tv_usec;
									 
	given   = (theother.tvval.tv_sec * 1000000LL) 
				 + theother.tvval.tv_usec;

	ntime = current - given;

	// Parse the new time into the nTime structure
	nTime.tv_sec 	= ntime / 1000000LL;
	nTime.tv_usec 	= ntime % 1000000LL;
	
	// ::printf ("current: %llu\n", current);
	// ::printf ("given: %llu\n", given);
	// ::printf ("result: %llu\n", ntime);
	
	return nTime;
}

// ========================================================================
// METHOD ::operator+
// ========================================================================
timeval timestamp::operator+ (const timestamp &theother) const
{
	unsigned long long current;
	unsigned long long given;
	unsigned long long ntime;
	struct timeval	   nTime;
												
	current = (tvval.tv_sec * 1000000LL) 
				 + tvval.tv_usec;
									 
	given   = (theother.tvval.tv_sec * 1000000LL) 
				 + theother.tvval.tv_usec;

	ntime = current + given;

	// Parse the new time into the nTime structure
	nTime.tv_sec 	= ntime / 1000000LL;
	nTime.tv_usec 	= ntime % 1000000LL;
	
	return nTime;
}

// ========================================================================
// METHOD ::operator-
// ========================================================================
timestamp *timestamp::operator- (time_t sub) const
{
	timestamp *res = new timestamp (*this);
	(*res) -= sub;
	return res;
}

timestamp *timestamp::operator- (timeval sub) const
{
	timestamp *res = new timestamp (*this);
	(*res) -= sub;
	return res;
}

// ========================================================================
// METHOD ::delta
// ========================================================================
int timestamp::delta (const timestamp &other, intervaltype ikind) const
{
	if (ikind > 0)
	{
		if ((*this) < other)
		{
			return ((other.tvval.tv_sec - tvval.tv_sec) / ikind);
		}
		return ((tvval.tv_sec - other.tvval.tv_sec) / ikind);
	}
	
	if (ikind == months)
	{
		int mymonths;
		int hismonths;
		bool mydaybeforehis = false;
		
		tm();
		other.tm();
		
		mymonths = (12 * tmval.tm_year) + tmval.tm_mon;
		hismonths = (12 * other.tmval.tm_year) + other.tmval.tm_mon;
		if (tmval.tm_mday < other.tmval.tm_mday) mydaybeforehis = true;
		
		if (mymonths < hismonths)
		{
			if (mydaybeforehis)
			{
				return (hismonths - mymonths);
			}
			return ((hismonths - mymonths)-1);
		}
		if (mydaybeforehis)
		{
			return (mymonths - hismonths) -1;
		}
		return (mymonths - hismonths);
	}
	
	return 0;
}

// ========================================================================
// METHOD ::interval
// ========================================================================
time_t timestamp::interval (int iv, intervaltype ikind)
{
	if (ikind>0) return (ikind * iv);
	if (ikind == months) return (30 * days * iv);
	if (ikind == years) return (365 * days * iv);
	return 0;
}

