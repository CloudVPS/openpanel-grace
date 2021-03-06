// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

// ========================================================================
// smtp.cpp: Class for sending mail to an SMTP host.
// ========================================================================
#include <grace/smtp.h>
#include <grace/system.h>
#include <grace/defaults.h>

// ========================================================================
// CONSTRUCTOR
// -----------
// Sets up default values for the smtp host and sender address.
// ========================================================================
smtpsocket::smtpsocket (void)
{
	value pw;
	string myaddress;
	uid_t myuid;
	
	erno = 0;
	
	setsmtphost ("localhost");
	hostname = core.net.hostname();
	myuid = core.userdb.getuid();
	pw = core.userdb.getpwuid (myuid);
	
	myaddress = "%s@%s" %format (pw["username"], hostname);
	setsender (myaddress, "Mail System");
}

// ========================================================================
// DESTRUCTOR
// ========================================================================
smtpsocket::~smtpsocket (void)
{
}

// ==========================================================================
// METHOD smtpsocket::authenticate
// ==========================================================================
void smtpsocket::authenticate (const string &u, const string &p)
{
	username = u;
	password = p;
}

// ========================================================================
// METHOD ::setsmtphost
// ========================================================================
void smtpsocket::setsmtphost (const string &hostname, int port)
{
	smtphost = hostname;
	smtpport = port;
}

// ========================================================================
// METHOD ::setsender
// ========================================================================
void smtpsocket::setsender (const string &address, const string &name)
{
	sender = address;
	sendername = name;
}

// ========================================================================
// METHOD ::setsender
// ------------------
// Does lame-ass RFC-822 parsing. Don't expect miracles. TODO: Make this
// less naive.
// ========================================================================
void smtpsocket::setsender (const string &address)
{
	if (address.strchr ('<') >= 0)
	{
		value tval;
		string mailpart;
		string namepart;
		
		tval = strutil::splitquoted (address, '<');
		namepart = tval[0];
		mailpart = tval[1];
		
		namepart.chomp();
		mailpart.cropat ('>');
		sender = mailpart;
		sendername = namepart;
		return;
	}
	
	sender = address;
	sendername = "Mail System";
	
}

// ========================================================================
// METHOD ::sendmessage
// ========================================================================
bool smtpsocket::sendmessage (const value &rcptto, const string &subject,
							  const string &body, const string &mailfrom,
							  const string &contenttype)
{
	setsender (mailfrom);
	setheader ("Subject", subject);
	setheader ("Content-type", contenttype);
	return dosmtp (rcptto, body);
}

bool smtpsocket::sendmessage (const value &rcptto, const string &subject,
							  const string &body, const string &mailfrom)
{
	
	setsender (mailfrom);
	setheader ("Subject", subject);
	return dosmtp (rcptto, body);
}

bool smtpsocket::sendmessage (const value &rcptto, const string &subject,
							  const string &body)
{
	setheader ("Subject", subject);
	return dosmtp (rcptto, body);
}

// ========================================================================
// METHOD ::dosmtp
// ---------------
// Perform the actual SMTP ritual towards any number of senders.
// ========================================================================
bool smtpsocket::dosmtp (const value &rcpts, const string &body,
						 bool genheaders)
{
	bool supportsauth = false;
	tcpsocket sock;
	string line;
	string nbody;
	bool transactionComplete = false;
	value rcptto;
	rcptto = rcpts;
	
	// Turn rcptto into array if it isn't one.
	if (! rcptto.count())
	{
		string rcpt = rcptto.sval();
		rcptto = 0;
		rcptto.newval() = rcpt;
	}
	
	// ----------------------------------------------------------------------
	// Open the connection.
	// ----------------------------------------------------------------------
	if (! sock.connect (smtphost, smtpport))
	{
		erno = SMTPERR_CONNFAIL;
		err = errortext::smtp::connfail;
		return false;
	}
	
	try
	{
		// ------------------------------------------------------------------
		// Parse the SMTP banner.
		// ------------------------------------------------------------------
		line = sock.gets();
		if (line.toint() != 220)
		{
			erno = SMTPERR_SERVERR;
			err = errortext::smtp::start;
			err.strcat (line);
			sock.close();
			return false;
		}
		
		// Handle continuation lines.
		while (line[3] == '-') line = sock.gets();
		
		// ------------------------------------------------------------------
		// Send HELO and parse reply.
		// ------------------------------------------------------------------
		sock.puts ("EHLO %s\r\n" %format (hostname));
		line = sock.gets();
		if (line.toint() != 250)
		{
			erno = SMTPERR_SERVERR;
			err = errortext::smtp::helo;
			err.strcat (line);
			sock.close();
			return false;
		}
		
		// Handle continuation lines.
		while (line[3] == '-')
		{
			line = sock.gets();
			string replystr = line.mid (4);
			if (replystr.strncasecmp ("auth ",5) == 0)
			{
				value splt = strutil::splitspace (replystr);
				foreach (w, splt)
				{
					if (w.strcasecmp ("plain") == 0)
						supportsauth = true;
				}
			}
		}

		if (username && supportsauth)
		{
			string authstr;
			authstr.strcat ('\0');
			authstr.strcat (username);
			authstr.strcat ('\0');
			authstr.strcat (password);
			authstr = authstr.encode64();
			sock.puts ("AUTH PLAIN %s\r\n" %format (authstr));
			line = sock.gets();
			if (line.toint() != 235)
			{
				erno = SMTPERR_SERVERR;
				err = "%s%S" %format (errortext::smtp::authplain, line.mid(4));
				sock.puts ("QUIT\r\n");
				sock.close();
				return false;
			}
		}

		// ------------------------------------------------------------------
		// Send MAIL FROM and parse reply.
		// ------------------------------------------------------------------
		sock.puts ("MAIL FROM: <%s>\r\n" %format (sender));
		line = sock.gets();
		if (line.toint() != 250)
		{
			erno = SMTPERR_SERVERR;
			err = errortext::smtp::mailfrom;
			err.strcat (line);
			sock.close();
			return false;
		}
		
		// Handle continuation lines.
		while (line[3] == '-') line = sock.gets();
		
		// ------------------------------------------------------------------
		// Send RCPT TO for each recipient and parse repl(y|ies).
		// ------------------------------------------------------------------
		foreach (rcp, rcptto)
		{
			sock.puts ("RCPT TO: <%s>\r\n" %format (rcp));
			line = sock.gets();
			if (line.toint() != 250)
			{
				erno = SMTPERR_SERVERR;
				err = errortext::smtp::rcptto;
				err.strcat (line);
				sock.close();
				return false;
			}
			
			while (line[3] == '-') line = sock.gets();
		}
		
		// ------------------------------------------------------------------
		// Indicate start of DATA block, parse reply.
		// ------------------------------------------------------------------
		sock.puts ("DATA\r\n");
		line = sock.gets();
		if (line.toint() != 354)
		{
			erno = SMTPERR_SERVERR;
			err = errortext::smtp::data;
			err.strcat (line);
			sock.close();
			return false;
		}
		
		if (genheaders)
		{
			// --------------------------------------------------------------
			// Send headers.
			// --------------------------------------------------------------
			if (! headers.exists ("From"))
			{
				sock.puts ("From: \"%S\" <%s>\r\n" %format (sendername,sender));
			}
			if (! headers.exists ("To"))
			{
				if (rcptto.count() == 1)
				{
					sock.puts ("To: %s\r\n" %format (rcptto[0]));
				}
				else
				{
					sock.puts ("To: Undisclosed Recipients\r\n");
				}
			}
			foreach (hdr, headers)
			{
				sock.puts ("%s: %s\r\n" %format (hdr.id(), hdr));
			}
			sock.puts ("\r\n");
		}
		
		// ------------------------------------------------------------------
		// Send body followed by dot-on-a-single-line. Parse reply.
		// ------------------------------------------------------------------
		nbody = strutil::regexp (body, "s@\r\n\\.\r\n@\r\n..\r\n@g");
		sock.puts (nbody);
		sock.puts ("\r\n.\r\n");
		
		line = sock.gets();
		if (line.toint() != 250)
		{
			erno = SMTPERR_SERVERR;
			err = errortext::smtp::deliver;
			err.strcat (line);
			sock.close();
			return false;
		}

		// ------------------------------------------------------------------
		// End transaction.
		// ------------------------------------------------------------------
		transactionComplete = true;
		sock.puts ("QUIT\r\n");
		line = sock.gets();
		sock.close();
	}
	catch (...)
	{
		sock.close();
		if (! transactionComplete)
		{
			erno = SMTPERR_BROKENPIPE;
			err = errortext::smtp::connclose;
			return false;
		}
	}
	
	return true;
}

// ========================================================================
// METHOD ::error
// ========================================================================
const string &smtpsocket::error (void)
{
	return err;
}

// ========================================================================
// METHOD ::setheader
// ========================================================================
void smtpsocket::setheader (const statstring &name, const string &value)
{
	headers[name] = value;
}
