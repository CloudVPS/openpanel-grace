#ifndef _TIMESTAMP_H
#define _TIMESTAMP_H 1

#include <grace/str.h>

#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

/// Useful values for indicating intervals.
enum intervaltype
{
	seconds = 1,
	minutes = 60,
	hours = 3600,
	days = 86400,
	weeks = (86400*7),
	months = -1,
	years = -2
};

/// Representation of a date/time value.
class timestamp
{
public:
					 /// Constructor.
					 timestamp (void);
					 
					 /// Copy-constructor.
					 timestamp (const timestamp &);
					 
					 /// Copy-constructor (deletes original).
					 timestamp (timestamp *);
					 
					 /// Constructor (init from unix timestamp).
					 timestamp (time_t);
					 
					 /// Constructor (init from timeval)
					 timestamp (timeval);
	
					 /// Internal initialization method.
	void			 init (void);
	
					 /// Cast to unix timestamp.
	time_t			 unixtime (void) const;
	
					 /// Cast to unix time structure.
	const struct tm	&tm (void) const;
	const struct tm &tm (void);
	
					 /// Convert to RFC822 format.
	const string	&rfc822 (void) const;

					 /// Convert to RFC822 format.
	const string	&rfc822 (void);

					 /// Convert to ctime() format.
	const string	&ctime (void) const;

					 /// Convert to ctime() format.
	const string	&ctime (void);
	
					 /// Convert to ISO (yyyy-mm-ddThh:mm:ss) format.
	const string	&iso (void);

					 /// Convert to ISO (yyyy-mm-ddThh:mm:ss) format.
	const string	&iso (void) const;

					 /// Convert to ISO (yyyy-mm-ddThh:mm:ss) format.
	const string	&isodate (void);

					 /// Convert to ISO (yyyy-mm-ddThh:mm:ss) format.
	const string	&isodate (void) const;

					 /// Convert to freeform string.
	const string	&format (const string &) const;

					 /// Convert to freeform string. Uses the strftime
					 /// conventions:
					 ///   - %%A localized full weekday.
					 ///   - %%a localized short weekday.
					 ///   - %%B localized full month name.
					 ///   - %%b localized short month name.
					 ///   - %%C century (two digits).
					 ///   - %%c localized ctime().
					 ///   - %%D alias for %%m/%d/%y
					 ///   - %%d day of the month (01-31).
					 ///   - %%e day of the month (1-31).
					 ///   - %%F alias for %%Y-%m-%d
					 ///   - %%G this week's year (4 digits)
					 ///   - %%g this week's year (2 digits)
					 ///   - %%H hour (00-23)
					 ///   - %%I hour (01-12)
					 ///   - %%k hour (0-23) single digits preceded by whitespace.
					 ///   - %%l hour (1-12)
					 ///   - %%M minutes (00-59)
					 ///   - %%m month (01-12)
					 ///   - %%p localized am/pm string
					 ///   - %%S seconds (00-59)
					 ///   - %%s unix timestamp
	const string	&format (const string &);
	
					 /// Calculate interval with another timestamp.
					 /// \param ts The other timestamp.
					 /// \param ikind The kind of interval to measure.
	int				 delta (const timestamp &ts, intervaltype ikind = seconds) const;
	
					 /// Set from a unix timestamp.
	void			 unixtime (time_t);
	
					 /// Set from a timeofday.
	void 			 timeofday (timeval in);
	
					 /// Convert from an iso-date (+time)
	void			 iso (const string &);
	
					 /// Set from a unix time structure.
	void			 tm (const struct tm &);

					 /// Set from a unix time structure.
	void			 tm (const struct tm *);

					 /// Set from an RFC822 string.
	void			 rfc822 (const string &);
	
					 /// Set from a ctime string.
	void			 ctime (const string &);
	
					 /// Copy from another object.
	void			 copy (const timestamp &);
	
					 /// Static method for converting an interval to seconds.
					 /// \param iv Number of time units.
					 /// \param ikind Type of time units.
	static time_t	 interval (int iv, intervaltype ikind = seconds);

	timestamp		&operator= (time_t orig)
					 {
					 	unixtime (orig);
					 	return *this;
					 }
	timestamp		&operator= (timeval orig)
					 {
					 	this->timeofday (orig);
					 	return *this;
					 }
	timestamp		&operator= (const timestamp &orig)
					 {
					 	copy (orig);
					 	return *this;
					 }
	timestamp		&operator= (timestamp *orig)
					 {
					 	copy (*orig);
					 	delete orig;
					 	return *this;
					 }
					 
	timestamp		&operator= (unsigned long long orig)
					 {
					 	// Parse the new time into the nTime structure
						tvval.tv_sec 	= orig / 1000000LL;
						tvval.tv_usec 	= orig % 1000000LL;
					 	
					 	return *this;
					 }
					
					 /// Get current time
					 /// in useconds
					 /// \return unsigned long long uSeconds
	unsigned 
	long long		 getusec( void )
					 {
						return ((tvval.tv_sec * 1000000LL) 
							 + tvval.tv_usec);
					 }
	
	int				 month (void) { return tm().tm_mon +1; }
	int				 mday (void) { return tm().tm_mday +1; }
	int				 year (void) { return tm().tm_year +1900; }
	int				 hour (void) { return tm().tm_hour; }
	int				 minute (void) { return tm().tm_min; }
	int				 second (void) { return tm().tm_sec; }
					 
	timestamp		&operator += (time_t);
	timestamp		&operator -= (time_t);
	timestamp		&operator += (timeval);
	timestamp		&operator -= (timeval);
	
					 
	timestamp		*operator+ (time_t ti) const
					 {
					 	timestamp *res = new timestamp (*this);
					 	(*res) += ti;
					 	return res;
					 }
	timestamp		*operator+ (timeval ti) const
					 {
					 	timestamp *res = new timestamp (*this);
					 	(*res) += ti;
					 	return res;
					 }
					 
	timeval			 operator- (const timestamp &) const;
	
	timeval			 operator+ (const timestamp &) const;
	
	timestamp		*operator- (time_t) const;
	timestamp		*operator- (timeval) const;
	
	bool			 operator< (const timestamp &other) const
					 {
					 	unsigned long long current;
						unsigned long long given;
											
						current = (tvval.tv_sec * 1000000LL) 
							   + tvval.tv_usec;
									 
						given   = (other.tvval.tv_sec * 1000000LL) 
							   + other.tvval.tv_usec;
											
						return ( current < given );						
					 }
	bool			 operator<= (const timestamp &other) const
					 {
					 	unsigned long long current;
						unsigned long long given;
											
						current = (tvval.tv_sec * 1000000LL) 
							   + tvval.tv_usec;
									 
						given   = (other.tvval.tv_sec * 1000000LL) 
							   + other.tvval.tv_usec;
											
						return ( current <= given );						
					 }
	bool			 operator== (const timestamp &other) const
					 {
					 	unsigned long long current;
						unsigned long long given;
											
						current = (tvval.tv_sec * 1000000LL) 
							   + tvval.tv_usec;
									 
						given   = (other.tvval.tv_sec * 1000000LL) 
							   + other.tvval.tv_usec;
											
						return ( current == given );						
					 }
					 
	bool			 operator!= (const timestamp &other) const
					 {
					 	unsigned long long current;
						unsigned long long given;
											
						current = (tvval.tv_sec * 1000000LL) 
							   + tvval.tv_usec;
									 
						given   = (other.tvval.tv_sec * 1000000LL) 
							   + other.tvval.tv_usec;
											
						return ( current != given );						
					 }

	bool			 operator>= (const timestamp &other) const
					 {
					 	unsigned long long current;
						unsigned long long given;
											
						current = (tvval.tv_sec * 1000000LL) 
							   + tvval.tv_usec;
									 
						given   = (other.tvval.tv_sec * 1000000LL) 
							   + other.tvval.tv_usec;
											
						return ( current >= given );						
					 }
	bool			 operator> (const timestamp &other) const
					 {
					 	unsigned long long current;
						unsigned long long given;
											
						current = (tvval.tv_sec * 1000000LL) 
							   + tvval.tv_usec;
									 
						given   = (other.tvval.tv_sec * 1000000LL) 
							   + other.tvval.tv_usec;
											
						return ( current > given );						
					 }

	bool			 operator< (timeval other) const
					 {
					 	unsigned long long current;
						unsigned long long given;
											
						current = (tvval.tv_sec * 1000000LL) 
							   + tvval.tv_usec;
									 
						given   = (other.tv_sec * 1000000LL) 
							   + other.tv_usec;
											
						return ( current < given );						
					 }
	bool			 operator<= (timeval other) const
					 {
					 	unsigned long long current;
						unsigned long long given;
											
						current = (tvval.tv_sec * 1000000LL) 
							   + tvval.tv_usec;
									 
						given   = (other.tv_sec * 1000000LL) 
							   + other.tv_usec;
											
						return ( current <= given );						
					 }
	bool			 operator== (timeval other) const
					 {
					 	unsigned long long current;
						unsigned long long given;
											
						current = (tvval.tv_sec * 1000000LL) 
							   + tvval.tv_usec;
									 
						given   = (other.tv_sec * 1000000LL) 
							   + other.tv_usec;
											
						return ( current == given );						
					 }
	bool			 operator>= (timeval other) const
					 {
					 	unsigned long long current;
						unsigned long long given;
											
						current = (tvval.tv_sec * 1000000LL) 
							   + tvval.tv_usec;
									 
						given   = (other.tv_sec * 1000000LL) 
							   + other.tv_usec;
											
						return ( current >= given );						
					 }
	bool			 operator> (timeval other) const
					 {
					 	unsigned long long current;
						unsigned long long given;
											
						current = (tvval.tv_sec * 1000000LL) 
							   + tvval.tv_usec;
									 
						given   = (other.tv_sec * 1000000LL) 
							   + other.tv_usec;
											
						return ( current > given );						
					 }
	
	
	bool			 operator< (time_t other) const
					 {	
						return ( tvval.tv_sec < other );						
					 }
	bool			 operator<= (time_t other) const
					 {
						return ( tvval.tv_sec <= other );
					 }
	bool			 operator== (time_t other) const
					 {
						return ( tvval.tv_sec == other );
					 }
	bool			 operator>= (time_t other) const
					 {
						return ( tvval.tv_sec >= other );
					 }
	bool			 operator> (time_t other) const
					 {
						return ( tvval.tv_sec > other );
					 }
	
	
protected:

	struct tm		 tmval; //< Worked out calandar date and time.
	struct timeval	 tvval; //< Basic value in secs+msecs since epoch.
	int				 timezone; //< Timezone offset in seconds.
	string			 stval; //< The string representation.
	string			 stformat; //< The format for the string representation.
	
	bool			 tmset; //< If true, the tmval has been previously calculated.
	bool			 stset;	//< If true, the stval has been previously formatted.
};

extern int __system_local_timezone;
extern bool __system_timezone_set;

#endif
