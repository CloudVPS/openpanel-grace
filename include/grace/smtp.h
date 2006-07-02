#ifndef _SMTP_H
#define _SMTP_H 1

#include <grace/tcpsocket.h>
#include <grace/value.h>
#include <grace/str.h>
#include <grace/strutil.h>

/// SMTP client class.
/// Implements the SMTP protocol for sending email through a server.
class smtpsocket
{
public:
					 /// Constructor.
					 /// Defaults the SMTP connection to localhost
					 /// and the sender address to the current username
					 /// with the system hostname as the email domain.
					 smtpsocket (void);
					 
					 /// Destructor. Boring, since it doesn't involve
					 /// any acts of blowing things up.
					~smtpsocket (void);
	
					 /// Sets the address of the SMTP server.
					 /// \param hostname SMTP hostname.
					 /// \param port SMTP tcp port defaults to 25.
	void			 setsmtphost (const string &hostname, int port=25);
	
					 /// Sets the email sender. If no separate From header
					 /// is supplied this will also be supplied as the
					 /// From-address.
					 /// \param address The email address.
					 /// \param name The sender's name.
	void			 setsender (const string &address, const string &name);
	
					 /// Sets the email sender. Expects an email address
					 /// either in the raw form or the RFC name+email form:
					 /// - john@doe.org OR
					 /// - "John 'john-john' Doe" &lt;john@doe.org&gt;
					 ///
					 /// The first form will default to a name of
					 /// "Mail System" if the sender address is used
					 /// in the From header.
	void			 setsender (const string &address);
	
					 /// Dispatch an SMTP message.
					 /// \param rcptto Recipient(s). Can be either an
					 ///               array of one or more string
					 ///               objects containing recipient
					 ///               addresses or a string object
					 ///               containing a single email
					 ///               address.
					 /// \param subject The email subject.
					 /// \param body The email body.
					 /// \param mailfrom The sender address.
					 /// \param contenttype The body content-type.
	bool			 sendmessage (const value &rcptto,
								  const string &subject,
								  const string &body,
								  const string &mailfrom,
								  const string &contenttype);
	
					 /// Dispatch an SMTP message.
					 /// \param rcptto Recipient(s). Can be either an
					 ///               array of one or more string
					 ///               objects containing recipient
					 ///               addresses or a string object
					 ///               containing a single email
					 ///               address.
					 /// \param subject The email subject.
					 /// \param body The email body.
					 /// \param mailfrom The sender address.
	bool			 sendmessage (const value &rcptto,
								  const string &subject,
								  const string &body,
								  const string &mailfrom);

					 /// Dispatch an SMTP message.
					 /// \param rcptto Recipient(s). Can be either an
					 ///               array of one or more string
					 ///               objects containing recipient
					 ///               addresses or a string object
					 ///               containing a single email
					 ///               address.
					 /// \param subject The email subject.
					 /// \param body The email body.
	bool			 sendmessage (const value &rcptto,
								  const string &subject,
								  const string &body);

					 /// Set an outgoing mail header.
					 /// \param name The header name.
					 /// \param val The header value.
	void			 setheader (const statstring &name, const string &val);
	
					 /// Get the last reported error.
	const string	&error (void);

					 /// Act on an SMTP transaction. Assumes all
					 /// proper headers and properties have been set.
	bool			 dosmtp (const value &rcptto, const string &body);

protected:
	string			 smtphost; ///< The hostname of the remote SMTP.
	int				 smtpport; ///< The tcp port of the remote SMTP.
	string			 sender; ///< The sender address.
	string			 sendername; ///< The sender name.
	string			 hostname; ///< My hostname.
	value			 headers; ///< Output headers.
	string			 err; ///< Error data.
};

#endif
