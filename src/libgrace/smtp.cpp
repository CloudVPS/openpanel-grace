// ========================================================================
// smtp.cpp: Class for sending mail to an SMTP host.
//
// (C) Copyright 2005-2006 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam 
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
	
	setsmtphost ("localhost");
	hostname = kernel.net.hostname();
	myuid = kernel.userdb.getuid();
	pw = kernel.userdb.getpwuid (myuid);
	
	myaddress.printf ("%s@%s", pw["username"].cval(), hostname.str());
	setsender (myaddress, "Mail System");
}

// ========================================================================
// DESTRUCTOR
// ========================================================================
smtpsocket::~smtpsocket (void)
{
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
		
		delete mailpart.cutafter ('>');
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
bool smtpsocket::dosmtp (const value &rcpts, const string &body)
{
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
		sock.printf ("HELO %s\r\n", hostname.str());
		line = sock.gets();
		if (line.toint() != 250)
		{
			err = errortext::smtp::helo;
			err.strcat (line);
			sock.close();
			return false;
		}
		
		// Handle continuation lines.
		while (line[3] == '-') line = sock.gets();

		// ------------------------------------------------------------------
		// Send MAIL FROM and parse reply.
		// ------------------------------------------------------------------
		sock.printf ("MAIL FROM: <%s>\r\n", sender.str());
		line = sock.gets();
		if (line.toint() != 250)
		{
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
			sock.printf ("RCPT TO: <%s>\r\n", rcp.cval());
			line = sock.gets();
			if (line.toint() != 250)
			{
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
		sock.printf ("DATA\r\n");
		line = sock.gets();
		if (line.toint() != 354)
		{
			err = errortext::smtp::data;
			err.strcat (line);
			sock.close();
			return false;
		}
		
		// ------------------------------------------------------------------
		// Send headers.
		// ------------------------------------------------------------------
		if (! headers.exists ("From"))
		{
			sock.printf ("From: \"%S\" <%s>\r\n", sendername.str(),
						 sender.str());
		}
		if (! headers.exists ("To"))
		{
			if (rcptto.count() == 1)
			{
				sock.printf ("To: %s\r\n", rcptto[0].cval());
			}
			else
			{
				sock.printf ("To: Undisclosed Recipients\r\n");
			}
		}
		foreach (hdr, headers)
		{
			sock.printf ("%s: %s\r\n", hdr.name(), hdr.cval());
		}
		sock.printf ("\r\n");
		
		// ------------------------------------------------------------------
		// Send body followed by dot-on-a-single-line. Parse reply.
		// ------------------------------------------------------------------
		nbody = strutil::regexp (body, "s@\r\n\\.\r\n@\r\n..\r\n@g");
		sock.puts (nbody);
		sock.printf ("\r\n.\r\n");
		
		line = sock.gets();
		if (line.toint() != 250)
		{
			err = errortext::smtp::deliver;
			err.strcat (line);
			sock.close();
			return false;
		}

		// ------------------------------------------------------------------
		// End transaction.
		// ------------------------------------------------------------------
		transactionComplete = true;
		sock.printf ("QUIT\r\n");
		line = sock.gets();
		sock.close();
	}
	catch (...)
	{
		sock.close();
		if (! transactionComplete)
		{
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
