// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _SMTPD_H
#define _SMTPD_H 1

#include <grace/thread.h>
#include <grace/tcpsocket.h>
#include <grace/defaults.h>

$exception (smtpdListenPortException, "Cannot set up listenport");
$exception (smtpdPigsFlyException, "Pigs fly");

typedef int smtpeventmask;

#define SMTP_INFO 0x01
#define SMTP_ERROR 0x02
#define SMTP_DELIVERY 0x04
#define SMTP_AUTH 0x08

/// A base class for implementing an SMTP service.
class smtpd : public thread
{
friend class smtpworker;
public:
							 /// Constructor.
							 smtpd (void);
							 
							 /// Destructor.
		virtual				~smtpd (void);

							 /// Explicitly set listening tcp port.
							 /// This can be useful for grabbing a
							 /// privileged port before daemonizing
							 /// without immediately spawning the
							 /// thread.
							 /// \param port The tcp port to listen to.
		void				 listento (int port);
		
							 /// Explicitly set listening tcp port
							 /// and ip address.
							 /// \param addr The IPv4 address.
							 /// \param port The TCP port number.
		void				 listento (ipaddress addr, int port);
		
							 /// Set minimum worker thread count.
							 /// \param num The amount requested.
		void				 minthreads (int num);
		
							 /// Set maximum worker thread count.
							 /// \param num The amount requested.
		void				 maxthreads (int num);
		
							 /// Spawn the daemon thread and workers.
		void				 start (void);
		
							 /// Terminate all threads.
		void				 shutdown (void);
		
							 /// Set the name printed in the SMTP banner.
							 /// \param str The banner string.
		void				 setbannername (const string &str);

							 /// Run-method, keeps track of worker threads.
		void				 run (void);

	protected:
		threadgroup			 workers; ///< Group for worker threads.
		tcplistener			 lsock; ///< Listening socket.
		int					 listenport; ///< Requested listening port.
		int					 minthr; ///< Minimum worker thread count.
		int					 maxthr; ///< Maximum worker thread count.
		string				 banner; ///< SMTP banner string.
		
		bool				 _shutdown; ///< If true, daemon should quit.
		lock<int>			 load; ///< Lock and counter for active threads.
		lock<int>			 tcplock; ///< Lock for the listener.
		smtpeventmask		 mask; ///< Mask for worker events.

							 /// Virtual method should implement a check
							 /// against a sender/recipient pair.
							 /// \param mailfrom The origin address.
							 /// \param rcptto The recipient address.
							 /// \param env The envelope data.
		virtual bool		 checkrecipient (const string &mailfrom,
											 const string &rcptto,
											 value &env);
											 
							 /// Virtual method should implement delivery
							 /// of a message and envelope.
							 /// \param body The raw message body.
							 /// \param env The envelope data.
							 /// Inside the envelope data the following
							 /// member variables should be present:
							 /// - \b from The MAIL FROM address.
							 /// - \b rcpt A list of RCPT TO addresses.
							 /// - \b transaction-id An internal message-id.
							 /// - \b ip The remote SMTP address.
							 /// - \b helo The HELO string.
		virtual bool		 deliver (const string &body,
									  value &env);
									  
							 /// Virtual method should implement handling
							 /// of events sent by worker threads.
		virtual void		 eventhandle (const value &ev);
		
		virtual bool		 authplain (const string &user,
										const string &pass,
										value &env);
		
							 /// Internal method, generates a 
							 /// transaction-id.
		string				*maketransactionid (void);
		
							 /// Internal method, removes <> from an
							 /// email address.
		void				 normalizeaddr (string &s);
};

/// Worker thread for the smtpd class.
class smtpworker : public groupthread
{
public:
						 /// Constructor. Links back to parent and spawns.
						 smtpworker (smtpd *pop)
						 	: groupthread (pop->workers, "smtpworker")
						 {
							parent = pop;
							spawn();
						 }
						 
						 /// Destructor.
						~smtpworker (void)
						 {
						 }

						 /// Run method. Accepts an SMTP socket and does
						 /// the SMTP tango.
	virtual void		 run (void);

protected:
	smtpd				*parent; ///< Link to parent smtpd.
};

#endif
