// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#include <grace/timeofday.h>
#include <grace/str.h>
#include <grace/strutil.h>
#include <grace/system.h>

#include <string.h>

extern int __system_local_timezone;
extern bool __system_timezone_set;



timeofday::timeofday (void)
{
	init ();
}



timeofday::timeofday (const timeofday &orig)
{
	copy (orig);
}



timeofday::timeofday (timeofday *orig)
{
	copy (*orig);
	delete (orig);
}



timeofday::timeofday (timeval orig)
{
	Tval.tv_sec 	= orig.tv_sec;
	Tval.tv_usec	= orig.tv_usec;
}



void timeofday::setTime (int 	stSeconds,
						 int 	stMinutes,
						 int 	stHours,
						 int 	stDay,
						 eMonth stMonth,
						 int 	stYear)
{
	time_t	t;				// new timestamp
	struct 	tm tm_time;		// The given time struct
	
	// This function will set altzone which is the differnce between UTC and
	// our local time.
	tzset ();
	
	tm_time.tm_sec	=	stSeconds;
	tm_time.tm_min	=	stMinutes;
	tm_time.tm_hour	=	stHours;
	tm_time.tm_mday	=	stDay;
	tm_time.tm_mon	=	(int) stMonth;	// enum: eMonth to --> int
	tm_time.tm_year	=	stYear - 1900;	// Year since epoch
	
	tm_time.tm_isdst = 	0;		        // TODO: this is WRONG
	

	// Create new time
	t = mktime (&tm_time);

	// Change the object it's timestamp,
	// after this you can use this new time on onother 
	// time object, by example: waitUntil (const timeofday &)...
	// 
	// t = prefered time
	// tv_usec will be default 0... no u_secs given 
	Tval.tv_sec 	= 	t;
	Tval.tv_usec 	= 	0;
	
}


// Wait until the given time has been reached,
// We re-calculate the remaining time every 10s
// The last seconds within 10s will not be checked.
//
// NOTE: When using this object the object's time
// will be updated every 10s when waiting. 
// DON'T USE THIS FUNCTION WHEN YOU USE THIS OBJECT 
// as a TIMESTAMP 
void timeofday::waitUntil (const timeofday &destTime)
{
	// Remaining time
	timeofday	TRemain;
		
	// remaining usecs
	unsigned 
	long long 	usecRemain;


	// First initialize the current time
	
	this->init ();
	
	while (true)
	{
		// If the current time is greater than the given time
		// we should not wait anymore
		if (*this >= destTime)
		{
			return;
		}
			
		TRemain 	= destTime - *this;
		usecRemain 	= TRemain.getusec ();
		
		// Sleep 10s or The last miliseconds
		if (usecRemain > 10000000LL) {
			__musleep (10000LL); 
		} else {
			__musleep (TRemain.getusec () / 1000LL);
			return;
		};
			
		this->init ();
	}
}


void timeofday::init (void)
{
	gettimeofday (&Tval, NULL);
	
	// Get timezone information
	if (! __system_timezone_set)
	{
		struct tm tmp;
		time_t ti;
		
		ti = core.time.now ();
		localtime_r (&ti, &tmp);
#ifdef HAVE_GMTOFF
		__system_local_timezone = tmp.tm_gmtoff;
#else
		struct tm gtmp;
		gmtime_r (&ti, &gtmp);
		__system_local_timezone = mktime (&tmp) - mktime (&gtmp);
#endif
		__system_timezone_set = true;
	}
	
	//Tval.tv_sec += __system_local_timezone;
}




timeval timeofday::operator- (const timeofday &other) const
{
	unsigned long long current;
	unsigned long long given;
	unsigned long long ntime;
	struct timeval		 nTime;
												
	current = (Tval.tv_sec * 1000000LL) 
				 + Tval.tv_usec;
									 
	given   = (other.Tval.tv_sec * 1000000LL) 
				 + other.Tval.tv_usec;

	ntime = current - given;

	// Parse the new time into the nTime structure
	nTime.tv_sec 	= ntime / 1000000LL;
	nTime.tv_usec 	= ntime % 1000000LL;
	
	return nTime;
}

timeofday *timeofday::operator- (timeval sub) const
{
	timeofday *res = new timeofday (*this);
	(*res) -= sub;
	return res;
}

timeval timeofday::operator+ (const timeofday &other) const
{
	unsigned long long current;
	unsigned long long given;
	unsigned long long ntime;
	struct timeval		 nTime;
												
	current = (Tval.tv_sec * 1000000LL) 
				 + Tval.tv_usec;
									 
	given   = (other.Tval.tv_sec * 1000000LL) 
				 + other.Tval.tv_usec;

	ntime = current + given;

	// Parse the new time into the nTime structure
	nTime.tv_sec 	= ntime / 1000000LL;
	nTime.tv_usec 	= ntime % 1000000LL;
	
	return nTime;
}

timeofday *timeofday::operator+ (timeval sub) const
{
	timeofday *res = new timeofday (*this);
	(*res) -= sub;
	return res;
}

timeofday &timeofday::operator += (timeval other)
{
	unsigned long long current;
	unsigned long long given;
	unsigned long long ntime;
													
	current = (Tval.tv_sec * 1000000LL) 
				 + Tval.tv_usec;
									 
	given   = (other.tv_sec * 1000000LL) 
				 + other.tv_usec;

	ntime = current + given;

	// Parse the new time into the nTime structure
	Tval.tv_sec 	= ntime / 1000000LL;
	Tval.tv_usec 	= ntime % 1000000LL;

	return *this;
}

timeofday &timeofday::operator -= (timeval other)
{
	unsigned long long current;
	unsigned long long given;
	unsigned long long ntime;
													
	current = (Tval.tv_sec * 1000000LL) 
				 + Tval.tv_usec;
									 
	given   = (other.tv_sec * 1000000LL) 
				 + other.tv_usec;

	ntime = current - given;

	// Parse the new time into the nTime structure
	Tval.tv_sec 	= ntime / 1000000LL;
	Tval.tv_usec 	= ntime % 1000000LL;

	return *this;
}
