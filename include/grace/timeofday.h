// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#include <grace/str.h>
#include <grace/value.h>
#include <grace/strutil.h>
#include <sys/time.h>

class timeofday
{
public:

	enum 		eMonth
				{
					emJANUARY		=	0,
					emFEBRUARY		=	1,
					emMARCH			=	2,
					emAPRIL			=	3,
					emMAY			=	4,
					emJUNE			=	5,
					emJULY			=	6,
					emAUGUST		=	7,
					emSEPTEMBER		=	8,
					emOCTOBER		=	9,
					emNOVEMBER		=	10,
					emDECEMBER		=	11
				};
				
				timeofday( void );
				timeofday( const timeofday &);
				timeofday( timeofday * );
				timeofday( timeval );
	
				~timeofday( void ){};

	
				/// Wait's until the given time has been reached
				/// Every 10s teh time will be re-calculated to 
				/// prevent differnces
				/// \param destTime Time to wait for
	void		waitUntil( const timeofday &);
	
				/// Set's the current time of a object
				/// \param stSeconds 	0-59 max = 61 for leap seconds
				/// \param stMinutes 	0-59
				/// \param stHours 	 	0-23
				/// \param stDay		1-31
				/// \param stMonth		of type eMonth: timeofday::emJANUARI
				/// \param stYear		Actual year number
	void		setTime( int 	stSeconds,
						 int 	stMinutes,
						 int 	stHours,
						 int 	stDay,
						 eMonth stMonth,
						 int 	stYear
					   );

				/// Init time value stored in this class,
				/// this is normally the time when the object 
				/// has been created but this function can be
				/// used in an existing object to set the current 
				/// time.
	void 		init( void );
	
				/// Get timeval from current stored time
	timeval		gettime( void )
				{
					return Tval;
				}
	
				/// Get the total number of u_secs
	unsigned 
	long long	getusec( void )
				{
					return ((Tval.tv_sec * 1000000LL) 
							 + Tval.tv_usec);
				}
	
				/// Copy data fom onother object into
				/// the current object
	void		copy( const timeofday &orig )
				{
					Tval.tv_sec  = orig.Tval.tv_sec;
					Tval.tv_usec = orig.Tval.tv_usec;
				}
				
	timeofday	&operator += (timeval);
	timeofday	&operator -= (timeval);

	timeval 	operator+ ( const timeofday &) const;
	timeofday 	*operator+ ( timeval ) const;
	
	timeval 	operator- ( const timeofday &) const;
	timeofday 	*operator- ( timeval ) const;
	
	timeofday	&operator= ( timeval orig )
				{
					Tval = orig;
					return *this;
				}
					
	timeofday	&operator= ( const timeofday &orig )
				{
					copy (orig);
					return *this;
				}
					
	timeofday	&operator= ( timeofday *orig )
				{
					copy( *orig );
					delete( orig );
					return *this;
				}

	bool		operator< (const timeofday &other) const
				{
					unsigned long long current;
					unsigned long long given;
											
					current = (Tval.tv_sec * 1000000LL) 
							   + Tval.tv_usec;
									 
					given   = (other.Tval.tv_sec * 1000000LL) 
							   + other.Tval.tv_usec;
											
					return ( current < given );						
				}
					
	bool		operator<= (const timeofday &other) const
				{
					unsigned long long current;
					unsigned long long given;
												
					current = (Tval.tv_sec * 1000000LL) 
								 + Tval.tv_usec;
									 
					given   = (other.Tval.tv_sec * 1000000LL) 
								 + other.Tval.tv_usec;
											
					return ( current <= given );					 						
				}
					
	bool		operator== (const timeofday &other) const
				{
					unsigned long long current;
					unsigned long long given;
											
					current = (Tval.tv_sec * 1000000LL) 
								 + Tval.tv_usec;
									 
					given   = (other.Tval.tv_sec * 1000000LL) 
								 + other.Tval.tv_usec;
											
					return ( current == given );					 	
				}
					
	bool		operator>= (const timeofday &other) const
				{
					unsigned long long current;
					unsigned long long given;
												
					current = (Tval.tv_sec * 1000000LL) 
								 + Tval.tv_usec;
									 
					given   = (other.Tval.tv_sec * 1000000LL) 
								 + other.Tval.tv_usec;
											
					return ( current >= given );					 	
				}
					
	bool		operator> (const timeofday &other) const
				{
					unsigned long long current;
					unsigned long long given;
												
					current = (Tval.tv_sec * 1000000LL) 
								 + Tval.tv_usec;
									 
					given   = (other.Tval.tv_sec * 1000000LL) 
								 + other.Tval.tv_usec;
											
					return ( current > given );					 	
				}
		
protected:

	struct timeval Tval;
	

};
